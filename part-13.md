# Programming for Wanabes XIII. Crypters part I

This is the first instalment to dive you deep into the awesome world of crypters. You will learn everything about these little guys which are a usual component of different types of malware, including the RATs that we are discussing right now.

Technically, crypters, in their traditional form, are closer related to virus, than to any other malware, in the sense that they require in deep knowledge of the binary format of the target system (ELF, PE,..), but this looks like a good time to start digging into them and add some spice to our RATs.

# What's a crypter?
Strictly speaking, a crypter is a program able to encrypt a binary (another program) and modify it so it will decrypt itself at run-time. In practice, the part of the modified program that actually decrypts the code at run-time is also often known as crypter. I prefer to refer to this last one as _stub_ or _crypter stub_, but you will see some people naming it just crypter.

The crypter, usually encrypts the code of the program, that is, the executable sections and optionally, the read-only section where stuff like literal strings ends up (so `strings` will show nothing). 

I will mostly talk about binary crypters, however, the concept as defined in previous paragraphs, can also be applied to scripting languages or bytecode-based platforms (like Java or .NET CLR). However, in these cases the term obfuscator is often used (and I would say more correct), specially for the scripting language, as many times, the code is not really crypted but just made hard to read. 

For scripting language the whole thing ends up orbiting around the specific `eval` function provided by your language of choice. Most scripting language have such a function (`eval` is usually the name) that just executes a text string. 

For bytecode related platforms, the crypter may have to make use of the so-called reflection API provided by the platform. This is in general pretty straightforward from a technical point of view, and fully documented in the official documentation of the platform, but if you want to get this things covered, let me know in the comments. 

Both cases are (in the general case) simpler than crypting a binary, so we will focus on that case... the hard one :).

Overall, I will try to deal with the concept in a general way, so you will learn the basics in a way such as you should be able to tailor what you have learned in this instalment (and the next ones) to your specific needs. 

# How does a crypter works?
A crypter actually works like a virus. It crypts selected parts of the binary and then it injects some code and makes sure it gets executed before anything else in that binary. The last part very much resembles what a virus does... doesn't it?

So, the crypter program behaves a bit like a virus whose payload crypts the binary. For doing that, the crypter has to:

* Find the parts of the binary to encrypt (usually `.text` and `.rodata` sections)
* Crypt them
* Inject the `stub` in the binary
* Patch the `stub` and the binary header to ensure the `stub` is properly executed at run time.

Note that the `stub` gets executed once the program has been fully loaded, so all code is already in memory. We will go into all the gory details a bit later, but for now, let's look at the big picture.

_Note:You may crypt shared libraries that you manage yourself. That is actually an exception to the previous sentence, but it is somehow a subset of what we are going to discuss here._

That is what the crypter does. 

The crypter's `stub` has to do some other things: 

* Change the memory permissions so it can write into those blocks. This is done using the `mprotect` system call. We will usually crypt the `.text` and `.rodata` sections that do not have write permission when loaded by the system.
* If the selected algorithm cannot be applied directly in that memory area (because, for instance, it works on data blocks/chunks), it may need to allocate memory to copy the data to decrypt or at least some buffer to work with.
* Then it can decrypt the block of memory
* And finally give control to the Original Entry Point (aka `OEP`), that is, literally the original entry point before running the crypter on our binary.... So, yes, we need to store it somewhere.

Even when technically it is not necessary, malware will likely set the permision back to its original configuration, so there is nothing strange about that program, once it has been decrypted and is running normally.

Let's get started getting our cipher ready.

# Cryptography 101

Yes, I said we will dive deep into the topic, so we need to know a little bit about cryptography, or in other words, how to crypt and decrypt sequences of bytes.

Cryptography is used on different security aspects:  authentication, privacy, data integrity, non-repudiation. In the case of a crypter the relevant aspect is privacy. In other words, avoiding non-authorised people to peek into our code. At least not easily.

There are different cryptography algorithms to transform a sequence of bits in another sequence of bits different of the original one, but in a way that we can recover the original sequence, using some private data (known as key). That's mostly it about pivacy... pretty easy right?.

You can think on different alternatives to achieve that goal. For instance, just adding a value to each byte, will completely transform the sequence and we can easily recover the original message just substracting that same value. This is actually one of the simplest ciphers ever and it is know as [Caesar's Cipher](https://en.wikipedia.org/wiki/Caesar_cipher). 

Mathematically speaking, cryptography algorithms use what is known as [trapdoor functions](https://en.wikipedia.org/wiki/Trapdoor_function). Those are functions that are very easy to evaluate in one direction but very difficult to calculate its inverse, unless you have some extra information. That information is known as _trapdoor_ or _key_.  At least, this is the basis for the more advanced cryptography algorithms.

Look at our Caesar's cipher.... encrypting the message is very easy. We just add a number to each byte. Recovering the original message is also easy if we know the _trapdoor_, in this case the number we added to each byte of the original message. But if we do not know that, we have to go brute-force and try all the 255 possibilities.... And now, it should be obvious to you why this is a weak cipher :).

_Note: The Caesar cipher, doesn't really fit into the mathematical concept of trapdoor function, but it is good enough to illustrate the concept. In real world we will be talking about more complex things like the [Euler's totient function](https://en.wikipedia.org/wiki/Euler%27s_totient_function)._

That is the key of cryptography. Overall, all codes can be broken, the trick is that breaking them will take that long that it makes no sense to even try... we are talking about millions of years here. Actually this is also the key to obfuscation.... anything can be reversed, the point is that the effort to do that reversing doesn't pay off or won't be done according to some time frame constraint.

Keep this in mind, because these time is not engraved on stone. Some cryptography algorithms where considered very safe years ago, when the computers had a very limited power. For those computers breaking those old algorithms will take hundred or thousands of years, but that same algorithm may be broken in days or hours with new hardware... In other words, what is extremely safe right now, may be weak in a few years.

And this is enough for now, we do not need to talk about symmetric and asymmetric encryption (but I have just introduced the terms for the more curious of you), that will come up naturally when we talk about ransomware in the future :).

To finish with this quick introduction to cryptography, let's quickly talk about the family of algorithms we are going to use.

# Xor Encoding
Right now you may be thinking : _This guy is kidding me?....That xor thingy is pretty basic._.... You will be surprised my friend. I'll show you.. you just need to keep reading.

Xor encoding is very straightforward, and indeed has its drawbacks, but implemented carefully is a pretty decent cipher. 

I'm sure you already know how it works, but just in case here is a quick description:

    For each byte in the plain text
	  encrypted_value = plain_value ^ Key_Value

Where `^` is the C operator for `xor`. We can extend this for a key longer than 1 byte. Let's switch to C code:

```C
char key[KEY_LEN];
char msg[MSG_LEN];
int  i, j;         // Indexes for msg and key respectively

for (i = 0, j = 0; i < MSG_LEN; msg[i++] ^= key[j++], j%=KEY_LEN) ;
```

We have used the comma `,` operator above to make the code shorter. It just executes the statements in order and returns the result of the last one. Together with `for` (that is just syntactic sugar for `while`) our code gets more compact (not necessarily shorter). In case you got a bit confused, the `for` loop above is equivalent to:

```C
i = 0;
j = 0;
do {
  msg[i] = msg[i] ^ key[j];
  i++;
  j++;
  j = j % KEY_LEN;
} whle (i < MSG_LEN);
```

Right?. The module operator in j is just to reset the key every time we exhaust the key values, so we can crypt messages longer than the key, just repeating the key over and over.

Now, if we make `KEY_LEN = MSG_LEN` we get a pretty hardcore cipher known as [one-time pad](https://en.wikipedia.org/wiki/One-time_pad) or _Vernam Cipher_. This cipher is actually impossible to break if the following four rules are met (this is taken literally from the wikipedia article referenced above):

1. The key must be random (uniformly distributed and independent of the plaintext), and sampled from a non-algorithmic, chaotic source such as a Hardware random number generator.
2. The key must be at least as long as the plaintext.
3. The key must never be reused in whole or in part.
4. The key must be kept completely secret by the communicating parties.

So... yes, Xor encrypting is pretty basic, but with the code we have wrote so far we can already implement a few algorithms... basic xor cipher, Vernam Cipher and... RC4

# RC4. Initialisation
RC4 is a cryptography algorithm designed by Ron Rivest. He is the `R` on `RSA`, and according to some people also the `R` on `RC4` _Rivest Cipher 4_ but for other is an acronym of _Ron's Code 4_, anyway, if you like more the last name or the first name, it doesn't make a difference :). This cipher is still very simple, so we can implement it easily in asm (yeessss... we will do that), but it is also a real algorithm used in real life, so... not more trivial `memfrob`s in our code.

RC4 is just a xor encoder that uses a permutation matrix, initialised using the key, to generate a sequence of values to actually xor encode the message.... It can be seen as a Vernam algorithm where a pseudo-random number sequence, generated from the given user key, is used as the encryption key (with size equal to the plain text) for the message.

_Note that generating the sequence this way actually breaks rule one to make this code unbreakable..._

A permutation matrix is a common concept in many ciphers and usually is represented by the letter `S` (you will see that if you check other ciphers). Basically, it is a matrix that allows us to change the order of a given set. If you ever tried to program a card game, you will have come through this concept to shuffle the cards in the deck...

The first part of the algorithm is known as _Key-Scheduling Algorithm_ or KSA. 

> _An algorithm here is just a sequence of mathematical operations we have to do in a certain order to achieve some goal. In this case encrypting a message._

This algorithm just shuffles the permutation matrix based on the user provided key. The permutation matrix (vector actually) is initialised to identity.  Identity is just mathematical jargon for saying **ONE** for _Maths things_ that involve many numbers like a matrix or a vector, in this case it just set all elements of the vector such as `S[i] = i'. As a permutation matrix, this means that a set going through that matrix will not change (will return the 255 elements in order 1 returns 1, 2 retursm 2, and so on)... which is what happens when we multiply a number by 1. Enough about algebra. I promise.

Then, the permutation matrix is shuffled 255 times. The whole KSA can be implemented with the C code below:

```C
  for (i = 0; i < 255; S[i] = i,i++);
  for (j = 0, i = 0; i < 256; i++) {
    j = (j + S[i] + key[i % klen] ) % 256;
    SWAP(S[i],S[j]);
  }
```

Where `SWAP` is a macro that swaps the values of the two variables passed as parameters. 

There are different ways to swap two values. In this case, I chose the addition/substraction technique that doesn't requires an intermediate variable:

```C
#define SWAP(a,b) a += b; b= a -b; a-=b;
```

After this step, our permutation matrix `S` is in a state that depends on the selected key. Now is time to crypt our data.

## Modulo Operator
In case you are not familiar with the modulo operator, here is a short introduction as we will be using it all over the place. The modulo operator calculates the remainder of the integer division of its two parameters. Let's name `N` the number we want to divide (dividend) and `D` the number we are dividing by (the divisor). Let's name `C` the result of the integer division and let's name `R` the remainder of the operation.

As you may remember from school: `N / D = C + R`

The modulo operator just return the value `R`. It is normally used to bound the range of values of a given number. Let' see an example:

    0 % 5 = 0   (0/5 = 0)
    1 % 5 = 1   (1/5 = 0 + 1)
    2 % 5 = 2   (2/5 = 0 + 2)
    3 % 5 = 3
    4 % 5 = 4
    5 % 5 = 0   (5/5 = 1 + 0)
    6 % 5 = 1   (6/5 = 1 + 1)

As you can see if we calculate `% 5` of any number we are just keeping that number between `0` and `4`. So, `% 256` keeps that number between `0` and `255` which is actually a byte. Later on, we will see how to take advantage of this fact... actually the algorithm was designed on-purpose to work on bytes. Thanks Mr Rivest!

> _Many cryptography algorithms are designed to work on bit streams... which are convenient for hardware implementation but may be a bit tricky to implement on SW_.

As a final note, implementing the modulo operation is not straight forward, however there are simplified versions when we divide by powers of two... We will come to this later.

# RC4. Crypting
The crypting process is also very simple, it basically uses the permutation matrix to generate a pseudo-random sequence of values, which will be `XOR`ed with the plain text. Seen like that, it is a _Vernam Cipher_ where the key is generated automatically instead of randomly selected... This actually breaks the first rule to make _Vernam Cipher_ impossible to break (have I alread said that?).... so RC4 can be broken.

The RC4 Pseudo-Random Generator together with the basic xor cipher uses the following code:

```C
  i = j = 0;
  int cnt = 0;
  while (cnt < MSG_LEN) {
    i = (i + 1) % 256;
    j = (j + S[i]) % 256;
    
    SWAP(S[i],S[j]);
    
    msg1[cnt++] = msg[cnt] ^ S[ (S[i] + S[j]) % 256];
  }
```

Where `msg` is the plain text and `msg1` is the buffer that will contain the result crypted message. Actually for RC4 we can decrypt in the same buffer that contains the plain text. In the code above, the value `S[(S[i] + S[j]) % 256]` is the so-called pseudo-random sequence generated by the RC4 pseudo-random generator. I have just compacted both operations (the pseudo-random number generation and the xor cipher) in one line.

## Pseudo-Random Numbers?
We talk about pseudo-random number generation because the numbers generated this way, are not really random... actually there is a correlation between them because, they are generated through an operation on previous values. Knowing the so-called _Seed_ of the generator (in this case this is actually knowing the key) is possible to regenerate the whole sequence... Something that doesn't happen with a real random sequence (that is what independent means statistically speaking). 

Modern OSes uses [entropy pools](https://en.wikipedia.org/wiki/Hardware_random_number_generator#Using_observed_events) to produce better random sequences (from an statistical point of view), specially when compared to the old [linear congruential generators](https://en.wikipedia.org/wiki/Linear_congruential_generator) used in the early stages (old TCP spoofing was exploiting that). For doing that, they measure different things like keyboard or mouse activity, disk accesses, etc... and uses those values to produce random sequences. It is very easy to produce pseudo-random numbers but very difficult to produce real random ones. YOu can look for the difference between `/dev/random` and `/dev/urandom` to learn more about this topic... ever wonder by `ssh` ask you to press keys and move the mouse when generating your keys?...

RC4 is a real world algorithm. It is used by the WEP authentication algorithm for wifi networks, or by the Microsoft Point-to-Point Encryption and was used to encrypt PDF until version 2.0 of that file format. It is not that popular neither powerful nowadays, but it is still a real world example, and you can still see it on the wild.

# An RC4 crypter
Now that we have selected our cipher, it is time to write the crypter. Let's start writing some general pseudo-code and work it out from there:

     Open and map binary
     Find the file parts to crypt
     Crypt it
     Insert stub
     Patch Entry-Point

So this is pretty straightforward, however each of these steps will require some explanation, so we will go through this list one by one and I will try to explain all the details about each step.

_Note:Many of the things that come now have already been described in other articles from me, but I will re-write them here just for completeness. If you have already read those, there is not much new for you in the next sections._

## Open and Map Binary
The first thing we need is to be able to access the content of the binary, the actual file, because we need to modify it. We can open the file, allocate memory in the heap, load the data, modify it and finally wrote it back into the file, or we can open the file and _memory map_ it. The advantage of `mmap`ing the file is that, in one shot, we read the file in memory and any change in memory will just be automatically written back to the file when we close it.

In order to map the file we need its size, that we can obtain using the `fstab` system call. You should known it from previous instalments.

```C
#define DIE(s) {perror(s);exit(1);}

int main (int argc, char *argv[]) {
  
  if (argc != 2) {
    fprintf (stderr, "Invalid number of parameters\n");
    fprintf (stderr, "Usage: crypter binary\n");
    exit (-1);
  }
  // Open file
  int fd;
  if ((fd = open (argv[1], O_RDWR, 0)) < 0) DIE ("open");
  // get size
  struct stat _st;
  if (fstat (fd, &_st) < 0) DIE ("fstat");
  // Map file
  unsigned char *p;
  if ((p = mmap (0, _st.st_size, PROT_READ | PROT_WRITE,
		 MAP_SHARED, fd, 0)) == MAP_FAILED) DIE ("mmap");
```

All this code should be very straightforward to you. We are actually just executing three system calls in a row. `open` and `fstat` we have already used many times. `mmap` is new so let's explain what it does.

`mmap` does a map into virtual memory. What does it maps?, well, usually it either maps physical memory, and then `mmap` is used to allocate memory, or it maps a file, and then `mmap` can be used to access the file as it is where stored in memory (this can also be used as shared memory between processes). In general, `mmap` may be supported by any block device driver... they just need to implement the functionality in the associated driver, what not always makes sense.

Let's go through all the parameters (double check the man page for further details):

* First parameter is the memory address we want to map the memory into. When we pass 0, we just tell `mmap` to chose a suitable address for us. In our case, we do not care about the address where our file is mapped. This address needs to be page aligned... in case you want to specify one.
* Second parameter is obviously the size of the block. Note that, even when you can provide any number for this parameter, `mmap` will map memory in multiples of the page size.... that means that, unless our file has a size multiple of the page size, the last page will be just partially used.
* Third parameter are permissions. In this case we want to be able to read and to write...
* Forth parameter are flags. Check the man page for a exhaustive list. In our case we just use the flag `MAP_SHARED` that means, that our changes in the memory will be written back to the file when we are done.
* Fifth parameter is a file descriptor for the device we want to map. In this case it is just a file so we pass the file descriptor we obtained from `open`
* Sixth parameter allows us to specify an offset in the device to start the mapping., In this case we want to map the whole file. Actually, most of the information we need is at the very beginning of it, in the ELF header.

So far so good.


## Find the code
Now we need to find the part of the program that we need to crypt. For doing this, we need to dive into the details of the ELF format. For a detailed description I recommend you to check [this](https://0x00sec.org/t/dissecting-and-exploiting-elf-files/7267) and also the [ELF spec](http://refspecs.linuxbase.org/elf/elf.pdf)... that eventually will become your best friend :).

The spec is quite extensive, but for our current discussion we just need to focus in two structures: The ELF header and the Section Table.

The ELF header is found at the very beginning of the file and contains the information needed to get us to the Section Table that give us infomation about what part of the file is code, what is data, and so on.

Let's first look at the header

```C
typedef struct
{
  unsigned char	e_ident[EI_NIDENT];	/* Magic number and other info */
  Elf64_Half	e_type;			/* Object file type */
  Elf64_Half	e_machine;		/* Architecture */
  Elf64_Word	e_version;		/* Object file version */
  Elf64_Addr	e_entry;		/* Entry point virtual address */
  Elf64_Off	    e_phoff;		/* Program header table file offset */
  Elf64_Off	    e_shoff;		/* Section header table file offset */
  Elf64_Word	e_flags;		/* Processor-specific flags */
  Elf64_Half	e_ehsize;		/* ELF header size in bytes */
  Elf64_Half	e_phentsize;	/* Program header table entry size */
  Elf64_Half	e_phnum;		/* Program header table entry count */
  Elf64_Half	e_shentsize;	/* Section header table entry size */
  Elf64_Half	e_shnum;		/* Section header table entry count */
  Elf64_Half	e_shstrndx;		/* Section header string table index */ 
} Elf64_Ehdr;
```

This is taken from `/usr/include/elf.h` and it is the header for 64bits ELFs (I won't deal with 32bits in this series... you can do that yourself as an exercise).

The first fields can be used for some sanity check that we are going to skip (just to keep this short), so the fields that we are interested on are actually the ones below:

```C
  Elf64_Off	    e_shoff;		/* Program header table file offset */
  Elf64_Half	e_shentsize;	/* Program header table entry size */
  Elf64_Half	e_shnum;		/* Program header table entry count */
  Elf64_Half	e_shstrndx;		/* Section header string table index */   
```

With this new information we can add some technical output to our crypter, that will make it look cooler:

```C
  // Find code segment
  Elf64_Ehdr *elf_hdr = (Elf64_Ehdr*) p;
  // Sanity checks oimitted
  printf ("Section Table located at : %ld\n", elf_hdr->e_shoff);
  printf ("Section Table entry size : %ld\n", elf_hdr->e_shentsize);
  printf ("Section Table entries    : %ld\n", elf_hdr->e_shnum);
```

Remember that `p`, is the pointer returned by `mmap` and also remember that the ELF header is just at the beginning of the file, so if we just cast `p` into and `Elf64_Ehdr` pointer we can read the data at `p` using that structure. Now we just need to go through the _Section Table_ and find the sections we are interested on.

> **Short digression on casting**
> Casting a variable in programming means forcing the type of that variable. As you already now, it doesn't matter what type you assign to your variable in your program, it ends up as a series of bytes in memory. How are those bytes interpreted by a programming language depends on the type (which is something programming language dependant). But we can force the language to interpret the bytes at some memory position in a different way... That is what a cast operator in C actually does.
> 
> Imagine that, at position `ADDR` in memory we find the values: 0x41, 0x42, 0x43, 0x00. If we cast a pointer to that address to be a `char` pointer, we will see the string `ABC`, but if we cast it to be a `int` type, we will see the value 0x00434241 (for a little endian processor), but if we cast it to a pointer to `u16_t` we will read 0x4241 and if we read the next value we will read 0x0043... The values in memory are always the same, but depending how do we interpret them in our programming language some things may be easier or harder to do. 
> 
> Remember.... types are just an illusion \ö/

With this information we can now traverse the _Section Table_ with a couple of lines:

```C
  int i;
  Elf64_Shdr *sh = (Elf64_Shdr*)(p + elf_hdr->e_shoff) ;
  for (i = 0; i < elf_hdr->e_shnum; i++) {
   (... do your thing ...)
  }
```

The piece of code above shows the wonders of pointers that only C offers....

## Processing the Section Table

We will have to discuss this a bit more in detail later, but for the time being, we will rely on the section table to find the relevant parts of the binary to encrypt. When crypting our own programs, that is not an issue as we can generate the program normally, including all the normal sections, and strip it out later. However, this won't work on an heavily stripped binary.

If we use the utility `sstrip` on the binary, one of the things it does is to wipe out the section table... which is actually not needed to run the program, and therefore our code to look for specific sections won't work. In that case, it is not that straightforward to figure out which part of the binary is code and which not... But we will come back to this later... Let's continue with the nominal case.

_Note, if you check a few binaries in your system you will see that most of them (if not all) contains their section table. You can use `readelf -S` for checking that, or keep reading and use your own tool :)._

Before, unveiling what we need to do in the loop, let's take a quick look to the Section Table entry structure. We can get it, again, from `/usr/include/elf.h`:


```C
typedef struct
{
  Elf64_Word    sh_name;                /* Section name (string tbl index) */
  Elf64_Word    sh_type;                /* Section type */
  Elf64_Xword   sh_flags;               /* Section flags */
  Elf64_Addr    sh_addr;                /* Section virtual addr at execution */
  Elf64_Off     sh_offset;              /* Section file offset */
  Elf64_Xword   sh_size;                /* Section size in bytes */
  Elf64_Word    sh_link;                /* Link to another section */
  Elf64_Word    sh_info;                /* Additional section information */
  Elf64_Xword   sh_addralign;           /* Section alignment */
  Elf64_Xword   sh_entsize;             /* Entry size if section holds table */
} Elf64_Shdr;

```

Once again, we are only interested in a few fields for our task at hand:

* `sh_name`. This field will allow us to get the section name. We will see in a second how to get those names
* `sh_offset` which tell us where in the file is the data associatd to this section. In other words, the offset on the file to the data we have to crypt.
* `sh_size` and this is the size of the section or if you prefer the size of the data we have to encrypt.
* `sh_flags` this contains different flags for the section, including the associated permissions...

All the fields are pretty easy to interpret, except the `sh_name`. That is because all strings are stored in a special section. This way, it is easy to deal with duplicated strings and also a smart way to keep all other structures with a fixed size, so they can be navigated as arrays. Anyhow, the section that contains the string is special and is included in the header:

```C
  Elf64_Half	e_shstrndx;		/* Section header string table index */
```

And this works like any other section, the unique difference is that the offset associated to that section points to the area on the file that contains all the strings associated to the ELF format itself. Yes, in this section you will find the name of sections and symbols used by the binary, but not the strings you use in your program... Your `"Hello World!"` string is stored in the `.rodata` section (_Read-Only Data_)... That, `.rodata`, is one of the sections we will be interested on encrypting... I'm not even sure if crypting the string table section will break the binary.... that's something to try.

Taking this into account, the field `sh_name` in the section structure, is just an offset in that area that contains the strings, so, in order to get the name of section `i`, we have to:

```C
  char *s_name   = p + sh[elf_hdr->e_shstrndx].sh_offset; // String Table Data
  char *sec_name = s_name + sh[i].sh_name;                // Just access using the offset
```

## Crypting code and data

With this information we can write a preliminary version of our main loop. Let's include all the code so you can compile and try it:

```C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <elf.h>

#define DIE(s) {perror(s);exit(1);}
#define SWAP(a,b) a += b; b= a -b; a-=b;

int rc4 (unsigned char *msg, int mlen, unsigned char *key, int klen) {
  int           i,j;
  unsigned char S[256]; // Permutation matrix
  
  // KSA: Key-Schedulling Algorithm
  for (i = 0; i < 255; S[i] = i,i++);   
  for (j = 0, i = 0; i < 256; i++) {
    j = (j + S[i] + key[i % klen] ) % 256;
    SWAP(S[i],S[j]);
  }
  // Encoding
  i = j = 0;
  int cnt = 0;
  while (cnt < mlen) {
    i = (i + 1) % 256;
    j = (j + S[i]) % 256;
    
    SWAP(S[i],S[j]);
    
    msg[cnt] = msg[cnt] ^ S[(S[i] + S[j]) % 256];
    cnt++;
  }
  printf (" [%d bytes encoded]", cnt);
  return 0;
}

int main (int argc, char *argv[]) {
  
  if (argc != 2) {
    fprintf (stderr, "Invalid number of parameters\n");
    fprintf (stderr, "Usage: crypter binary\n");
    exit (-1);
  }
  
  // Open file
  int fd;
  if ((fd = open (argv[1], O_RDWR, 0)) < 0) DIE ("open");
  
  // get size
  struct stat _st;
  if (fstat (fd, &_st) < 0) DIE ("fstat");
  
  // Map file
  unsigned char *p;
  if ((p = mmap (0, _st.st_size, PROT_READ | PROT_WRITE,
		 MAP_SHARED, fd, 0)) == MAP_FAILED) DIE ("mmap");
		 
  // Find code segment
  Elf64_Ehdr *elf_hdr = (Elf64_Ehdr*) p;
  // Sanity checks omitted
  printf ("Section Table located at : %ld\n", elf_hdr->e_shoff);
  printf ("Section Table entry size : %d\n",  elf_hdr->e_shentsize);
  printf ("Section Table entries    : %d\n",  elf_hdr->e_shnum);  

  int        i;
  Elf64_Shdr *sh = (Elf64_Shdr*)(p + elf_hdr->e_shoff) ;
  char       *s_name = p + sh[elf_hdr->e_shstrndx].sh_offset;
  char        *key ="0x00Sec!\0";  // Use 8 characters to make asm simpler.
  char        *name = NULL;

  for (i = 0; i < elf_hdr->e_shnum; i++) {
    name = s_name + sh[i].sh_name;
    printf ("Section %02d [%20s]: Type: %d Flags: %lx | Off: %lx Size: %lx => ",
	    i, name,
	    sh[i].sh_type, sh[i].sh_flags,
	    sh[i].sh_offset, sh[i].sh_size);
    //Find `.text` and `.rodata`
    if (!strcmp (name, ".text") || !strcmp (name, ".rodata")) {
      // encrypt section
	  rc4 (p + sh[i].sh_offset, sh[i].sh_size, (unsigned char*)key, strlen (key));
      printf (" - Crypted!\n");
    } else printf ("\n");
  }

  // TODO: Inject stub here
  munmap (p, _st.st_size);
  close (fd);
  return 0;
}

```

This small program, will list all the sections in the binary and find `.text` and `.rodata` as targets to encrypt, and actually encrypt them using the RC4 cipher we wrote earlier in this instalment.

Now you can run the program against any binary (actually better try it against a copy not the real one) and see what happens. As an example I will use my beloved `xeyes`.

Let's first make a copy in our worked directory, and dump the beginning of the `.text` section.

    $ cp /usr/bin/xeyes .
    $ objdump -d xeyes  | grep -A10 "<.text>"
    0000000000001ed0 <.text>:
        1ed0:       41 54                   push   %r12
        1ed2:       55                      push   %rbp
        1ed3:       31 d2                   xor    %edx,%edx
        1ed5:       53                      push   %rbx
        1ed6:       48 89 f3                mov    %rsi,%rbx
        1ed9:       31 f6                   xor    %esi,%esi
        1edb:       48 83 ec 50             sub    $0x50,%rsp
        1edf:       89 7c 24 0c             mov    %edi,0xc(%rsp)
        1ee3:       31 ff                   xor    %edi,%edi
        1ee5:       64 48 8b 04 25 28 00    mov    %fs:0x28,%rax

That looks pretty normal code. Let's crypt it and take a look again

    $ ./crypter-1.0 xeyes
    Section Table located at : 22248
    Section Table entry size : 64
    Section Table entries    : 27
    Section 00 [                    ]: Type: 0 Flags: 0 | Off: 0 Size: 0 =>
    Section 01 [             .interp]: Type: 1 Flags: 2 | Off: 238 Size: 1c =>
    Section 02 [       .note.ABI-tag]: Type: 7 Flags: 2 | Off: 254 Size: 20 =>
    Section 03 [  .note.gnu.build-id]: Type: 7 Flags: 2 | Off: 274 Size: 24 =>
    Section 04 [           .gnu.hash]: Type: 1879048182 Flags: 2 | Off: 298 Size: 50 =>
    Section 05 [             .dynsym]: Type: 11 Flags: 2 | Off: 2e8 Size: 6f0 =>
    Section 06 [             .dynstr]: Type: 3 Flags: 2 | Off: 9d8 Size: 497 =>
    Section 07 [        .gnu.version]: Type: 1879048191 Flags: 2 | Off: e70 Size: 94 =>
    Section 08 [      .gnu.version_r]: Type: 1879048190 Flags: 2 | Off: f08 Size: 50 =>
    Section 09 [           .rela.dyn]: Type: 4 Flags: 2 | Off: f58 Size: d80 =>
    Section 10 [               .init]: Type: 1 Flags: 6 | Off: 1cd8 Size: 1a =>
    Section 11 [                .plt]: Type: 1 Flags: 6 | Off: 1d00 Size: 10 =>
    Section 12 [            .plt.got]: Type: 1 Flags: 6 | Off: 1d10 Size: 1b8 =>
    Section 13 [               .text]: Type: 1 Flags: 6 | Off: 1ed0 Size: 1a12 =>  [6674 bytes encoded] - Crypted!
    Section 14 [               .fini]: Type: 1 Flags: 6 | Off: 38e4 Size: 9 =>
    Section 15 [             .rodata]: Type: 1 Flags: 2 | Off: 38f0 Size: 308 =>  [776 bytes encoded] - Crypted!
    Section 16 [       .eh_frame_hdr]: Type: 1 Flags: 2 | Off: 3bf8 Size: ac =>
    Section 17 [           .eh_frame]: Type: 1 Flags: 2 | Off: 3ca8 Size: 464 =>
    Section 18 [         .init_array]: Type: 14 Flags: 3 | Off: 4bd8 Size: 8 =>
    Section 19 [         .fini_array]: Type: 15 Flags: 3 | Off: 4be0 Size: 8 =>
    Section 20 [                .jcr]: Type: 1 Flags: 3 | Off: 4be8 Size: 8 =>
    Section 21 [            .dynamic]: Type: 6 Flags: 3 | Off: 4bf0 Size: 220 =>
    Section 22 [                .got]: Type: 1 Flags: 3 | Off: 4e10 Size: 1f0 =>
    Section 23 [               .data]: Type: 1 Flags: 3 | Off: 5000 Size: 5c0 =>
    Section 24 [                .bss]: Type: 8 Flags: 3 | Off: 55c0 Size: f98 =>
    Section 25 [      .gnu_debuglink]: Type: 1 Flags: 0 | Off: 55c0 Size: 34 =>
    Section 26 [           .shstrtab]: Type: 3 Flags: 0 | Off: 55f4 Size: f4 =>
    $ objdump -d xeyes  | grep -A10 "<.text>"
    0000000000001ed0 <.text>:
        1ed0:       d0 61 97                shlb   -0x69(%rcx)
        1ed3:       9a                      (bad)
        1ed4:       15 4a 02 22 fa          adc    $0xfa22024a,%eax
        1ed9:       22 47 8e                and    -0x72(%rdi),%al
        1edc:       ff 4a 7c                decl   0x7c(%rdx)
        1edf:       95                      xchg   %eax,%ebp
        1ee0:       5f                      pop    %rdi
        1ee1:       6b fc 7f                imul   $0x7f,%esp,%edi
        1ee4:       ac                      lods   %ds:(%rsi),%al
        1ee5:       ad                      lods   %ds:(%rsi),%eax

OK... This looks very bad. We can say that our crypter is encrypting the code of the program. If we try to run it....

    $ ./xeyes
    Illegal instruction

That illegal instruction is likely the `9a` that `objdump` already complained about. This is normal, we haven't addded our stub yet... But, as RC4 is a xor cipher, if we run our crypter again on the same binary, we effectively decrypt it (that is what the `stub` will eventually do):

    $ ./crypter-1.0 xeyes
    $ $ objdump -d xeyes  | grep -A10 "<.text>"
    0000000000001ed0 <.text>:
        1ed0:       41 54                   push   %r12
        1ed2:       55                      push   %rbp
        1ed3:       31 d2                   xor    %edx,%edx
        1ed5:       53                      push   %rbx
        1ed6:       48 89 f3                mov    %rsi,%rbx
        1ed9:       31 f6                   xor    %esi,%esi
        1edb:       48 83 ec 50             sub    $0x50,%rsp
        1edf:       89 7c 24 0c             mov    %edi,0xc(%rsp)
        1ee3:       31 ff                   xor    %edi,%edi
        1ee5:       64 48 8b 04 25 28 00    mov    %fs:0x28,%rax

# Conclusion
I think this is enough for this instalment. It may look like we haven't advanced much, but we are actually half way to get a decent crypter working.

As a summary: We have learned how RC4 works and implemented it on C. We have also got familiar with some of the main ELF structures, the header and the Section Table and implemented a simple program to navigate the table and find the segment of interest to crypt with our brand-new RC4 implementation.

In the next instalment we will implement RC4 in assembler to make it fit in a few hundred bytes, inject this code in the binary and patch the stub and the ELF header to decode the `.text` ans `.rodata` sections at run-time. Stay tuned!

