# Programming for Wannabees. Part III. Your first Shell Code

Let's go on with our special ASM/C programming course. At this point, we roughly know how a computer works, its main components, what is machine code, what is assembly code and how to compile simple programs for a few architectures.

In this part we are going to write our first shellcode. Yes, we are that advanced. But, before we get there, we will be exploring a few more new concepts. Let's start.

# Processor Native Word Size
In this part we will start dealing with stuff bigger than 1 byte, and to understand what is going on, we need to introduce the _Processor Native Word Size_. In a sense, you already know what it means. Whenever you talk about 32bits and 64bits processors/programs... well, that is the processor native word size.

However, beyond being the type of Linux distribution you choose to install, this value has some low level implications:

* This value is the native size of the processor registers. Do you remember the registers within the processor?. Sure you do. So, a 32 bits processor have 32bits registers and a 64bits processor have 64bits registers. This is not completely accurate but, for now, just consider this size as the one the processor is comfortable with.
* This value is usually also the width of the data bus.
* And, it is usually also the width of the address bus... 

Overall, and without going into the electronics within the processor, what you need to know is that each processor is optimised to work with its native word size. This way, a 32bits processor will perform arithmetic operations or access memory faster when it deals with a 32bits long value that when it deals with a 16bits long value. I know, this is a bit contra-intuitive and a bit of an act of faith but it will be very long and tedious to go through the details to understand this. Actually I'm not sure I could successfully guide you through that path.

To further illustrate this, below is a fragment of the ""Intel's  80396 Programmers's Reference Manual"" from 1986 (http://microsym.com/editor/assets/386intel.pdf). You can find it in section 2.2 ""Data Types"" page 24.


> Note that words need not be aligned at even-numbered addresses and
> doublewords need not be aligned at addresses evenly divisible by four. This
> allows maximum flexibility in data structures (e.g., records containing
> mixed byte, word, and doubleword items) and efficiency in memory
> utilization. When used in a configuration with a 32-bit bus, actual
> transfers of data between processor and memory take place in units of
> doublewords beginning at addresses evenly divisible by four; however, the
> processor converts requests for misaligned words or doublewords into the
> appropriate sequences of requests acceptable to the memory interface. Such
> misaligned data transfers reduce performance by requiring extra memory
> cycles. For maximum performance, data structures (including stacks) should
> be designed in such a way that, whenever possible, word operands are aligned
> at even addresses and doubleword operands are aligned at addresses evenly
> divisible by four. Due to instruction prefetching and queuing within the
> CPU, there is no requirement for instructions to be aligned on word or
> doubleword boundaries. (However, a slight increase in speed results if the
> target addresses of control transfers are evenly divisible by four.)



Summing up, try to program your processor using its native word size. It may look that you are wasting some space, but that is the right way to do it. 

I will just give a small hint on this topic before going on.


# An Example: Processor Word Size and Memory.
We have just said in the previous section that a 32bits processor will operate faster on a 32bit value and will also access faster a 32bits value in memory than a 16 bits one.

The first thing you can infer from that sentence is that, even if the smaller addressable value in memory is 8 bits (do you remember the size of our memory drawers from Part I?), a 32bits processor can read 32bits from memory at once.

This simple sentence has quite some concepts behind it. Let's go one by one

## Simplified Memory Hardware Model
The memory system in a PC can be quite elaborated and it is beyond the scope of this course to go into those details, however, discussing a simplified model of the system memory will be beneficial for us.

In its simplest way, a memory chip provides the following pins... 

- `AX` pins usually known as Address pins (with X=0...WordSize)
- `DX` pins usually known as Data pins    (with X=0...WordSize)
- `Cx` pins usually know as Control pins. (depends on the processor)

The CPU also have similar `AX` and `DX` pins that are connected to the memory. This connection is not direct (we really need a bus between them), but for this simplified model we can consider that they are directly connected.

The control pins allows the _Processor_ to command the memory. In this group you will usually find: 

- `OE/CS` (Output Enable, Chip Select). These pins will, in a sense, _ACTIVATE_ the memory chip (either the input or the output). 
- `WR/RD` (Wrtite, Read). These ones are used to tell the memory if we want write into the memory or to read values from it.

So, whenever the CPU wants to read a value from the memory, it puts the value of the address to access in its address pins (`AX`) which are connected to the memory `AX` pines, usually by a bus. Then, the right control signal are exercised in the memory chip, and it will put in its `DX` pins the value in the memory address indicated by `AX`.

OK, let's put some numbers to better understand this:

    ----+                          +-----------
        +- A0 ---------------  A0 -+
        +- ... -------------- ... -+
    CPU +- A31--------------- A31 -+  Memory 
        +- ...                     |
        +- D0 ---------------- D0 -+
        +- ... -------------- ... -+
        +- D31 -------------- D31 -+
    ----+                          +------------

Imagine that the `RIP` register (do you remember the Instruction Pointer?), is pointing to address 4, so, the instruction on address 4 is the next one to execute. The CPU has to read from memory that instruction, so it puts in the address bus the value

    1098765432109876543210
    33222222222211111111119876543210
    --------------------------------
    00000000000000000000000000000100

Only pin `A2`  will be set. That actually means address 4. So, these pins are also connected to the memory. Whenever the `RD` control signal is activated in the memory, the chip will access address 4 and put in `D0` to `D31` the value stored at that position.... Remember this is a simplified model... a lot more things goes on when accessing memory.

Then, the memory will put the value of address 4 in `D0-D7`. As each memory position is 8 bits, we only need 8 physical pins to send the value to the CPU. What happens is that is a huge waste. In general, the memory, will not put only 1 byte in the `Dx` lines, it will put as much as it can... in this example it means that it will output 4 bytes, starting from address 4, using the 32 `Dx` signals in the bus.

_Again, this is a simple example. Some times, your memory only has a data bus 8 bits long and multiple memory chips are used to access 16, 32 or 64 bits words... in those cases, there is a real physical constraint with regards to memory aligned accesses. This little things are the ones that makes a difference between two systems._


## CPU Native Word Size
As you can imagine by now, there is a relation between the processor and the rest of the computer with regards to this native word size. A 32bits processor will interface to a _32bits memory system_ (is not that straightforward, but roughly that is what happens and, when it says _system_ it may actually be saying several chips). 

Now, imagine that you want to just read a byte. You put your address in the `Ax` lines and ask the memory chip to spit the content of that address. The memory chip will put in the `Dx` the content of that address you request plus the 3 next addresses. The CPU will then read those 4 bytes from the bus and... just get the lower 8 bits from that value. In a sense, that requires something extra to do than just reading the whole 32bits value into our 32bits register. You see what I mean? (in this case it is just discarding 3 out of the 4 bytes)

This becomes a bit more tricky if instead of a 32 bits memory chip you use four 8 bits memory chips. For instance, check Intel processors datasheet. From the 80386 and on you will see that the lower bits of the data bus are nor directly mapped to these `AX` pins. You have to look for `BE#X` pins or `REQ#X` depending on the processor version.

As you can see, what I had described is just a very simplified example to illustrate how a processor works in a more efficient way when it only has to deal with its native internal word size. The reality is more complex and unless you need to design your own computer (the motherboard at least), it does not really matter (specially with the large cache memory nowadays). 


## Little/Big Endian
So, now, we'll always try to read data from memory using the native word size of our processor (not really but the compiler will do that for us). Let's assume the native word size is 4 bytes (32bits processor). The question now is: How are those bytes mapped between the memory and our registers?.

Let's assume the following memory layout:


    |  ...   | Drawer ...
    +--------+
    |  0x44  | p +  3
    +--------+
    |  0x33  | p + 2
    +--------+
    |  0x22  | p +  1
    +--------+
    |  0x11  | p
    +--------+

Let's assume too, that we want to read the content of address `p`, in register `eax`. Which value do you think you will get in the `eax` register?

`0x44332211` or `0x11223344`

The answer is: it depends. It depends on your processor. If your processor is `Little Endian` you will get the first value. Otherwise, if your processor if `Big Endian` you will get the second value. We have met the `Endianness`.

In general, you do not care about your processor Endianness. You just write and read your values to/from memory and the processor will do the right thing. Endianness becomes important when you have to interchange data with computers that may have a different endianness. This happens very often in network programming when using Open protocols that have to work with any kind of machine. 

Enough introduction. Let's see how all this concepts can be of any practical use.

# Pointers
LoL. You may be thinking: _OMG!, this guy is gonna kill me. Everybody says that pointers are the most tricky part of C programming and he is just starting with this. Really, man, I give up_.

OK guys. Do not give up. You will see, in a sec that this is a lot simpler that you think. Furthermore, if you are doing assembly programming... well, you cannot do much without using pointers.... Just repeat to yourself:  I have to go through this to finally understand those shellcodes everybody talks about... Repeated again... again... are you ready now?

So, what is a pointer again?. A pointer is just a position of memory that contains the address of another position of memory. :dizzy_face:

The first thing you may figure out from that cryptic recursive definition is that, a pointer have to have a size equal to the number of `Ax` pines in the processor. In other words, it has to have the size of the address bus of the processor.

In plain words. A 32bits processor with a 32bits address bus (i.e a intel 386) will require 32bits to store any potential memory address any program can ever reference. The same for a 64bits processor with a 64bits address bus, a pointer will need 8 bytes to reference any possible memory address. In this last case, for a 64bits processor, we need 8 bytes to cover possible value the processor can output in the address bus (the so-called addressing space) and therefore a pointer is stored in 8 consecutive address positions. (Again, this can become a bit more complicated in reality, but this concept is enough for now).

Let's see this with an example. Imagine the following memory layout:


    |  ...   | p + 4 = 0x400004
    +--------+
    |  0x40  | p + 3 = 0x400003
    +--------+
    |  0x00  | p + 2 = 0x400002
    +--------+
    |  0x00  | p + 1 = 0x400001
    +--------+
    |  0x04  | p     = 0x400000
    +--------+

The example above show a 32bits pointer at address (0x400000) pointing to address 0x400004 on a 32bits little endian machine. A memory address storing a memory address.

# Hello World
_""Fine, all that stuff is really confusing. Give me an example to understand what you are talking about...""_ Sure, there you go, the _Hello World_ program.

I'm pretty sure you know the ""Hello World"" program, but in case you don't, this is a very simple program that shows the message ""Hello World"" in the console. 

The way to do this on Linux is to write to the **standard output** (the console). The standard output is known by the system as the file descriptor `1` for any process... We will go in detail on file descriptors later in the course, for now, you just need to know that if you pass `1` as first parameter to the system call `write` you will be writing to the console.

So, knowing that the `write` system call is known by Linux as 1 (on a x86_64 arch), and applying everything we have already learn, this is how our little program will look like:

```
	global _start
_start: mov rax, 1    ; SYS_write = 1
	mov rdi, 1    ; fd = 1
	mov rsi, msg  ; buf = msg 
	mov rdx, 13   ; count = 13 (the number of bytes to write)
	syscall  ; (SYS_write = rax(1), fd = rdi (1), buf = rsi (msg), count = rdx (13))  

	;;  Exit program
	mov rax, 0x3c  ; SYS_exit = 0x3c
	mov rdi, 0     ; status = 0
	syscall ; (SYS_exit = rax (0x3c), status = rdi (0))
	
msg:	db 'Hello World!',0x0a
```

I hope you can identify the two system calls in there. The first one to write the message, and the second one to exit the program with status 0.  If you do not know how to compile the program, you need to go back and check Part I of this course.

# Labels and Assembler commands
There are two new elements in our tiny program. The first one is a label. A label is a name we can use to reference a part of our program (actually a memory position). In this case, the label `msg` is used to reference some data in memory, our ""Hello World"" message. In general, we do not know where in memory our program will be loaded, so using symbolic names let us write our programs without caring about that. Even if we use offsets to reference memory positions independently of our actual location in memory, labels will let us ask the compiler to calculate those offsets for us.

Actually, we've already seen this in the past.... can you spot the label we have been using so far? ... Anybody `_start`?

The second thing is that `db` instruction on the program. That is not a processor opcode, but an assembler instruction. Assembler instructions are only understood by the assembler, and does not directly translate into opcodes in the program. We already know one of those assembler instructions... Yes `global`.

The `db` assembler instruction probably stands for `Data Byte` (TBH I do not know for sure). It allows us to set some memory area with a sequence of bytes. In this case we can see two parts in the `db` instruction. The first part is a string. The assembler will output one byte per char starting at position `msg`. Then we can see an extra byte, separated by a comma, and expressed in hexadecimal. Sure, you can just put the decimal value (`10`) there and everything will stay the same. You can also write your string as a list of the ASCII values for each character separated by commas... but that is not very practical.

So, in this little program, where is our pointer?. We said that a pointer is a memory address that contains a memory address. In this case, the memory address is actually a register, specifically the register `rsi`. Do you remember that we said registers are just very fast memory within the processor that are referenced by a name?... well, if you are more comfortable changing the definition above to specifically also talk about registers that's fine. Anyway, I hope you have seen the point... er! ;)

# A C version
Let's now try to write the C version for this program. It would look like this:

```
#include <unistd.h>

int main ()
{
        register void *p = ""Hello World!\n"";
        write (1, p, 13);
        _exit (0);
}

```

Again, we can easily identify the two system calls in the program ( `write` and `_exit`). We already know that the second parameter to write has to be a pointer, a memory address containing the address, in memory, to the string to print. Let's take a look to the assembly generated by `gcc`:


    $ objdump -d -M intel hello
    (...)
    0000000000400544 <main>:
      400544:	55                   	push   rbp
      400545:	48 89 e5             	mov    rbp,rsp
      400548:	53                   	push   rbx
      400549:	48 83 ec 08          	sub    rsp,0x8
      40054d:	bb 5c 06 40 00       	mov    ebx,0x40065c
      400552:	ba 0d 00 00 00       	mov    edx,0xd
      400557:	48 89 de             	mov    rsi,rbx
      40055a:	bf 01 00 00 00       	mov    edi,0x1
      40055f:	e8 dc fe ff ff       	call   400440 <write@plt>
      400564:	bf 00 00 00 00       	mov    edi,0x0
      400569:	e8 c2 fe ff ff       	call   400430 <_exit@plt>
    (....)


Let's skip the first 4 instructions (that's the stack stuff that we haven't discussed yet), and let's try to find our pointer... Have you spot it?

Sure, you see how do we copy `rbx` into `rsi` after setting `ebx` (the 32bits part of `rbx`)  to `0x40065c`... and what is in there?... Let's check it

    $ gdb ./hello
    (gdb) x/s 0x40065c
    0x40065c:	 ""Hello World!\n""

_Note: You have to run all commands above. You may get different addresses in your system_

# C pointers
I guess you have already figure out how to declare a pointer in C. Sure, you have to use the `*`. However, in C we need to specify types. In this specific example it does not really makes a difference, but in the general case the pointer type is important and useful.

A C pointer is, therefore, declared this way:

```
type *pointer;
```

This declares a pointer to a memory address containing a value of a certain type. So... which types does C knows. This is the list:

    char     Byte                 Minimal addressable element (not necessarily 8 bits)
    int      Integer              Default integer type
    short    Integer              Usually half of the default integer or equivalent to int
    long     Integer              Usually double of the default integer or equivalent to int
    float    Floating Point       Single Precision Floating Point   
    double   Floating Point       Double Precision Floating Point
    void     Nothing              Nothing or Anything

C also supports compound types, but we will not talk about those right now.

Confused again?. This is a simple program to figure out the size of each type in your system and better understand the difference between all those types:

```
#include <stdio.h>
int main ()
{
	printf (""Size of void*  : %ld\n"", sizeof(void*));
	printf (""Size of short  : %ld\n"", sizeof(short));
	printf (""Size of int    : %ld\n"", sizeof(int));
	printf (""Size of long   : %ld\n"", sizeof(long));
	printf (""Size of float  : %ld\n"", sizeof(float));
	printf (""Size of double : %ld\n"", sizeof(double));
	return 0;
}
```

The `stdio.h` at the beginning is required to use the function `printf`. The function `printf` (PRINT Formatted) lets us print messages using format strings to compose complex outputs. In this case, we are using the `%ld` format string to print the long value returned by `sizeof`. This basically tells `printf`, I have a number here that I want you to convert into a string... please do it.

We can add many of those `%` in the format string and provide additional parameters to the function to fill them. Check the `printf` man page for details about the format strings you can use with `printf`.

Finally, as you can imagine, `sizeof` returns the size, in bytes, of a given type or variable. 

In our test program we used a `void*` variable. This is a pointer to `void` what, for a pointer, means a raw pointer or a pointer to anything. This is actually the C equivalent to the assembly pointer we used in our ASM code.

We will come back to the C pointers later to fully understand the implications of pointer's type. But I think this is enough for now

# Your First Shellcode
So, believe it or no, you have already learn all the bits and pieces to write a very basic shellcode.  A shellcode, in its simplest form, is a piece of code that starts a shell. It is usually feed into a vulnerable program using a exploit, effectively enabling the attacker to acquire a shell with the same privileges of the vulnerable program. In general an attacker will be targeting processes running as `root` to get full access to the machine.

In Linux, you can execute a process using the `exec` system call. This system call has 3 parameters, but for your first shellcode you can set to 0 the last two. The only parameter we need is the first one... a pointer to the name of the program to run.... that in this case would be `/bin/sh'.

```
section .text
        global _start

_start:
        mov rax, 0x3b           ; SYS_exec
        mov rdx, 0              ; No Env
        mov rsi, 0              ; No argv
        mov rdi, cmd            ; char *cmd
        syscall

cmd:    db '/bin/sh',0

```

Wow!... it is roughly the same program that the `Hello World` we wrote before!!!!. Are you missing the `exit` system call?... take a look to the exec man page (`man 2 exec`) to know why we do not need it any more.

## `/bin/sh`
You may be wondering: why `/bin/sh`?. I always use `bash`, or `dash`, or `zsh`, or `ksh`,...  Sure, you can run many different shells (command interpreters) but in almost any Unix out there, independently of the actual shell you usually use, you always will have `/bin/sh`. In general, it is a soft link to a real shell.

The reason for this, at least one of them, is that the system runs a lot of shell scripts for doing different things. You have shell scripts executed during the boot process, whenever you start or stop a service, when you launch some applications,... Imagine that whenever a user wants to change its default shell, the system will have to update all those scripts... what about the ones you wrote on your own, those the system knows nothing about... they will just break.

Therefore, as a convention, all Unix system have a binary at `/bin/sh` that runs a shell and all shell scripts rely on the existence of that file... Well... not all Unix system. Keep reading.


# ARM Shell code
So, we should be able to port our x86_64 asm shell code to ARM very easily. In case you are feeling lazy, this is how it may look like.

```
.text
.globl _start

_start:	mov r7, #11
	mov r1, #0
	mov r2, #0
	ldr r0,=msg
	swi #0


.data
msg:
	.asciz ""/system/bin/sh""
```

have you seen it?... sure, this code is for Android. Android had mesh up the standard Linux disk tree, and the default shell is no longer at `/bin/sh` but at `/system/bin/sh`. If you are going to test the code in another ARM platform as a BeagleBone Black, a BananaPi or an Olinuxino running a standard Linux distro (usually Debian), just change the string to the well-known location `/bin/sh`. The rest of the code should just work.

We can compile it like this:

    $ arm-linux-gnueabi-as -o sh-arm.o sh-arm.s
    $ arm-linux-gnueabi-ld -o sh-arm sh-arm.o

Let's take a closer look to the code. Did you notice it?. Yes, there are some differences when compared to our Intel code. This is for two reasons. The first one is that `NASM` only produces code for intel processors, so we cannot use it for ARM. You should had noted this before... I'm amaze nobody had asked about this from the previous parts. Anyways, the syntax of the GNU assembler (`as`) is slightly different. This one is known as AT&T assembly whereas the one used by `NASM` is known as Intel assembly... As a wannabe hacker you should learn both :P

So, the GNU assembler uses the assembler instruction `.asciz` to add a zero-terminated ASCII string to the memory. It is the same thing that the `nasm` `db`, but automatically adding the 0 at the end. The second comment is that we have to use `ldr` to load our pointer in our `r0` register.

Well, `ldr` is an ARM __pseudo-instruction__. The bottom line is that you cannot directly load 32bits values into a register in an ARM processor. I will not go into the details (you can google it), but roughly, ARM produces a very compact machine code, and tries to encode a lot of information on each 32bits machine code word, including the mnemonic parameters. This limits the size of the values that can be directly loaded into a register. The `ldr` pseudo instruction is expanded by the assembler in the right sequence of instruction to load a 32bits value in a register. There are more pseudo-instructions for ARM and we will go through them as needed.

The conclusion of all this is that, for ARM you have to use the syntax above to load a 32 bits constant or address (which in practical terms are the same thing) into a register.

# MIPS
OK guys. My MIPS setup is so crappy that it is a pain to keep including it in this course. If any of you wants to contribute this section, just let me know. Until I get this development environment sorted out I will skip the MIPS sections from now on.


# Conclusions
In this part we have had our first encounter with pointers at the lowest level and we have learn how to use them together with a system call. Using these two simple concepts we manage to create a shell code. This shell code is not usable in the wild, but you will learn how to update it for practical purposes later in this course. If you cannot wait, check the @unh0lys0da   article here https://0x00sec.org/t/linux-shellcoding-part-1-0/289 or the read classical ""Smashing the Stack for Fun and Profit"" from Aleph one!
