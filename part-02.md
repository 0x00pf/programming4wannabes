# Programming for Wannabes. Part II

Glad to see you have come back to this humble course. Hope you are eager to get a lot more stuff to digest. Grab some coffee and relax.

I have been claiming that this course is going to be different to all those that you find over the Internet. Right now, I should introduce a whole bunch of boring things (numeric representation, addressing modes, instructions groups,...) and guess what?... I won't do that.

In order to avoid going through all that boring stuff and to try to follow a learn-by-example approach, we need to be able to build programs. In the previous part we've got a glimpse on that, but we cannot yet produce a executable out of some assembly code.

Furthermore, in the previous part we started looking to the HW to help us introduce some concepts. In this part, we are going to look a bit to the upper layers and how they lay one over the other.

Finally, you need to know that we are going to use `nasm` in this course. This is a very well-know and powerful assembler that will help us to produce machine code out of our assembly code. So, before you keep reading, go and install it!

# Our First Assembly Program
So, let's start with the simplest, Linux OS-compliant program we can write. There you go:

```
	global _start
_start:	mov rdi, 10
	mov rbx, 20

	add rdi,rbx

	mov rax, 0x3c
	syscall
```

Well, the central part of the program should look familiar to you by now. We are using different registers in this case, but we are again adding two numbers. There are two main  differences with respect to our previous example:

- First, we are defining a symbol that will be `global` and will mark up the first instruction of our program. That is the `_start`. Try to remove the `global` declaration and you will get an error when you try to compile it saying that the `_start` symbol cannot be found.
- We have two new instructions at the very end. We will explain these in detail in a sec, but first let's compile and run the program to check that we are good to go.

Let's compile our program:

    $ nasm -f elf64 -o c2-1.o c2-1.asm
    $ ld -o c2-0s c2-1.o
    $ ./c2-0s; echo $?
    30

Very good. We have built an executable out of our assembly source code and when we run it, the program returns the result of our operation. I believe this is the simplest ASM program you can actually write that does something. Yep, the bash variable `$?` contains the return value of the last executed command. Try to change the numbers in the ASM file, recompile and check that the `$?` will contain the sum.

So.... we have something to start to work with.... and a lot of things to explain.

# Producing and Executable
Let's start from the last step. The generation of the binary. To produce a program out of an ASM source code, we will have to convert it into machine code and then convert it into a program that can be executed by a given OS. Some times we do this in just one step, but it is actually two separated stages.

As you should know by now, we are working with Linux. The default executable format is ELF. We are writing 64bits ASM, so we will tell our assembler that we need some object code (that's the name for those .o files you obtain from your assembler or compiler) in a specific format. In this case `elf64`. We use the command-line flag `-f` to specify that. The `-o` stands for _output_ and let us indicate the file were we want the result stored. Then we just append the list of source files to assemble.

Using the right format for our object code, is important, because that will allow us to use all other standard tools in the system. One of those tools is the linker.

Let's look to the standard process of producing an executable:

- The **source code** (in any language) is converted to **object code**. This is done by the compiler. An assembler can be seen as a compiler for assembly code.
- The **object code** is not directly executable. it needs to be converted into an application, and this task is done by the linker. In our case we have one single object file but, in general, an application is composed of multiple object files (coming from different source code files), static libraries, dynamic libraries, etc... The linker is the program that _links_ all those pieces together, and it does this in a way that the OS can make sense out of it.

In this simple case we are invoking the linker (`ld`) and we are instructing it to produce a executable named `c2-0s` (the `-o` flag again) out of a single object file. In general, the compiler is able to invoke the linker under the hood, and that is why you can produce a binary out of your source code invoking `gcc`... but be aware that, to do that, `gcc` is actually calling other programs, including the linker `ld`.

We will be using the assembler and the linker continuously. Do not worry if you haven't fully grasp the idea. You'll do as we go. For now, if you haven't fully understand this, just keep in mind that you have to use `nasm` to compile your ASM code, and `ld`, to produce an executable out of it.

# System Calls
At the beginning I said that we will be dealing with the other layers in our simplified SW model (OS, standard library,..). We have just seen that we need to produce our executable in a format that the OS can understand. But we have to interact with the OS.

For our simple example, our interaction with the OS consists on returning a value that we've calculate inside our program whenever our program executing ends and returns control to the calling process (usually `bash` the command-line interpreter). If we keep the value in a register inside our program there is no way to let anybody else know about it. 

So, in order to interact with the OS we have to use a system call (**syscall**). The operating system offers some services to the applications running on it. This services are accessible using these system calls. Opening files, mapping memory, reading directory contents,... all these actions requires interaction with the hardware (the hard drive, the memory management unit, the file system driver -ok, this one is not HW :)-,...) and are managed by the OS. 

In the last two lines of our program we are actually using one of those services. The system call we are using is `SYS_EXIT`, that finishes the process returning a value to the calling process.

System calls are identified by a number, and the number for the `exit` syscall is `0x3c`. As you can see, in order to access a system call in a x86/64bits processor, we have to do 3 things (actually you can only see, explicitly, two of them in our example).

1. Load the system call number in the `rax` register. 
2. Load other required parameters in other registers. This we are not doing directly
3. Use the `syscall` instruction to jump into kernel mode.

Yes, system calls are executed by the kernel (the OS), so what we basically do, is to put some values in some registers and then give control to the OS. The way to do that is platform-dependent. The `syscall` we used above is the standard way to invoke a system call on a x86 64bits. For 32 bits you usually invoke the software interruption 0x80 (`int 0x80`).

Let's reproduce the last lines of our program here again:
```
	add rdi,rbx
	mov rax, 0x3c
	syscall
```

We can clearly identify how we set `rax` with the value `0x3c` (the `exit` system call). We can also clearly identify how we give control to the kernel using the mnemonic `syscall`. But what is not clear is how do we pass the result of our operation to the `exit` call so the `$?` bash variable gets actually modified.

Well, for the `exit` system call, the `rdi` register have to be set with the value we want our program to return. That is why we have done the addition directly on the `rdi` register, so we do not have to explicitly set it before calling our system calls. Reordering the source code:

```
mov rax, 0x3c
add rdi, rbx          ====>  exit (a+b)
syscall           
```

In this link you can find a list of system calls and the registers that you should use to pass parameters to them.

http://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/

# Getting our Function to Work
Now, we completely understand our assembly code, and we also know how to produce an executable out of it. The last piece in the puzzle is how the compiler+linker know that they have to run the code marked with the `_start` label.

In order to understand this, we are going to move into C and try to reproduce our assembly program. The same program in C would look like this:

```
#include <unistd.h>

int main (void)
{
  int a = 10;
  int b = 20;
  
  a = a + b;
  _exit (a);
}
```

So, in C, things are a bit different. Once compiled, whenever we run our program it will start executing whatever we write in the `main` function. This is how the language was defined. The code inside the `main` function, declares two integer variables (two integer numbers), adds them and then calls a function called `_exit`, passing as parameter the result of the sum.

This is pretty much the same thing we have done in ASM some paragraphs above, but using the C programming language. Yes, we removed the `register` keyword here, the compiler will not honour it in the `main` function directly, so, let's forget about it for now.

Let's now go line by line for the less experienced readers. Advanced readers can safely skip the next sections

## First Line
The first line found in the program is:

```
#include <unistd.h>
```

This line is a pre-processor directive. Anything starting with a `#` in a C program is a pre-processor directive. This directives are instruction to a program called `cpp`. This program is run before the actual compilation and it effectively modifies our source code in different ways before it is compiled.

> NOTE: Old compilers requires the directive pre-processor be placed at column 0. If you get a weird error message and everything looks fine, try to put your pre-processor directives at the beginning of the line

In this case, the `#include` directive, as you can imagine, includes the indicated file at that position in the source code. Not sure what does this mean?. OK, no problem. Let's see what the pre-processor will produce for our program:


    $ cpp c2-1.c | less

Now take a look to `unistd.h`... it is located in the system include folder:

    $ more /usr/include/unistd.h

Well, that was not a great idea. This file has a lot more pre-processor directives... some that we have not used yet. However, I hope you get the idea. The content of `unistd.h` is included at the `#include` location in our source code... and any other pre-processor directives in that file are processed recursively.

Actually, for our simple C program we just need one line (in fact we not even need that, but let's be legal); the prototype of the `_exit` function. If you look for it in the output of the pre-processor you will find something like this:

```
extern void _exit (int __status) __attribute__ ((__noreturn__));
```

That is the unique line we really need. We will describe in detail what a prototype is and how to use them later in this course. For the time being, you just need to know, that, in order to be able to use a function that is defined outside our C source code, we have to provide the definition for that function... (this is not the complete history but it is enough for now).  In general, what we need to tell the compiler is the return type and number and types of parameters... but we haven't talked about types yet... just keep in mind that you need to tell the computer how the function you want to call looks like and you do that with a function prototype.

## The `main` function
The next line in our C program is the `main` function. As we have already said, this is the function that gets executed whenever we launch our program. The `main` function is usually declared in two different ways.

The second one is the one we used in this example. Whenever you are not interested on command-line parameters, you can declare `main` as a function that returns an integer (`int`) and does not receive any parameter (`void`).

However, you usually want to access command-line parameters provided by the user. In those cases, the `main` function is declared as:

```
int main (int argc, char &argv[])
```

You can change `argc` and `argv` for whatever identifiers you want. However those are the names universally used for the `main` function parameters.

The first argument `argc` is an integer that indicates the number of command-line parameters the user has provided when launching the application. The `argv` is an array of strings. One string for each space separated parameter provided by the user (unless you quote the parameters). Again, we will talk about this later. Right now, we just need to now that `main` is the function that gets first executed and it can be declared in, at least, two different ways...

## The _exit() Function Call
The body of the function should be clear by now, so it is only the last line that needs some explanation. The last line is a function call... you will recognize that by the parenthesis. In this case, the `_exit` function is a wrapper around the `exit` system call, provided by the standard C library. Here we see clearly how the standard C library lays on top of the operating system interface, and gives us a simpler interface to access the functions provided by the OS.

To my knowledge, there is no standard way to directly access system calls from a C program without using the standard C library. Apparently the old `_syscall` function is deprecated and it is, anyway, a function from the standard C library.

In a normal C program, you will usually see the `exit` function to exit a program, instead of the `_exit` that we are using. The first one is a higher level version defined in `stdlib.h` (the standard library header) instead of the, slightly lower level versiondefined in the `unistd.h` (UNIX standard include). Check the man page for `_exit` to know about the differences.

In short, `_exit` is the closest we can get to the `SYS_exit` system call from C... it does a couple things less than the standard `exit`. OK, nevermind... we are going to get rid of it anyway pretty soon.

# Wait a Second...
So...when I write a C program, my program starts running at `main`, but when I write an ASM program, it starts running at `_start`?... why?

That is a good question. Actually, that is not true. In both cases the function/code that gets executed at the very beginning is the one pointed by `_start`. However, a standard C program, does quite a lot of things before the `main` function gets actually executed. And know what...we can actually make our C program look a lot more like the ASM version.

What happens is that the linker (do you remember that guy) is adding a default version of `_start` that, at the very end calls our `main` function. This code is contained in a file called `crt1.o`... and sure, we can get rid of it.

Let's rename our `main` function to `_start`. Our program will look like this:

```
#include <unistd.h>

void _start (void)
{
  register  int a = 10;
  register  int b = 20;

  a +=b;
   _exit (a);
}
```

Now we have to tell the compiler that we do not want to use `crt1.o`. The compiler knows this file (and some other ones used for application start up) as a _start file_:


    $ gcc -nostartfiles -o c2-2 c2-2.c
    $ ./c2-2; echo $?
    30


Good. Our program still works the same. We have made it start at `_start` instead of at `main` and, now, it looks a lot closer to our original ASM code. Actually we have achieve a big improvement. Let's produce a static version of our original C code using the start files, and our last version without using them.

    $ gcc -static -o c2-1s c2-1.c
    $ gcc -static -nostartfiles -o c2-2s c2-2.c
    $ ls -l c2-?s | awk '{print $9, $5;}'
    c2-0s 737
    c2-1s 872956
    c2-2s 5420

Wow... that is a big difference in size!... We are still far away of the size of our first assembly version, but that is not bad. Try to do an `objdump` on the two files produced from C source code to see what's the difference.

# The Last Step
There is one last thing we can do, to get even closer to our original ASM code. We can get rid of the C library!

If you had paid attention, I said before that we need the standard C library to invoke a system call in a portable way. But what if we do not care about portability?... Yes, we already know how to invoke the `exit` system call from ASM... So, why we do not get rid of the C library and we provide our own implementation for the `_exit` function?. Let's try:

So, let's create a new file named `exit.s` and let's declare the `_exit` symbol in there together with our ASM code to call the `exit` syscall:

```
    .global _exit
_exit:
    mov $0x3c, %eax
    syscall
```

This should be pretty basic. You may be missing something... don't you?. Sure. Where is our parameter? Well, it is actually there just because of the standard C **ABI** (_Application Binary Interface_). The ABI defines, among other things, how parameters are passed to functions. We will be discovering it as we go on through this course. For know, it is enough to know that the first parameter we pass to a C function is stored in the `rdi` register (supposing it fits there). 

Note that this is for x86 64bits processors. 32 bits and other processor follow different ABIs and the parameters are passed in different ways. This is one of the reasons why it is a bad idea to do what we are doing if we are planing to re-compile our programs for different platforms... This is one of the reasons why the standard C library is there... To make our C code portable.

Let's try to recompile our program. We do not have to change anything on the code. We are just going to use our new `_exit` function and tell the compiler to forget about the standard C library (`libc`) version of `_exit`.

    $ gcc -static -nostartfiles -nostdlib -o c2-0s c2-2.c exit.s
    $ ls -l c2-?s | awk '{print $9, $5;}'
    c2-0s 737
    c2-1s 872956
    c2-2s 5420
    c2-3s 1341

The first thing to note is that the `-nostdlib` flag is the one that let us remove the standard C library. The second is that we can pass assembly source files directly to `gcc`!!!. The third one is that our program that was close to 1MB when we started is now slightly above 1KB!. We have got pretty close to the size of our original assembly version!

Actually, if we strip the binaries, the difference of removing the standard C library is not that relevant, but our C version size still doubles the original assembly one:

    $ sstrip -z c2-0s
    $ sstrip -z c2-1s
    $ sstrip -z c2-2s
    $ sstrip -z c2-3s
    $ ls -l c2-?s | awk '{print $9, $5;}'
    c2-0s 148
    c2-1s 789552
    c2-2s 589
    c2-3s 369

# A Word on the Linker
Before finishing this paper, let me share with you a couple of words about the linker. You may be wondering (I did some time ago) who decides that `_start` is the first function to run, or why my `.text` segment goes to address `0x400000` (again on a 64 bits computer). OK, the answer to all these questions is: **THE LINKER**.

You can modify those values using linker flags. For instance, let's change our entry point to `_start1` and put our `.text` segment at `0x500000` instead of `0x400000`

    $ gcc -static -nostdlib -nostartfiles -o kk c2-3.c exit.s -Wl,-e_start1 -Ttext=0x500000
    $ readelf -hs kk
    ELF Header:
      Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
      Class:                             ELF64
      Data:                              2's complement, little endian
      Version:                           1 (current)
      OS/ABI:                            UNIX - System V
      ABI Version:                       0
      Type:                              EXEC (Executable file)
      Machine:                           Advanced Micro Devices X86-64
      Version:                           0x1
      Entry point address:               0x500000
      Start of program headers:          64 (bytes into file)
      Start of section headers:          1048800 (bytes into file)
      Flags:                             0x0
      Size of this header:               64 (bytes)
      Size of program headers:           56 (bytes)
      Number of program headers:         3
      Size of section headers:           64 (bytes)
      Number of section headers:         8
      Section header string table index: 5

    Symbol table '.symtab' contains 12 entries:
       Num:    Value          Size Type    Bind   Vis      Ndx Name
         0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
         1: 0000000000500000     0 SECTION LOCAL  DEFAULT    1
         2: 00000000004000e8     0 SECTION LOCAL  DEFAULT    2
         3: 0000000000500030     0 SECTION LOCAL  DEFAULT    3
         4: 0000000000000000     0 SECTION LOCAL  DEFAULT    4
         5: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS c2-3.c
         6: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS
         7: 0000000000701000     0 NOTYPE  GLOBAL DEFAULT    3 __bss_start
         8: 0000000000500000    38 FUNC    GLOBAL DEFAULT    1 _start1
         9: 0000000000701000     0 NOTYPE  GLOBAL DEFAULT    3 _edata
        10: 0000000000701000     0 NOTYPE  GLOBAL DEFAULT    3 _end
        11: 0000000000500026     0 NOTYPE  GLOBAL DEFAULT    1 _exit

As you can see, the `_start1` function is located at `0x500000` that is actually our entry point (check the ELF header just above).

As you may have already figured out, the `-Wl,XXX` is the way to feed options to the linker from the compiler. Alternatively you can create `.o` object files and then link manually, passing the options to the linker normally. The same way we did with our first ASM program.

You may be wondering now. That's fine... more command-line options. But usually I do not set those command-line options when I compile my programs. Where does all those values comes from?

Again a very legit question. I will not completely answer it, but I will give you some hints to get some fun yourself in case you are interested on this stuff. 

The first hint is **'linker script'**. It is very likely that you will never see one of those, even if you become a professional programmer. And it is even more likely that you will ever have to write one of those. However, if you go into the world of embedded systems... well, you will surely have to deal with them.

The second one is:

    $ gcc -o c2-1 c2-1.c -Wl,-verbose


# CONCLUSIONS
In this part we have learn how to convert an ASM source file into a executable program for the Linux OS. We have also got a basic idea on how to use system calls and also how the program start up process in a C program relates to a similar program written in ASM code. 

Finally, we have learn about the main parts of a C program which are automatically and silently added to our code whenever we compile it. We have also seen how the standard library provides a portable and uniform interface for our C programs and how it does hides the platform-specific details to interface with the Operating System. In a sense we have also experienced how this Standard library, stacks on top of the OS, that stacks on top of the _Bare Metal_. 

In this trip, we have seen how to strip down a C program to reduce it to its minimal expression... really close to what we can achieve writing our code directly in ASM... and how we lost, in the process, the portability of our code.


