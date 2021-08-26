# Programming for Wanabes IX. Finding Files in asm 
Now, that the code to traverse directories is ready is time to get back to assembler and re-implement our code using this language. This will give as a chance to learn a lot of new assembly instructions and idioms.

To get started let's write a base program that we can use to kick off this development. We will just show a message and exit. We had already done this in the past, so it should be quick an unpainful.

# Hello world again

We will use the mini libc we implemented in the previous instalment, but for the assmebler part of the course I prefer to use NASM, so I have to slightly change the code we already have.

```nasm
	global mfw_exit
	global mfw_write
	global mfw_close
	global mfw_openat
	global mfw_newfstatat
	global mfw_getdents
	global mfw_open
	global mfw_lstat
	global mfw_puts
	
	;; Syscalls
mfw_write:
	mov eax, 0x01
	syscall
	ret
	
mfw_openat:
	mov eax, 0x101
	syscall
	ret
	
mfw_close:
	mov eax, 0x03
	syscall
	ret

mfw_exit:
	mov eax, 0x3c
	syscall
	ret

mfw_newfstatat:
	mov r10, rcx
	mov eax, 0x106
	syscall
	ret

mfw_getdents:
	mov eax, 78
	syscall
	ret

mfw_open:
	mov eax, 0x02
	syscall
	ret

mfw_lstat:
	mov eax, 0x06
	syscall
	ret
```

Not that hard, AT&T assembly uses the character `$` for literals and also the source and destination operands in the move are changed. You may have notice that we declared a global symbol `mfw_puts` but there is no label for it. We will implement it in a sec.

# `mfw_puts`
You may remember this function from previous instalments. Its C implementation is:

```C
int mfw_puts (char *s) {
  while (*s) mfw_write (1, s++, 1);
}
```

So it is time to do our first loop in assembler. As you will see in a second, assembler loops are way more simpler than in C... just jump around as needed.

```nasm
;; rdi -> string to print
mfw_puts:
	mov   rsi, rdi  ;; Put string on RSI as expected by write
	mov   rdi, 1
	mov   rdx, rdi
mfw_puts_l0:	
	cmp   BYTE [rsi], 0
	jz    mfw_puts_exit
	call  mfw_write   ;; write (RDI, RSI, RDX)
	inc   rsi
	jmp   mfw_puts_l0
mfw_puts_exit:
    xor rax,rax
	ret
```

_NOTE:Add this code to the end of the minilibc,asm file, the one in previous section._

The function receives the string to print in `RDI`. Then we need to scan the string, if the value is `0` we are done and we return from the function. So, this is what the function does in detail:

* Copy `RDI`, the string we pass as parameter into `RSI`. The `write` syscall we will run to print the characters, expects the buffer to print in `RDI`.
* Then we set `RDI` to 1 or stdout if you prefer. That is the first parameter to `write` 
* And we set `RDX` to 1, because we are going to print the string character by character.
* Then we check if the current content of `RDI` (the address pointed by `RSI` aka the string to print) is 0. We do this with the `CMP` instruction. The brackets around `RSI` indicates an indirect addressing, or in other words, we are interested on the content of the address pointed by `RSI` not the `RSI` value itself
* The next instruction, `JZ` means that we will jump to the address indicated whenever the `Zero` flag is set. We will talk about flags in a sec, but for now. the `Zero` flag or `Z` is set whenever we run an arithmetic or logical operation, and the result is 0. The `cmp` is actually a difference between the two values passed as parameters. So, if it is zero, we finish the loop.
* Otherwise, we call the `write` system call, and increase the `RSI` value. That means that we move to the next char in the string.

This is a very simple code and I hope you have followed the explanation. We can write this code in many different ways. I will show you a couple so you get familiar with the use of the jumps and the comparisons.

```nasm
mfw_puts1:
	mov   rsi, rdi
	mov   rdi, 1
	mov   rdx, rdi
	jmp   mfw_puts1_next
mfw_puts1_l0:	
	call  mfw_write
	inc   rsi
mfw_puts1_next	:
	cmp   BYTE [rsi], 0
	jnz    mfw_puts1_l0
mfw_puts1_exit:
	ret
```

This is exactly the same, but first jumping into the check and then looping. This is the code that `gcc` will usually generate. 

Just for our understanding, let's take a look to the code that `gcc` generated for our C version to see a couple of interesting details

```asm
000000000000066a <mfw_puts>:
                                
 66a:   55                      push   rbp
 66b:   48 89 e5                mov    rbp,rsp
 66e:   48 83 ec 10             sub    rsp,0x10                ;; Set stack frame and allocate local vars
 672:   48 89 7d f8             mov    QWORD PTR [rbp-0x8],rdi ;; Store string on local variable
 676:   eb 1e                   jmp    696 <l0_next>
l0:
 678:   48 8b 45 f8             mov    rax,QWORD PTR [rbp-0x8] ;; Retrieve pointer to string from stack
 67c:   48 8d 50 01             lea    rdx,[rax+0x1]           ;; Increase pointer
 680:   48 89 55 f8             mov    QWORD PTR [rbp-0x8],rdx ;; Store increased pointer for next iteration
 684:   ba 01 00 00 00          mov    edx,0x1                 ;; Just call write
 689:   48 89 c6                mov    rsi,rax
 68c:   bf 01 00 00 00          mov    edi,0x1
 691:   e8 47 03 00 00          call   mfw_write
l0_next: 
 696:   48 8b 45 f8             mov    rax,QWORD PTR [rbp-0x8] ;; Retrieve pointer to next character
 69a:   0f b6 00                movzx  eax,BYTE PTR [rax]
 69d:   84 c0                   test   al,al                   ;; And check for 0
 69f:   75 d7                   jne    678 <l0>
 6a1:   90                      nop
 6a2:   c9                      leave
 6a3:   c3                      ret

```

Some comments on the gcc code:

* First, it uses the second kind of loop we described (jump into the comparison first).
* Second, it systematically stores things in the stack. The string pointer is treated as a local variable
* Third it uses `lea` to increase the pointer.... We will discuss this later
* Forth it uses `test` instead of `cmp`.... We will also discuss this in a second.

The code above was the default gcc code. It is 58 bytes long. Let's see what we get when we compile for size optimisation:


```asm
                                
 6aa:   80 3f 00                cmp    BYTE PTR [rdi],0x0     ;; Check if current char is 0
 6ad:   48 89 fe                mov    rsi,rdi                ;; Store string pointer in `RSI`
 6b0:   75 01                   jne    6b3 <mfw_puts+0x9>     ;; If character != 0 continue
 6b2:   c3                      ret
 6b3:   53                      push   rbx  ;; Preserve RBX. We will use it for temporal data
l0:
 6b4:   48 8d 5e 01             lea    rbx,[rsi+0x1]           ;; Increase pointer using RBX
 6b8:   ba 01 00 00 00          mov    edx,0x1                 ;; Set parameters to call syscall
 6bd:   bf 01 00 00 00          mov    edi,0x1
 6c2:   e8 e2 01 00 00          call   mfw_write
 6c7:   80 3b 00                cmp    BYTE PTR [rbx],0x0      ;; Check if next character is 0
 6ca:   48 89 de                mov    rsi,rbx                 ;; Update pointer
 6cd:   75 e5                   jne    l0                      ;; If not cero, repeat
 6cf:   5b                      pop    rbx
 6d0:   c3                      ret

```

This code is 39 bytes long, so, yes it has been a good size reduction

And this is what we get when compiling with max optimisation (`-O7`):


```asm
     750:       80 3f 00                cmp    BYTE PTR [rdi],0x0  ;; Check current char is 0
     753:       74 2b                   je     780 <exit>
     755:       53                      push   rbx
     756:       48 89 fe                mov    rsi,rdi       ;; Copy String in `RSI`
     759:       0f 1f 80 00 00 00 00    nop    DWORD PTR [rax+0x0]  ;; NOP... Alignment?
l0:	 
     760:       48 8d 5e 01             lea    rbx,[rsi+0x1] ;; Inc String
     764:       ba 01 00 00 00          mov    edx,0x1       ;; Set parameters
     769:       bf 01 00 00 00          mov    edi,0x1
     76e:       e8 24 14 00 00          call   mfw_write
     773:       80 3b 00                cmp    BYTE PTR [rbx],0x0 ;; Is 0?
     776:       48 89 de                mov    rsi,rbx        ;; Update pointer
     779:       75 e5                   jne    760 <l0>       ;; Jump if not 0
     77b:       5b                      pop    rbx
     77c:       c3                      ret
     77d:       0f 1f 00                nop    DWORD PTR [rax]
exit:	 
     780:       f3 c3                   repz ret

```

This code is a bit closer to our original one. It is 50 bytes long, something in between the vanilla code and the size optimised one.

Just for comparison purposes, let's show the opcodes of our `mfw_puts` version, to get an idea of the size:

```asm
0000000000400113 <mfw_puts>:
  400113:       48 89 fe                mov    rsi,rdi
  400116:       bf 01 00 00 00          mov    edi,0x1
  40011b:       48 89 fa                mov    rdx,rdi

000000000040011e <mfw_puts_l0>:
  40011e:       80 3e 00                cmp    BYTE PTR [rsi],0x0
  400121:       74 0a                   je     40012d <mfw_puts_exit>
  400123:       e8 a8 ff ff ff          call   4000d0 <mfw_write>
  400128:       48 ff c6                inc    rsi
  40012b:       eb f1                   jmp    40011e <mfw_puts_l0>

000000000040012d <mfw_puts_exit>:
  40012d:       c3                      ret
```

Ours is just 27 bytes :).... That's pretty cool.... We could also write the function like this:

```asm
000000000040012e <mfw_puts2>:
  40012e:       48 89 fe                mov    rsi,rdi
  400131:       bf 01 00 00 00          mov    edi,0x1
  400136:       48 89 fa                mov    rdx,rdi
  400139:       48 89 f8                mov    rax,rdi

000000000040013c <mfw_puts2_l0>:
  40013c:       80 3e 00                cmp    BYTE PTR [rsi],0x0
  40013f:       74 07                   je     400148 <mfw_puts2_exit>
  400141:       0f 05                   syscall
  400143:       48 ff c6                inc    rsi
  400146:       eb f4                   jmp    40013c <mfw_puts2_l0>

0000000000400148 <mfw_puts2_exit>:
  400148:       c3                      ret

```

This version is the same size, but it is way more efficient, as we have removed the `call` instruction that, as you already know, pushes `RIP` in the stack, branches, do a register assignment, and then pops `RIP` from the stack to continue. Now, we just call `syscall`, so we are saving quite a few instructions for each string character.

# Flags
The so-called processor flags are stored usually in a special register. The flags are just special conditions raised by the operations performed in the ALU (Arithmethic Logic Unit) and other processor status. You usually do not use the flags directly (they are just bits in a special register as I said), most of them are used by jump instructions to allow us to change the program flow based on the value of the flags. Other flag are special and are changed using processor instructions


This sounds very abstract so we better describe then right away:

* CF 	Carry flag. Used to indicate when an arithmetic carry or borrow has been generated out of the most significant arithmetic logic unit (ALU) bit position. This is very relevant when implementing mathematical operations. 
* PF 	Parity flag. Parity flag changes according to the number of 1s in a given ALU result. If the number of 1s is even and 0 if it is odd. The `JO` will jump when parity is odd and `JNO` if not.
* AF 	Adjust flag. This flag is the same than the carry flag but only for the last nibble (last 4 bits of the number). It is relevant when working with BCD (Binary Coded Decimal) numbers, but that is really old stuff. We won't use this
* ZF 	Zero flag. This flag get activated whenever the result of the last operation is zero. 
* SF 	Sign flag. This flag indicates the sign of the last operation. When set to 1 the value is negative, otherwise is positive. 
* OF 	Overflow flag. This flag gets activated whenever the last operation performed produced and overflow. That is, the result is bigger than the size of the register that has to store that result.

There are a few more flags but they are not related to the ALU and introducing them right now will not help us at all. They will get in later when needed.

The jump instructions `JXX` are composed adding two characters (that `XX`) that defines the flags that will be checked to jump or not. This is a table of all possibilities, you will see than some combination of letters are redundant:

JXX         Description                          Flag
---         -----------                          ------
E, Z	    Equal, Zero	                         ZF == 1
NE, NZ	    Not Equal, Not Zero	                 ZF == 0
O	        Overflow	                         OF == 1
NO	        No Overflow	                         OF == 0
S	        Signed	                             SF == 1
NS	        Not Signed	                         SF == 0
P	        Parity	                             PF == 1
NP	        No Parity	                         PF == 0
C, B, NAE	Carry, Below, Not Above or Equal     CF == 1
NC, NB, AE	No Carry, Not Below, Above or Equal	 CF == 0
A, NBE	    Above, Not Below or Equal	         CF==0 and ZF==0
NA, BE	    Not Above, Below or Equal	         CF==1 or ZF==1
GE, NL	    Greater or Equal, Not Less	         SF==OF
NGE, L	    Not Greater or Equal, Less	         SF!=OF
G, NLE	    Greater, Not Less or Equal	         ZF==0 and SF==OF
NG, LE	    Not Greater, Less or Equal	         ZF==1 or SF!=OF

The chose of one or another just depends on what your code has to do.... We will be seeing examples in this course, however, some of those, as for instance, the parity flag, are not used very often (maybe on communications software), and the carry is not frequently used either unless you are implementing your own mathematical routines.

Do not worry about all this information. The important thing is that there are multiple jump instructions and that their behaviour depends on the result of the last arithmetic or logical operation performed by the processor.

# The gcc tricks

In the code generated by `gcc` we have seen a couple of _curious_ ways of doing thing. Let's quickly comment them.

The first one was the use of the `LEA` instruction to increase a register:

```nasm
lea    rbx,[rsi+0x1] 
```

The `LEA` instruction takes it name from the acronym _Load Effective Address_. It basically loads in the first operand, the address indicated in the second. When compared with `MOV`, `LEA` loads the address and `MOV` loads the value at that address (note the `[]` to indicate the indirect addressing mode.

The advantage of using `LEA` for this is that we can do quite some complex operations on the right side, and those operations are performed by the memory access unit and not by the arithmetic unit, as it happens when using normal `add`, `sub`, `mul`, `inc`, etc... And the advantage of that is that the memory accessing unit is always involved on the execution of the instruction and therefore, the calculation is done for free... in the sense that it will not add further delays, and also will left the arithmetic unit free for other operations.

We could change our code to use `lea rsi,[rsi+1]` instead of using `inc`... That will be a bit faster, but will require 1 extra byte... Depending what you are interested on, chose one or the other.

The other difference with our code is the use of `test` instead of `cmp`. The difference between these two is that the first one is an `AND` operation between the two operators. An `AND` operation will only be 0 if both values are equal, so it serves our purpose. The `cmp` instruction is an arithmetic operation, it substracts both operators and actually discards the result. They work the same and for our purpose there is no difference except that  `test` requires one bit less than `cmp` (we do not have to provide a value to compare). On the other hand `cmp` is more explicit and makes the code more readable... Anyhow. that `test al,al` instruction before a conditional jump, is so common that all of you will know what it means before you finish reading this course.

In general, writing C code, compiling it and look at the generated assembly is a good way to learn. You will see things that you do not known and then you will ask google and get a stackoverflow answer with a lot of useful information that will boost your progress.

# Adding a `_start` and compiling

Well, enough on printing characters, loops and flags. Let's just write our main program and compile it.

The main program that I have named `select_file01.asm` will look like this:

```nasm
	;; Select file
	
    global    _start
	;; List the functions we will use
	extern    mfw_puts
	extern    mfw_exit
	
    section   .text
_start:
	;; Show welcome message
	mov rdi, wellcome_msg
	call mfw_puts

	;; Call select_target
	
	;; All done. Exiting
	xor rdi, rdi
	call mfw_exit
	
	;; --------------------------
	section .data
wellcome_msg:	db 	"Malware 0.1", 0x0a
```

No big surprises here. We just shown a message and then we exit. As you may have noticed already, assembly language is pretty verbose so I will need to split the code in different files to make it manageable. Right now, we just have two files, the main program and the pseudo system library we created, but that is enough to write a simple `Makefile` to build our program.


```makefile
select_asm01: select_file01.o minilibc.1.o
        ld -o $@ $^

%.o: %.asm
        nasm -f elf64 -o $@ $<

```

In case you are not familiar with `make` here is a quick crash course:

Makefiles are composed of rules. Each rule has one or more lines. The first line indicates what you want to _make_ followed by a colon `:`. After the colon you add the files that are needed to build that target. Whenever one of those files changes, the rule will get activated.

The second and next lines (if any) are the list of commands to execute in order to use the files on the right of the first line to build the file on the left. In order to make these rules shorter,  `make` defines several automatic variables... those starting with `$`. In our case we are using the following:

    $@ : Represent the target of the rule. That is, the string at the left of the colon 
	$^ : Represent the dependencies of the rule. The string at the right of the colon
	$< : Represents just the first dependency.
	
Also, we have used a general rule, to compile all `.asm` files into `.o` files using `nasm`. That is the one with `%.o:%asm`... that basically means, _Convert any file with extension .asm into a file with extension .o and the same name_.

Then, to build the program we use `ld` with a specific set of objects.

We will be modifying the `Makefile` as we go and adding more functionalities. The explanation above is what you will be using most of the time... but there are a few extra things that we will have to learn.

# `select_target` 

Let's start implementing our `select_target` function. Let's write this function in a separated file. Now you should know how to update your Makefile to use the new file you create. If you do not know, ask in the comments.

In our file we will start adding the external references to the functions we want to use and then start coding our function creating a stack frame:

```nasm
	global mfw_select_target
	extern mfw_puts
	extern mfw_openat
	extern mfw_getdents
	extern mfw_close

	section .text
mfw_select_target:
	push  rbp
	mov   rbp, rsp
	sub   rsp, 0x10
	
	;; Our code goes here
	
	leave     		; Set RSP=RBP and pops RBP
	ret
```


So, let's go step by step, let's start declaring 2 longs (16 bytes or 0x10 bytes). We put the general stack frame preamble. Remember our function will have to be recursive so we really need to have a stack frame and keep the relevant information in the stack to pass it from call to call. Then we just call `leave` at the end. This is equivalent to restore `SP` and `pop rbp`... do as you prefer.

# Adding local variables

Even when `nasm` seems to have some macros to help us dealing with the declaration of local variables, let's try to avoid the use of specific assembler features. Be free to explore yourself the extras provided by your preferred assembler. So, what we are going to do is to use constants to reference the offsets in the stack. This will give us some facility to write the code, at the same time that we keep it simple and clear.

```nasm
mfw_select_target:
	FD	    EQU     0x08
	I	    EQU     (FD + 0x08) 
	N	    EQU     (I + 0x08) 
	STE     EQU     N

	push  rbp
	mov   rbp, rsp
	sub   rsp, STE
	(...)
```

So, we have a long at `0x08` that we will use to store or file descriptor (`FD`) and another long at `0x10` ('I') that we will use as a general counter to navigate the directory entries and finally another long at `0x18` to read the number of bytes read from `getdent`. We still have to declare a buffer and a structure, but as I said, let's go step by step.

With this definitions, we can access the local variables like this:

```nasm
   mov rax, [rbp - FD]   ;; rax = FD
   mov [rbp - FD], rax   ;; FD = rax
```

Now, we can add the code to open and close the directory:

# Open/Close directory

This is how it is going to look like:

```nasm
mfw_select_target:
  ;; Constants and stack frame code. See above
  ;; At function entry, the registers are as follow:
  ;;    RDI  -> file descriptor (0 from main)
  ;;    RSI  -> current folder name
  mov   rdx, 0q200000     ;; Set 3rd parameter to O_RDONLY | O_DIRECTORY
  call  mfw_openat
  mov   [rbp-FD], rax
  
  ;; Iterate directory... TO be writen


  mov   rdi, [rbp-FD]
  call  mfw_close
```

So far so good. We already have `rdi` and `rsi` set from the function call. We have conveniently selected the order of the parameters to match `openat` so we do not have to do anything with them. The 3rd parameter is the flags that should be set to `O_RDONLY | O_DIRECTORY`. You can find the values of this constants in `/usr/include/bits/fcntl-linux.h`... Check previous instalments to figure out how to get to that file:

    $ egrep 'O_RDO|O_DIR'  /usr/include/bits/fcntl-linux.h
    #define O_RDONLY             00
    #ifndef __O_DIRECTORY
    # define __O_DIRECTORY  0200000
    #ifndef __O_DIRECT
    # define __O_DIRECT      040000
    # define __O_TMPFILE   (020000000 | __O_DIRECTORY)
    # define O_DIRECTORY    __O_DIRECTORY   /* Must be a directory.  */
    # define O_DIRECT       __O_DIRECT      /* Direct disk access.  */



`O_RDONLY` is just 0 so we can ignore it. and `O_DIRECTORY` is `0200000`.. and octal number. `nasm` uses the format `0qNUM` or `NUMo` to represent octal values.... that is why we use `0q200000` as a flag.

Then we just call `mfw_openat` and we store the return value (that we get on register `rax`) into our local variable.... Easy isn't it?

At this point we can start doing things with our directory.... We will come to this in a sec.

We finish the function closing the file. We just get file descriptor from the local variable and we put it in `RDI` to pass it as first parameter to `mfw_close`.

# Testing the code
The code above, just works because I tested myself. So, I will tell you how you can debug your code. The first thing you have to do is to compile your program with debug information. For that we need to make a small change in our Makefile:

```makefile
all: select_asm01 select_asm02

DEBUG=-F dwarf -g

select_asm01: select_file01.o minilibc.1.o
        ld -o $@ $^

select_asm02: select_file02.o select_target.o minilibc.1.o
        ld -o $@ $^

%.o: %.asm
        nasm -f elf64 ${DEBUG} -o $@ $<


```

I have created a second binary for this new update. You do not have to, for me, as I write this, is good to have all versions available at all times for checking/testing things. For compiling a second `select_asm02`, I just added the general rule, including the new `select_target.o`, and then I used the general rule `all:` as the first rule, to get it fired when invoking `make`. In order to complete the rule `all`, the two binaries are required... and that will fire all the required rules to get them compiled.

I have also declared a variable named `DEBUG`, containing the `nams` flags to add debug information to the object files, so we can use `gdb` (or other debugger) to debug our program. The good thing about using a variable, is that we can remove the debug information just removing the variable and recompiling... Also we will have to write less if we want to change the flags (for instance to use a different debug format).

Now, you can load the program with `gdb` and check that everything is working fine:

    $ gdb -q ./select_asm02
    Reading symbols from ./select_asm02...done.
    (gdb) b select_target.asm:21
    Breakpoint 1 at 0x4000ed: file select_target.asm, line 21.
    (gdb) r
    Starting program: /mnt/mia/work/projects/security/p4w_malware/07/09/select_asm02
    Malware 0.1
    /tmp/
    Breakpoint 1, mfw_select_target () at select_target.asm:21
    21              call mfw_openat
    (gdb) n
    22              mov  QWORD [rbp-FD], rax                ;Store fd in local var
    (gdb) p $rax
    $1 = 5
    (gdb) n
    23              mov   rdi, [rbp-FD]
    (gdb) x/d $rbp - 8
    0x7fffffffe278: 5

In case something goes wrong, you will get a negative value on `rax`. That is actually `-errno`. Then you can just check the error codes using the command:

    $ errno -l
	
# Reading the directory
Now, it is time to start reading the directory, using our old friend `getdents`. If you remember the C code, we will have to run a loop and navigate multiple records in the buffer returned by the syscall. I will keep trying to make the code small. This roughly means that I will keep using registers while they are available, instead of local variables. This may change later, but for now we are still OK.

Still, we need some changes in the constants we defined before. Now, we need a buffer and some offsets to access the struct returned by `getdents`. These is how the constants have changed

```nasm
mfw_select_target:
	BUF_SIZE EQU    0x400           ; 1024 bytes buffer for getdents (see C version)
	FD	     EQU     0x08           ; Directory File Descriptor (stack index for local var)
	BUF      EQU     (FD + BUF_SIZE); The buffer is also in the stack as a local variable
	STE      EQU     BUF            ; Mark of end of local variables
	D_NAME   EQU     0x12           ; Offset to field d_name in struct dirent
	D_RECLEN EQU     0x10           ; Offset to field d_reclen in struct dirent
```

The opening and closing of the folder is exactly the same. But now we will have to do two loops. The first loop will call `getdents` until it returns 0 (i.e. the directory have been completely read), and then another loop to process all the entries returned for each call to `getdents`. Let's start with the first loop. This goes just after opening the directory:

```nasm
loop0:
	mov  rdi, QWORD [rbp-FD]
	lea  rsi, [rbp-BUF]
	mov  rdx, BUF_SIZE
	call mfw_getdents
	
	test ax,ax
	jz   done
	js  loop0
	mov  r9, rax
    ;; 
    ;; Second loop goes here
	;; (...)
done:	
	;; Close directory
```


This loop is pretty straightforward. We start calling `mfw_getdents`. First parameter is the file descriptor that we have stored in a local variable, second parameter is the address of the buffer (we use `lea` to load the address and not the value), and the third parameter is the size of the buffer that is just a constant.

`mfw_getdents` returns the number of bytes read in `rax`. We store that value in register `r9`, this is going to be our counter for the second loop. After that we check for zero (`jz` _Jump if Zero)) and negative number (`js` _Jump if Sign_). When we get zero we are done and we proceed to close the folder, and in case the value is negative (that's an error), we keep reading... but we should likely just stop the program. I leave up to you to check the possible errors returned by `getdents` to decide if it makes sense to keep trying or we should just abort.

I believe this is pretty clear, but just in case you are having a hard time following this, let's recall for a second the original C code:

```C
 while (1) {
    n = mfw_getdents (fd, buf, BUF_SIZE);
    if (n < 0) continue; // Silently ignore errors
    if (n == 0) break;
    (...)
    }
```

Now, let's change the variables by the registers and local variables of our asm

```C
 while (1) {
 loop0:
    RAX = mfw_getdents ([RBP-FD], [RBP-BUF], BUF_SIZE);
    if (RAX == 0) goto done;
    if (RAX < 0) goto loop0; // Silently ignore errors

    (...)
    }
```

Hope it is clear now.

# Printing the directory
Now we can write the second loop. As mentioned, this loop will iterate through the buffer returned by `mfw_getdents` printing the different folder entries. Let's see the code and then we can comment it.


```nasm
	lea r8, [rbp-BUF] 	    ; Points to struct linux_dirent record

loop1:	
	lea rdi, [r8 + D_NAME]	; d_name in rdi
	call mfw_putln

	mov dx, WORD [r8 + D_RECLEN] ; Get Record len
	sub  r9, rdx
	jz loop0                ; If it is zero, get more data
	
	;; Otherwise Update pointers
	add r8, rdx
	jmp loop1
```

In this piece of code we are going to use `r8` as our pointer to go through the different entries (`struct dirent`) in the buffer, so we initialise it to point to the first one, which is at the very beginning of the buffer. Then our loop starts.

First we print the current record. The `d_name` field is at offset `0x12` (that is the `D_NAME` constant we defined before), so we load that address on `rdi` in order to printy the name.

Then we recover the size of the record. Remember the records have variable length, because of the different sizes of the file names in the folder. The size of the record is at offset `0x10` (the `D_RECLEN` constant). We read that value in register `dx`. In this case we are using the 16 bits version of the register because that is the size of the value we are reading. Remember: `RDX` 64bits, `EDX`, 32bits, `DX` 16 bits.

Now that we now the size of this record, we can update our counter, which, if you remember is `r9`. Once we have updated the counter we have to check if we are done, that is, the counter is zero, so we just `JZ` after the substraction. 

In case there is still data to be processed we need to update or `struct dirent` pointer that we keep in `r8`. So we add the size of the current register and `r8` points now to the next register. You can refer to [this previous instalment](https://0x00sec.org/t/programming-for-wanabes-vii-finding-files-i/25662) for some diagrams of the memory layout o this structure.

Again, let's recall the original C code:

```C
   for (i = 0; i < n;) {
      de = (struct linux_dirent *)(buf + i);
	  puts (de->d_name);
      i += de->d_reclen;
```

In this case we have wrote the code a little bit different. So, let's rework a bit this C code before introducing the registers and local variables of the asm

```C
  // n containts the return from `getdents`
  register int len; // Auxiliar variable in an unused register
  de = (struct linux_dirent *) buf;
  while (1) {
     puts (de->d_name);
	 len = de->d_reclen;
	 n-=len;
	 if (!n) goto loop1; // more getdents
	 de = (struct linux_dirent*)((char *)de + len);
  }
```

And this is what we got:

```C
  r9 = n; // Return from `getdents`
  r8 = (struct linux_dirent*) buf

   while (1) {
     mfw_putln ((r8+D_NAME))
	 dx = *(r8+D_RECLEN)
	 r9 -= dx;
	 if (!r9) goto loop0;
	 r8 += dx
   }
```

Note that in C, due to the pointer arithmethic we need to first cast `de` to a char pointer so when we add `len` we add `len` bytes and not `len` `struct linux_dirent` elements. In assembler there is no pointer arithmetic so we just add the value. Now you can see why the original C code was more convenient.

# What did gcc did?
Just out of curiosity, let's see the code generated by gcc using the following flags:

    -fomit-frame-pointer -fno-stack-protector -Os



```nasm
0000000000000792 <select_target>:
 792:   41 56                   push   r14
 794:   41 55                   push   r13
 796:   48 89 f7                mov    rdi,rsi
 799:   41 54                   push   r12
 79b:   55                      push   rbp
 79c:   be 00 00 01 00          mov    esi,0x10000
 7a1:   53                      push   rbx
 7a2:   48 81 ec 00 04 00 00    sub    rsp,0x400
 7a9:   e8 11 ff ff ff          call   6bf <mfw_open>
 7ae:   49 89 e5                mov    r13,rsp        ;; r13 buffer
 7b1:   41 89 c4                mov    r12d,eax       ;; r12d FD
loop0:
 7b4:   ba 00 04 00 00          mov    edx,0x400
 7b9:   4c 89 ee                mov    rsi,r13
 7bc:   44 89 e7                mov    edi,r12d
 7bf:   e8 b6 fe ff ff          call   67a <mfw_getdents>
 7c4:   85 c0                   test   eax,eax
 7c6:   41 89 c6                mov    r14d,eax      ;; r14d bytes read
 7c9:   78 e9                   js     7b4 <loop0>
 7cb:   74 1e                   je     7eb <done>
 7cd:   31 db                   xor    ebx,ebx       ;; ebx counter
loop1: 
 7cf:   48 63 eb                movsxd rbp,ebx
 7d2:   4c 01 ed                add    rbp,r13       ;; buffer + i
 7d5:   48 8d 7d 12             lea    rdi,[rbp+0x12];; d_name offset
 7d9:   e8 20 ff ff ff          call   6fe <mfw_puts>
 7de:   0f b7 45 10             movzx  eax,WORD PTR [rbp+0x10] ;; eax = d_reclen
 7e2:   01 c3                   add    ebx,eax        ;; i += reclen
 7e4:   41 39 de                cmp    r14d,ebx       ;; i > n?
 7e7:   7f e6                   jg     7cf <select_target+0x3d>
 7e9:   eb c9                   jmp    7b4 <loop0>
done:
 7eb:   44 89 e7                mov    edi,r12d
 7ee:   e8 e3 fe ff ff          call   6d6 <mfw_close>
 7f3:   48 81 c4 00 04 00 00    add    rsp,0x400
 7fa:   31 c0                   xor    eax,eax
 7fc:   5b                      pop    rbx
 7fd:   5d                      pop    rbp
 7fe:   41 5c                   pop    r12
 800:   41 5d                   pop    r13
 802:   41 5e                   pop    r14
 804:   c3                      ret


```


Those are 114 bytes, against the 96 bytes of our version. No big difference. The code is pretty similar, actually, it is better to count up as in the original C code so we save an operation. This is a good exercise for you, to sharpen your just learnt assembly skills.

To conclude with this instalment, I will just add here the whole source code just in case you got lost at some point during the explanation:

```asm
	global mfw_select_target
	extern mfw_puts
	extern mfw_putln
	extern mfw_openat
	extern mfw_getdents
	extern mfw_close

	section .text
	
mfw_select_target:
	BUF_SIZE EQU    0x400
	FD	 EQU     0x08
	BUF      EQU     (FD + BUF_SIZE)
	STE      EQU     BUF
	D_NAME   EQU     0x12
	D_RECLEN EQU     0x10

	push  rbp
	mov   rbp, rsp
	sub   rsp, STE
	;; Open Directory
	;; RDI and RSI should be all set
	mov rdx, 0q200000 	;O_RDONLY | O_DIRECTORY
	call mfw_openat
	mov  QWORD [rbp-FD], rax		;Store fd in local var

loop0:
	mov  rdi, QWORD [rbp-FD]
	lea  rsi, [rbp-BUF]
	mov  rdx, BUF_SIZE
	call mfw_getdents
	

	test ax,ax
	jz   done
	js  loop0
	mov  r9, rax
	lea r8, [rbp-BUF] 	; Points to struct linux_dirent record

loop1:	
	lea rdi, [r8 + D_NAME]	; d_name in rdi
	call mfw_putln

	mov dx, WORD [r8 + D_RECLEN] ; Get Record len
	sub  r9, rdx
	jz loop0                ; If it is zero, get more data
	
	;; Otherwise Update pointers
	add r8, rdx

	jmp loop1
done:	
	;; Close directory
	mov rdi, QWORD [rbp-FD]
	call mfw_close
	leave     		; Set RSP=RBP and pops RBP
	ret
```

_Note: Our asm version, once stripped is just 816 bytes static binary._

# Conclusion
In this instalment we have learn how to write loops in assembly and also learned a few tricks from `gcc` just looking to the code it generates. We have learn how to deal with local variables and use structs in assembler and also how to create Makefiles and compile our assembly with debug information so we can find errors during the development.

Then we have just completed the first part of the `select_target` function that allows us to access all the files in a given directory. In the next instalment we will add the `stat` syscall so we can select the appropriated files for our malware.



## Read the whole series here
[Part VIII, File Details](https://0x00sec.org/t/programming-for-wanabes-viii-file-details/25738)
[Part VII. Finding files](https://0x00sec.org/t/programming-for-wanabes-vii-finding-files-i/25662)
[Part VI. Malware Introduction](https://0x00sec.org/t/programming-for-wannabes-part-vi-malware-introduction/25595)
[Part V. A dropper](https://0x00sec.org/t/programming-for-wannabes-part-v-a-dropper/23090)
[Part IV. The stack](https://0x00sec.org/t/programming-for-wannabes-part-iv/22421)
[Part III. Your first Shell Code](https://0x00sec.org/t/programming-for-wannabees-part-iii-your-first-shell-code/1279)
[Part II and a Half. Part II for ARM and MIPS](https://0x00sec.org/t/programming-for-wannabes-part-ii-and-a-half/1196)
[Part II. Shrinking your program](https://0x00sec.org/t/programming-for-wannabes-part-ii/1164)
[Part I. Getting Started](https://0x00sec.org/t/programming-for-wannabes-part-i/1143)
