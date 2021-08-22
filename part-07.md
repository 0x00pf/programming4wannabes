# Programming for Wanabes VII. Finding files I
It is time to get started with more advanced code. We will be introducing multiple concepts from this point on and hopefully we will boost our programming skills in no time. In the previous instalment we identified the ability to find files in the disk as a feature required by several malwares.

Actually it is a feature required by many other applications and will let us learn about new system calls, loops and structures. Without further ado, let's jump into the topic

# Getting Ready to Read directories
Whenever you need to read the content of a folder and you want to be portable between platforms, the right way to proceed is using the POSIX interface.

I will first dump here a shrink down version of the general program from the [previous instalment](https://0x00sec.org/t/programming-for-wannabes-part-vi-malware-introduction/25595/). In the rest of this text we will just work out the `select_target` functions. Everything else will stay the same for the time being. So, this is our starting point:

```C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>  // POSIX directoy reading interface
#include <dirent.h>

char *the_folder="/tmp/";

typedef int (*PAYLOAD_FUNC)(char *);

int payload (char *target) {
  printf ("Doing malware things to %s\n", target);
}

int select_target (PAYLOAD_FUNC pf) {
  return 0;
}

int main () {
  while (select_target(payload));
}
```

No big surprises here, a bunch of include files with the functions and data structures we will need and the functions we introduced for our generic malware. The only tricky thing here is the definition of a function type. I introduced this silently in the previous instalment, but this time we should look deeper into this so you understand what we are doing.

## Function Pointers
One of the data types that we can use in C are the so-called pointers. We had introduced them earlier in this series, but for completeness let's quickly define them again: a pointer is just a variable that contains a memory address. It is said that it _Points to_ that address, hence the name... _Pointer_.

Usually pointers point to addresses containing data (variable pointers), but there is no reason why a pointer wouldn't point to an address containing code... a function for instance. In assembly this is very straight forward, we just need to do `call/jmp` with some kind of indirect addressing (that is, using a register or variable that contains the address to jump into, instead of the direct address, so we can control that value programatically). Let's see this with an example

    DIRECT                 INDIRECT
                           mov    payload, %rax
    call payload           callq  *%rax


In the direct code we are using the `payload` address directly. In the indirect code we load the function address in a register and then we jump to the address stored in that register. In general, when you declare a function pointer variable, that pointer will be stored somewhere in the stack, and instead of loading the address directly on `RAX` (like in this example), we will load `RAX` with that stack value. 

Let's change the `select_address` above to actually call `payload` and let's take a look to the generated code:

```C
int select_target (PAYLOAD_FUNC pf) {
  pf (the_folder);
  return 0;
}
```

This produces the following assembler. You have to compile it with `-O2` so the code gets slightly optimised, otherwise, gcc will generate code to store the parameter (the function pointer) in the stack and just after that read that stack value and put it in `RAX`. In other words it just moves the parameter around doing nothing.

```asm
00000000000006c0 <select_target>:
 6c0:   48 83 ec 08             sub    $0x8,%rsp
 6c4:   48 89 f8                mov    %rdi,%rax
 6c7:   48 8b 3d 42 09 20 00    mov    0x200942(%rip),%rdi        # 201010 <the_folder>
 6ce:   ff d0                   callq  *%rax
 6d0:   31 c0                   xor    %eax,%eax
 6d2:   48 83 c4 08             add    $0
 6d6:   c3                      retq

```

We already know all this, but let's refresh our minds once again:

* We get our parameter (the `payload` address in this case) in `RDI`
* We copy it into `RAX`
* Put the `the_folder` variable in  `RDI` (remember `RDI` contains the first parameter)
* Run the function indirectly (jump to the content of `RAX` that in this case is `payload`)

So, that's it. In this case we are using the pointer directly, but we could store it in memory and then we will be talking about a function pointer variable. This is obvious and straightforward in asm, but in C we need to use a kind of cryptic way to define function pointers:

```C
return_type (*var/type) (parameters);
```

So you just need to put parenthesis (and an `*`, after all we are defining a pointer) around the variable or type that you want to define. Let's see a few examples:

```C
int (*func)(int, int);
```
This declares a variable named `func` that is a pointer to a function returning an integer, and expecting two integers as parameters. 

```C
typedef int (*FUNC)(int, int);
FUNC func;
```

This renames a function pointer type (that is what `typepdef` does) to represent the same function we defined above. Then it declares the same variable but using the new type name. This makes the code more readable, but other than that, there is no difference at all.

Also note that the assembler generated to call our function (via the function pointer) is independent of the actual types in the declaration.... you can call the function with whatever you want... but the function will likely not work as expected, or even crash. The types definitions are just used by the compile to let us know that we are doing what we are suppose to do. Just change the type definition and recompile, you will get the same code.


# Reading a directory the POSIX way

Now we can get back to the main topic, how to read the content of a directory. Remember, virus, ransomware, spyware, all of them need to scan the disk to find different types of files. Let's see how to do this.

We will start doing it the _Right Way_, that is, how it is expected to be done by any normal system application. And that is using the POSIX interface that is composed of three functions:

    opendir  Opens a directory for reading
    readdir  Read one directory entry each time it is called
    closedir Closes the directory
	
This API is intended to mimic the normal file interface (the stream like interface offered by `fopen/fread/fclose`), but just using slightly different data structures. Using this function our `select_target` function will look like this:

```
int select_target (PAYLOAD_FUNC pf) {
  struct dirent *de;
  DIR           *d;
  
  if (!(d = opendir (_the_folder))) {perror ("opendir:"); exit (EXIT_FAILURE);}

  while (1) {
      errno = 0;
      if (!(de = readdir (d))) {
	    if (errno) perror ("readdir:");
	    break;
      }
      pf (de->d_name);
    }
  closedir (d);
  return 0;
}
```

The first thing to note is that, `DIR*` is the type used by all the function, it is similar to the classical `FILE*` that we use with files (when using the stream interface). Conceptually it is the same, a stream abstraction of a directory. The `opendir` and `closedir` are intended to intialise the structure and to finish the processing respectively (and release resources). Not much more to say about them, you need to call `opendir` before start reading the directory, and you have to call `closedir` whenever you are done processing your folder. Yes, the parameter to `opendir` is just a string containing the folder to process. We will see later what those functions really do under the hood.

The interesting function is `readdir` that is the one that actually reads directory entries one by one.

# `structs`
Before looking into `readdir` in detail, we need to introduce a new C keyword: `struct`. A `struct` is a so-called compound type. It is a compound type because it is composed of other types. Each one of those types together with the new we give to them is known as a field. You can think about a `struct` like a variable that groups more variables together in a convenient way.

The way to declare them is like this:

```C
struct name_of_the_struct {
  type1  field1;
  type2  field2;
  ....
};
```

A more specific example could be:

```C
struct linux_dirent {
  long           d_ino;
  long           d_off;
  unsigned short d_reclen;
  char           d_name[];
};

struct linux_dirent de,*pde;
```

The code above defines a new type named `struct linux_dirent` (note that you need to use `struct` to refer to the new type) composed of two longs (64bits integer), one short (16 bit integer) and a string of unknown size. You can add as many fields as you want, but in this case we are using only 4.

After the `struct` definition we have defined two variables. One is a `struct` and the other one is a pointer to a `struct`. Once the variables are declared, we can access the fields using the `.` for the struct one and the `->` operator for the pointer. Just like this:

```C
de.d_ino = 12345;
de.d_off = 0;
pde = &de;
pde->d_ino = 54321;
pde->d_off = 1;
```

Whenever you need to pass structs as parameters to function, it is usual to redefine them using `typedef`s in order to minimise the writing. Imagine a function that returns one of those `struct linux_dirent` structs and receives as parameter two of them. The prototype will look like:

```C
struct linux_dirent my_func (struct linux_dirent p1, struct linux_dirent p2);
```

This is a lot of writing and also it is harder to figure out the function prototype at one glance. Now imagine, you have 20 more function in your API to deal with this data type....So we could just create an _alias_ for this type:

```C
typedef struct linux_dirent LDIRENT;
LDIRENT my_func (LDIRENT p1, LDIRENT p2);
```

Which is way more easy to read. However this is a matter of personal use. Both approaches will produce the same code. Some people prefers to write everything so it is always clear what is that type (a struct in this case), and other prefer to redefine them. In the standard C library you will find both.

_Note: It is not common (but indeed possible) to pass and return structs in C functions, usually you will use pointers instead. The reason is that C passes parameter by value. This means that all parameters we pass have to be copied. It is easier to copy 4/8 bytes for a pointer than the roughly 24 bytes required by the struct in our example._

All this may look  complicated at first glance, but you will get used to this very quickly. This data structures are all over the place when writing non trivial programs

However, in this course we are not just learning the syntax of C, we are going deeper.

# `structs` are just memory blocks
So, what is really a `struct` ?. Short answer: It is just a memory block. When we declare a variable of type struct, we are just allocating enough space to contain all the struct elements either in the stack, in case our variable is local to a function, in the data segment, in case it is a global variable, or in the _Heap_ in case we allocate the memory block dynamically. This last case we will cover later in this course.

For our previous example we have:

```C
struct linux_dirent {
  long           d_ino;      // 8 bytes
  long           d_off;      // 8 bytes
  unsigned short d_reclen;   // 2 bytes
  char           d_name[];   // This is a placeholder we will talk about in a sec
};
```

So, this structure requires 18 bytes, which will likely be rounded up to 24 bytes to keep the memory alignment (check previous instalments when we introduce the native word size). You can just add a `printf` using the `sizeof` operator to find out the actual size of the struct. In memory it will look like:


    ADDR+18 -> | d_name
    ADDR+16 -> | d_reclen (2 bytes) 
    ADDR+8  -> | d_off    (8 bytes) 
    ADDR   --> | d_ino    (8 bytes)
                +--------------


When declaring a variable, such a variable just names that memory block... Think about it as a label, and therefore it is inmutable (you cannot change its value). It is the same with arrays... they are like pointers but not completely (we will talk about this again when arrays pop up in our way later in the course).

When you declare a pointer to a struct, you are just allocating memory to store an address that will point to memory block. Note that when declaring a pointer to an structure, the structure is not magically created. It is just a pointer. You need to allocate the memory block for the structure by other means.

You can now add the `struct` we defined above to your program (we will do that in a sec) and declare a local variable in the `select_file` function. Then take a look to the generated code. The beginning of the function will allocate extra space (the `sub $0xVAL,%rsp` at the beginning) to accommodate the new variable.

# What about the `d_name` field?

Many of you may be wondering this.... what does that `char d_name[]` means. Well, it is actually a placeholder. A field added to the `struct` to point to whatever comes after the rest of the fields. Or to get access to a specific point inside the struct if you prefer. This technique is used when the programmer needs to deal with variable length items. 

In this example we do not known how long the name of the directory entry will be. When this happens we usually have two options. We either provide enough space so the longest possible name will fit in our memory block (and/or we limit the longest possible name with additional checks in the code), or we dynamically allocate space for the directory entry whenever we find out its size. Allocations just don't happen magically... even on interpreted languages all these processes are happening under the hood... whenever you add two strings in python a lot of allocation and memory movement happens.

Let's see how would this work. Imagine we are allocating our structure in the stack. Note that the actual memory block is created/managed by the `readdir` function not for us. The function gives us a pointer to the memory it manages/allocates. As, for the time being, we only know how to allocate memory in the stack, let's assume `readdir` allocates memory in the stack, however it is likely to use some global storage or the heap. You should have an idea of the why at this point.

Also, let's assume, that the syscalls used by `readdir` (remember `readdir` is a libc function not a system call), will let us known the size of file name it is reporting. Let's imagine the length of the filename is `len`.

Then `readdir` will allocate in the stack 24 bytes + len, so there is enough space to store the `struct linux_dirent` fields plus the string. In this case the stack will look like this:

    ADDRS+18+len -> | \0
    ADDRS+18     -> | d_name (the string goes here)
    ADDRS+16     -> | d_reclen (2 bytes) 
    ADDRS+8      -> | d_off    (8 bytes) 
    ADDRS       --> | d_ino    (8 bytes)
                    +--------------

Whenever we access the `d_name` field that is located at `ADDR+18`, we will find a variable length string containing the name of the file being read by `readdir`.

From a syntactic point of view `char d_name[]` represents a character string of unknown size. In practise it is just indicating the offset in the memory block holding the structure where the string will be.

This technique is also sometimes used in network programming when the length of the packet is unknown until the packet header is read and the field containing the packet size can be read.

# Reading the directory

Now that we know what a `struct` is, we can start using `readdir`. This function returns a pointer to a `struct dirent`. This type is defined in `#include <dirent.h>` and as you had already figure out the name comes from _DIRectory ENTry_.

So, each time we run `readdir` we will get the information of one of the files in the directory. We have to call it again and again until the whole directory is read. So, the question is: when should we stop?. Well, the answer, is in the `man` pages. Never underestimate the amount of information provided by the man pages. So this is what it says:

    RETURN VALUE
           On  success, readdir() returns a pointer to a dirent structure.  (This structure
           may be statically allocated; do not attempt to free(3) it.)
    
           If the end of the directory stream is reached, NULL is returned and errno is not
           changed.   If  an error occurs, NULL is returned and errno is set appropriately.
           To distinguish end of stream and from an error, set errno to zero before calling
           readdir() and then check the value of errno if NULL is returned.


Crystal clear. Now we can recall the main loop in our `select_target` function and look at it again:

```C
  while (1) {
      errno = 0;
      if (!(de = readdir (d))) {
	    if (errno) perror ("readdir:");
	    break;
      }
      pf (de->d_name);
    }
```

We had already introduced the `while` loop in the past. It just loops _while_ the condition we set in the `while` is true. In this case, `while(1)` means that the loop will run forever, because the condition is always true (!= 0).

_NOTE:C doesn't has a boolean type. Conditional operators traditionally returns FALSE as 0 and TRUE as not zero. Setting the while condition to 1  means that it is always true. You could set it to 31173 and it will work the same, but why would you type 5 numbers when you can just type 1?._

Then we are prepared to call `readdir`. We set `errno` to zero as proposed in the man page, and call the function, if we get a `NULL` we fall into the `if`. Then we check again the `errno` variable and if it has changed we show and error. In either case, we had an error or we have reached the end of the list, we leave the while loop using `break`.

>NOTE: The expression we use above `if (!p)` is equivalent to `if (p == 0)` or if you prefer `if (p == NULL)`, the compiler will see that `p` is a pointer and will change 0 to a compatible representation of `NULL`. The way to write this is a matter of personal taste and some people says `if (!p)` is bad style, and some other say it is good.... Just do whatever better suits you, but be aware of what is going under the hood. The key point here is that NULL is a special value and doesn't need to be the integer 0. This may be confusing for the beginner. You can take a look to [the c faq NULL section](http://c-faq.com/null/) for details.

Otherwise, we access the field `d_name` in the struct `struct dirent` that contains the name of the directory entry returned by `readdir` and pass it to the `payload` function. 

# Reading directories with system calls

We have a working function able to read the content of a directory in the disk using the POSIX interface. Overall, when writing malware we would like to minimise application dependencies and in the extreme case that implies just using the OS and avoid all libraries... However this is not always possible.

In this case, instead of using the POSIX function we can use the associated Linux system call. This is perfectly fine, however the drawback is that the POSIX version will work with all POSIX compatible operating systems and the non-POSIX version will be Linux specific. In other words, all POSIX compatible system have the `opendir/readdir/closedir` functions but each one will have different system calls to access the directories. That is what standards are for.

> NOTE: That our program will work in any POSIX complaint system (Linux, NetBSD, OpenBSD, Solaris, MacOs...) means that we can recompile for those systems and the program will still work, it doesn't mean that any compiled version of our program will run magically in all POSIX compliant OSes

> NOTE2: Linux is not officially POSIX complaint. Despite of possible minor divergences the main issue is that POSIX certification, as most certifications out there, are really achieved by paying a fee...

Anyhow, malware is usually target specific (platform-wise) and getting rid of the standard libc will make our program very small and give us much more control on what is in it and what is not.

So, the system call that we have to use is known as `getdents`. Sure, you got it, it stands for _GET Directory ENTries_. There is a man page for it and it says that there is no wrapper provided by libc, so we have to write our own if we want to use it (the man page already says how to do that):


```C
#include <sys/syscall.h>

int getdents (int fd, char *buf, int len)
{ return syscall (SYS_getdents, fd, buf, len); }
```

We will not go all the way down to the kernel right now. We implement it using the `syscall` standard function instead of invoking the `syscall` processor instruction directly so we can still use C code and we do not need to start adding assembler at this point. We will get to that a bit later.

In order to use this function, we need a file descriptor for the directory. We can get this using the standard `open` system call. This will do the trick:

```C
int select_target (PAYLOAD_FUNC pf) {
  char                buf[BUF_SIZE];
  struct linux_dirent *de;
  int                 fd, n, i;
  
  // Open directory using open
  if ((fd = open (folder, O_RDONLY | O_DIRECTORY)) < 0) exit (1);

  while (1) {
    // Read directory entries
    }
  }
  close (fd);
  return 0;
```

At this level, we manage the directory exactly the same than a file. We `open` it... and we `close` it whenever we are done. Now is time to see how to use `getdents`.

## Using `getdents`
The `getdents` prototype is as follows:

    int getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count);

It receives as parameters a file descriptor (the one we got from the call to `open` with flag `O_DIRECTORY`), then a pointer to the `struct linux_dirent` (actually an pseudo-array of items of this type) and finally the size of the buffer we pass as parameter in the second parameter. You will understand this in a second.

As you can see the second parameter is of type `struct linux_dirent` instead of the `struct_dirent` that we used with the POSIX version. These structures are slightly different, but we can get them from the man pages of the `readdir` function and `getdents` system call respectively. Anyway we had already introduced it previously when we talked about `structs .

So, how does `getdents` works?. It doesn't return just one directory entry... it returns as many as will fit in the buffer we pass as second parameter, and that number will vary depending on..... Yes sure, on the length of the name of each entry. So the return value (the number of bytes read) is important in order to extract the information.

The man page also include example code on how to use the system call. I will include here a simplified version to explain how does this syscall work:

```C
  char                buf[BUF_SIZE];
  struct linux_dirent *de;
  int                 fd, n, i;
(...)
  while (1) {
    n = getdents (fd, buf, BUF_SIZE);
    if (n < 0) exit (1);
    if (n == 0) break;

    // Build file name
    for (i = 0; i < n;) {
      de = (struct linux_dirent *)(buf + i);
      
	pf (de->d_name);
    i += de->d_reclen;
  }
```


First we call the syscall and process errors and end condition. And after that we have to process all the entries reported by the syscall...and we do not know how many are there. That number will depend on the size of the buffer we pass to the syscall. So we run our loop over bytes and not over `struct linux_dirent` items because we do not know the size of each entry in the array (actually this is why it is not really an array). 

The variable `n` contains the number of bytes read by `getdents`.

The first entry will be at offset zero of our buffer. We access it casting our general buffer to the structure, do what we want to do and then we increase the offset by the size of the directory entry that is stored in the field `reclen`. This will update the offset in the buffer to point to the next entry and we repeat the process.

>NOTE: Casting a pointer is just forcing it into some type. This only make sense for the compiler. In reality, the memory is the same, regardless the cast operation we apply. Casting will allow us to tweak our view of a given memory block. Imaging our memory block is 16 bytes. We can see it as 16 `unsigned chars`, 8 `shorts`, 4 `ints`, 2 `longs` or 1 `longlong`. The memory block will have the same content but in our program the values that will get after casting will be different.
> 
> Example: 
> A 8 bytes memory block at address ADDR contains.
>
>     addr =  | 0x00 | 0x01 | 0x02 | 0x03 | 0x04 | 0x05 | 0x06 | 0x07 | 
>```C
> long *l = (long *) addr; // l[0] or *l will be 0x0001020304050607
> int  *i = (int  *) addr; // i[0] or *i will be 0x00010203 and i[1] or *(i+1) = 0x04050607
> char *c = (char *) addr; // c[0] = 0x00, c[1]= 0x01, .... c[7] = 0x07
> ```

Let's see this with an example. Imagine a folder containing just a file named `a.txt`. This is what `getdents` will return in the buffer:

    buf+64     -> | inode
                  +-------------- 
    buf+59     -> | a.txt\0
    buf+57     -> | 24
    buf+49     -> | offset
    buf+41     -> | inode
                  +--------------
    buf+38     -> | ..\0
    buf+36     -> | 21 
    buf+28     -> | offset
    buf+20     -> | inode  <----------+-- addr + 20
                  +--------------     |           ^
    buf+18     -> | .\0               |           |
    buf+16     -> | 20     -----------+-----------+
    buf+8      -> | offset            |
    buf       --> | inode  <----------+--- addr
                  +--------------

_NOTE: According to the man page, the offset is the distance from the start of the directory to the next dirent struct, however after printing the values I get on my test program those number look strange. I may need to double check, but may be related to the actual EXT3 filesystem that stores the directories as linked lists. For the time being we can use `reclen` to deal with the buffer returned by `getdents`, and ignore `d_off`_

As you can see we always get the current (`.`) and the parent ( `..`) directories and then the rest of files. In this case we only have an extra file and our 1024 bytes buffer will be mostly empty after reading the whole folder. A directory containing many files may fill the buffer completely and we may need to call `getdents` again to keep reading the directory.

# Opaque data types. The `DIR` struct

Now we could figure out what is in the `DIR` type we used with the POSIX interface. It is not that we need that, but figuring out this kind of things will boost your learning... so it is up to you to skip this section or not. 

The `DIR` type is a so-called opaque data type in the sense that the programmer (that is us) cannot see what is in it. Compare this to the `struct dirent` we have been used in our examples, where we can see the different fields and we actually need to use them.

Opaque data types are used together with an API that does what we need so we do not need to access the structure directly. This has the advantage that new versions of the SW may change the internal structure of the data type and, as far as the API doesn't change our program will still work. This concept is known generically as _Encapsulation_.

Making a structure opaque is just a matter of not exposing the internal structure. That's means, the structure is not defined in the .h files available to the programmer. We will see how to do this later. For the time being this is not relevant.

So, with all the information we have, and after learning how to use the POSIX API we can figure out what is in this `DIR` data type and also how to implement the different functions. The structure would be more or less like:

```C
typedef struct __my_dirstream {
	int      fd;             // File descriptor returned by open. Required by getdent
	char     buf[BUF_SIZE];  // Buffer to read directory entries (to call getdent)
	int      n;              // Number of bytes to process
	int      off;            // Number of bytes already processed
} MY_DIR;
```

I will leave as exercise to the reader the implementation of the POSIX interface using `open/getdents/close`. It is a nice exercise to get more fluent with the C programming language. Just do it, it is pretty straightforward with all the information we have learnt so far and will help you to get comfortable with C... you may need to add more fields to the structure above depending on how do you implement the API.

# The final version
Just for your convenience this is the final complete version of our directory listing program:

```C
#define _GNU_SOURCE  // Needed by syscall
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>  
#include <sys/stat.h>   // Stat systemcall

#include <sys/syscall.h>

#define BUF_SIZE 1024

#define MFW_EXIT(s) do {perror (s); exit (EXIT_FAILURE);} while (0)

// Dirent Data struct
struct linux_dirent {
  long           d_ino;
  off_t          d_off;
  unsigned short d_reclen;
  char           d_name[];
};


char folder[1024];

// getdent wrapper. Not provided by glibc
int getdents (int fd, char *buf, int len)
{ return syscall (SYS_getdents, fd, buf, len); }

typedef int (*PAYLOAD_FUNC)(char *);

int payload (char *target) {
  printf ("Doing malware things to %s\n", target);
}

int select_target (PAYLOAD_FUNC pf) {
  char                buf[BUF_SIZE];
  struct linux_dirent *de;
  struct stat         st;
  int                 fd, n, i;
  
  // Open directory using open
  if ((fd = open (folder, O_RDONLY | O_DIRECTORY)) < 0) MFW_EXIT("open:");

  while (1) {
    n = getdents (fd, buf, BUF_SIZE);
    if (n < 0) MFW_EXIT ("getdents:");
    if (n == 0) break;

    for (i = 0; i < n;) {
      de = (struct linux_dirent *)(buf + i);
	  pf (de->d_name);
      i += de->d_reclen;
    }
  }
  close (fd);
  return 0;
}

int main (int argc, char *argv[]) {
  strcpy (folder, argv[1]);
  while (select_target(payload));
}

```

It has some minor changes and all the required includes and defines. I would recommend to go through it and try to understand the stuff that is not described in this text. Do not hesitate to ask in the comments if you do not understand something.

# Conclusions

We have learnt how to read a directory using the standard POSIX interface and also using system calls. We have also learnt about function pointers and `structs`. A lot of stuff to digest. I know.

This is the first step to implement the `select_target` function. The second one is to be able to determine the details of each file in the directory and thus select the target needed by each specific malware. This is what we will deal with in the next instalment.

Note that these articles are intended for newbies, so be free to ask in the comments about any doubt. There is no stupid question when you are starting so do not be shy, I'll try to answer all of your doubts and I'm also interested on knowing if the level of the text is too easy or too hard, so your feedback will be pretty much appreciated. 

However I would recommend to first try to answer your question by yourself, using Google, and rechecking the previous instalments. It is not just bad [nettiquette ](https://en.wikipedia.org/wiki/Etiquette_in_technology) it is also way better for you to learn. The things you learn by yourself remind steady in your memory and broads your view of the topic.
