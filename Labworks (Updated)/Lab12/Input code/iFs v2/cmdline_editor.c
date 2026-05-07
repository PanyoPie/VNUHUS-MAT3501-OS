#include <termios.h>
#include <unistd.h>

#define HISTORY_SIZE 50
#define DEFAULT_bufLen 1024

/* ── Terminal raw mode ─────────────────────────────────────────────────── */

static struct termios orig_termios;

static void disableRawMode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static void enableRawMode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);          // restore automatically on exit

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON); // no echo, read byte-by-byte
    raw.c_cc[VMIN]  = 1;             // block until 1 byte available
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

/* ── History ───────────────────────────────────────────────────────────── */

static char history[HISTORY_SIZE][DEFAULT_bufLen];
static int  history_count = 0;

static void history_add(const char *line) {
    if (line[0] == '\0') return;
    if (history_count > 0 &&
        strcmp(history[history_count - 1], line) == 0) return;

    if (history_count < HISTORY_SIZE) {
        strncpy(history[history_count++], line, DEFAULT_bufLen - 1);
    } else {
        memmove(history[0], history[1],
                (HISTORY_SIZE - 1) * sizeof(history[0]));
        strncpy(history[HISTORY_SIZE - 1], line, DEFAULT_bufLen - 1);
    }
}

/* ── Editor ────────────────────────────────────────────────────────────── */

/* Redraws the line, positions cursor at new_pos. */
static void redraw(char *cmdLine, int old_len, int pos,
                   const char *newbuf, int new_len, int new_pos) {
    /* Go to start of current content */
    for (int i = 0; i < pos; i++) write(STDOUT_FILENO, "\033[1D", 4);

    /* Print new content */
    write(STDOUT_FILENO, newbuf, new_len);

    /* Erase leftover chars if new line is shorter */
    for (int i = new_len; i < old_len; i++) write(STDOUT_FILENO, " ", 1);
    for (int i = new_len; i < old_len; i++) write(STDOUT_FILENO, "\033[1D", 4);

    /* Move cursor to new_pos */
    for (int i = new_pos; i < new_len; i++) write(STDOUT_FILENO, "\033[1D", 4);

    memcpy(cmdLine, newbuf, new_len);
    cmdLine[new_len] = '\0';
}

int readLine(char *cmdLine, int bufLen) {
    if (!cmdLine || bufLen <= 0) return -1;

    enableRawMode();

    int  pos      = 0;
    int  len      = 0;
    int  hist_idx = history_count;
    char saved[bufLen];

    memset(cmdLine, 0, bufLen);
    memset(saved,   0, bufLen);

    while (1) {
        char ch;
        if (read(STDIN_FILENO, &ch, 1) != 1) continue;

        /* Enter */
        if (ch == '\r' || ch == '\n') {
            write(STDOUT_FILENO, "\r\n", 2);
            cmdLine[len] = '\0';
            history_add(cmdLine);
            disableRawMode();
            return len;
        }

        /* Ctrl-C */
        if (ch == 3) {
            write(STDOUT_FILENO, "\r\n", 2);
            cmdLine[0] = '\0';
            disableRawMode();
            return -1;
        }

        /* Ctrl-U: clear line */
        if (ch == 21) {
            redraw(cmdLine, len, pos, "", 0, 0);
            pos = len = 0;
            continue;
        }

        /* Backspace */
        if (ch == 127 || ch == '\b') {
            if (pos > 0) {
                char tmp[bufLen];
                memcpy(tmp, cmdLine, len);
                memmove(&tmp[pos-1], &tmp[pos], len - pos);
                redraw(cmdLine, len, pos, tmp, len - 1, pos - 1);
                pos--; len--;
            }
            continue;
        }

        /* Escape sequence */
        if (ch == '\033') {
            char s1, s2;
            if (read(STDIN_FILENO, &s1, 1) != 1) continue;
            if (read(STDIN_FILENO, &s2, 1) != 1) continue;
            if (s1 != '[') continue;

            /* 4-byte sequences: ESC [ <digit> ~ */
            if (s2 >= '0' && s2 <= '9') {
                char tilde;
                if (read(STDIN_FILENO, &tilde, 1) != 1) continue;
                if (s2 == '3' && tilde == '~') {          /* Delete */
                    if (pos < len) {
                        char tmp[bufLen];
                        memcpy(tmp, cmdLine, len);
                        memmove(&tmp[pos], &tmp[pos+1], len - pos - 1);
                        redraw(cmdLine, len, pos, tmp, len - 1, pos);
                        len--;
                    }
                }
                /* other 4-byte sequences (Page Up/Down etc.) are consumed and ignored */
                continue;
            }

            if (s2 == 'A') {                              /* Up */
                if (hist_idx > 0) {
                    if (hist_idx == history_count)
                        strncpy(saved, cmdLine, bufLen);
                    hist_idx--;
                    int nlen = strlen(history[hist_idx]);
                    redraw(cmdLine, len, pos,
                           history[hist_idx], nlen, nlen);
                    pos = len = nlen;
                }
            } else if (s2 == 'B') {                       /* Down */
                if (hist_idx < history_count) {
                    hist_idx++;
                    const char *src = (hist_idx == history_count)
                                      ? saved
                                      : history[hist_idx];
                    int nlen = strlen(src);
                    redraw(cmdLine, len, pos, src, nlen, nlen);
                    pos = len = nlen;
                }
            } else if (s2 == 'C' && pos < len) {          /* Right */
                write(STDOUT_FILENO, "\033[1C", 4);
                pos++;
            } else if (s2 == 'D' && pos > 0) {            /* Left */
                write(STDOUT_FILENO, "\033[1D", 4);
                pos--;
            } else if (s2 == 'H') {                       /* Home */
                for (int i = 0; i < pos; i++)
                    write(STDOUT_FILENO, "\033[1D", 4);
                pos = 0;
            } else if (s2 == 'F') {                       /* End */
                for (int i = pos; i < len; i++)
                    write(STDOUT_FILENO, "\033[1C", 4);
                pos = len;
            }
            continue;
        }

        /* Printable characters */
        if (ch >= 32 && ch < 127 && len < bufLen - 1) {
            hist_idx = history_count;        // snap back to live input
            char tmp[bufLen];
            memcpy(tmp, cmdLine, len);
            memmove(&tmp[pos+1], &tmp[pos], len - pos);
            tmp[pos] = ch;
            redraw(cmdLine, len, pos, tmp, len + 1, pos + 1);
            pos++; len++;
        }
    }
}

/* ── Demo main ─────────────────────────────────────────────────────────── 

#define bufLen 256

int main(void) {
    char cmdLine[bufLen];

    while (1) {
        printf("$ ");
        fflush(stdout);

        int n = readLine(cmdLine, bufLen);
        if (n < 0) { printf("Cancelled.\n"); continue; }
        if (strcmp(cmdLine, "exit") == 0) break;
        printf("→ \"%s\"\n", cmdLine);
    }
    return 0;
}
*/