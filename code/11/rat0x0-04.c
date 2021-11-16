#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char *argv[]) {
  pid_t pid;
  printf ("This is RAT0X0 version 0.1\n");

  strcpy (argv[0],"[Jbd2/sda0-8]");
  if ((pid = fork()) != 0) return 0;
  setsid(); // Remove TTY
  if ((pid = fork()) != 0) return 0;

  while (1) usleep (1000);
}
