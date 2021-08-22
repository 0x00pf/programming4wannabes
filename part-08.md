# Programming for Wanabes VIII. File Details
We have learnt how to scan directories and list their content, now we need to figure out how to get the details of the directory contents so we can chose the files we are interested on. This is actually very simple and requires one single system call.

# The `stat` system call

The `stat` system call, allow us to get all the details of a specific fie. The prototype of this syscall is :

```C
int stat(const char *pathname, struct stat *statbuf);
```

And the `struct stat` (now we know what a struct is don't we?), contains the following information:

```C
struct stat {
       dev_t     st_dev;         /* ID of device containing file */
       ino_t     st_ino;         /* Inode number */
       mode_t    st_mode;        /* File type and mode */
       nlink_t   st_nlink;       /* Number of hard links */
       uid_t     st_uid;         /* User ID of owner */
       gid_t     st_gid;         /* Group ID of owner */
       dev_t     st_rdev;        /* Device ID (if special file) */
       off_t     st_size;        /* Total size, in bytes */
       blksize_t st_blksize;     /* Block size for filesystem I/O */
       blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */

       /* Since Linux 2.6, the kernel supports nanosecond
          precision for the following timestamp fields.
          For the details before Linux 2.6, see NOTES. */

        struct timespec st_atim;  /* Time of last access */
        struct timespec st_mtim;  /* Time of last modification */
        struct timespec st_ctim;  /* Time of last status change */
};
```

The most interesting field for us is `st_mode`, but there is a lot of other useful information that we will be using in the future. The structure is describe in the `stat` man page for your future references.

# Understanding the `st_mode` field

The `st_mode` field encodes the type of file and also the permissions. The man page includes some sample code showing us how to access that information. So, the way to access the type of file is using the bit mask `S_IFMT`:

```C
struct st sb;
stat (a_file_name, &sb);
int type = sb.st_mode & S_IFMT
```

The `&` operator is a bitwise AND.... It basically matches two binary numbers and only the bits that are the same remains. Let's see what is in `S_IFMT`. I can tell you directly, but I believe it is going to be good for you to learn how to get this information by yourself, so you can find whatever you want  in the future.

We will start looking in the header files indicated by the man page. At the top of the man page you will see the `includes` you need to add to your program to use the system call. 


    NAME
           stat, fstat, lstat, fstatat - get file status
    
    SYNOPSIS
           #include <sys/types.h>
           #include <sys/stat.h>
           #include <unistd.h>


The `sys/types.h` sounds pretty generic so I will skip it (you can actually look into it, but you won't find anything). So let's look into `sys/stat.h` that sounds more like the specifics for `stat`.

    $ grep "IFMT" /usr/include/sys/stat.h
    # define S_IFMT         __S_IFMT
    #define __S_ISTYPE(mode, mask)  (((mode) & __S_IFMT) == (mask))

Well, looks like the actual value is defined somewhere else, but we can see also a macro to quickly check against the different types. We can use it like:

```C
__S_ISTYPE(sb.st_mode,S_IFREG)
// is equivalent to
(((sb.st_mode) & __S_IFMT) == (S_IFREG)
```

That second define is called a macro. They work the same than the normal defines (they are just substituted by its value in the code before compiling) but we can use parameters to write more complex expressions. When we use parameters, the `define` is said to define a macro instead of a constant.

So, in order to find out the actual value of `S_IFMT`, we need to look into the includes, included by the include :). 

    $ grep "#include" /usr/include/sys/stat.h
    #include <features.h>
    #include <bits/types.h>         /* For __mode_t and __dev_t.  */
    #include <bits/stat.h>

_NOTE:I'm using grep to show this information. It is, in general, very handy to do it this way, but I would recommend, at the beginning, to find these information manually, that is, opening the file in an editor and browse through it. The reason is that you will see how this system include files look like and get familiar with them. You will also find curious thing that will spark your curiosity._

Again, we can go through all of them systematically, but `bits/stat.h` looks like the best candidate.

    $ grep "IFMT" /usr/include/bits/stat.h
    #define __S_IFMT        0170000 /* These bits determine file type.  */

We found it!. Actually if we open the file, we will also find, all the other relevant constants. These are the ones:

```C
#define __S_IFMT        0170000 /* These bits determine file type.  */

/* File types.  */
#define __S_IFDIR       0040000 /* Directory.  */
#define __S_IFCHR       0020000 /* Character device.  */
#define __S_IFBLK       0060000 /* Block device.  */
#define __S_IFREG       0100000 /* Regular file.  */
#define __S_IFIFO       0010000 /* FIFO.  */
#define __S_IFLNK       0120000 /* Symbolic link.  */
#define __S_IFSOCK      0140000 /* Socket.  */

/* Protection bits.  */

#define __S_ISUID       04000   /* Set user ID on execution.  */
#define __S_ISGID       02000   /* Set group ID on execution.  */
#define __S_ISVTX       01000   /* Save swapped text after use (sticky).  */
#define __S_IREAD       0400    /* Read by owner.  */
#define __S_IWRITE      0200    /* Write by owner.  */
#define __S_IEXEC       0100    /* Execute by owner.  */
```

But. Wait a minute!. Those numbers look a bit weird isn't it?

# Base 8, octal numbers
So far we have been using decimal numbers (using base 10) and also hexadecimal numbers (using base 16). OK, true, and binaries (using base 2). However there is another base that is useful when working with computers. This is base 8 and the numbers represented in this base are said to be in octal format.

In C, you can write octal numbers just adding a `0` at the beginning of the number, the same way that we add a `0x` to represent an hexadecimal value. Octal representation is useful when we need to manipulate blocks of 3 bits (0 to 7.... that's eight values, hence octal). So, let's try to understand the values of the constants used by `stat`.

To understand how this matches to the hexadecimal representation, let's just count using both bases:

    HEX   OCT  BIN
    0     0    0000 0000
    1     1    0000 0001  <--
    2     2    0000 0010  <--
    3     3    0000 0011 
    4     4    0000 0100  <--
    5     5    0000 0101
    6     6    0000 0110
    7     7    0000 0111
    8    10    0000 1000
    9    11    0000 1001  <--
    A    12    0000 1010  <--
    B    13    0000 1011
    C    14    0000 1100 <--
    D    15    0000 1101
    F    16    0000 1110
    10   17    0000 1111
    11   21    0001 0000

As you can see, when using the octal representation, the first digit of our number is actually the value of the lower 3 bits of the number. Each position in the number, represents the next 3 bytes, so the octal representation is very useful when we need to work with blocks of 3 bits, instead of 4 (we use hexadecimal in those cases).

For instance, check this out. Hopefully it will look familiar to you:

    chmod 777 afile
          000 111 111 111 -> 0777
          0001 1111 1111  -> 0x1ff

Both number `0x1ff` and `0777` are the same number (511 in decimal), but the octal representation allows us to write the digits as groups of three bits. In this case, each bit represent the execution, read and write permissions for the file. Imagine to use `chmod` with the decimal or hexadecimal numbers... It would be very tricky to change permissions of a file like that.

Anyhow and summing up, octal representation is used here and there whenever it is convenient to access the bits in a number in groups of three and not four. And one of these cases is the file permissions.

# Back to the `st_mode` constants
Now, we can look again to the `st_mode` constants:

```C
#define __S_IFMT        0170000 /* These bits determine file type.  */

/* File types.  */
#define __S_IFDIR       0040000 /* Directory.  */
#define __S_IFCHR       0020000 /* Character device.  */
#define __S_IFBLK       0060000 /* Block device.  */
#define __S_IFREG       0100000 /* Regular file.  */
#define __S_IFIFO       0010000 /* FIFO.  */
#define __S_IFLNK       0120000 /* Symbolic link.  */
#define __S_IFSOCK      0140000 /* Socket.  */

/* Protection bits.  */

#define __S_ISUID       04000   /* Set user ID on execution.  */
#define __S_ISGID       02000   /* Set group ID on execution.  */
#define __S_ISVTX       01000   /* Save swapped text after use (sticky).  */
#define __S_IREAD       0400    /* Read by owner.  */
#define __S_IWRITE      0200    /* Write by owner.  */
#define __S_IEXEC       0100    /* Execute by owner.  */
```

Let's first figure out the structure of this field. Representing the different octal values as bit masks. You can check the table in the previous section to verify the values, but we just use blocks of 3 bits....

    001 111 000 000 000 000  -> __S_IFMT   (0170000)
	000 100 000 000 000 000  -> __S_IFDIR  (0040000)
	000 010 000 000 000 000  -> __S_IFCHR  (0020000)
	000 110 000 000 000 000  -> __S_IFBLK  (0060000)
	001 000 000 000 000 000  -> __S_IFREG  (0100000)
	000 001 000 000 000 000  -> __S_IFIFO  (0010000)
	001 010 000 000 000 000  -> __S_IFLNK  (0120000)
	001 100 000 000 000 000  -> __S_IFSOCK (0140000)
      ^ ^^^	
    000 000 100 000 000 000  -> __S_ISUID  (0004000)
	000 000 010 000 000 000  -> __S_ISGID  (0002000)
	000 000 001 000 000 000  -> __S_ISVTX  (0001000)
            ^^^	
	000 000 000 100 000 000  -> __S_IREAD  (0000400)
	000 000 000 010 000 000  -> __S_IWRITE (0000200)
	000 000 000 001 000 000  -> __S_IEXEC  (0000100)
	            ^^^

As we can see the `__S_IFMT` is a mask to extract the high bits from the field that identify the type of file. Also note how the constant for the types of files have been defined as high numbers so we can compare directly just after ANDing the mask.

After the type of file, we find the special file attributes that indicates if the file is _SetUID_ or _SetGUID_ and also if the sticky bit is activate. And after that follows the file permissions for the owner, the group and the rest of users.

Yes, you are right, `bits/stat.h` only defines the mask for the owner. Actually, the constant defined above shouldn't be used by normal programs, we should use the ones redefined in `sys/stat.h`. I will include them here for you to check them out:

```C
/* Protection bits.  */

#define S_ISUID __S_ISUID       /* Set user ID on execution.  */
#define S_ISGID __S_ISGID       /* Set group ID on execution.  */

#define S_IRUSR __S_IREAD       /* Read by owner.  */
#define S_IWUSR __S_IWRITE      /* Write by owner.  */
#define S_IXUSR __S_IEXEC       /* Execute by owner.  */
/* Read, write, and execute by owner.  */
#define S_IRWXU (__S_IREAD|__S_IWRITE|__S_IEXEC)

#define S_IRGRP (S_IRUSR >> 3)  /* Read by group.  */
#define S_IWGRP (S_IWUSR >> 3)  /* Write by group.  */
#define S_IXGRP (S_IXUSR >> 3)  /* Execute by group.  */
/* Read, write, and execute by group.  */
#define S_IRWXG (S_IRWXU >> 3)

#define S_IROTH (S_IRGRP >> 3)  /* Read by others.  */
#define S_IWOTH (S_IWGRP >> 3)  /* Write by others.  */
#define S_IXOTH (S_IXGRP >> 3)  /* Execute by others.  */
/* Read, write, and execute by others.  */
#define S_IRWXO (S_IRWXG >> 3)
```

I had removed a couple of lines to make easier reading the file. Here you can see how all constants are redefined, and the group and other permissions are just redefined as shifted versions of the original user masks we have just seen.

> NOTE: The `>>` operator shifts all the bits of the left hand operand to the right as many positions as the right hand operand indicates. `S_IRUSR >> 3` will shift `S_IRUSR` value 3 positions to the right. In this case: `S_IRUSR = __S_IREAD = 0000400` shifting this three positions to the right will produce `040` (remember octal digits works on groups of 3 bits).

Well, this has been a kindof a digression, but this concepts are usually confusing for the beginners and I though it would be great to add some explanation in the course,

# Back to our `select_target`

So, know we can modify our `select_target` to find the kind of files we are interested on. This is how the new function will look like:

```C
int select_target (PAYLOAD_FUNC pf) {
  char                buf[BUF_SIZE];
  struct linux_dirent *de;
  struct stat         st;
  int                 fd, n, i;
  
  if ((fd = open (folder, O_RDONLY | O_DIRECTORY)) < 0) MFW_EXIT("open:");

  while (1) {
    n = getdents (fd, buf, BUF_SIZE);
    if (n < 0) MFW_EXIT ("getdents:");
    if (n == 0) break;

    for (i = 0; i < n;) {
      de = (struct linux_dirent *)(buf + i);
	  
      if ((fstatat (fs, de->d_name, &st)) < 0) {
		  perror ("stat:");
		  continue; // Just ignore the error
      }
      if (((st.st_mode & S_IFMT) == S_IFREG)
		  && (st.st_mode & 00111))
		  pf (target);
      
      i += de->d_reclen;

    }
  }
 done:
  close (fd);
  return 0;
}
```

Two comments on this code:

1. We have used `fstatat` instead of `fstat` or `stat`, so we do not have to build the full path to the file before calling `stat`. This syscall uses the directory file descriptor as base and tried to look for the file **AT** the directory that we pass as first parameter. In this case it is very convenient and we avoid allocating memory for strings and concatenating them.
2. This is the `select_target` for a virus. We are checking that the directory entry is a regular file (`S_IFREG`) and then we check that it is executable. In this case we are just checking for all possible executable permissions but that may be different in a real case.

The permission checking could also be written like:

```C
st.st_mode & 00111; // Is the same than
st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)
```

Second one is better as you can easily see what we are comparing to... and the generated code would be the same... But first one is shorter and I chose that.

Now, you can try to change the program to look for other kind of files as it my happen in the case of ransomware or spyware. But you need a last piece of knowledge in order to be able to complete the implementation of `select_target`.

# Recursive functions
The problem with our current `select_target` is that, it can only scan a single directory. In general, we should be able to scan the whole disk, that means that, we need to modify the function so, each time we find a directory, we also scan it. Or in other words, each time we find a directory we need to call ourselves again with the new directory name to scan.

A function that call itself is known as a recursive function. Recursive functions are very powerful and usually allows us to write very small and elegant code to deal with complex problems. A classical example is traversing a tree. It is way easier to do it with a recursive function that with normal iterative code.

In general, recursive function trades code complexity with memory usage. That is normal, we always trade either speed, memory or complexity. That's life. A recursive function will make extensive use of the stack creating stack frames again and again each time it calls itself. But other than that they are neat solutions to many problems, and usually requires way less code that an iterative solution.

_NOTE:The old BASIC programming language in the first microcomputer in the 80 didn't have a stack. They have the concept of subroutine but lacking a stack, you couldn't call a function recursively. That together with the design of the language was the cause of a lot of [spaguetti code](https://en.wikipedia.org/wiki/Spaghetti_code) in the programs at that time._

# A recursive `select_target`
So, it's time to modify our program to be able to scan the whole disk. For that we will need to modify the function signature, so we get the current folder being scanned in the stack frame of our function and we can continue our work at the right place after processing every subfolder.

We will also add some messages and some indention to the function, for easily check that your function is working fine:


```C

int level = 0;
char tabs[1024];

int payload (char *target) {
  printf ("%s Doing malware things to %s\n", tabs, target);
}

int select_target (int old_fd, char *folder, PAYLOAD_FUNC pf) {
  char                buf[BUF_SIZE];
  int                 flag = 1;
  struct linux_dirent *de;
  struct stat         st;
  int                 fd, n, i;
  
  // Scan directory

  // Open directory using open
  printf ("%s Processing : %s\n", tabs, folder); 
  if ((fd = openat (old_fd, folder, O_RDONLY | O_DIRECTORY)) < 0) MFW_EXIT("open:");
  // Update indentation string
  tabs[level] = ' ';
  level ++;
  
  while (flag) {
    n = getdents (fd, buf, BUF_SIZE);
    if (n < 0) MFW_EXIT ("getdents:");
    if (n == 0) break;

    // Build file name
    for (i = 0; i < n;) {
      de = (struct linux_dirent *)(buf + i);
      if ((fstatat (fd, de->d_name, &st, 0)) < 0) {
	    perror ("stat:");
	    continue; // Just ignore the error<- This is a bug can you fix it?
      }
      if (((st.st_mode & S_IFMT) == S_IFREG)
	        && (st.st_mode & 00111))
	           pf (de->d_name);
      else if (((st.st_mode & S_IFMT) == S_IFDIR)
	             && !(de->d_name[0] == '.'
		              && (de->d_name[1] == 0
		                  || (de->d_name[1] == '.' && de->d_name[2]==0))))
	                          select_target (fd, de->d_name, pf);
	 
      i += de->d_reclen;
    }
  }
  // Remove indentation
  tabs[level] = 0;
  level--;
  close (fd);
  return 0;
}
```

Despite the indentation thingy (we just add a space to a string every time we enter the function and remove it every time we left), there are two main changes:

```C
  if ((fd = openat (old_fd, folder, O_RDONLY | O_DIRECTORY)) < 0) MFW_EXIT("open:");
```

We have changed `open` for `openat`. This works the same than `statat`, we just pass as first parameter a file descriptor and, if the second parameter (the `pathname`) is relative it will open the file from the indicated directory, otherwise, if the path is absolute, will behave like a normal open.

This is convenient so we do not need to build the full file name ourselves. That is not a big deal (`strcpy`+ `strcat`), but this way we do not have to.

The second change is the recursive call. Basically, we just need to check if the directory entry is a directory. If that is the case we call ourselves again with the sub-directory name. However, remember the `.` and `..` entries we mentioned in last instalment?.... Sure, you do... well, we need to skip those, otherwise we get into an infinite loop.... This is the rest of the check.

```C
if (((st.st_mode & S_IFMT) == S_IFDIR)
	             && !(de->d_name[0] == '.'
		              && (de->d_name[1] == 0
		                  || (de->d_name[1] == '.' && de->d_name[2]==0))))
	                          select_target (fd, de->d_name, pf);
```

So, our `select_target` for malwares that need to look for files is ready. 

# Removing libC

So, we have been learning a lot about C programming, and we haven't talked much about asm. We will be looking to assembler in the coming instalments, but before starting with that we are going to remove the libc dependencies from our current test program, so we can have full control on the assembler version we are going to generate.

So far, we are using the following system calls:

    exit
    write
    openat
    close
    getdents
    fstatat

So, our first task will be to generate a mini libc version for our program. This is easier than expected:

```asm
	.global mfw_exit
	.global mfw_write
	.global mfw_close
	.global mfw_openat
	.global mfw_newfstatat
	.global mfw_getdents
	
mfw_write:
	mov $0x01, %eax
	syscall
	ret
	
mfw_openat:
	mov $0x101, %eax
	syscall
	ret
	
mfw_close:
	mov $0x03, %eax
	syscall
	ret

mfw_exit:
	mov $0x3c, %eax
	syscall
	ret

mfw_newfstatat:
	mov %rcx, %r10
	mov %0x106, %eax
	syscall
	ret

mfw_getdents:
	mov $78, %eax
	syscall
	ret

mfw_open:
	mov $0x02, %eax
	syscall
	ret

mfw_lstat:
	mov $0x06, %eax
	syscall
	ret

```

Have you notices something strange?. The implementation of all syscalls is pretty straightforward, except for the `fstatat`. This syscall has a peculiarity. The C ABI and the kernel ABI are different for the forth parameter. C function get that parameter on `RCX` as we already know, but the kernel syscalls expect them on `R10`. I forgot about that and expend quite sometime figuring out why the syscall was failing.

# The final version
So, this is how the final version will look like:

```C
#include <fcntl.h>
#include <sys/types.h>  
#include <sys/stat.h>   // Stat struct

#define BUF_SIZE 1024

// XXX: Move this to a .h file
// Dirent Data struct
struct linux_dirent {
  long           d_ino;
  long           d_off;
  unsigned short d_reclen;
  char           d_name[];
};

int     mfw_getdents (int fd, char *buf, int len);
int     mfw_exit (int r);
int     mfw_openat(int dirfd, const char *pathname, int flags);
int     mfw_newfstatat (int dirfd, char *p, struct stat *st, int flags);
int     mfw_close (int fd);
size_t  mfw_write(int fd, const void *buf, size_t count);

#define MFW_EXIT(s) do {mfw_exit (1);} while (0)

typedef int (*PAYLOAD_FUNC)(char *);

// Global vars
int level = 0;
char tabs[1024];

int mfw_puts (char *s) {
  while (*s) mfw_write (1, s++, 1);
}

// Helper function to write tabbed strings 
int mfw_print_tstr (char *s, char *v) {
  mfw_puts (tabs);
  mfw_puts (s);
  if (v) mfw_puts (v);
  mfw_puts ("\n");
}

int payload (char *target) {
  mfw_print_tstr ("   ++ Doing malware things to ", target);
}

int select_target (int old_fd, char *folder, PAYLOAD_FUNC pf) {
  char                buf[BUF_SIZE];
  struct linux_dirent *de;
  struct stat         st;
  int                 fd, n, i;

  tabs[level] = ' ';
  level ++;

  mfw_print_tstr (">> Entering ", folder);

  if ((fd = mfw_openat (old_fd, folder, O_RDONLY | O_DIRECTORY)) < 0) goto clean;
  
  while (1) {
    n = mfw_getdents (fd, buf, BUF_SIZE);
    if (n < 0) continue; // Silently ignore errors
    if (n == 0) break;

    for (i = 0; i < n;) {
      de = (struct linux_dirent *)(buf + i);
      if ((mfw_newfstatat (fd, de->d_name, &st, 0)) < 0) goto next;
      
      if (((st.st_mode & S_IFMT) == S_IFREG) && (st.st_mode & 00111))
	    pf (de->d_name);
      else if (((st.st_mode & S_IFMT) == S_IFDIR)
	       && !(de->d_name[0] == '.'
		     && (de->d_name[1] == 0
		         || (de->d_name[1] == '.' && de->d_name[2]==0))))
	    select_target (fd, de->d_name, pf);
    next:
      i += de->d_reclen;
    }
  }
 clean:
  mfw_print_tstr ("<< Leaving ", folder);
  tabs[level] = 0;
  level--;
  mfw_close (fd);
  return 0;
}

int main (int argc, char *argv[]) {
  for (int i = 0; i < 1024; tabs[i++] = 0);
  while (select_target(0, argv[1], payload));
}

```

As I did last time. This version has a few updates that I haven't described in the text. Try to understand what they are for and do not hesitate to ask your questions in case you cannot figure it out by yourself.

I named the asm code in previous section `minilibc.S`. So, in order to compile my program I have to do:

    gcc -o select_files select_file.c minilibc.S

# Conclusion
We have now working code to scan a disk and test some basic file information as the type of file and the permissions. We have also learned how to navigate the system include files to find out the information we need and also how to master the octal numeric representation.

We have removed the libC dependencies and we are ready for a asm implementation. We will find out if that asm implementation worth the extra effort, and after that we will be ready to get started with some simple payload....
