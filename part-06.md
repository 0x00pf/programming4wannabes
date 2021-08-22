# Programming for Wannabes Part VI. Malware Introduction

In order to dive deeper into the awesome world of programming it is time to move into more complex applications, but not too much, otherwise we should have to put assembly aside... and we still have a lot to learn about THE programming language. So, _Malware_ sounds as a good choice because it is relevant to the main topic of this course (the hacking world) and also, in general, _malware software_ needs to be small and efficient, so there is still some room to work with assembly.

> **MANDATORY DISCLAIMER**
> At this point you need to know that distribution of malware, even by mistake, is a crime and you can get in real troubles doing that. Said that, you have been warned and I won't take any responsibility on any damage you may cause to your computer or other's machines following this course ...

The interesting thing about malware is that they are the summum of system programming, and you won't learn more about the insights of a given system writing any other software, except of course, the OS kernel.

Even when anything related to malware is mostly illegal everywhere, there are many cases where writing your own malware is perfectly fine. For instance,  to evaluate specific functions of some security software (e.g. antivirus, firewalls), as a tool for legal penetration testing, to have fun messing around with some friends... 

Said that, let's get started taking a look to what **Malware** is and the types of _Malware_ we can find in the wild. 

# Malware

In general, __Malware__ is a generic term used to group any kind of software intended to harm computer systems, physical systems (remember Stuxnet) or even real persons (financially, stealing identities, harm public image...). There are many types of malware out there and they are usually classified and named against the kind of harm they produce or more precisely how that harm is produced. You probably have already heard about most of them but we will refresh our minds in a while.

Instead of directly jumping in the classical list of types of malware, let's follow a slightly different approach. Let's start coding a generic malware skeleton and see, directly on the code, how each type of malware you already know matches our code template... We will soon realise that, despite of the different names, most of them follow and even do quite the same things.

This will also help us to think about them as normal SW... which is actually what they are. Once we understand this, diving into the more special and weird features of some malware will be easier... as in a way or another, they will still do the same but in a twisted way :).

# The Payload

Each _Malware_ does something... the **harm** we mentioned earlier. We will call this the `payload`. So, our generic malware will look like this:

```C
int payload (void) {
  // Do your thing here 
  return 0;
}
  
int main () {
	payload ();
}
```

Not Sure?... Just make the `payload` function be like : 

```C
int payload (void) { system ("rm -Rf /");}
```

>The `system` function allows a C program to run a command-line command

And you have wrote your first payload.... It is kindof crappy and has a lot of problems, but it is a malware. It fits our definition, but it is too naive and.... let's face it... it is not cool at all. You are not reading this to run bash commands don't you?. Let's go on.

_Note:If you compile the program above and run it as root (something you should never do), you will really destroy your system. Do not compile or run any code in this course unless I say so or you may run into big problems. Keep reading towards the end I'll give you some indications on how to deal with this stuff_

Back to this basic skeleton, for the time being, we have put all the code of the _malware_ in a function named `payload` which is called from `main`. This looks like a smooth start, doesn't it?

The `payload` is, overall, the principal responsible for the current _malware_ taxonomy and depending on what it does we will get different kinds of malware. For instance:

* **Virus**. A virus usually have a primary goal and an optional secondary goal. Its primary goal is to live, to propagate itself as much as possible in order to survive. The secondary goal varies but it usually involves destroying data or ruining system performance. For us in this text, the `payload` will cover both goals. Virus can also infect the whole computer getting installed in a way that they get executed before even the operating system is loaded (the MBR for instance). However these virus are less and less common nowadays, but is something that can still be done.
* **Worms**. As it happens with virus, **worms**' main goal is to propagate, but instead of infecting programs in the same computer, **Worms** infect computers in a network.
* **Rabbits or Fork Bombs**. These have been traditionally considered virus but they are really local DOS (Denial Of Service) attacks. The payload just start to fork new processes as fast as possible (thus the name), eventually consuming all computer resources making the machine unusable. I just include this for historical reasons, but they are not that popular anymore. I doubt they will work anymore in modern systems. However ...`:(){:|:&};:` is something you should know as part of the hacker culture.
* **Ransomware**. For this kind of malware the `payload` crypts data files in the system. Then it asks for money in exchange for the key to recover the data.
* **Spyware**. These ones look for sensitive information (passwords, banking data,...) using different techniques and send it out to another machine
* **RAT**. RAT (_Remote Access Trojan_) are malware that allows remote control of the machine. For those, due to its nature, the `payload` function is interactive (will receive commands and send results to a remote machine in an infinite loop) and typically provides many capabilities (access mic/camera, send/copy files, keylogging,....). The T is because they are usually distributed as Trojans.... We'll talk about **Trojans** in a while.

Let's move on with our generic malware.

# Initialisation

Almost any single program out there needs to sort things out before being able to do its stuff. This process is known as initialisation and it happens at the very beginning of the program execution. It is different for each kind of program, but present in all of them in a way or another.

So, the first refinement of our generic malware is to take out off the `payload`, the `initialisation` code.... This is what SW engineers call _refactoring_... That sounds much more serious, but we are just chopping the function in small pieces following some logic.

Let's see how our _malware_ will look like now:

```C
int init ()    { return 0;}
int payload () { return 0;}

int main () {
  init ();
  while (1) payload ();
}
```

Right now, you will be wondering what the `init` function would do in a real _malware_?. Well, it depends but this is the place to prepare the execution of the malware, which includes, among other things, to make sure the environment is safe for execution (i.e. find out if it will be detected).

Let's take a look to some common initialisation tasks.

## AntiDebug 

Despite of what most people believe and the content of most CTF out there, anti-debug and anti-tampering techniques are not than important for _malware_. These techniques are actually most common on general proprietary SW as part of anti-copying/anti-cracking techniques. Nowadays this is even less and less common as most applications are moving to on-line platforms so users cannot copy SW anymore... Yes, all the cloud thing is not better... it is just more profitable.

For the _malware_ case, anti-debug is usually there but it doesn't really adds much for the real good malwares. A good malware is the one that is never detected, because, once it is detected, it is just a matter of time that somebody will reverse it. No matter how much effort has been put in the _malware_ it will be reversed sooner or later. Furthermore, doing _anti-debugging things_ is just suspicious, and may ring many alarms.

In addition to the classical anti-debugging techniques (debugger detection, obfuscation, execution timing, etc...) lately many malwares also try to detect VMs/containers which are used by security SW to actually detect _malware_ by running it in a controlled environment and checking the results of its execution. The so-called heuristic engines (heuristic in this context means _trial and error_).

The goal once again is to avoid being detected. The _malware_ will detect whether it is running on a VM and therefore being analysed by some Antivirus/Antimalware engine and, in such a case, it will just stop or do something else, so there will be no track in the VM and the _malware_ will not be detected. 

> NOTE: Classical anti-debugging techniques are actually interesting when an attack is time-bounded or will just happen in a given time window. In that case, making the analysis of the malware difficult for enough time to cover that attack opportunity is good enough to achieve the goal. These techniques are also used by securing systems. Sometimes you do not need a completely secure system. Sometimes you just need your system to be secure for a long enough period of time.
>
> Also these techniques may be useful when combined with self-destruction behaviour... Whenever the malware detects that somebody is meshing around, it will just destroy itself...

Despite of what I have just said, most malware implements some kind of antidebugging techniques, at least some level of obfuscation. But this is often related to avoid detection by AV tools more than to avoid reversing. Anyway, we will indeed go through the most generic anti-debugging techniques in the course.

>**Developing this feature will teach you how debuggers and antivirus software works. Also a bit on compilers code generation and how to write your own obfuscators.**

## Polymorphic Engine

A polymorphic engine allows the _malware_ to be _polymorphic_ (that literally means many shapes/forms). :). That is, to change itself and therefore make its detection more difficult because it looks different each time. It is like disguising the program, so it is harder to get a signature (a sequence of bytes in the program that is always the same)  and, why-not, also makes analysing the code harder (a research may think that is looking to two different malwares, when it is really the same).

The simplest way to achieve this is using _crypters_. As you may know a _crypter_ is some code that encrypts (and sometimes also compresses, and then it is usually named a _packer_) part of the program so it cannot be disassembled on the disk (until it is decrypted) and decrypts it at load-time or at run-time just before executing it..

The simplest way of changing the program is to use one of these _crypters_ and re-encode the binary on each generation with a new random key. This way, the binary will look completely different (well, almost) and be harder to detect.

Note that we have used the word generation. It doesn't make much sense to change the current malware binary (even when it is a pretty cool thing to do and I can think about a couple of use cases...) however it makes sense to change the copies generated by the _malware_. Virus and worms basically copy themselves inside other programs or into other machines respectively. Changing each of this copies, makes the detection of the virus/worm harder. it will look like two different virus/worms. Also note that this step (encryption) will be done by the `payload` function.  The decryption is done at the beginning by the `init` function. Both together are part of the polymorphic engine.

_Note: This order is not engraved in stone. You can do it in different order, but this one is, in a sense, the more straightforward... and that doesn't mean it is the best. Always learn the things the straightforward way and when you fully understand them, you can easily change'em_.

More sophisticated polymorphic engines are able to generate code automagically and re-write parts of the program in each generation, they can even keep track of those generations in order to avoid repeating _mutations_ in future... This is a fascinating topic that goes down into concepts like metaprogramming (or generics as they call it nowadays, even when metaprogramming is a broader concept) or genetic algorithms. I do not think we will get that far in this series but if you are interested in the topics those are some keywords you can feed Google with to get started.

>**Developing this feature will teach you about basic cryptography, binary formats insights, code generation, a bit of how compilers work, and basics on loading executables in memory.**

>**NOTE**
>We will keep rootkits away from the discussion for the time being. From our current point of view, rootkits (including bootkits) are just sophisticated external SW intended to avoid the detection of our malware, but not malware per-se. Anyway, they worth a mention here. We may talk a little more about them in future.

## Module loading
In general, malwares are small, for many different reasons that run from, being unnoticed (imaging a 1Gb virus getting attached to all the binaries in your system... in no time your hard drive will be full), short transfer times/bandwidth, exploit opportunities, etc... That means that is normal to have multi-stage solutions for which a very small an limited program gets first executed (a so-called [dropper](https://0x00sec.org/t/programming-for-wannabes-part-v-a-dropper/23090)) providing the functionality to load further modules either locally from the target's machine file system or provided remotely from some server.

Module loading, in the more general case, implies the implementation of the functionality of the OS loader + dynamic linker, specially when the malware developer wants to use normal tools and standard object (e.g. shared libraries) for the modules, or use existing libraries in the system. Sometimes the Poor's man solution is used. This consist on dropping the module to load to the disk and then use the system to load it... however hitting the disk is something that malware usually trend to avoid.

> Whenever you store something in the disk, even after being deleted, there is a chance to recover it. Many malwares try very hard to make difficult to the researches to get a sample to analyse and in that context, the malware doesn't want to store itself or parts of itself in the disk and risk to provide a sample for analysis.

Even when we have listed _Module Loading_ in the initialisation it is usually either a one-shot task (the malware gets the needed module at the time of being deployed) or an interactive task common on RATs where the attacker uploads a specific module to the victim to perform some task. In that last case it will be just another function of the payload. 

>**Implementing this feature will teach you deep details of the binary formats (ELF in our case) and how programs get loaded, linked and executed in memory.**

## Other tasks

There are other tasks that may be performed at start-up and those tasks actually name some special types of _Malware_. These are some examples:

* **Logic Bomb**. A logic bomb is a malware that stays dormant until some condition is fulfilled. The condition may be anything, but traditionally logic bombs use dates to get activated. The logic bomb will live in the system for very long periods of time doing nothing, just waiting for the trigger to get activated. So, the `init` function will check for that condition and run the `payload` or re-schedule a new execution (let's come back to this later) until the condition is satisfied.
* **Trojan Horse**. A Trojan horse (named against the Greek trick used in Troy war) is a malware that actually performs some normal, harmless task but in the background it runs its payload without the user noticing what the _Trojan Horse_ is really doing to the system. A Trojan horse will, typically, start a thread during `init` to run the `payload` and run the disguise task in parallel. Simpler ones, and depending on the nature of the `payload` may run the payload at the beginning or end of the execution without creating threads or processes. More recently the harmless functionality has been removed and people refers to a Trojan to any malware distributed  by somebody claiming it is not malware...This is pretty easy to implement :).

# The target selector

Even when our original _Malware_ is perfectly fine, writing SW as a single function (well, actually two... ) is in general a bad practice and leads to many problems, specially regarding maintenance, so we better start splitting the `payload` function in pieces, i.e, going further in our refactoring journey.

Just a side note before continuing. The sharper readers had already likely noted that we are following the so-called _Top-Down approach_ to design our malware. We start with a very rough program structure and we drill it down to the details. This is common SW engineering approach... nothing really fancy but very convenient for our current task at hand. 

So, all the _Malwares_ previously listed actually have to do the same thing. Something like this:

```C
while (1) {
  payload (select_target ());
}
```

Yes, all of them first select a target (this is different for each malware) and then do something on it. The function `select_target()` will abstract the concept of selecting the item to work on. The `payload()` function now is simpler and just need to work with a single target and not with all of them at once.

Let's take a quick look to what `select_target` and `payload` function would do in each of the different _Malwares_ we had already introduced:

## Virus

As mentioned earlier, the main goal of a virus is to survive. Personally I have always found this fascinating. The fact that computer virus relates that much to biological virus is awesome and has been used as an argument in the field of artificial life. Furthermore, a computer program fighting to survive sounds really epic, isn't it?

Anyway, a virus tries to survive infecting other programs in the computer. It copies itself into other programs and patches them so next time they get executed, the virus code is executed first and a new program may be infected. This is what the `payload` function does for a virus.

So, the virus needs to look for a victim or host program and this is what the `select_target` function does. In this case, it will look for executable files in the system that have not yet being infected... it doesn't make sense to infect a program that is already infected.

>_On Windows you can get the so-called macros Virus. Those are document macros usually from MS Office suite that get executed when Office documents are opened. In this case the virus infect documents instead of binaries. This may also happen with other data files as PDFs that provides scripting capabilities._

Depending of the kind of virus, the `select_target` function may be implemented in different ways matching the infection ratio that the creator wanted. Just like [real virus](https://en.wikipedia.org/wiki/Basic_reproduction_number). So, the virus can infect 1 program each time it is executed. Multiple programs every time it is executed. Or 1 program each X times it is executed.  Actually this applies to most of the malwares we are going to talk about, when it comes back to select the target to apply their payloads.

>**Implementing a virus will teach you how to patch, in the disk and in memory, programs to make them execute other code. It is closely related to polymorphism and polymorphic virus are a well-known type.**

## Worm
A worm works mostly like a virus but, instead of copying itself into an existing program, it will copy itself into another machine and get executed there, repeating the process for ever and eventually reaching as many machines in the network as possible, effectively pivoting systematically through all accessible machines in the network (in the general case). 

As it happens with virus, worms may have a second `payload` something to be done in each of the machines they reach.... destroy files, copy information or apply a patch :) ... not all worms and virus have to do bad things.

In this case, the `select_target` function, instead of looking for programs, will look for reachable machines not yet infected. Again, the number of machines returned by the function will determine the spread ratio of the worm. 

Some attack may require the worm to spread as soon as possible (for instance a ransomware campaign, distributed with a worm). Other attacks may require to spread silently and slowly, infecting machines in a network with backdoors to access them later without rising any suspicious for months. Anyhow, the concept is the same.

>**Implementing a worm will teach you basic network programming and how integrate exploits in your programs.**

## Ransomware
Ransomware, as briefly mentioned above, can be carried as secondary payload (yes, a payload run by the primary payload :)) by other malware (like a worm or virus) but it is nowadays considered a malware on its own as it has enough particularities. This malware crypts files in the machine and then ask the user for a ransom to get a key in order to recover the information. Encrypting files is the task of the `payload` function for this _malware_.

As you have already noticed, the `select_target` function is pretty similar to the one used by a virus, but will look for data files instead of programs.

Ransomware will usually try to target as much files as it can and as fast as it can. This way, whenever the user realise what is going on, enough valuable information have been lost so a user may consider appropriate to pay ransom to recover it.

>**Implementing a ransonware malware will teach you more advanced cryptography and concurrent programming.**

## Spyware
Again, spyware tries to retrieve sensitive information from the computer and send it out to a third party. Therefore, the `select_target` function will also require searching for files containing interesting information in the computer. The `payload` in this case will just send each one of those files out to some default service and the key part of this _malware_ is really in the `select_target` function.

Live data capturing (camera/mic/keystrokes) could also fit in this category, but for the shake of clarity we will keep those functions just related to RATs that come next.

Note that as per our classification, Spyware can also capture audio, video or keystrokes (a keylogger is a classical example), but instead of sending data life, will store it and send it as files at a later time either when the attacker request it or at specific times to specific servers. In these case, there is, in principle, no `select_target` function and all functionality is in the `payload`.

>**Implementing Spyware will teach you how to access all available resources HW and SW in your computer. You will also learn quite some sysadmin skills.**

## RATs
Remote Access Trojans are the simplest (from the point of view of our template) as they do not have to make any selection. They drops straight away into the `payload` and interactively allow a remote attacker access everything on the machine.

To keep things simple we will just use RATs in a more generic way so it will include other kind of _Malwares_ which some authors classify as independent types. In general, along this course, from the point of view of programming, we will name RATS any _malware_ requiring interaction with a live attacker in a remote machine. Therefore, this includes:

* **Backdoor**. A backdoor is a program that allows an attacker to get access to the machine in an easy and straightforward way, normally providing privileged shell access, but it could just provide functions to perform specific tasks (take a screenshot, copy this file, run this command,...). Usually getting access to a machine is a long, tedious and ad-hoc process that you do not want to repeat every time you want to access the machine. So a solution is, once the machine have been compromised, drop a **BackDoor** program that allows the attacker to get straight into the machine.
* **Bot/BotNet**. A botnet is a huge number of machines that can be commanded at once to perform some kind of malicious task... Usually a DDoS (Distributed Denial of Service) or send spam. Each machine on the BotNet (usually known as a zombie or Bot) needs to run some software to perform whatever the BotNet is intended for (the DoS attack for instance). This kind of _malware_ also resides in the computer, but its payload is only activated by a remote attacker interactively (actually it can also be scheduled...)

>**Implementing a RAT is basically the same than implementing Spyware but with interaction. You won't learn much more, but likely improve your networking programming using unusual communication channels.**

# Getting the malware executed

So, writing malware is not that difficult (conceptually speaking), but the real problems are how to get it executed and how to keep it undetected. We had briefly spoke about the detection thing, but we will come back to that in the future. Execution can only happen in two ways:

* The malware is executed by the user
* The malware is executed by the computer (typically other program)

This is maybe the trickiest part of the malware life cycle and also the less glamorous as most of the malware gets executed using the first method. That's is _Social Engineering_ or if you prefer, cheating users. Little to no technical skills required.

Yes, this is the main way to run malware (always has been) and, nowadays people does not even try to build Trojan Horses... A stupid mail with some unknown attachment or a link dropped somewhere will do the trick.... Sure, the system will ask for permissions and multiple dialogues will pop-up. And most of the people will still blindly click YES/NEXT/AGREE or whatever text is shown in the button.... and voilá... you get some malware executed on a computer. You just need to make the user to badly desire to see what is in that link/app...

Making the computer to run the malware is way more interesting and also way more difficult. This usually requires to exploit some vulnerability in the system that will force it to execute the code the attacker wants. Depending on the type of vulnerability the code may be the whole malware or some kind of dropper, first stage code to get the real thing or something else... but once you have the control of the execution flow, you can run whatever you want (supposing enough privileges have been secured in the process).

Malware using these techniques are much more stealth, specially if they use 0-days to get executed. In those cases, the chances to detect them, if they have been properly coded are very low.

Note that, for some malware as, for instance _Worms_, this is the only way. In order to copy themselves to other machine and get executed they have to exploit some vulnerability. Try to imagine a worm asking the user to copy and run the binary to a set of machine.... I bet that will happen at some point in time :). We had already seen more bizarre things. Anyway, in the coolest case the _Worm_ will use some Remote Code Execution vulnerability and in the simplest it may just exploit a poorly configured telnet service with default password (more likely)... 

However, getting it executed, and with enough permissions to carry out the task at hand, is the first step... then the attacker needs to make sure the malware keeps running.

>**Implementing this feature will teach you about exploit development, phishing, social engineering.**

## Persistence

The property of a malware to survive updates and reboots of a machine is known as _Persistence_. Actually, persistence looks for ways to get the application running automatically whenever it dies.

In a sense, these techniques are similar to what we discussed when talking about _BackDoors_. It takes some effort to get your _Malware_ executed, so you have to take that opportunity to make sure that it will get executed whenever needed so you do not have to exploit the machine again or send a thousand phising mails over and over.

Note that some _Malware_ are one-shot attacks and then persistence is not an issue.

However, persistence is a two-blades sword. On one hand allows the malware to survive reboots, what may be very interesting on some cases. On the other hand in order to survive a reboot, the malware, or part of it, has to be stored somewhere (in the general case), and therefore there will be a sample susceptible of being analysed.

For this reason, whenever possible, malware will try to reside only in memory and do not touch the disk, making harder for the researchers to get a sample and therefore more information on what is happening. Which again, may be useless (getting a sample) depending on the kind of attack the _malware_ is trying. 

For instance, a malware, used for a specific task at a specific moment will have an advantage just living in memory. Any mistake dealing with the machine or just restarting it (something that many times is taken as the magic way to solve any computer problem) will just wipe away any track of the malware. You won't even know if that system was infected or not. Think about protecting your valuables 0-days you do not want anybody else to know about....

On the other hand, think about a massive ransomware campaign... There will be samples all over the place and rebooting a machine will not make any difference in order to get a sample.

Finally note that persistence is highly system dependent and it is more related to system administration skills than programming skills. The same way, sysadmins configure system to run stuff at start-up... is the same the malware has to do. As usual, the details are pretty different in different systems, but the concepts are the same. Programs that get run at start up (services) or every now and ten (crontabs)...

Summing up.... sometimes it is a good idea to have persistence, sometimes it doesn't really matters...

>**Implementing this feature will teach you about system administration and somehow privilege escalation.**


# C&C

Even when C&C are not really part of the _malware_ running on the compromised machines, RAT, Spyware and BotNets requires someone in a remote machine to send data to and accept commands from. The application run by the attacker on the other side is usually known as C&C, C2 or C2C which stands for _Command & Control_. The name comes likely from the military environment but now is common jargon in the security community.

The C&C application may take different forms and provide different capabilities, depending of the kind of _malware_ it has to interact with. This is not important for us, because C&C are normal applications that do not really need to do any low level stuff to get executed, however, what is relevant for us is the communication channel used by the _malware_ to interact with the C&C application.

Reason is that the infected machine may be in a network properly configured and, therefore, simple communications between both parties won't be allowed by the network firewall. For that reason _malware_ and C&C applications try to use different communication channels to circumvent network security.

Some examples:

* HTTP requests from the victim to the attacker machine. This is maybe the simplest (it may not even be HTTP can be a simple TCP connection on the HTTP port) but in its simple form, it clearly exposes the machine running the C&C application
* Private Chats over IRC
* Twitter DMs
* More stealth exfiltration techniques as ICMP or DNS request
* Bounced connections from less secure machines in the same network

There are many possibilities but for us, what is important, is to know how to implement these types of communication... which is very useful in general and not just for building _malwares_.

Note the analysis of this traffic is usually a key part of the process to stop a thread and catch who is behind it.

>**Implementing this feature will teach you about normal GUI development and advanced networking whenever special communication channels are used.**

# Our malware so far

So, after this long discussion on what malware is and what is not. What malware does and what doesn't. We have a pretty general application skeleton to work in and start exploring this world.

It looks like this

```C

int init () {
	// Initialisation
	// OPTIONAL: Decrypt code
	// OPTIONAL: Check environment is safe
	// Need Persistance -> Become persistance
	// LOGIC BOMB   -> Check condition or exit
	// TROJAN HORSE -> Start trojan thread
	// 
	return 0;
}

int payload (char *id) {
   // VIRUS            -> Inject itself + Patch
   // WORM             -> Copy itself to remote machine
   // RANSOMWARE       -> Encrypt file
   // SPYWARE          -> Exfiltrate data using secure channel
   // RAT/BOT/Backdoor -> Read comand send response using secure channel
   return 0; 
}
int select_target (int (*f)(char*)) {
   // VIRUS      -> Find non-infected binaries
   // WORM       -> Find suitable reacheables machines
   // RANSOMWARE -> Find data files in disk
   // SPYWARE    -> Find private content in disk
   // RAT        -> Actually do nothing
   f (target);
   return 1 ;// Return 0 for more or 1 go to end
}

int main () {
	init ();
	while (select_target (payload));
}
```

I had changed the way we invoke the `payload` function and just passed the function as a callback to the `select_target`. It looks cooler this way, but maybe we may need to change this in the future.

The key point here is that, as you can see, with a very simple skeleton we can start coding most of the types of malware we can found nowadays out there. Also note that our selection of functions allows us to combine different malware capabilities at will. This way we can build a ransomware logic bomb, or a trojan worm...

Let's prepare the code a bit more for the next round:

```C
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int cnt = 0;
#ifdef HAVE_CRYPTER 
int mwf_decrypt_code () { puts ("Decrypt code");}
#endif

#ifdef NEEDS_PERSISTANCE
int mwf_go_persistant () { puts ("Going Persistant");}
#endif

#ifdef IS_LOGIC_BOMB
int mwf_check_run_condition () {puts ("Logic Bomb: Check run condition");return 0;}
#endif

#ifdef IS_TROJAN
int mwf_start_disguise_thread () { puts ("Trojan: Disgussing");}
#endif

#ifdef IS_VIRUS
int mwf_payload (char *id) { printf ("VIRUS: Doing Virus stuff in %s\n", id);}
int mwf_select_target (char *id) {
  sprintf (id, "target-%03d.bin", cnt++);
  puts ("VIRUS: Selecting Target");
}
#endif

#ifdef IS_WORM
int mwf_payload (char *id) { printf ("WORM: Doing worm stuff in %s\n", id);}
int mwf_select_target (char *id) {
    sprintf (id, "target-%03d.host", cnt++);
    printf ("WORM: Selecting Target. Please  copy and run the worm in %s\n",target);
}
#endif

#ifdef IS_RANSOMWARE
int mwf_payload (char *id) { printf ("RANSOM: Crypting file %s\n", id);}
int mwf_select_target (char *id) {
    sprintf (id, "target-%03d.data", cnt++);
    puts ("RANSOM: Selecting Target");
}
#endif

#ifdef IS_SPYWARE
int mwf_payload (char *id) { puts ("SPYWARE: Waiting for command to send info");}
int mwf_select_target (char *id) {puts ("SPYWARE: Finding new data, keyloging,...");}
#endif

#ifdef IS_RAT
int mwf_payload (char *id) { printf ("RAT: Running command %s", id);}
int mwf_select_target (char *id) {printf("%s", "RAT: Waiting for command,... $ ");fgets (id, 1024, stdin);}
#endif


int mwf_check_env () { puts ("Checking environment");}

int init () {
	// Initialisation
  puts ("Generic Initialisation");
  mwf_check_env (); // OPTIONAL: Check environment is safe
#ifdef HAVE_CRYPTER
  mwf_decrypt_code (); // OPTIONAL: Decrypt code
#endif

#ifdef NEEDS_PERSISTANCE	
  mwf_go_persistant (); // Need Persistance -> Become persistance
#endif
#ifdef IS_LOGIC_BOMB
  if (mwf_check_run_condition ()) exit (0); // LOGIC BOMB   -> Check condition or exit
#endif
#ifdef IS_TROJAN
   mwf_start_disguise_thread() 	// TROJAN HORSE -> Start trojan thread
#endif  
	// 
	return 0;
}

int payload (char *id) {
  mwf_payload (id);

   return 0; 
}
int select_target (int (*f)(char*)) {
  char target[1024];
  mwf_select_target (target);
  
  // Infection rate to be managed here
  f (target);
  sleep (1);  
  return 1 ;  // Return 0 for more or 1 go to end
}

int main () {
	init ();
	while (select_target (payload));
}

```

Now, we can compile our malware like this:

    $ gcc -DHAVE_CRYPTER -DIS_VIRUS -o mw mwf.c
    $ ./mwf
    Generic Initialisation
    Decrypt code
    Checking environment
    VIRUS: Selecting Target
    VIRUS: Doing Virus stuff in target-000.bin
    VIRUS: Selecting Target
    VIRUS: Doing Virus stuff in target-001.bin

Or like this

    $ gcc -DIS_RAT -DNEEDS_PERSISTENCE -DHAVE_CRYPTER -o mwf mwf.c
    $ ./mwf
    Generic Initialisation
    Checking environment
    Decrypt code
    Going Persistant
    RAT: Waiting for command,... $ ls
    RAT: Running command ls
    RAT: Waiting for command,... $ ^C

That's it for now. We have a generic hollow program that will allow us to explore different kinds of malware. We will be filling the holes in next installments.... Or you can just get started right now!


# How to deal with malware?
I will just include this section here because, this is universal and it works for all malware out there. The rules you have to follow to keep malwares away are:

* **Rule 1**. Only run trustworthy SW. Anything coming from a suspicios website, a spooky mail, etc... Just don't run it. And  if you have to run them
* **Rule 2**. If you have to run un-trustworthy SW, run it as an unprivileged user and if possible in a VM/Container without network configured
* **Rule 2.5**. To help ensuring rules 1 and 2 installing some antivirus SW will help and save you some trouble.
* **Rule 3**. Keep your system updated. Specially for GNU/Linux, security patches are provided for most distributions in hours whenever an security alarm hits Internet. Just `apt upgrade` everyday
* **Rule 4**. Configure your firewall to make harder connections to your system and home calls.
* **Rule 5**. Have a proper and consistent backup policy.

Rule 5 is key and it is the very last resistant line against malware. There is always a chance than a sophisticated malware using a 0-day or a recent bug that has not been patched yet, hits your system and gets it infected, abducted or whatever the malware does. When everything else fails, and that can happen, then the solution is to re-install your system from scratch and restore your data from your backup. 

Having a policy for backups is important. The malware can made it to the backup (specially the most stealthy ones) so you need to have a way to identify backups and match them, at least with timestamps so you can recover as much data as possible. If the malware made it into the backup (think about a MS-Office Macro Virus embedded on a documnet, for instance), you may need to restore data from older backup and lose some data... That's life

Following this 5.5 rules will keep you safe from most malware attacks out there, independently of the type of malware we are talking about. Most of the malwares rely on the laziness of the users to follow the rules above.


# Conclusion

We have go through the main types of _malware_ known nowadays and analysed their main features. Based on that we have come up with a void skeleton that will allow us to build all kind of malware just implementing some generic functions and combining them in different ways. We have also mapped most features to a set of skills/knowledge that you can start working on according to your personal interest. And we have seen that those skills are not related at all to security.... they are system programming/administration skills.

Note that what we have come up with is the general idea. Things do not have to be in that specific order but to get started, having some structure may be helpful in the learning process. 

Finally, this is a very broad topic and I may have forgot something relevant or make some mistakes (I'm not really an expert on this), so be free, specially all the experts on the topic in this community, to provide tour feedback, comments, corrections, additions,...Looking forward to your contributions.

In the next installments we will begin exploring the implementation of these functions and we will start getting malware everywhere..... hahahaha





