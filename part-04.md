# Programming for Wannabes. Part IV. The Stack

Some people have asked me to continue this series... and actually this paper was half written almost two years ago.... so I decided to _finish him_... you know, the _Mortal Kombat_ way... Not sure if I will write Part V... but Part IV is here :slight_smile:

--- 

Let's continue our journey towards the ultimate mastering of the computer which will lead us to full enlightenment. So far, we have learnt a few things about computers and, in the meantime, we have already create our first shellcode. However, we have just scratched the surface of this whole field... you still have things to learn.

In this part we are going to dive into how programs are built and get in contact with the insights of some exploitation techniques.

# Stacks
In order to go further in our study we have to introduce a new computer element. **The Stack**. The Stack is just a very simple data structure. It is, maybe, the one and only data structure that is available when programming in assembly. Do to panic, you will see in a second that it is damn simple.

I'm pretty sure you already know this, but just in case, let's add a simple explanation here. You can think about a __stack__ as a physical __stack__. Think for instance about the stack you build when you wash your dishes. You wash one dish and you put it aside. Then you wash another one, and you put it on top of the previous,... and so on. At the end you have a stack of dishes. 

Now, let's imagine that is lunch time and you have to take a few dishes to serve your food. What do you do?. Yes, you take first the dish on the top of the stack. The last one you put in the stack. You can try to get the one in the bottom, but that is pretty difficult.

So, a **stack** is a so-called LIFO structure. LIFO stands for _Last In, First out_. The last dish you put in the stack is the first one you are gonna take whenever you are hungry. There is other major structure called FIFO. I bet you can figure out by know how does it work.

# The STACK
At processor level, a stack is basically a pointer to a memory block. Usually, the processor provides a dedicated register to hold that pointer and, guess what?, it is usually named _Stack Pointer_. This will be engraved in your soul by the end of this instalment. This is one of those special register we had mentioned in previous parts of this course. Now, we know the most important two (among those special registers): The _Instruction Pointer_ and the _Stack Pointer_. They are special because some processor instructions only work on the values of those registers.

For the specific case of the _Stack Pointer_, there are two basic operations you can perform with a stack:

* `PUSH`. This operation pushes/stores a value in the stack. 
* `POP`. This operation pops/retrieves a value from the stack

In general, this is all you need to know at assembly level. However, let's go a bit deeper in the implementation, just for the sake of our personal enlightenment :). After all this is what we are trying to achieve... Do you know why hackers uses a hood? don't you?..It is because they had reached an enlightenment level that transcend their physical body and their head actually emits light... that's why  they cover their head with a hood... it is not about looking cool... is about not looking weird.

Anyways, the first thing we have to know about a _stack_ implementation is that it grows. This is not strictly correct, but it is the best way to see it now. What this means is that we can implement the stack in two different ways:

* We can make the stack grow up, towards the high memory addresses
* We can make the stack grow down, towards the low memory addresses

Most processors nowadays makes the stack grow down, or allow us to chose the model we prefer. The only architecture I know it grows the stack up is PA-RISC. What does this means, well, this just means that, whenever you `push` a value in the stack, the _Stack Pointer_ is decremented and, whenever you `pop` a value from the stack, the _Stack Pointer_ gets incremented.

The second thing we have to know about a _stack_ implementation is where the _Stack Pointer_ points. There are two ways of doing this. You (actually the processor designer) can decide that the _Stack Pointer_ points to the last used position in the stack, or you can decide that the _Stack Pointer_ points to the last free position in the stack. For Intel processors the _Stack Pointer_ always points to the last used stack position.

With all this information we can now figure out how to implement the `PUSH` and `POP` instructions

                 SP First Free               SP Last Used
    ===========================================================
    Grow Down     PUSH V                      PUSH B
                  --------------------------------------------
                  MOV [SP], V                SUB SP, WS
                  SUB SP, WS                 MOV [SP], V
    
                  POP V                      POP V
                  ----------------------------------------------
                  ADD SP, WS                 MOV V, [SP]
                  MOV V, [SP]                ADD SP, WS
    ============================================================
    Grow Up       PUSH V                     PUSH B
                  ----------------------------------------------
                  MOV [SP], V                ADD SP, WS
                  ADD SP, WS                 MOV [SP], V
    
                  POP V                      POP V
                  ----------------------------------------------
                  SUB SP, WS                 MOV V, [SP]
                  MOV V, [SP]                SUB SP, WS


In the table above, `V` represents the value we want to store in the stack, and `WS` is the default processor word size in bytes (check the https://0x00sec.org/t/programming-for-wannabees-part-iii-your-first-shell-code/1279/1 for details). `SP` is the stack pointer (usually a special register, and the `[]` operator means that we want to access the contents of the address pointed by whatever we put inside the `[]`. This is called indirect addressing, but... we haven't talked about addressing so far, so, just remember it allows to access the content of a memory position.

You may be wondering... why should I care about the table above?. I will just do `PUSH` and `POP` and the processor will take care of doing the right things with the stack pointer. That is true. However, sometimes you will need to manipulate the registers yourself or, while reversing code, you will find manipulations of the _Stack Pointer_ like the ones shown above (compilers do this things)... so it is convenient to understand how does this works.

> Modern processors uses microcode to implement they machine code. This is, each opcode in the processor is a small program of lower level instructions that deals directly with the datapaths on the processor. `PUSH` and `POP` in real-life microcode won't look like our example above, but could be pretty similar. VAX machines were one of the first one to use microcode to implement its instruction set.


# What is the Stack used for?

The generic response is: _to store temporal values_. This is a pretty generic answer. Some more specific examples are:

* Implement nested functions storing the return addresses in the stack
* Store local variables (actually local scoped variables)
* Store function parameters for certain ABIs
* Enable recursion

Actually, all the examples given above, can be condensed in a single concept. The so called `stack frame` of a function.... But to go deeper in that we first need to know what a function is.

# Functions

In the computers realm, functions mimics the mathematical concepts. In that sense, a function is something that receives some parameters and return some value/values. This is the case for most high level languages like C or Python. However, at assembly level, this concept does not really exist. The high level languages just make use of the processor instructions to provide that abstraction.... After all this is what computer sciences are all about... to create the illusion of something happening that is not really happening. It is about fooling people :).

You can achieve this in many different ways, as we will see in a while. However, each programming language and also the operating system defines some default rules so all components in the system can talk to each other. We have already mentioned this a few times, it is the so-called __ABI__ (Application Binary Interface). As we progress in this section this concept will make more sense... I hope.

For us, a function is a piece of code, stored somewhere in memory that we can execute at any time, passing some data in, and optionally, receiving some data back. Let's see how we can do this.

## Assembly support

So, how does the processor supports the implementation of functions. Well, the first thing we have to do is to be able to change the flow of our program. So far, all the programs we have seen work sequentially. We start at a given instruction, and the program reads those instruction one after the other.

In general, we will want to call our functions many times. Actually that is one of the fundamental reasons to define functions, to avoid copying the same code whenever it is needed, making our programs grow in size, and making it really difficult to maintain. So, what we need is a way to change the `Instruction Pointer`. We could use our well known `MOV` instruction to update the `RIP` register (the instruction pointer) and force the processor to continue the execution in a new memory address. However it does not work like that (well it does sometimes)... To be honest I have never tried that outside of a debugger, but, anyway, there are very good reasons to use the processor mnemonics to chance the Instruction Pointer, instead of doing it directly by ourselves.

It is not the right time to go into these details, but for the more advanced and curious reader you can search for concepts as processor pipelines, branch prediction units and processor cache to figure out why it is better to use the proper instructions when changing program flow.

So, the machine instructions traditionally used to _call a function_ in assembler for the Intel platform are `CALL` and `RET`. Let's see what `CALL` does.

First thing `CALL` does is to push into the stack the address of the next instruction. The so-called _Return Address_ (remember this name). Then it changes the instruction pointer to the address passed as parameter.

After invoking a `CALL` instruction, our program will continue in the address passed as parameter and store in the stack the address we have to return to in order to continue the execution whenever we are done with our function. So basically a `CALL` instruction does something like:

```
CALL ADDR  -> PUSH RIP + 1
              JMP  ADDR
```

The `RET` instructions does the opposite. it recovers a value from the stack and sets `RIP` to that value. Unless we had messed up with the stack in our function (let's talk about smashing the stack in a while...), the address in the stack should be the instruction just after the `CALL`. 

```
RET      -> POP RIP
```

Let's rewrite the function we used in the first part of this course with information we have now:

```
f1: mov eax, 0xa
    mov edx, 0x14
    add eax, edx
    ret

main:(... Program starts here....) 
     call f1
      (... program continues here ...)
```

So far so good. Now we know how the processor allows us to run a piece of code located somewhere in the memory and, when done, return back just to the point where we were before calling the function. 

## ABI. Parameters and return values
The function we have just written, does not receive any parameter and does not return anything. Well it actually returns a value under some platforms but we will see why in a second.

So, how can we pass parameters and receive returns values from an assembly function?. The answer is... it is up to us. At assembly level we can do whatever we want. However there are a few reasonable options to deal with this in a systematic and ordered way. Let's check them out:

* We can push our parameters in the stack and receive the result in a register. This is the way used by `x86` (Intel 32 bits) applications...
* We can store our parameters in the registers and receive the result also in registers. This is roughly how a `x86-64` application works. For 64bits processors we have a lot of registers so it is faster to just use the registers to pass the parameters than using the stack (after all the stack is memory and is slower than the registers). However, for functions that receives a lot of parameters, the stack is used when no more registers are available.... Now you know why it is better to define functions with a reasonable number of parameters.
* We can use some registers to pass parameters in and some registers to pass parameters out. This is what ARM and MIPS does... again, when there are enough registers available to hold the parameters and return values.

> When our function can return more than one value we would use output parameters passed as reference so they can be modified by the function. In this case return values are manages exactly the same as parameters are, in a sense they are also parameters. You can figure out what would happen if you try to just store return values in the stack within the function.... It won't work straight away. But do that when you have finished reading this instalment.

In principle, the `ABI` depends on the operating system, however, in practice, it is mostly driven by the processor. This way, Linux follows the so-called System V ABI, and Windows have its own, but both uses the stack for 32 bits and registers for 64bits, even when Windows defines more cases and different registers may be used for the parameters.

Now, let's modify our test function so it will get the two numbers to be added as parameters. We will chose to pass parameters in registers and return the result also in registers. As we said above, this is the Linux 64bits ABI. Specifically for this case:

```
Param 1  | Param 2 | Param 3 | Param 4 | Return
   RDI   |    RSI  |    RDX  |    RCX  |   RAX
```

So, the code will now look like this:

```asm
f1: mov rax, rdi	; Copy Param 1 in RAX
    add rax, rsi	; Add Param1+Param2 in RAX
    ret	

main:(... Program starts here....) 
     mov rdi, 0x0a	 ; Param 1
     mov rsi, 0x14	 ; Param 2
     call f1
			; RAX contains Param1 + Param2
      (... program continues here ...)

```

That was easy right?. 

Actually, this function is compatible with the System V ABI followed by GNU/Linux... or in other words, we can call this function from a C program and it will work as expected:

This is the assembly function in file `ex.S`... let's use AT&T asm so we can just use the file directly with `gcc` for compiling.

```Assembly
	.global f1
f1:	mov %rdi, %rax
	add %rsi,%rax
	ret
```

This is the C main program in file `ex.c`:

```
#include <stdio.h>

int f1 (int a, int b); // Prototype

int main (void) {
  printf (""Result: %d\n"", f1 (10, 20));
}
```

Yes... we are polite and we include the prototype for our asm function.

Now we can compile:

    $ gcc -o ex ex.c ex.S
	$ ./ex
	Result: 30


I would say that if you do not find this to be really cool... then you are reading the wrong tutorial :)

Note that if we decide to use a different solution to deal with our parameters (using the stack or a different set of registers) this won't work... because we will not be using the GNU/Linux System V ABI and our function will not be compatible with the system it is intended to be used with... However, from another asm function we can call other functions in any way we want... it is just that it will not play nicely with tools like C compilers as we have just seen.

## The Function Prologue and Epilogue
We already know how to call a function (some code somewhere in memory) and we have already made a decision on how to get data in and out. This is enough when you work in assembly or with very simple functions, but for higher level languages we usually need a few more things.

These are some examples:

* We will usually need some local variables. These variables will only exists within our function and will disappear after the `RET` instruction... or at least, their values will not be valid any more.
* For languages like C++, the local variables may be objects. In general, destroying an object implies the execution of the so-called destructor... that is just another name for a function that has to be executed before returning. 
* We may need to be able to write re-entrant code in order to write recursive functions

What this means is that, for higher level languages, we usually have to do some additional things when entering and leaving a function. In general, all this is generated by the compiler and for application programmers it does not matter at all. However, if you are reversing some code generated from a high level language you will be exposed to these constructors/structures.

The code that is executed when entering the function is known as _Function Prologue_, and the code executed when leaving a function is known as _Function Epilogue_. Let's dive a bit more on this.


## Minimal Prologue and Epilogue. The Stack Frame

In the simplest case, a C function prologue and epilogue will look like this:

```Assembly
	push rbp
	mov  rbp, rsp
	sub  rsp, 0x10
	; PUSH registers used locally
        (... FUNCTION BODY ...)
	; POP Registers used locally
	mov rsp, rbp
	pop rbp
	ret
```

You will see code like this all over the place when reversing applications. So, let's invest some time understanding this concept.

The first three instructions are the function prologue. What the compiler usually do is to copy the current stack pointer in an auxiliary register. For Intel platforms this is usually `RBP` where `BP` stands for __Base Pointer__ (and `R` roughly means we are working with 64bits, `EBP` is the 32bits version and `BP` the 16bits version). This register is the one that defines the so called __Stack Frame__. We will get to this in a sec.

Let's look at the code in detail. We get into the function and the stack contains the return address as we described before. Then we store the current value of `RBP`... just because we are going to modify it and we want to restore it before returning, so the calling function does not have to care about changes to that register. Also, think about the case when a function is called from inside another function...

Then, we store in `RBP`, our base register, the current top of the stack. And after that, we increase the stack (remember it grows down on Intel platforms so we have to substract a value to grow the stack). For this specific function, the stack will look like this (supposing parameters are passed on registers - 64bits Intel-)

    ADDR
    X      Start    -->  Return Address
           RBP      -->  RBP
                    -->  4 bytes
                    -->  4 bytes
                    -->  4 bytes
    X-24   RSP      -->  4 bytes


The figure above shows the stack after the function prologue. In this example, the stack area going from the return address to the current value of the Stack Pointer is the so called stack frame for this function. OK, some books or authors may consider it slightly different, but what it is important is the concept. Note that if the ABI requires to push the parameters in the stack, those parameters will also be part of the function stack frame (they will be placed before the return address).... well, as I said, depends on who is writing about it.

So, the stack frame, is a chunk of the stack that contains all the information required by a function to work. This is very convenient. If we just call now another function within this function, a new stack frame will be added to the stack. When the function returns, the stack gets cleaned and the previous stack frame is just there. This way we can call multiple functions (even the same one) many times and keep the local variables of each invocation unmodified.

To finish this, let's take a look to the function epilogue. It just restore the stack pointer and the `RBP` base pointer, leaving the stack ready for executing `RET` and effectively getting rid of all local storage used by the function (the local variables).

Note that the `sub esp, 0x10` in the prologue is the equivalent to do 4 `pops` in the stack... Substracting a values from `RSP` allows us to allocate memory in the stack in a very easy way. The fact that the `RBP` base pointer holds the address of the original stack pointer allows us to release/free all that local memory just restoring the stack pointer. And we do that copying `RBP` back into `RSP`. See the stack figure above.


## Do we need the stack frame?

As we have seen, defining a stack frame makes our lives easier in the general case, however, specially when the functions are small it actually introduces a little overhead (both on execution time and also on memory) and does not provide much more. Also, not using a stack frame frees a register (`RBP` for intel processors) that can be used for operations in the function and therefore improve performance.

Sometimes, it may be interesting to get rid of it. and this can be done using the compiler flag `-fomit-frame-pointer`. Just for illustration process, these is the `func1` in our previous example with and width out frame pointer:

```
WITH FRAME POINTER                          | WITHOUT FRAME POINTER
55           push   rbp                     |
48 89 e5     mov    rbp,rsp                 |
c6 45 ff 10  mov    BYTE PTR [rbp-0x1],0x10 | c6 44 24 ff 10  mov    BYTE PTR [rsp-0x1],0x10
0f b6 45 ff  movzx  eax,BYTE PTR [rbp-0x1]  | 0f b6 44 24 ff  movzx  eax,BYTE PTR [rsp-0x1]
83 c0 01     add    eax,0x1                 | 83 c0 01        add    eax,0x1
5d           pop    rbp                     |
c3           ret                            | c3              ret

```

As you can see the saving is not that much.

## Other things that happens when you leave a function

What we have described is the very basics on what is going on under the hood when calling a function. However, depending on different factors a lot more things may happen.

We had already mentioned the C++ case. All those objects you create in your functions just does not go away magically. They are usually created in the stack and when the method returns, the stack has to be traversed to find out the locally created objects and call the appropriate destructors. This process is known generically as  [unwinding](https://en.wikipedia.org/wiki/Call_stack#Unwinding ""unwinding""). So... calling a C++ function/methods, despite of how simple the code may look like, can be doing quite a bunch of things under the hood and be way less efficient than a longer raw C version.

Also stack protections (we will talk about this later) requires extra code that needs to be executed when the function ends to detected corruptions in the stack.

## Local Variables

The last piece to understand the stack structure when a function is called are the local variables. Almost any decent programming language support local variables. A local variable is one that only exist in a given context, usually within a function. Compare this to a so-called global variable that is accessible from anywhere in the program at any time... Ah... yes... sure... global variables are evil (end of mandatory disclaimer).

So, the easiest way to create variables local to a function is to store them in the function stack frame. We had already seen how this is created for each function call, and therefore, anything we allocate in there will remain there for the function associated to it. So local variables are just allocated making room in the stack. In other words, they are declared changing the _Stack Pointer_.

To see this, and introduce the next concept we are going to learn, let's play with a simple hands-on example. Pay attention. What we are going to do right now, is something you can always do when you are not fully sure about something... just write a test and see what happens. This technique is invaluable during the learning process and you should use it as much as you can.

In this case, we are going to declare a few functions in a C program, with different local variables and see what happens. Yes sure, to declare a local variable in C you just need to declare the variable inside the function you want the variable to be local to...

```C
int func1 (void) {
  unsigned char a = 0x10;
  return a+1;
}
int func2 (void) {
  int a = 0x10;
  unsigned char b = 0x20;
  int c = 0x30;

  return a+b+c;
}

void func3 (int a, int b) {
  char str[100];

}
int main() {
}
```

Nothing really exciting. We declared three functions with some local variables of different types. Let's look at the assembly:


## `func1`

This is how `func1` looks like:

```
$ objdump -Mintel -d local_vars | grep -A7 ""<func1>""
00000000000005fa <func1>:
 5fa:   55                      push   rbp
 5fb:   48 89 e5                mov    rbp,rsp
 5fe:   c6 45 ff 10             mov    BYTE PTR [rbp-0x1],0x10
 602:   0f b6 45 ff             movzx  eax,BYTE PTR [rbp-0x1]
 606:   83 c0 01                add    eax,0x1
 609:   5d                      pop    rbp
 60a:   c3                      ret

```

We can see the function prologue (storing `rbp` and making it point to the stack). Then we see that `mov` instruction, copying the value `0x10` into `[rbp-0x1]`... That is our local variable. `RBP` will point to the top of to stack, as the stack grows to lower address (goes down) for intel machines, the next empty position in the stack is `rbp - 1`.

During reverse engineering code, you will have to identify the different local variables in the function. Most decent tools nowadays allows you to give names to these `rbp` offsets, or in other words, to name the local variables in the function you are reversing. That helps a lot during the process.

However here, we are missing a component that is common in functions but that, for this simple function, gcc have just thrown away because it is not needed. The peculiarity of this function is that it is a so-called leaf function. This means that this function does not call any other function. The practical effect of this is that we do not need to allocate memory in the stack for our local variables. 

Stack memory is usually allocated adjusting the Stack Pointer. In this case we just use the stack freely as... no other function will be executed that could overwrite our local variable values. 

Let's just quickly modify `func1` in our program like this:

```
int func1 (void) {
  unsigned char a = 0x10;
  a += func2 ();
  return a+1;
}
```

When compiling you will get a warning about `func2` not defined. You shall always honour warnings like those, but in this case, the prototype of the function actually matches the default prototype assigned  by gcc and everything is going to be fine. Anyway, be free to add `func2` prototype above `func1`  definition to remove the warning.

Now, we can take a look to the new code generated for `func1`.

```
4$ objdump -Mintel -d local_vars | grep -A11 ""<func1>:""
000000000000066a <func1>:
 66a:   55                      push   rbp
 66b:   48 89 e5                mov    rbp,rsp
 66e:   48 83 ec 10             sub    rsp,0x10
 672:   c6 45 ff 10             mov    BYTE PTR [rbp-0x1],0x10
 676:   b8 00 00 00 00          mov    eax,0x0
 67b:   e8 0c 00 00 00          call   68c <func2>
 680:   00 45 ff                add    BYTE PTR [rbp-0x1],al
 683:   0f b6 45 ff             movzx  eax,BYTE PTR [rbp-0x1]
 687:   83 c0 01                add    eax,0x1
 68a:   c9                      leave
 68b:   c3                      ret
```

Now we can see the instruction `sub rsp, 0x10` that allocates 16 bytes in the stack before calling `func2`. Why 16 byte?... Well, for 64bits intel processors the stack has to be aligned to blocks of 16 bytes. This is apparently related to the biggest register size that can be pushed into the stack (SSE extension). In this case we just need 1 byte (our variable is at `rbp-0x1`) but we have to allocate 16 at least.

The `leave` instruction is a high level procedure exit function complementary to `enter`. `leave` actually implements our function epilogue in one instruction... sets `RSP` to `RBP` and pop `RBP`. If you check the Intel manual for `enter` instruction you will see that this instruction does a lot... what roughly means that it is slow. Looks like compiler writers know that and they just generate the required instructions for each case. That's why you won't see `enter` very often on binaries.

## `func2`
Now, let's look at `func2`:

```
$ objdump -Mintel -d local_vars | grep -A12 ""<func2>""
000000000000067b <func2>:
 67b:   55                      push   rbp
 67c:   48 89 e5                mov    rbp,rsp
 67f:   c7 45 f8 10 00 00 00    mov    DWORD PTR [rbp-0x8],0x10
 686:   c6 45 f7 20             mov    BYTE PTR [rbp-0x9],0x20
 68a:   c7 45 fc 30 00 00 00    mov    DWORD PTR [rbp-0x4],0x30
 691:   0f b6 55 f7             movzx  edx,BYTE PTR [rbp-0x9]
 695:   8b 45 f8                mov    eax,DWORD PTR [rbp-0x8]
 698:   01 c2                   add    edx,eax
 69a:   8b 45 fc                mov    eax,DWORD PTR [rbp-0x4]
 69d:   01 d0                   add    eax,edx
 69f:   5d                      pop    rbp
 6a0:   c3                      ret

```

The first thing we notice is that the compiler has reordered the variables. We declared and `int`, then a `char` and then another `int`, but the variables are stored slightly differently:

             STACK CONTENT   RBP Index
               RET Addr
	RBP->       RBP
                c1	           -1
                c2             -2
			    c3             -3
			    c4             -4  [rbp - 0x4] <- int  (32bits)
				a1             -5
				a2             -6
				a3             -7
				a4             -8  [rbp - 0x8] <- int  (32bits)
				b              -9  [rbp - 0x9] <- char (8 bits)
				
As we can see, the `char` variable (that uses just 1 byte) is stored at the very end of the stack, this is probably the compiler trying to ensure memory alignment and optimising the use of stack memory. Anyway, the interesting thing here is than the compiler can re-order the local variables as it thinks is better (check [this](https://0x00sec.org/t/simple-buffer-overflow-demonstration/1131/4)).


## Then `func3`

Now it is time to look into `func3`. Things are going to get more interesting now:

```
00000000000006a1 <func3>:
 6a1:   55                      push   rbp
 6a2:   48 89 e5                mov    rbp,rsp
 6a5:   48 81 ec 90 00 00 00    sub    rsp,0x90
 6ac:   89 bd 7c ff ff ff       mov    DWORD PTR [rbp-0x84],edi
 6b2:   89 b5 78 ff ff ff       mov    DWORD PTR [rbp-0x88],esi
 6b8:   64 48 8b 04 25 28 00    mov    rax,QWORD PTR fs:0x28
 6bf:   00 00
 6c1:   48 89 45 f8             mov    QWORD PTR [rbp-0x8],rax
 6c5:   31 c0                   xor    eax,eax
 6c7:   90                      nop
 6c8:   48 8b 45 f8             mov    rax,QWORD PTR [rbp-0x8]
 6cc:   64 48 33 04 25 28 00    xor    rax,QWORD PTR fs:0x28
 6d3:   00 00
 6d5:   74 05                   je     6dc <func3+0x3b>
 6d7:   e8 64 fe ff ff          call   540 <__stack_chk_fail@plt>
 6dc:   c9                      leave
 6dd:   c3                      ret
```

The first thing we see just before the function prologue is an update of the `RSP` pointer. This is effectively the declaration of the array of our function (actually of all the local variables at once). Updating `RSP` is actually equivalent, as we have briefly mentioned already, to reserve a chunk of the stack because this part won't be used by `pushes` or `pops` or `calls` within the function (well, unless something goes really crazy in the program and we ..... overwrite the stack.... but we are getting to that in a while).

So, as the stack grows downwards (to the lower address), subtracting a value for RSP is the way to allocate memory in the stack.

> As you can see local variables are allocated/freed just decreasing/increasing `SP`... This means that they are not initialised automatically. In other words, this means that when you call a function many times one after the other, the same area of the stack will be used again and again to hold the function stack frame. If you do not initialise your variables yourself, they will hold values from previous invocations and you get those strange situations of functions that fail the first time but then they work fine.... It is not witchcraft is just variable initialisation...

Now the following question arise... Why is the compiler allocating `0x90` (144 bytes) instead of the 120 that we asked for?... Well, it is actually allocating the right size. if you pay attention to the rest of the code those `0x90` bytes includes a 8 bytes value at `[rbp-0x8]` (we will come back to this in a sec) and two values at `[rbp-0x84]` and `[rbp-0x88]` to store the parameters passed to the function (do you remember? par1 -> rdi/edi par2 rsi/esi). So 144-8-8-8 = 120.

Note however that for a 64bits machine the stack pointer needs to be always aligned to 16 bytes (yes bytes), in order to accommodate values for SSE registers. So, just keep in mind that sometimes the compiler will just allocate more stack space than needed in order to maintain its alignment and also it will pad small data types to also keep the memory properly aligned.

> Try to recompile the program with different values for the buffer size... specially odd numbers and see the result. Alternative re-read the section about `func1`

# Buffer overflows

So now we have got all the pieces to understand what is a buffer overflow and how it can be used to modify the normal program flow... which at the end is what an attacker is looking for, a way to execute arbitrary code, or at least some code that can provide some advantage or access to the machine.

Let's get back for a sec to our `func3` and how the stack is laid out after the function prologue is executed.

    X+0x8            Ret Address
    X        RBP  -> RBP
	X-0x8            Some data here
	(...)
	X-0x90   RSP  -> str (func3 buffer)

So, basically, if we can write more than `0x90` bytes into `str` we will start writing beyond the function stack frame and eventually we could overwrite the return address and therefore control the flow of the program. Actually we could overwrite the stack frame of the function that called the buggy one. 

Note that our `func3` does nothing and therefore there is no way to overwrite the buffer. Actually, we can only overwrite the buffer when the programmer have made a mistake developing the program. A so-called bug.

Let's make a mistake so we can learn more about how the stack can get smashed and its consequences.


## A vulnerable program

So, let's use a new program where we can actually overwrite the stack and also, let's take that chance to dump the stack content for better realise what is going on.

```
#include <stdio.h>

#define SIZE 13
#define OFF  12

void func (char *a) {
  long  first=0x1122334455667788;
  char s[50];
  char *d=s;
  int i;
  long *p;

  p = (&first) + OFF;
  for (i = 0; i < SIZE; p--,i++) printf (""%p -> %0lx\n"", p, *p);
  while (*d++ = *a++);

  p = (&first) +OFF;
  printf (""---------\n"");
  for (i = 0; i < SIZE; p--,i++) printf (""%p -> %0lx\n"", p, *p);
  
}

int main (int argc, char *argv[]) {
  printf (""Return address: %p\n----\n"", &&the_end);
  func (argv[1]);
 the_end:
  return 0;
}
```

So the program just dumps the stack... overwrites the buffer with the parameter passed through the command line, and them dumps the stack again, before leaving the function.

_Note:I manually adjusted the values of `SIZE` and `OFF` to just dump the interesting part for this example_

Let's see what happens when we run it:

```
$ ./vul `perl -e ""print 'A'x80;""`
Return address: 0x56330f66a833     <-----------------------+
----                                                       |
0x7ffd9b5eab88 -> 56330f66a833      <- Return Address -----+
0x7ffd9b5eab80 -> 7ffd9b5eaba0      <- RBP
0x7ffd9b5eab78 -> 2f0fef490dd7a400
0x7ffd9b5eab70 -> 7f2c512559f0
0x7ffd9b5eab68 -> 56330f66a88d
0x7ffd9b5eab60 -> 1
0x7ffd9b5eab58 -> f0b2ff
0x7ffd9b5eab50 -> 7ffd9b5eabb8
0x7ffd9b5eab48 -> 7f2c51247660
0x7ffd9b5eab40 -> 9                <- Buffer
0x7ffd9b5eab38 -> 7ffd9b5eab38
0x7ffd9b5eab30 -> 7ffd9b5eab40
0x7ffd9b5eab28 -> 1122334455667788 <- first
---------
0x7ffd9b5eab88 -> 4141414141414141 <- Return address overwritten
0x7ffd9b5eab80 -> 4141414141414141
0x7ffd9b5eab78 -> 4141414141414141
0x7ffd9b5eab70 -> 4141414141414141
0x7ffd9b5eab68 -> 4141414141414141
0x7ffd9b5eab60 -> 4141414141414141
0x7ffd9b5eab58 -> 4141414141414141
0x7ffd9b5eab50 -> 4141414141414141
0x7ffd9b5eab48 -> 4141414141414141
0x7ffd9b5eab40 -> 4141414141414141
0x7ffd9b5eab38 -> 7ffd9b5eab38
0x7ffd9b5eab30 -> 7ffd9b5eab91
0x7ffd9b5eab28 -> 1122334455667788
*** stack smashing detected ***: <unknown> terminated
Aborted
```

_NOTE:You will get completely different addresses in the first column. That's normal_

Yes, as you can see we can overwrite the return address in the stack so we could control the application flow whenever the function returns. Right?

Unfortunately things are not that easy nowadays and, as you can see, instead of crashing (because there is no code at address `0x4141414141414141`) we get a nice message informing us that the stack has been _Smashed_.....

> When trying to exploit a program, instead of using a sequence of `A`s as we did here, you better use a [De Bruijn Sequence](https://en.wikipedia.org/wiki/De_Bruijn_sequence) as it has interesting properties to find relevant offsets in memory

## Canaries

The reason why our program didn't crashed is because it is compiled with stack protection, or in other words the application uses canaries to detect changes in the stack. Let's see how this works checking the code of this function.

```
0000000000006fa <func>:
 6fa:   55                      push   rbp
 6fb:   48 89 e5                mov    rbp,rsp
 6fe:   48 83 ec 70             sub    rsp,0x70
 702:   48 89 7d 98             mov    QWORD PTR [rbp-0x68],rdi
 706:   64 48 8b 04 25 28 00    mov    rax,QWORD PTR fs:0x28
 70d:   00 00
 70f:   48 89 45 f8             mov    QWORD PTR [rbp-0x8],rax
(...)
 7e3:   48 8b 45 f8             mov    rax,QWORD PTR [rbp-0x8]
 7e7:   64 48 33 04 25 28 00    xor    rax,QWORD PTR fs:0x28
 7ee:   00 00
 7f0:   74 05                   je     7f7 <func+0xfd>
 7f2:   e8 c9 fd ff ff          call   5c0 <__stack_chk_fail@plt>
 7f7:   c9                      leave
 7f8:   c3                      ret

```

So, at the beginning of the function we can see how the function stores the value of `fs:0x28` into local variable `[rbp-0x8]`... that is the first available position in the stack after the return address and the `rbp` register.

The function does its stuff and at the end, it checks this local variable and compares it with the original value at `fs:0x28`. If the values doesn't mach, a special function named `__stack_chk_fail` is called. This is the function that shows the message in the console when we smash the canary.

Note that this code is only generated when we have a buffer in the function. Check back our `func1` and `func2` in the previous example... no canary... because, in principle we cannot override the stack without overflowing a buffer.

So, with this information, let's annotate once again the output of this simple program, but this time adjusting the sequence of `A` to the value that gets the canary check function executed:

```
$ ./vul `perl -e ""print 'A'x57;""`
Return address: 0x559e7cf9b833       <-------------------------+
----                                                           |
0x7ffd1a347e68 -> 559e7cf9b833       <--- Return Address ------+
0x7ffd1a347e60 -> 7ffd1a347e80       <-- RBP
0x7ffd1a347e58 -> 4c77054d1b6f8100   <-- Canary
0x7ffd1a347e50 -> 7f2d1ac4c9f0
0x7ffd1a347e48 -> 559e7cf9b88d
0x7ffd1a347e40 -> 1
0x7ffd1a347e38 -> f0b2ff
0x7ffd1a347e30 -> 7ffd1a347e98
0x7ffd1a347e28 -> 7f2d1ac3e660
0x7ffd1a347e20 -> 9                  <-- Buffer
0x7ffd1a347e18 -> 7ffd1a347e18
0x7ffd1a347e10 -> 7ffd1a347e20
0x7ffd1a347e08 -> 1122334455667788   <-- first
---------
0x7ffd1a347e68 -> 559e7cf9b833
0x7ffd1a347e60 -> 7ffd1a347e80
0x7ffd1a347e58 -> 4c77054d1b6f0041  <- First byte of canary overwritten
0x7ffd1a347e50 -> 4141414141414141
0x7ffd1a347e48 -> 4141414141414141
0x7ffd1a347e40 -> 4141414141414141
0x7ffd1a347e38 -> 4141414141414141
0x7ffd1a347e30 -> 4141414141414141
0x7ffd1a347e28 -> 4141414141414141
0x7ffd1a347e20 -> 4141414141414141
0x7ffd1a347e18 -> 7ffd1a347e18
0x7ffd1a347e10 -> 7ffd1a347e5a
0x7ffd1a347e08 -> 1122334455667788
*** stack smashing detected ***: <unknown> terminated
Aborted

```
To finish with this topic, just say that the initialisation code of the program (do you remember all that code that gets executed before `main` and that we got rid of it in a previous instalment?) generates a canary as a random number that gets stored at `fs:0x28`. `fs` is a segment register. We will not talk about this right now, just consider this a special memory address.

## Exploiting buffer overflows

Actually this section is just going to be a suggestion for you to read about the topic on the internet. There are plenty of great tutorials out there explaining how to exploit buffer overflows and how to circumvent canaries or deal with [ASLR](https://en.wikipedia.org/wiki/Address_space_layout_randomization) or [PIE binaries](https://en.wikipedia.org/wiki/Position-independent_code#PIE) (like the one in our example).

After all, this is a programming course, and the objective is for you to understand how to build programs and also how programs are built. Hope that at least, now you know enough about this topic to easily follow any exploitation tutorial out there and update it to make it work for your system... yes... things usually just doesn't work out of the box and knowing this details will help you figure out why a lot faster.

... that was easier than I expected :)


# The ARM Case

Now, that we know everything about the stack on Intel processors, let's take a look to what happens on other platforms. let's start with our beloved ARM. We are going to recompile the three function test program we used earlier in this section and look inside it.

```
$ arm-linux-gnueabi-gcc -o local_vars-arm local_vars.c
$ arm-linux-gnueabi-objdump -d local_vars-arm
```

As we did before, let's go function by function analysing the code generated:

## `func1` ARM

Before fully understand this code we need to learn a little bit of basics about ARM registers. As it happens with the Intel processor, ARM has some special registers. In principle, ARM names the registers as `RX` where `X` is a number running from 0 to 15 for the 32bits architecture (64bits uses other names). However a bunch of those have special meanings:

    REGISTER  ALIAS  DESCRIPTION
    R11       FP     Frame Pointer
	R12       IP     Intra Procedural Call
	R13       SP     Stack Pointer
	R14       LR     Link Pointer
	R15       PC     Program Counter


From those special ones, the following are relevant for our current discussion:

* `R11/FP`. This register is used as frame pointer in the same way that `RBP` on the Intel platform.
* `R13/SP`. This is the stack pointer and works exactly like the `RSP` Intel register
* `R14/LR`. This is called the Link Register. Whenever a function is called this register contains the _Return Address_. As you may remember, for intel processors, this value is stored in the stack before transferring the control to the function. ARM, PowerPC, Pa-RISC and SPARC uses this other approach of storing the value in a register
* `R15/PC`. This is the instruction pointer or program counter and we already know everything about it.

With this information, lets take a look 

```
00010468 <func1>:
   10468:       e52db004        push    {fp}            ; (str fp, [sp, #-4]!)
   1046c:       e28db000        add     fp, sp, #0
   10470:       e24dd00c        sub     sp, sp, #12
   10474:       e3a03010        mov     r3, #16
   10478:       e54b3005        strb    r3, [fp, #-5]
   1047c:       e55b3005        ldrb    r3, [fp, #-5]
   10480:       e2833001        add     r3, r3, #1
   10484:       e1a00003        mov     r0, r3
   10488:       e28bd000        add     sp, fp, #0
   1048c:       e49db004        pop     {fp}            ; (ldr fp, [sp], #4)
   10490:       e12fff1e        bx      lr

```

The first things the function does is to store in the stack the current frame pointer... in other words, the frame pointer of the function calling us. Then this function frame pointer is initialised to the current Stack Pointer value plus 0 in this case. We will see an example later when this is done differently, but for now we are just setting up our frame pointer to point to the top of of the stack.

Then we find the `sub sp,sp #12` that makes some room in the stack... I'll let up to you to figure out what this 12 value comes from... It is room for 3 words... it doesn't make much sense to me at first glance... so I smell some fun ahead in order to figure this out.

Then we have the main function code.. the code looks funny but if we enable optimisations we will not see most of the interesting things. Anyway, it allows us to see how ARM access local variables.... remember the `[rbp-0xNN]` for Intel processors?. Sure, so taking into account that `fp` is the new `RBP`, `strb r3, [fp, #-5]` stores register `r3` on the stack frame position 5, and the `ldrb` loads it from that same position. So this is how local variables looks like for ARM.

Finally the value to be returned is stored in `r0` and the stack restored.

Note that, instead of using a `RET` instruction taking the return value from the stack, the code branches to the position indicated by the link register that, if you remember, was updated with the proper return address, when calling the function. 

We will skip now `func2` as it doesn't give us much more information and let's go straight into `func3`.

## `func3` for ARM

Let's take a look to the assembly:

```
000104dc <func3>:
   104dc:       e92d4800        push    {fp, lr}
   104e0:       e28db004        add     fp, sp, #4
   104e4:       e24dd090        sub     sp, sp, #136    ; 0x88
   104e8:       e50b0090        str     r0, [fp, #-144] ; 0xffffff70
   104ec:       e50b1094        str     r1, [fp, #-148] ; 0xffffff6c
   104f0:       e59f3028        ldr     r3, [pc, #40]   ; 10520 <func3+0x44>
   104f4:       e5933000        ldr     r3, [r3]
   104f8:       e50b3008        str     r3, [fp, #-8]
   104fc:       e1a00000        nop                     ; (mov r0, r0)
   10500:       e59f3018        ldr     r3, [pc, #24]   ; 10520 <func3+0x44>
   10504:       e51b2008        ldr     r2, [fp, #-8]
   10508:       e5933000        ldr     r3, [r3]
   1050c:       e1520003        cmp     r2, r3
   10510:       0a000000        beq     10518 <func3+0x3c>
   10514:       ebffff8b        bl      10348 <__stack_chk_fail@plt>
   10518:       e24bd004        sub     sp, fp, #4
   1051c:       e8bd8800        pop     {fp, pc}
   10520:       00020f08        .word   0x00020f08

```

The first thing we must note is that the function prologue and epilogue are now different. For this function, the compiler has generated code to store the `fp` and `lr` registers in the stack (see the first push... yes, ARM allows you to push multiple registers in just one instruction). This means that.... the return address will be stored in the stack and therefore... it can be overwritten.... Yes, for a while you thought that smashing the stack will not work for ARM because the return address is stored in a register... don't you?

Actually, this works fine for calling just one function. In the general case, when a function is called and it also calls other functions (nested functions), we need to store the value of this register somewhere we can recover it at some point (actually when we return from the inner function), to return to the right place in the program. And the place for doing that is in the stack... normally. What if we store `fp` and `lr` somewhereelse?.... May that mitigate buffer overflows?... :think:

> ARM differentiates between leaf and non-leaf functions. A leaf function is a function that doesn't call any other function. Basically this means that we do not need to store the Link Register to return from the function (as far as we do not modify it in the function body). Non-leaf functions needs to store the return address stored in the Link Register and they do this usually in the stack frame pointed by register `fp`. There are also special instructions for calling those.

Now, take a look the the epilogue. Instead or branching (jumping) as in `func1`, in this case we just pop `lr` directly on the Program Counter register... effectively changing the program flow.

We can also see how the buffer is allocated substracting a value from `sp`. Note that this is ARM 32bits (all opcodes are 32bits and everything is 32bits aligned) so the stack is likely 8 bytes aligned. You can try to check this by yourself

Finally, note that the canary implementation follows the same concept that we discussed for the Intel processors... it is just stored somewhere else


# The MIPS Case

For completeness, less recompile our test programs for MIPS and check how all this is done for those processors:

```
$ mips-linux-gnu-gcc -o local_vars-mips local_vars.c
$ mips-linux-gnu-objdump -d local_vars-mips

```
## `func1` for MIPS

This is how `func1` for MIPS looks like:

```
004007b0 <func1>:
  4007b0:       27bdfff0        addiu   sp,sp,-16
  4007b4:       afbe000c        sw      s8,12(sp)
  4007b8:       03a0f025        move    s8,sp
  4007bc:       24020010        li      v0,16
  4007c0:       a3c20007        sb      v0,7(s8)
  4007c4:       93c20007        lbu     v0,7(s8)
  4007c8:       24420001        addiu   v0,v0,1
  4007cc:       03c0e825        move    sp,s8
  4007d0:       8fbe000c        lw      s8,12(sp)
  4007d4:       27bd0010        addiu   sp,sp,16
  4007d8:       03e00008        jr      ra
  4007dc:       00000000        nop

```

The first thing we notice is that registers are named differently. We already knew that, don't we? Just looking to the code, and without much knowledge about the MIPS architecture we can already identify the prologue and the epilogue of the function. The `sp` register is of course the Stack Pointer, and we see how space is allocated and free at the beginning and the end of the function.

Also, we can infer from the code that register `s8` is the one used as frame pointer, at least for this version of gcc. The prologue of the function adjusts `sp`, stores `s8` (our frame pointer) in the top of the stack and then sets `s8` to the bottom of the stack, so local variables are indexed with positive indexes.

The epilogue restores the `sp` value (frees memory `addiu sp,sp,16`), restores `s8` (`lw s8, 12(sp)` lw - _Load World_) and returns. As it happens with ARM, the return address (that's where the `ra` register takes its name, I told you to remember that name) is stored in a register instead of in the stack, and return from a function is performed just jumping back to the content of `ra` (`jr` stands for _Jump Register_).

Just in case you are not familiar with the indexed notation used by `objdump` for MIPS. The value `7(s8)` means in Intel mnemonics `[s8 + 7]` as we already know that `s8` is our `RBP` for MIPS, we see the usual pattern to access local variables. Just note that in this case we first update `sp` and then we set `s8` instead of setting `s8` and then updating `sp` as we have seen for the Intel platform. This is way the MIPS code uses positive indexes and the intel one uses negative indexes.

## `func3` for MIPS

`func3` is more interesting as we know, so let's take a look to how it looks like for a MIPS processor:

```
0040082c <func3>:
  40082c:       27bdff60        addiu   sp,sp,-160
  400830:       afbf009c        sw      ra,156(sp)
  400834:       afbe0098        sw      s8,152(sp)
  400838:       03a0f025        move    s8,sp
  40083c:       3c1c0042        lui     gp,0x42
  400840:       279c9010        addiu   gp,gp,-28656
  400844:       afbc0010        sw      gp,16(sp)
  400848:       afc400a0        sw      a0,160(s8)
  40084c:       afc500a4        sw      a1,164(s8)
  400850:       8f82804c        lw      v0,-32692(gp)
  400854:       8c420000        lw      v0,0(v0)
  400858:       afc20094        sw      v0,148(s8)
  40085c:       00000000        nop
  400860:       8f82804c        lw      v0,-32692(gp)
  400864:       8fc30094        lw      v1,148(s8)
  400868:       8c420000        lw      v0,0(v0)
  40086c:       10620005        beq     v1,v0,400884 <func3+0x58>
  400870:       00000000        nop
  400874:       8f828048        lw      v0,-32696(gp)
  400878:       0040c825        move    t9,v0
  40087c:       0320f809        jalr    t9
  400880:       00000000        nop
  400884:       03c0e825        move    sp,s8
  400888:       8fbf009c        lw      ra,156(sp)
  40088c:       8fbe0098        lw      s8,152(sp)
  400890:       27bd00a0        addiu   sp,sp,160
  400894:       03e00008        jr      ra
  400898:       00000000        nop

```

For `func3` as we had already seen for the previous architectures, the compiler generates code to store not just the frame pointer but also the return address. Also note the stack allocation size adjustment done by the compiler to keep properly alignment stack memory.

The function epilogue is the complementary. We restore `sp`, then recover the values for `ra` and `s8`... give the stack memory back, updating the `sp` register and finally returning to the value contained on `ra`.

The canaries are managed slightly differently, but overall we can see how the value is retrieved from a special memory location identified by the `gp` register. This register is intended to point to the middle of a 64K block of memory located in the heap and intended to store constants and local variables... Not the same thing that a segment register (remember `fs` on Intel) but fullfill the function.

# Conclusions

This is it for this part. This time we had learn how functions are implemented and the role the stack play on software. Also we had also found out why buffer overflows are dangerous and how canaries can be used to detect those overflows and prevent the exploitation of those bugs.

Finally we took a look to how all this looks for ARM and MIPS just to find that.... they work roughly the same way :)



* PREVIOUS: [Programming for Wannabes. Part III. Your first Shell Code](part-03.md)
* NEXT: [Programming for Wannabes. Part V. A Dropper](part-05.md)
