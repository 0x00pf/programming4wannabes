# Programming for Wanabes XI. Introduction to RATs
So my 0x00sec fellows've spoken and RATs will be. As anticipated in this first instalment we will mostly discuss theoretical concepts, which means not much code.... but will be some :). Let's start.

RAT stand for `Remote Access Trojan`, actually the last `T` could also be _Tool_ instead of _Trojan_ and then we will be talking about common tools used in corporate environments instead of malware. Conceptually both things do the same, i.e. allow a third party to remotely access and control a computer. Or if you prefer, they allow to break the privacy of people. The difference is that in some case there is some official and even legal support to install and run the tool and in the other... no. In any case, and most of the time, the users do not have a clue of what is going on :).

# RAT's functionality
The ultimate goal of a RAT is provide root shell access to the machine (or Administrator access if you prefer). Right, a _Backdoor_ is, somehow, the simplest form of a RAT. It is true that many RATs provide specific functions like capturing video or audio, steal credentials, etc... however all these actions are pretty straight forward whenever you have privileged access to the machine. Or in other words, when you are able to upload and run any SW you want.

So, I will keep aside specific task to perform in the victim as the one mentioned above for the time being, and get focused on the actual core of the RAT: enabling remote access. For doing this, we need the following:

* Remote Shell Access (execute specific commands is a kind of subset of this)
* Secure communication with a third party. That third-party will usually be a C2C application controlled by the attacker. This implies general data interchange (commands and responses but also file transfers in both directions)

We will look into this again in the context of this course, but I have already written about this in [here](https://0x00sec.org/t/remote-shells-part-i/269), [here](https://0x00sec.org/t/remote-shells-part-ii-crypt-your-link/306), [here](https://0x00sec.org/t/remote-shells-part-iii-shell-access-your-phone/508) and [here](https://0x00sec.org/t/remote-shells-part-iv-the-invisible-remote-shell/743)... 

On top of that, and I said on top, because what come next are orthogonal functions in the sense that they are used by many other types of malware and not specifically for RATs:

* Privilege Escalation. 
* Hiding
* Network Pivoting (Optional)
* Persistence (Optional)

## Privilege Escalation
This is a two-manifold topic. In one hand, it is common to have enumeration utilities available in the RAT infrastructure (either as part of the RAT or as a module that can be downloaded in the victim) to help the attacker get an administrator account. 

Imagine that the RAT got executed using some Social Engineering technique. In such a case, it will likely be executed as an unprivileged user, and the attacker will try to escalate that situation in order to have full access to the machine. 

On the other hand, the enumeration tools just provides the information about potential vulnerabilities in the machine. Then appropriated exploits are required in order to make those vulnerabilities into advantages to escalate user privileges.

Note that, some times the RAT gets executed already using a vulnerability (either local or remote) that gives it superuser privileges from time zero. In those cases the enumeration functions are just taking space and are not useful... Well, not exactly... What does this mean is that this kind of malware many times have a module/plug-in subsystem to enable the activation (and alternative download from the C2C) of functions when needed. Such a system in a malware context may be a bit complex, as sometimes you cannot just use standard functions provided by the system.

## Hiding
Other function that most RAT implement is some kind of hiding capabilities. There are a few options on how to do this, depending on how secure is the target system.

* __Hide on-sight__. On low-security systems sometimes it is just enough to give the RAT a cryptic-system-like name to make it invisible. Imagine the machine of a regular user that has just being compromised, for instance, to be used in a proxy chain or to become a zombie in a Botnet. You can just call your RAT something like `[kworker/u12:7]` and not even an advanced user may pay attention to it.
* __Install a rootkit__. Supposing the attacker has got root access to the machine s/he would easily install a rootkit to hide all tracks related to the RAT. Note that on a decently hardened machine kernel module loading would likely be disabled and run-time kernel patching may not be possible so maybe a user-space rootkit may be the only alternative.
* __Insert the RAT in a running process__. In this case, the RAT get itself copied in a running process and executed inside it, usually as a thread. This is a pretty stealth technique. For instance, imagine a Firefox that is already running, you will see firefox connections to many different server (supposing you have a hundred tab open as everybody does), together with the connection to the C2C machine. The basis of this is briefly described [here](https://0x00sec.org/t/running-binaries-without-leaving-tracks/2166). This technique also requires superuser permissions and makes persistence a bit tricky.

## Network Pivoting
This is an optional feature. It is common for RATs targeting cooperative environments where it is relatively easy (using social engineer for instance) to get into a machine with limited privileged from outside the network, and then reach internal machines, not accessible from the Internet, just jumping from different servers within the network. Sometimes the RAT offers a kind of proxy/router capability to connect the C2C to those internal machines.

Network pivoting can actually be seeing as a special case of privilege escalation, where the privileges are rising by accessing machines that may have access to services that other machines may not.

I bet all of you know what I'm talking about, but just in case. Imagine a web application. The front-end (all those pages and Javascripts) is accessible from the internet, but the back-end, the machine with the database and all the business logic is not, it only can be accessed from inside the target network.

As I said, network pivoting is a kind of systematic approach consisting on enumerating nearby computers accessible from the current compromised machine, determining any vulnerability or weak configuration, and then accessing it copying the RAT over and executing it there and effectively starting the process again from that machine.

Imagine for instance a machine in a corporate network. It has internet access, but the user also has access to some laboratory machines from that computer using SSH. Imagine that the user has configured ssh passwordless, what is pretty common specially when you need to access many machines remotely... Then we will have direct access to that machine for free...

## Persistence

Finally, this functionality is also optional and depends on the nature of the attack. As mentioned before in this course, persistence is the capability of a malware to keep executing despite of being stopped for whatever reason (usually a reboot).

In general, a RAT will like to have some persistence mechanism, so the access to the machine is not lost after a reboot, or whenever the process dies for whatever reason. Persistence, usually implies writing something somewhere in the disk... because the disk is the memory on the computer that will survive reboots (if by a chance the target have other persistent storage, that would also be an option, think about flash memory in embedded platforms).... And not just that, further disk modifications are required in order to execute again that SW saved in that persistent memory.

Very advanced RAT targeting network environments, just act like a virus/worms. They have the capability to infect other machines so, after an initial attack enough machines in the network will be infected to ensure that after rebooting any of them, there will be some instance of the malware running in the network that will infect again that machine. This is a very smart and powerful technique as the attacker is not storing anything on the disk at the same time that it achieves persistence... Anyhow, as you can see, this will not work on all cases and it targets a very specific environment.

# Let's write a RAT: r4t0x0

Our first RAT is going to be damn simple, but it is going to be a complete/real RAT. Let's go for the following:

* Hide on-sight
* Basic Persistence
* Remote shell

If we recall our original malware skeleton, the two first features have to be implemented in the `init` function. 

_Note: In this instalment we will just drop the code on `main` as it will be just a few lines._

For persistence we will be going low profile, just store the file on the home folder of the user looking like a configuration file and silently patch `bash_profile` to launch it on each user session. A more sophisticated RAT could, for instance, determine the actual shell being used and patch the appropriate start-up files or look for a folder in the home directory containing many files so the RAT will be more stealth. 

As I said we are going to keep it simple. The point of this exercise is that you understand how this works so, you will be able to extend the program to test any other technique you want to learn about or experiment with. Once you understand the basics you will be in the position to analyse different protection systems and start to try to figure out how to overcome... and then how to modify the protection system to detect you RAT, and then break it again,... and so forth. Lots of fun ahead.

Finally, the `remote shell` will be our main `payload`. In this case, as we mentioned in the [malware introduction](https://0x00sec.org/t/programming-for-wannabes-part-vi-malware-introduction/), RATs runs on an interactive loop, so there is no target selection function to feed the payload.

We will be going through the different features of the RAT one by one from scratch, so you will see a _possible_ process on how to determine what is the issue you want to solve and how to solve it step by step until you reach your goal. As it happens with many other problems in real life, the key point is to get to know what is the problem you want to solve... this seems pretty trivial, but many times people lost the point and end up overcomplicating things that doesn't really add to the solution of the problem. Anyhow, I hope the approach we are going to follow will help you not only to write malware or tools to detect malware, but to solve problems in general... That is the important thing to learn.

Let's get started with the first point.

# Hiding on-sight
Our goal here is to keep the RAT low profile and avoid it to be detected. As we discussed in the introduction sometimes just changing your name and being quiet will be more than enough to go unnoticed. 

The easiest way is to give the process the name of some system thread. Not even an advanced user will be able to spot the RAT on `ps` or `top` at first (or any other tools actually). However... your program has to be properly coded and be efficient otherwise... seen something that looks like a system thread taking 100% of the CPU, and popping on top of `top` all the time.... well, that will be suspicious. And then it is just a matter of time to get discovered.

Let's see how all this works. We will be using `ps`, but most monitoring tools work the same, actually reading data from `/proc/PID`. 

You can now run a `ps -ax` on your system an take a look to what you get. You can also install different distros on VMs and check differences between distros and kernels to chose a name that fits well in more platforms. These are some examples I get from an Ubuntu machine:

       10 ?        S      0:00 [migration/0]
       13 ?        S      0:00 [cpuhp/1]
       14 ?        S      0:03 [watchdog/1]
       22 ?        S      0:04 [ksoftirqd/2]
       24 ?        I<     0:00 [kworker/2:0H]
       46 ?        S      0:01 [ksoftirqd/6]
       48 ?        I<     0:00 [kworker/6:0H]
      358 ?        S      0:27 [jbd2/sda5-8]
      991 ?        S      1:01 [jbd2/sda6-8]
     2189 ?        S      0:00 [nfsd]
     2190 ?        S      0:00 [nfsd]
     2191 ?        S      0:00 [nfsd]

There are many more options, just take a look on your own system. 

User space programs are more susceptible to be noticed by the user/administrator, but, for instance in the example above, you can see a few threads of the kernel NFS server running... They all look the same, so it is likely that nobody will notice an extra line like those (except for the sequential PIDs)... However not everybody runs an NFS server... the `kworker` option sounds better but process status is `I`. That means it is a kernel `Idle` task that I believe is an state we cannot reach from user space... But I haven't researched this. Let me know in the comments if you can get that.

The `jdb2` is the _Journaling Block Device_ basically will be there whenever you use a file system with journaling capabilities like `ext3` or `ext4` common on GNU/Linux boxes. This appears with an `S` status that means, _Sleeping_ and that is what our RAT will do most of the time, so, let's go for this one.

We will change the process name, but we will also try to mimic the other columns values. A normal user will not notice those but a system administrator will and may find them suspicious, so we better are methodical. When we are done with this you will find a funny coincidence...

## Changing a process name
Let's first take a look to how does a normal process looks like. Let's compile this simple program that just go into sleep mode waiting for some user input (that is what `getchar` does... gets a char):

```C
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]) {
  printf ("This is RAT0X0 version 0.1\n");
  getchar ();
}
```

Now, let's compile and run it and on a different terminal let's see how `ps` sees this simple process:

    $ ps -ax | grep rat
    (...)
    23022 pts/24   S+     0:00 ./rat0x0-01

That doesn't look promising. We could already change the name to `jbd2SOMETHING` on the disk before running the program but that will be suspicious and unlikely to be executed by a user.... We should assume that the program will have an arbitrary name.

The simplest way to change the name is to overwrite the `argv[0]` parameter that is actually the file name of the program being executed:

```C
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]) {
  printf ("This is RAT0X0 version 0.1\n");
  strcpy (argv[0],"[Jbd2/sda0-8]");
  getchar ();
}

```

> Note: In principle your target name shall have the same length or less than the original one so it will fit in the current assigned memory. Otherwise you will need to shuffle around the stack to make room for the extra characters...

Let's try:

    $ ps -ax | grep rat
    (...)
    23431 pts/39   S+     0:00 grep rat

No trace of `rat0x0`. Let's see how this new `jbd2` looks like:

    $ ps -ax | grep bd2
       358 ?        S      0:27 [jbd2/sda5-8]
       991 ?        S      1:01 [jbd2/sda6-8]
     23371 pts/24   S+     0:00 [Jbd2/sda0-8]

I have just add a capital `J` to easily identify the process during development (you will have to kill it a few times). Note that `sda0` is not a valid partition name so it should be safe... alternatively you could also swap the `b` and the `d` in the name and use a valid partition name... Well, you can try different options to chose a name that will be unnoticeable, at least at first glance... 

However note than, when people suspect that there is something doggy going on, they will start looking on the details and sooner than later this will be uncover... As I said this is the most simplistic way to hide your RAT. And it works as far as nothing suspicious happens.

Anyhow, our process still shows a PTY (that means it is associated to a terminal) and also there is a `+` after the `S` that indicated that it is in the foreground.

# Going background
What comes next is basically the process you follow to code a classical daemon. Yes, no hacker/malware developer black magic... just good old system programming. When writing a daemon you need to disconnect the process of any terminal, session and process group so the process doesn't get terminated unexpectedly. You do a couple things more, but that is the very minimum to become a `daemon Let's go step by step. 

First, let's go background.

The way to achieve this is to `fork` and kill our parent. All this process management things always sounds funny when explained with words :). When we `fork`, we create a new process that is an exact copy of the original one. The only difference between the father and the child is that, after `fork` the PID of the child is returned to the father and 0 is returned to the child. Both process continuing execution in the line just after `fork` in the program. Usually the father process creates a child to do something and get some result, so it will have to eventually wait for the process to finish and for that it needs the PID (well, it is better to know it). The child doesn't need that, and can get its PID at any time just calling `getpid()`. No it won't know its parent.  Our powerful `rat0x0` will look like this now

```C
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char *argv[]) {
  printf ("This is RAT0X0 version 0.1\n");
  strcpy (argv[0],"[Jbd2/sda0-8]");
  pid_t pid = fork();
  if (pid!=0) return 0;

  while (1) usleep (1000);
}
```

As you can see we just create a new copy of ourselves using `fork` and then we kill the parent (the one that received a PID different of 0). I have also removed the `getchar` because we are trying to get our process disconnected from the terminal and that is a function that uses `stdin`.

If we run this process, `ps` will show now this:

    $ ps -ax | grep bd2
      358 ?        S      0:27 [jbd2/sda5-8]
      991 ?        S      1:01 [jbd2/sda6-8]
    26956 pts/24   S      0:00 [Jbd2/sda0-8]

Great... We have got rid of the `+`... But we still have an associated terminal to our process.

# Getting rid of the terminal
The way to release the terminal associated to the process is to actually leave the current session. I'm not going to go into the details about session and process groups here. In simple words, you start a session when you log-in. Any process you create after that belongs to the session. This allows the system to know which process to kill when the session is closed by the user. Did you ever wonder how the OS magically know what to kill if you just leave your session?

Daemons, and also our cute RAT do not want to get killed when the user closes the session from which they were started... well, daemons are usually started by the system at start up, but just in case you start one of them manually in a terminal. The way to do this is to create a new session for our process. And we do this with the `setsid` system call:

```C
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char *argv[]) {
  printf ("This is RAT0X0 version 0.1\n");
  strcpy (argv[0],"[Jbd2/sda0-8]");
  pid_t pid = fork();
  if (pid!=0) return 0;
  setsid ();
  while (1) usleep (1000);
}
```

Now we can compile and check again:

    $ ps -ax | grep bd2
      358 ?        S      0:27 [jbd2/sda5-8]
      991 ?        S      1:01 [jbd2/sda6-8]
    27496 ?        Ss     0:00 [Jbd2/sda0-8]


Damn, what is that `s` that just popped up?

# Stop being a Process Leader
If you read the man page for `setsid` it says:

> setsid()  creates  a new session if the calling process is not a process group leader.  The calling process is the leader of the new session (i.e., its session ID is made the same as its process ID).  The calling process also becomes the process group leader  of  a new process group in the session (i.e., its process group ID is made the same as its process ID).

Also you can check the man page for `ps` to veryfy what does the `s` means in its output.

Done?... There you go... That is what and where the `s` is/comes from. So, to stop being a session leader for this session, we have to... `fork` again.

```C
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char *argv[]) {
  pid_t pid;
  printf ("This is RAT0X0 version 0.1\n");
  strcpy (argv[0],"[Jbd2/sda0-8]");
  if ((pid = fork()) != 0) return 0;
  setsid(); // Remove TTY
  if ((pid = fork()) != 0) return 0;

  while (1) usleep (1000);
}

```

    $ ps -ax | grep bd2
      358 ?        S      0:27 [jbd2/sda5-8]
      991 ?        S      1:01 [jbd2/sda6-8]
    27742 ?        S      0:00 [Jbd2/sda0-8]

And there you go.... Same fingerprint than the kernel processes.... Well, the PID is way higher... Nobody is perfect.

> PIDs are reused when the maximal PID number is reached. This value is defined at `/proc/sys/kernel/pid_max` and usually is 32768... So you can start forking and exiting process until you get a pid below 1000/2000... Haven't tried this but it should work... Just do not fork to fast or you will look like a fork bomb

The code above is pretty simple and you should be able to code the assembler version by yourself. Just add the two new system calls in the `mfw.asm` file and do a couple comparisons and conditional jumps. If you do not want to do it yourself and you have any issue, be free to ask and , in any case, I will add the code in a later instalment anyway.

# Conclusion
In this instalment we have gone quickly through the main features of a RAT and after that we have defined a simple one to use as example in the comming instalments. It has very few features so we can keep it simple while we learn more about our system. So far we have learn how to manipulate the way the process is shown for system tools like `ps` and `top`... and at the same time, accidentaly :), we have learn how to convert any program in a `daemon`. So far, `rat0x0` just looks like a regular system daemon... nothing really special about it. That is the key of a good RAT... just look normal.




* Core functionality
- Remote access -> execution of any application
- File transfer
- Hiding
- C&C comms
- Enumeration -> Information extraction (system information querying)
- Privilege scalation -> Exploits
- Modules
  * Sniffer
  * Network Scanning pivoting
  * Credential extraction
  * Extract metadata from documents (mails, videos,...)
