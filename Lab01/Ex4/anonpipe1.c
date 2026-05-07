// Anonymous pipes between parent and child
// Parent sends a string to child using pipe a
// Child converts the string to uppercase and sends back to parent, using pipe b
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int main() {

  char *msg = "Hello child\n", buf[255];

  int fd[2], pid;

  if (pipe(fd) < 0) exit(-1);

  if ((pid = fork()) > 0) { // parent
    close(fd[0]);
    write(fd[1], msg, strlen(msg));
    wait(NULL); // wait for the child to exit
  } else { //child
    close(fd[1]); memset(buf, 0, 255);
    read(fd[0], buf, 254);
    printf("%s\n", buf);
  }

  return 0;
}