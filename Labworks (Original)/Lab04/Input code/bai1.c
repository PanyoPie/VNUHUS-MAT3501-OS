int turn = 0;

int write_data(char* buf, size_t len, int process_id) {
    int fileExist, fd;
    char* file = "output.dat";

    while (turn != process_id);

    fileExist = check_file_existence(file);
    if (fileExist == FALSE) {
        fd = open(file, O_CREAT);
        write(fd, buf, len);
    }

    turn = 1 - process_id;
}