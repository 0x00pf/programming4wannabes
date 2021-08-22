# Programming for Wanabes X. File details in asm
We have already code to scan a single folder and in this instalment we are going to extend it to scan complete folder trees and also get the details from the files so our malware can decide with file is interesting or not.

This is going to be pretty short as we already know everything needed to implement this extension.

# Refresher.... the code so far
In the previous instalment, towards the end I mentioned that using an ascending loop will have some benefits in this specific case so, I will include this modification in the base code.

You can check it as an exercise. It does exactly the same than the previous version, but counts from zero to the number of bytes returned by `getdents`, instead of decreasing that value until we get to zero.

This is the code.

```nasm
	global mfw_select_target
	extern mfw_puts
	extern mfw_putln
	extern mfw_openat
	extern mfw_newfstatat
	extern mfw_getdents
	extern mfw_close

	section .text
	
mfw_select_target:
	BUF_SIZE  EQU     0x400
	STAT_SIZE EQU     0x144
  	FD	  EQU     0x08
 	BUF       EQU     (FD   + BUF_SIZE)
	ST        EQU     (BUF + STAT_SIZE)
	STE       EQU     BUF
	D_NAME    EQU     0x12
	D_RECLEN  EQU     0x10
	ST_MODE   EQU     0x18
	
	;; Create Stack Frame
	push  rbp
	mov   rbp, rsp
	sub   rsp, STE
	
	;; Open Directory
	;; RDI and RSI should be all set
	mov  rdx, 0q200000 	;O_RDONLY | O_DIRECTORY
	call mfw_openat
	test al,al
	js   done1		; Exit if we cannot open the folder. Likely permission denied error
	
	mov  QWORD [rbp-FD], rax ;Store fd in local var
loop0:
	mov  rdi, QWORD [rbp-FD]
	lea  rsi, [rbp-BUF]
	mov  rdx, BUF_SIZE
	call mfw_getdents
	
	test ax,ax
	jz   done		    ; 0 means we are done reading the folder
	js   loop0 		    ; <0 means error.... we just try again

	mov r9, rax		    ; Loop limit
	lea r8, [rbp-BUF] 	; Points to struct linux_dirent record
	xor r14,r14 		; Loop counter = 0

loop1:
	lea   rdi, [r8 + r14  + D_NAME] ; Offset to current dirent name

;; ***********************************************
;; All new code goes here
;; *****************************************************
	;;  For the time being just print file name
	mov  rdi, rsi
	call mfw_putln

next:
	movzx rdx, WORD [r8 + r14 + D_RECLEN] ; Get Record len | Same size thqan mov
	add r14,rdx
	cmp r14, r9
	jge loop0                ; If it is zero, get more data
	jmp loop1
	
done:	
	;; Close directory
	mov rdi, QWORD [rbp-FD]
	call mfw_close
done1:	
	leave     		; Set RSP=RBP and pops RBP
	ret

```

Before continuing, you may have noticed the use of `movzx` instruction. This is new and we haven't talked about it before. This instruction and also it counterpart `movsx` allows us to read a value into a register that is smaller than the target register. Let's check the instruction

```nasm
	movzx rdx, WORD [r8 + r14 + D_RECLEN]
```

In this case we are moving a memory word (16 bits) into a 64 bits register. The `movZx` instruction will complete the target with zeros while the `movSx` will extend the sign. In this example, the value we want is 2 bytes, but we want to use it on the 64bits register for the arithmetic operations (actually the `edx` will likely be enough, but we would have to use the instruction in any case).

The difference between this instruction and a single move is that the last will not update the higher word on the register, and we should set the register to zero before copying only the lower 16bits. 

In the same way, if we are dealing with negative numbers...

# Negative numbers
So far we haven't care much about negative numbers... in a sense, we kind of magically assumed that they just work as it happens on C or any other high level language, however, there is a few things we need to know about number representation and its associated arithmetic.

Let's start thinking on a single byte (8 bits or 8 ones or zeros). As we know with 8 bits we can represent 256 values (from 0 to 255). That's perfect for natural numbers, but what happens if we need negative numbers?... And we need then, I can already told you that.

Well, in that case we need to encode the number differently. First thing is to store the sign of the number, and, that will take a bit.... I mean, it cannot take less... at least not without over-complicating the solution. Then if 1 bit is reserved for the sign, we have 7 bits to represent the actual number and that is 128 values. let's print a few of those numbers

    8 => 0 000 1000        -8 => 1 000 1000
	7 => 0 000 0111        -7 => 1 000 0111
	....
	1 => 0 000 0001        -1 => 1 000 0001
	0 => 0 000 0000         0 => 1 000 0000
	
So, we see a few problems with this representation. The first one is that we have two representations for the number zero. That is not convenient as can make computations ambiguous and we are also loosing the opportunity to represent one extra number.

The second problem of this representation is that multiplication is kind of easy, but addition is kind of a hell. 

Fortunately for us, some smart people long ago come up with a better representation for the negative numbers....

# 2-complements
This representation of the numbers also uses the most significant bit to indicate the sign, but the value of the number is encoded in a smarter way. Let's see our table of numbers again and then let's explore the benefits of this representation:

    8 => 0 000 1000        -8 => 1 110 1000
	7 => 0 000 0111        -7 => 1 111 1001
	....
	2 => 0 000 0010        -2 => 1 111 1110
	1 => 0 000 0001        -1 => 1 111 1111
	0 => 0 000 0000         

As we can see now there is one single representation for zero, that is actually zero (all bits zero). This has a consecuence... zero is somehow a positive number, because the most significant bit is 0 (that is our sign bit). This is why a signed char can take values from -128 to 127 (because the zero is part of the positives)

In addition to the sign, the rest of the number is constructed counting upward as usual for the positive numbers, and backwards for the negative ones... 

Actually the way to change the sign of a number, or if you prefer, calculate  the two's complement is as follows:

* Invert all bits in the number (this is the so called one's complement)
* Add 1

Let's use as example the number 5 and let's calculate the two's complement of it, or in other words, let's determine the bit representation of -5. 

    Number 5   ->  00000101
    NOT(5)     ->  11111010
	NOT(5) + 1 ->  11111011
	
The other big advantage of this representation is that basic arithmetic operation will just work. Just add the 5 and -5 above and it will result in zero. Substraction and multiplication also works out of the box. I won't go further into this topic. The interested reader shall read the [Wikipedia page](https://en.wikipedia.org/wiki/Two%27s_complement#Converting_to_two's_complement_representation), and if you are really into maths and scientific SW you also need to read [this](https://www.itu.dk/~sestoft/bachelor/IEEE754_article.pdf).

# Back to `movsx`
So, now that we know how a negative number is represented we can come back to the `movSx` where `S` stands for _sign_. This instruction works the same than `movzx` but performing what is know as sign extension.

Sign extension happens when copying some value of a specific datatype into another value but of a bigger datatype. Imagine you want to copy the value 7 in a byte in memory, into the 32 bits register `EDX`.

    EDX                                     Memory Byte
	XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX     00000111
	                                           |  (mov edx, BYTE [mem]
    XXXXXXXX XXXXXXXX XXXXXXXX 00000111 <------+
	
In the diagram above `X` means any value. It may be zero or one. When we move the byte into `EDX` we will just update the less significant byte... Anything else in the register will remind. However, when we use `movzx` we are forcing zeros in all the other bits in the register.... and when using `movsz` we are forcing the sign bit. Let's change the byte memory to some negative value

    EDX                                     Memory Byte
	XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX     11111110  (-2)
	                                           |  (movzx edx, BYTE [mem]
    00000000 00000000 00000000 11111110 <------+
	                                           |  (movsx edx, BYTE [mem]
    11111111 11111111 11111111 11111110 <------+

In the first case we copy the byte in the lower part of the register (actually `dl`) and then we set everything else to 0. En the second case we set everything else to the sign bit. This way, the result of the first case is 254 while in the second case it is still -2.

# Calling `statat`
Now we can just write the loop to call `statat` and check the file type. Let's split this in two parts. First the call to `statat` and then the check of the file.

The first part is pretty straight forward:

```nasm

loop1:
	lea   rdi, [r8 + r14  + D_NAME] ; Offset to current dirent name
	
	;; Skip . and .. names
	;; ---------------------------
	cmp WORD [rdi], 0x002e
	je next
	cmp WORD [rdi], 0x2e2e
	jne check_file
	cmp BYTE [rdi+2], 0
	je next
	
	;; Check file type and permissions
check_file:	
	lea    rsi, [rdi]	; Par2 : name
	mov    rdi, [RBP - FD]  ; Par1 : fd
	lea    rdx, [RBP - ST]  ; Par3 : struct stat
	xor    rcx, rcx		; Par4 : flags
	call   mfw_newfstatat
	
	test   al,al
	js     next		; Silently skip this file on error. Likely Permission denied
	
	;; ********************************
	;; Here the code to check the file
	;; *********************************
run_payload:
	;;  For the time being just print file name
	mov  rdi, rsi
	call mfw_putln

next:
	movzx rdx, WORD [r8 + r14 + D_RECLEN] ; Get Record len | Same size thqan mov
	(...)
```


The first part of the code just checks the file name and skips it in case it is `.` or '..' in order to avoid infinite loops. The check is done comparing against the ascii values (`0x0023` and `0x002323`) of both strings. For the second case, I first tried to read a `DWORD` to just do a comparison, but it looks like the size of `..` is exactly three and I was getting some randon stuff in the most significant byte... any other way to do the check I thought about just ended in longer code... but let me know in the comments if you found a better way.

Then we just found the call to `newfstatat`. Nothing special here, we have already used this from C, we just set the second parameter first, because we already have that value in `rdi`, so we just do that assignment fist before overwriting `rdi` and we take advantage of _moving_ data between registers.

Finally we check if the syscall failed and silently continue in that case.

# Checking file type
Now we need to check the file type. As we did in C, we are going to look for executable files. Note that this is a basic check and in the real world you may need to do further checks. For instance, a virus will need to check that the file is also an `ELF` binary and not just a bash script... both are executable files but their structures are pretty different. Even when it is possible to infect a `bash` file that is something you do not really need special skills to do.

This part of the code also performs the recursive call that allows us to scan the whole filesystem tree.

```nasm
	;; Check if it is a directory
	mov  eax, DWORD [rdx + ST_MODE]
	and  eax, 0q0170000
	cmp  eax, 0q0040000
	jz   scan_folder	; If it is a directory... scan recursively
	cmp  eax, 0q0100000 
	jnz  next               ; If it is not a regular file.... skip it
	
	;; If we got a regular file then let's check permissions
	mov  eax, DWORD [rdx + ST_MODE]
	and  eax, 0q00111	; Execution permisions
	jz   next		; If no execution permision set... skip the file
	jmp  run_payload	; Otherwise run the payload on it
	
scan_folder:
	;; Before the recursive call we need to store current state in the stack
	;; File descruptor and getents are already there. We just store the registers
	;; This way, we only use the memory when scanning a subfolder
	push  r8 		; Current getdents buffer
	push  r9		; Number of bytes in getents buffer
	push  r14               ; Current getendts buffer ofsset 

	call  mfw_select_target ; RDI and RSI already set to the right parameters
	;; Restore evertything and keep going
	pop   r14		; PUSH/POP are 2 bytes long... mov reg, [bp-XX] is 4
	pop   r9
	pop   r8
	jmp   next		; Continue
```

The first thing we do us to get the `st_mode` field from the `struct stat` returned by `newfstatat`. Then we mask the `__S_IFM` value that we have found when developing our C version and then we check if we are looking to a directory or a regular file. If the entry is a directory we jump to `scan_folder` to perform the recursive traversal of the just found subfolder, otherwise we check the permissions and if they don't match we just discard this entry and do on to process the next one.

When we call ourselves recursively to traverse the subfolders we need to store in the stack the local variables we are holding on registers for efficiency. These are `r8` (the `getents` buffer we are processing), `r9` (the number of bytes in that buffer) and `r14` (the current offset in the buffer of the entry we are processing right now).

We could declare extra local variables in the stack as we did for the `FD`, but in this case we decided to just push and pop the values just before the `call`. This way, the code is shorter and we only perform that operation (saving to memory) only when it is necessary. Note that a `mov` is principle more efficient (faster) but it produces a bit longer code (4 bytes vs the 2 bytes required by the `push/pop`).


# The final code

As usually, this is the final code of our `select_target` function:

```nasm
	global mfw_select_target
	extern mfw_puts
	extern mfw_putln
	extern mfw_openat
	extern mfw_newfstatat
	extern mfw_getdents
	extern mfw_close

	section .text
	
mfw_select_target:
	BUF_SIZE  EQU     0x400
	STAT_SIZE EQU     0x144
  	FD	  EQU     0x08
 	BUF       EQU     (FD   + BUF_SIZE)
	ST        EQU     (BUF + STAT_SIZE)
	STE       EQU     BUF
	D_NAME    EQU     0x12
	D_RECLEN  EQU     0x10
	ST_MODE   EQU     0x18
	
	;; Create Stack Frame
	push  rbp
	mov   rbp, rsp
	sub   rsp, STE
	
	;; Open Directory
	;; RDI and RSI should be all set
	mov  rdx, 0q200000 	;O_RDONLY | O_DIRECTORY
	call mfw_openat
	test al,al
	js   done1		; Exit if we cannot open the folder. Likely permission denied error
	
	mov  QWORD [rbp-FD], rax ;Store fd in local var
loop0:
	mov  rdi, QWORD [rbp-FD]
	lea  rsi, [rbp-BUF]
	mov  rdx, BUF_SIZE
	call mfw_getdents
	
	test ax,ax
	jz   done		; 0 means we are done reading the folder
	js   loop0 		; <0 means error.... we just try again

	mov r9, rax		; Loop limit
	lea r8, [rbp-BUF] 	; Points to struct linux_dirent record
	xor r14,r14 		; Loop counter = 0

loop1:
	lea   rdi, [r8 + r14  + D_NAME] ; Offset to current dirent name
	
	;; Skip . and .. names
	;; ---------------------------
	cmp WORD [rdi], 0x002e
	je next
	cmp WORD [rdi], 0x2e2e
	jne check_file
	cmp BYTE [rdi+2], 0
	je next
	
	;; Check file type and permissions
check_file:	
	lea    rsi, [rdi]	    ; Par2 : name
	mov    rdi, [RBP - FD]  ; Par1 : fd
	lea    rdx, [RBP - ST]  ; Par3 : struct stat
	xor    rcx, rcx		    ; Par4 : flags
	call   mfw_newfstatat
	
	test   al,al
	js     next		; Silently skip this file on error. Likely Permission denied
	
	;; Check if it is a directory
	mov  eax, DWORD [rdx + ST_MODE]
	and  eax, 0q0170000
	cmp  eax, 0q0040000
	jz   scan_folder	; If it is a directory... scan recursively
	cmp  eax, 0q0100000 
	jnz  next               ; If it is not a regular file.... skip it
	
	;; If we got a regular file then let's check permissions
	mov  eax, DWORD [rdx + ST_MODE]
	and  eax, 0q00111	; Execution permisions
	jz   next		    ; If no execution permision set... skip the file
	jmp  run_payload	; Otherwise run the payload on it
	
scan_folder:
	;; Before the recursive call we need to store current state in the stack
	;; File descruptor and getents are already there. We just store the registers
	;; This way, we only use the memory when scanning a subfolder
	push  r8 		; Current getdents buffer
	push  r9		; Number of bytes in getents buffer
	push  r14       ; Current getendts buffer ofsset 

	call  mfw_select_target ; RDI and RSI already set to the right parameters
	;; Restore evertything and keep going
	pop   r14		; PUSH/POP are 2 bytes long... mov reg, [bp-XX] is 4
	pop   r9
	pop   r8
	jmp   next		; Continue
	
run_payload:
	;;  For the time being just print file name
	mov  rdi, rsi
	call mfw_putln

next:
	movzx rdx, WORD [r8 + r14 + D_RECLEN] ; Get Record len | Same size thqan mov
	add r14,rdx
	cmp r14, r9
	jge loop0                ; If it is zero, get more data
	jmp loop1
	
done:	
	;; Close directory
	mov rdi, QWORD [rbp-FD]
	call mfw_close
done1:	
	leave     		; Set RSP=RBP and pops RBP
	ret

```

# Conclusions
As I said this was very short, we already know a lot of assembler and system programming to code this part so this was just work we had to do. We took the chance to talk a little bit about number representation, and we got a recursive function working in assembly... which is not that hard once we learned what that involves in the previous instalments.

For the next instalment we should start looking into some payload.... I envision a first theoretical instalment to introduce the related concepts before jumping in to the code....

So, now it is time for you to decide:

[poll type=regular results=always chartType=bar]
* VIRUS
* RANSOMWARE
* SPYWARE
* RAT
[/poll]


## Read the whole series here
[Part IX. Finding Files in asm](https://0x00sec.org/t/programming-for-wanabes-ix-finding-files-in-asm/25794)
[Part VIII, File Details](https://0x00sec.org/t/programming-for-wanabes-viii-file-details/25738)
[Part VII. Finding files](https://0x00sec.org/t/programming-for-wanabes-vii-finding-files-i/25662)
[Part VI. Malware Introduction](https://0x00sec.org/t/programming-for-wannabes-part-vi-malware-introduction/25595)
[Part V. A dropper](https://0x00sec.org/t/programming-for-wannabes-part-v-a-dropper/23090)
[Part IV. The stack](https://0x00sec.org/t/programming-for-wannabes-part-iv/22421)
[Part III. Your first Shell Code](https://0x00sec.org/t/programming-for-wannabees-part-iii-your-first-shell-code/1279)
[Part II and a Half. Part II for ARM and MIPS](https://0x00sec.org/t/programming-for-wannabes-part-ii-and-a-half/1196)
[Part II. Shrinking your program](https://0x00sec.org/t/programming-for-wannabes-part-ii/1164)
[Part I. Getting Started](https://0x00sec.org/t/programming-for-wannabes-part-i/1143)
