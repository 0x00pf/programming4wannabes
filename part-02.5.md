# Programming for Wannabes. Part II and a Half

If you had read [Part II](part-02.md4) of this series you may have missed a couple of details. Consider this post as a short addendum to Part II including those details. 


The first you may have noted is that there was no ARM or MIPS code in there. Actually, the paper was already quite long and, to be honest, I thought it should be straightforward to repeat what we did for the x86 with any of those processors. However, I tried myself for the Lulz and I found some glitches that may be useful to mention.

So let's go with ARM

# ARM system calls
Calling a system call from ARM works is, conceptually, done in the same way that in the Intel processors. You have to set a specific register with the number of the system call you want to invoke and then get into kernel mode.

ARM defines 15 registers, named from `r0` to `r12`. The last 3 have special names and special functions, we will go into the details later in this course. For the time being we will only use the general purpose registers (those `r0` to `r12`).

So, the system call number goes into `r7`... yes, there is a reason, but it does not really matter now. Then the additional parameters needed by the system call goes into registers `r0` to `r5`.

With all this information and, taking into consideration that system calls follows the same numbering that the Intel 32bits, we can write our exit function like this:

```
.text
.globl _exit

_exit: mov r7, #1
       swi #0

```

As it happened with the Intel processor, the kernel follows the default processor C ABI. This means that when you call a C function, the first parameter goes int `r0` and when you call a system call the first argument also goes in `r0`. That's why we do not have to do anything to capture the parameter we pass to the `_exit` function from C.

I will reproduce the C code again, in here for your convenience:

```
#include <unistd.h>

int _start (void)
{
  int a = 10;
  int b = 20;

  a = a + b;
  _exit (a);
}

```

Now, we can proceed as we did with Intel, but we have to be aware that... `gcc` generates ARM 32bits opcodes and `as`, the assembler generates Thumb opcodes... at least that was what happen with my `gcc` and `as`. Thumb is 16bits long and... well you cannot just mix 16bits and 32bits opcodes directly. So there are two options to solve this problem.

* **Option 1**. Force 32bits passing the compiler the `-marm` flag
* **Option 2**. Mark the assembler code as a Thumb function, using the `.thumb_func` directive before the declaration of your function.

I tried both, but in this paper, let's use option 1... I haven't checked all the details for option 2 so I may be saying something stupid :P

```
arm-linux-gnueabi-gcc -static -fPIC -nostartfiles -nodefaultlibs -nostdlib -marm -o c2-3-arm c2-2.c exit_func-arm1.s
```

As you can imagine `exit_func-arm1` is the assembly code for option 1. The one we shown above.

Now you can test your program in your Android Phone or in any other ARM machine (BeagleBone Black, BananaPi, Olinuxino, RaspberryPi,...)... To check it in your phone take a look to this paper ([Improving your Android Shell](https://0x00sec.org/t/improving-your-android-shell/886) ).

# MIPS system calls
With MIPS I had a quite tough time. The toolchain for my test router used and old version of binutils and that caused me a lot of problems.

First one was that I couldn't use the names of the registers but its number. MIPS registers are named in a more complex way:


    $0	      	$zero	        Hard-wired to 0
    $1		$at		Reserved for pseudo-instructions
    $2  - $3	$v0, $v1	Return values from functions
    $4  - $7	$a0 - $a3	Arguments to functions - not preserved by subprograms
    $8  - $15	$t0 - $t7	Temporary data, not preserved by subprograms
    $16 - $23	$s0 - $s7	Saved registers, preserved by subprograms
    $24 - $25	$t8 - $t9	More temporary registers, not preserved by subprograms
    $26 - $27	$k0 - $k1	Reserved for kernel. Do not use.
    $28		$gp		Global Area Pointer (base of global data segment)
    $29		$sp		Stack Pointer
    $30		$fp		Frame Pointer
    $31		$ra		Return Address

(taken from http://www.cs.uwm.edu/classes/cs315/Bacon/Lecture/HTML/ch05s03.html)

We can see again how the last registers are used for special purposes. As I said, we will come to this in a future instalment. For now, we will just use `$v` and `$a` registers.

So, how do we invoke a system call on a MIPS processor?. Again, we have to put the system call in a register and go into kernel mode. The register for the system call is `$v0` or `$2` and the instruction to go into kernel mode is `syscall`.

In my case I found the syscall number, disassembling one of my test programs. Then I found that, at least for `SYS_exit`, this page seems to have the right numbers ( https://w3challs.com/syscalls/?arch=mips_o32 ).

If I told you that parameters, as stated in the table above, goes into `$aX`, then you should be able to write your exit function... something like this:

```
.globl _exit
.text
	
_exit:  li $2, 4001
	syscall
```

Let's go with the glitches I mentioned at the beginning. The first one is that... at least for my toolchain, the first function getting executed is `__start` instead of `_start`. It took me a while to realize that (even when the linker was complaining), those two underscores are difficult to see when it is late night. Therefore, we need to change our C code and change the name of our function from `_start` to `__start`.

The second one was really frustrating, and I haven't completely understood what the problem is. Apparently, for some reason that I do not know, my toolchain cannot compile static binaries. Any attempt to do that will produce a binary that crashes on my router. I have to do some experimentation, but for the time being this is a mystery.

So, for MIPS, at least for me, I couldn't go that far as I went for Intel and ARM. Even when the `exit` function gets substituted (actually I changed to name to something not in libc and the function got called properly) by our minimal system call. However, I didn't manage to get completely rid of the libc.

`$ mips-linux-uclibc-gcc -nostartfiles -o c2-2-mips c2-2-mips.c exit_func_mips.s`

Even though doesn't look like there is any libc dependency. `nm` only shows an undefined symbol:

```
$ nm n
00440414 A __bss_start
00400120 r _DYNAMIC
00440414 A _edata
00440414 A _end
004003c0 T _exit
00440414 A _fbss
004403e0 A _fdata
00400360 T _ftext
004403f0 A _GLOBAL_OFFSET_TABLE_
004483e0 A _gp
         U _gp_disp
004403e0 D __RLD_MAP
00400360 T __start
```

Anyway, I have found that this stuff becomes tricky with routers, specially if you do not have a toolchain that actually matches your router and also that, many of them run old and very stripped down version of linux that imposes additional constraints in the code.

Let's finish with some numbers as we did with intel:

    c2-3-arm  320 bytes  (ARM version)
    c2-2-mips 1.8 Kbytes (MIPS version

These values are after `stripping` the binaries. Not bad...

Well, this is it for ARM and MIPS... at least for me and for now :slight_smile: 

# Conclusions
We have done a short journey from the C realm to the kernel border and we have found quite a lot of stuff in between. It is interesting to understand all this... it changes a bit the way you see the programs running in your computer. We use to think that a C program is pretty low level and it is very fast and optimized. We have seen a bit of what it goes into a C program... now, just think about what is in there when using a scripting language... _So many CPU cycles_....

In next part we will come back to general programming... maybe...

* PREVIOUS: [Programming for Wannabes. Part II. Systemcalls](part-02.md)
* NEXT: [Programming for Wannabes. Part III. Your first Shell Code](part-03.md)
