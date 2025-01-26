# Programming for Wannabes. Part I. Your first Program

If you are reading this, it is because you want to be a hacker. Therefore, you are automatically a [wannabe](http://www.catb.org/jargon/html/W/wannabee.html). It does not sound that cool, but it is actually pretty cool. It means that you still have a lot of exciting things to discover!!!

First things first. This is going to be a joint course on C and assembly programming. Those are the only two languages a real hacker has to know. Yes. Really. The only two languages that will let you do the real hacking. You can do some stuff with Python or PowerShell... but that is Skid stuff. You know what the S in skid stands for, don't you? It stands for Script :stuck_out_tongue_winking_eye:

Easy, this was just a joke to catch your attention. Now seriously, learning scripting languages is a powerful tool and will save you a lot of time in many cases. Mastering Python is indeed a great skill, and it is something you should invest time in. Analogously, C and assembly are the only way forward for some topics: reverse engineering, kernel level rootkits, and some system level programming can only be done with those low-level languages. So, if you really want to be a hacker... man, there is no way around, you've got to learn CASM (C + ASM :).

Two notes before we start:

- I'm not a hacker myself. Not even a wannabe. Really, I do not have any interest in becoming a hacker. This basically means that you should take this course as a starting point. When you get done with it... then... your journey will really start.
- No. HTML is not a programming language. If you think that... you really need to go through this course. :stuck_out_tongue_closed_eyes:

The course will be organized as follows:

- Some introduction to what a computer is. These are low-level languages, and you need to get to know low-level stuff
- That is it for now :P

# How have you ended up here?
That is a good question. In all honesty, there are zillions of C/ASM programming tutorials and courses out there. I know, they look useless. They are not what you are looking for. That's fine... and that is why you are reading this right now.

To understand why all those tutorials haven't helped you, we need to learn a little bit about the different layers beneath the tools we want to build... Yes, layers. There are lots of them.

So, let's start this trip bottom-up. At the very low level, you have what is usually known as the _Bare Metal_. That is roughly the computer processor and the memory. We may consider the BIOS to be also at this level and some other elements usually grouped under the generic name of chipset.

On top of the _Bare Metal_, we find the _Operating System_. The operating system is in charge of managing the HW. There is a lot to do there, and each hardware device has its own peculiarities (that's why we have all those drivers). The operating system gives us standard services to access the disk, the memory, the USB devices... You can indeed do all this at the _Bare Metal_ layer but... then you will end up writing an operating system...

The next layer we find is the _System Libraries_ layer. These are normal fragments of code that are used for many different programs. We group those pieces of code in a file and we call it a library. Then we can use that code from our programs. There are, literally, thousands of libraries available for any OS out there.

The top level of this stack of layers is the _Programs_ (this also includes Application Level Libraries). These are the things you want to write yourself when you decide to learn how to program a computer. You write these programs in some programming language. C and ASM are two of those programming languages... There are hundreds of them, and you should learn quite a few and choose the right one for the task you want to accomplish. 

# Going Top Down
OK, fair enough. Now that we are at the top level, let's look down for a while. You have learned a specific language.

Let's say you have learned C using one of those online tutorials. The original C language has 32 keywords. The syntax is pretty simple and easy to learn. Yes, really, it is a pretty simple language. However, knowing the C keywords and syntax does not let you do much.

You need to learn something more. At least, one thing called the _Standard C library_. Without this, you cannot even print anything on the screen, read any user input, or deal with files. And this happens with nearly all programming languages out there. You need a minimal library to be able to interact with your operating system and then let your operating system deal with the _Bare Metal_.

Therefore, learning the syntax of a language and the basics of its standard libraries only brings you halfway (in the best case) to where you were aiming to be.

Learning C and ASM to become a hacker requires you to learn not just the language keywords, syntax, idioms, (put whatever nowadays buzzword in here)... You also need to learn how to use the system libraries, how your operating system works, and also how your computer works.

That is why there is a huge difference between a C programming course that teaches you how to code C and a hacking programming course that teaches you how to do stuff. Usually, after you have learnt the syntax, you have to go into specific courses tutorials: Network Programming, Kernel Programming, 3D graphics programming, GUI programming,... Each one of those has their own peculiarities and requires quite some time to manage them. In a sense we will be covering here what is generally known as _System Programming_. This cover quite some of the topics usually related to hacking.

Hope that at this point you have understood, that hacking/system programming is not about learning a programming language. That is the simplest part. You have to learn a lot of other things... summing up. You have to learn what is going on every time you type a line of code.

So, it is time to learn the basics about a computer. C and ASM are low level programming languages. We will see soon how true this is. This means, that, to really master those programming languages you need to know, at least, the basics of how does a computer works.

# The Simplest Computer in The World
Despite of how people things technology has advance in the last decades, the reality is that computers haven't change much in nearly 50 years. They are faster, they are smaller, they consume less power... but they still work the same way. Indeed, this is a simplification, but at the level at which we are going to work this is actually the case.

For illustration purposes, let's introduce the SCTW-2000. This is a fictional computer that will let us introduce different concepts in a generic way. It could easily be one of the popular home microcomputers from the 80s. We will match the SCTW-2000 with real examples, but we will avoid using real computers, at least at the beginning and for the sake of simplicity.

The SCTW-2000, as many other computers out there is basically composed of two main parts: CPU and memory

Let's dive into each of these elements to get the big picture.

# The Memory
There are a lot of different types of memories: RAM, ROM, SRAM, DRAM, SDRAM, PROM, FLASH, Serial FLASH, ... From a SW point of view, unless we go really low level (something we are not going to do in this course), we do not care about the type of memory our computer have. That is important for the HW guys but not for us. At least not now. What is important for us is how does this element fit within the overall computer architecture.

For the time being, let's imagine the computer's memory as a huge bunch of drawers, one on top of the other. Inside each drawer we can store one number. An 8 bits number, or in other words, a number between 0 and 255 (that is 2^8 values... 8 bits).  Finally, let's number the drawers, starting for the one on the bottom (number 0), and giving consecutive numbers to the drawers on top. Something like this
  
    +--------+
    + 8 bits | Drawer 3
    +--------+
    | 8 bits | Drawer 2
    +--------+
    | 8 bits | Drawer 1
    +--------+
    | 8 bits | Drawer 0
    +--------+

The higher the memory in our computer, the taller the drawer pile will be. For instance, if our computer have 64KB of RAM... yes it sounds ridiculous nowadays, it is just an example. So a computer with 64 KB of memory will have 65536 drawers one on top of the other.

Each drawer is known as a memory address, and the number inside the drawer is the content of that memory address.

This is it about the memory for now. Let's look to the CPU

# The CPU
The CPU is the other main component of the computer. It is composed of the following parts:

* A set of registers
* A Arithmetic-Logic Unit (ALU)
* Some control logic.


## Registers
Let's start with the registers. You can see the CPU registers as a small piece of memory that is inside the CPU and therefore it is super fast, and can hold numbers bigger than the 8 bits stored in each of our fictional memory addresses. On the other hand, this is a very small memory, something in the range of 4 to 32 positions (drawers). Compare to the 65536 drawers for our ridiculous small 64KB memory.... Each of this positions are known as a register. Registers can be numbered starting from 0 like the memory or they can be named.

For instance the Z-80 processor, named their registers as: `AF, BC, DE, HL, IX, IY`. The Intel processors traditionally named their registers as: `AX, BX, CX, DX, SI, DI`,... Motorola processor used more systematic naming schema for their registers: `A0-A7` (_address registers_) `D0-D7` (_data registers_) for its 68K. 

As the number of available registers grows, it is usual to follow the Motorola schema and name the registers with a consecutive number. This way, X86-64 (64 bits intel/amd processors) name their registers (the general purpose ones) `r0-r15` (lower register have names compatible with the i386 processor). Same with MIPS and ARM processor.

Any way. Most of the processors used to build computers have a set of registers. A very fast and very small memory area inside the processor itself.

## The ALU
The second element of the processor is the Arithmetic-Logic Unit. And yes, it basically performs arithmetic and logic operations. You can add, substract, multiply, and, or, xor values using this processor element... and that is what a computer actually does most of the time.

The ALU performs this operations either using the values stored in the registers or values taken from the memory, and stores the results, again, either in a register or a memory address. The available options actually depends on the processor. Old processor only could perform operation on registers and some only on certain registers. Nowadays, this is no longer the case.

This is really it for the ALU

## The Control Unit
The last part of the processor we are interested on is the Cotrol Unit. In all honesty, in this simplified computer model, the control unit is anything else inside the processor except the registers and the ALU (branch prediction unit, cache management, bus signaling, pipeline management, memory management unit...). We won't go in details about most of those circuitries in this course, however, there are a couple of basic functions that we have to be aware of.

One of these things  is controlling the CPU external interface. In other words, the state of all those little pins that go out of the CPU chip. Using these pins, the processor can talk to the memory to read and write values in those drawers and can also interface to different types of hardware...

So, when the ALU needs some data from the memory to perform some operation, it asks the control unit to activate the right pins on the processor to command the memory chip to read or write a given memory position. The memory chip will get the value to write from some CPU pines or put the value to read in some other pins (or multiplex these two pin sets, that is using the same pins for reading/writing and even addressing the memory). OK, it does not really work like this, but otherwise we will have a very, very long introductory chapter ;)

The control unit is also in charge of reading the program from memory, parsing it and executing the machine code....

# Machine Code
We will not talk much about machine code... we will be talking about ASM, but, you at least need to know what this machine code is. For that, we need to understand how the CPU runs a program and how does a program looks like.

The first thing we have to introduce is the __Program Counter__ (`PC`) or __Instruction Pointer__ (`IP`). The name depends on the processor family you are working with. This is a special register in the CPU that indicates which memory address in the main memory contains the next instruction to run. Whenever the processor executes an instruction, the `IP` is increased by one (actually it may be more than one, if the current instruction takes several bytes... more on this in a sec), unless the instruction is a branch/jump. In that case, a new value is assigned to the `IP` so the next instruction will be taken from a new position.

But, you may be wondering... what are those instructions?. OK, no panic. Let's go back for a second to our fictional SCTW-2000. Now that we know a lot about processors, I can tell you about the specs for the awesome SCTW-2000:

- It has 2 registers named `EBX` and `EBP` (you will see later why we have chosen these names), plus an instruction pointer named `RIP`
- The ALU can perform the following operation: `ADD` two registers
- It has an instruction `MOV` to assign values to registers.
- After reset (or on power on) all registers are set to 0 (including `RIP`)

Now, to fully specify the SCTW-2000 we have to define the instruction it can run. The numbers the control unit will be able to understand... that is _the machine code_. For the SCTW-2000 it is something like this:

    OPCODE     | MNEMONIC    | Description
    -----------+-------------+-----------------------------------------------
    0xbb XX    | MOV EBX, XX | Copies the value XX in register R0
    0xbd XX    | MOV EBP, XX | Copies the value XX in register R0
    0x01 XX YY | ADD XX, YY  | Adds the values of register XX and register YY
               |             | ands stores the result on register XX
    0x90       | NOP         | No Operation. Does nothing
    0x00       | HALT        | Stops the processor

As you can see we have used three columns for this table. The first column contains only numbers (1, 2 or 3 numbers depending on the instruction). Yes, instruction may take, and actually take more than one single memory address. This is the __machine code__, the sequence of numbers stored in memory that the CPU can understand.  We have chose some arbitrary opcodes (not so arbitrary but for the time being, it does not really matter which OPCODES we've chosen).

Remembering those number is hard for humans. Sure, we can manage for the SCTW-2000, but a real processor may easily have hundreds of those OPCODEs. For that reason, we use something a bit easier to remember for us (that's why this is named [mnemonics](https://en.wikipedia.org/wiki/Mnemonic_(disambiguation)) - from the greek "_memory related_", yes a bit of a free translation)... That is actually the assembly language that you are trying to learn. The ASM language has to be converted into machine code... that is the task of the program know as __assembler__.

# Our first program
To illustrate all this, let's write a simple program that adds two numbers: `10` and `20`. The assembly code for the SCTW-2000 and for such a stupid program will be:

```nasm
MOV EBX, 10
MOV EBP, 20
ADD EBX, EBP
HALT
```

Now let's do the work of the assembler by hand using the opcode/mnemonic table above:

```nasm
ASM             MACHINE CODE
MOV EBX, 10  -> 0xbb 0x0a
MOV EBP, 20  -> 0xbd 0x14
ADD EBX, EBP -> 0x01 0x00 0x01
HALT         -> 0xff
```
Now we can put the program in our memory:

    ADDR-07 | 0xff |
    ADDR-06 | 0x01 |
    ADDR-05 | 0x00 |
    ADDR-04 | 0x01 |
    ADDR-03 | 0x14 | 
    ADDR-02 | 0xbd |
    ADDR-01 | 0x0a |
    ADDR-00 | 0xbb | <= RIP

Our program requires 8 bytes of memory. We copy the program starting at position 0, and after resetting the SCTW-2000 (sure, we are using SDRAM for the SCTW-2000), the `RIP` will point to address 0 and will start reading the machine code and executing the instructions. 

> Not all processor starts execution at address 0. You have to check the datasheet for the processor to figure out the boot process and conditions

# C as a Low-Level Programing Language
Before finishing this first part, we have to quickly introduce the other language we are going to work with in this course and we also have to justify why we shall consider C a low level programming language.

For doing that, we are going to try to produce exactly the same code we have generated by hand in the previous section, using a C program. Instead of the SCTW-2000, we are going to use an Intel 64bits... You will see how similar these two processors are :)

Let's write this program in a text file. Call it `ph1.c`

```C
int f1 (void)
{
  register  int a = 10;
  register  int b = 20;

  a += b;
  return a;
}
int main (void)
{
  int a;
  a = f1();
}
```

The program looks a bit weird. The reason is that we are not working at the bare metal level. We have an operating system and a file format to honour, so the compiler has to create quite some more stuff than just the machine code for our code. Moreover, we have to use everything we put in the code, otherwise, the compiler will realise that, and remove the code detected as dead (the one that will never be used)... which is bad for our didactic purposes.

We will progressively go into all those details. Right now, it is not really needed to completely understand what is going on. You just need to realize a couple of things.

First, concentrate on the function `f1`, at the beginning of the program. No need to fully understand the syntax, just pay attention to the following:

* The `register` keyword in C, tell the compiler that we want to use a register. By default it will try to use memory for variables (actually the stack, but we haven't talked about it yet), but, as you can see we can force the compiler to produce machine code that uses the registers... as we did with our assembler version.
* Then you can see the equivalent to our assembler program. We store 10 in one variable (that will be hold in a register), 20 in another, and then we add both number and store it in the first variable. This is roughly the ASM code we wrote before!!!!

Let's compile the program and take a look to the machine code:

`$ gcc -fomit-frame-pointer -o ph1 ph1.c`

The `-fomit-frame-pointer` is just to remove some code generated by the compiler that is not relevant right now. And what we've got out of this compilation is:

    $ objdump -M intel -d ph1 | grep -A 20 '<f1>'
    00000000004004b4 <f1>:
      4004b4:	55                   	push   rbp
      4004b5:	53                   	push   rbx
      4004b6:	bb 0a 00 00 00       	mov    ebx,0xa
      4004bb:	bd 14 00 00 00       	mov    ebp,0x14
      4004c0:	01 eb                	add    ebx,ebp
      4004c2:	89 d8                	mov    eax,ebx
      4004c4:	5b                   	pop    rbx
      4004c5:	5d                   	pop    rbp
      4004c6:	c3                   	ret

I had kindly asked  `objdump` to produce intel assembly because it's closer to the one we used for our awesome SCTW-2000 (that is the `-M intel` flag).  

Let's look at `0x4004b6` to `0x4004c0`:

      4004b6:	bb 0a 00 00 00       	mov    ebx,0xa
      4004bb:	bd 14 00 00 00       	mov    ebp,0x14
      4004c0:	01 eb                	add    ebx,ebp

So, this looks pretty similar to the machine code, we generated for our fictional SCTW-2000 processor. The main differences:

- gcc is generating 32 bits values for our constants. Even if we declare our variables as `char`, a 32 bits value will be generated... Later in the series we will learn why.
- The `add` instruction is a bit different. Intel machine code encodes the registers in the opcode to save space, that is why the machine code for the `add ebx,ebp` is just `0x01 0xeb`, instead of our `0x01, 0x00, 0x01`. Check the intel manual for full details

Well, we have managed to write some C code that almost exactly matches the machine code we want the computer to run... this is why C is considered a low level programming language :).

# For the Lulz
Just for fun. Let's see how the ASM for ARM will look like.

First let's compile the program for ARM:

`$ arm-linux-gnueabi-gcc -fomit-frame-pointer -o ph1-arm ph1.c`

And now, let's look at the code:


    $ arm-linux-gnueabi-objdump -M intel -d ph1-arm | grep -A 20 '<f1>'
    Unrecognised disassembler option: intel
    0000840c <f1>:
        840c:	e92d0030 	push	{r4, r5}
        8410:	e3a0400a 	mov	r4, #10
        8414:	e3a05014 	mov	r5, #20
        8418:	e0844005 	add	r4, r4, r5
        841c:	e1a03004 	mov	r3, r4
        8420:	e1a00003 	mov	r0, r3
        8424:	e8bd0030 	pop	{r4, r5}
        8428:	e12fff1e 	bx	lr

OK, the OP codes are completely different, but the assembly is pretty accurate. You can see that, for ARM, the `add` instruction accepts 3 parameters instead of just 2. Other than that... it is pretty similar to our SCTW-2000 ASM and also to the Intel 64bits ASM.

And for MIPS we get:

    $ mips-linux-gcc -fomit-frame-pointer -o ph1-mips ph1-1.c
    $ mips-linux-objdump -d ph1-mips | grep -A 20 '<f1>'
    00400720 <f1>:
      400720:	2402000a 	li	v0,10
      400724:	24030014 	li	v1,20
      400728:	00431021 	addu	v0,v0,v1
      40072c:	03e00008 	jr	ra
      400730:	00000000 	nop

This is a bit different but still... change `li` to `mov` and `addu` to add... and there you go.


# Conclusions
Well, this concludes the first part of this course. We have quickly seen the main components of a computer, and the relation between the machine code, the assembler and the C programming language. We have also seen that knowing the language syntax is just a small part of what you need to know to actually write useful code. Finally, we have checked why C is considered a low level programming language and we've also got a glimpse of how the language can be used to control what is actually executed by the processor.

Please, let me know in the comments if you have found this useful, if it is comprehensive, if you are missing something, if you got bored to dead... and click the heart icon if you are interested in more installments for this series (I know some people read the posts but never comment on them)... I won't bother you guys in case you are not interested on this kind of stuff.

# Appendix. Installing required tools
In case you need help to get the tools we used in this tutorial, here are some pointers :slight_smile:

For intel you can just install  build-essential

    apt-get install build-essential

This should install all the tools you need, including the compiler. For non debian based distros you need to look for the packages for `gcc` and the so-called `binutils`.

Forar ARM things are also easy

     apt-get install gcc-arm-linux-gnueabihf
     apt-get install gcc-arm-linux-gnueabi

The first one is for Hardware Floating-Point and should work for any recent ARM. Just go for that. You may also need to install the binutils package for ARM

    apt-get install binutils-arm-linux-gnueabi

For MIPS you won't find a toolchain in your distro repository, so you have to get one from somewhere else or compile your own. Check this page to chose one. I cannot remember which one I used for this post. 

https://www.linux-mips.org/wiki/Toolchains

Just uncompress the toolchain anywhere and set your `PATH` to the directory containing all the binaries... That's it


* NEXT: [Programming for Wannabes. Part II. Systemcalls](part-02.md)
