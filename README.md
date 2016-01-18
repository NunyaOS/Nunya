# The Basekernel Operating System Kernel

Basekernel is a simple operating system kernel for research, teaching, and fun.

Basekernel is *not* a complete operating system, but it is a starting
point for those who wish to study and develop new operating system code.
If you want to build something *different* than Windows, Linux, or Multics,
Basekernel may be a good place to try out your new ideas.

Basekernel can boot an Intel PC-compatible machine (or VM) in 32-bit protected
mode, with support for VESA framebuffer graphics, ATA hard disks, ATAPI optical
devices, basic system calls, and paged virtual memory.  From there, it's
your job to write filesystem code, a windowing system, or whatever you like.

Our goal is to keep Basekernel simple and easy to understand,
so that others can build on it and take it in different directions.
We encourage you to fork the code, give it a new name,
and develop something completely new on top of it.

To be clear, this is raw low-level code, and there is no guarantee that
it will work on your particular machine, or work at all. If you fix bugs
or solve build problems, we would be happy to accept your contributions.

## How to Run Basekernel

From a standard Linux X86 machine with the GCC compiler:

```
cd src
make
```

That will create `basekernel.iso`, which is an image of an optical disk that can be mounted in a virtual machine.  Next, set up a virtual machine system
like VMWare, VirtualBox, or Bochs, and direct it to use that image. In VirtualBox, for example, you can select `Type > Linux` and `Version > Other Linux (32-bit)`
Finally, point the virtual machine to `src > basekernel.iso`

You should see something like this:

<img src=screenshot.png align=center>

## Tweaks and Projects

Now that you have the code running, here is a whole raft of ideas to
try, starting some single-line code changes to more elaborate projects:

* Modify main.c to display a custom startup message to welcome the user.

* Tweak console.c to display things in a color of your choosing.

* Examine the rtc.c module, and write a little code in main.c
that will display a real-time clock.

* Modify the infinite loop in main.c to display a kernel command
prompt, and read some commands from the user to do things like 
show system statistics, display the current time, reboot, etc.

* Add some functions for drawing lines and shapes to graphics.c, and then use
in main.c to make an interesting status display for your operating system.

* Modify the console code so that it scrolls the display up when you
reach the bottom, rather than clearing and starting over.

* Try writing to an address somewhere into the (unmapped) user address space
above 0x8000000.  Notice how you get a page fault, and the system halts?
Modify the page fault handler to automatically allocate a fresh memory page
and map it into the process each time this happens.

* Implement proper memory resource management.  Create some system calls
that allow each process to request/release memory in an orderly manner,
and map it into the process in a suitable location.  Establish a policy
for how many pages each process can get, and what happens when memory
is exhausted.

* Write some easily-recognizable data to a file, then mount it as a virtual disk
image in your virtual machine of choice.  Use the ATA driver to load and
display the raw data on the virtual disk.

* Write a read-only driver for the ISO-9660 (CDROM) filesystem, and use it with the  ATAPI driver to browse and access an entire filesystem tree.

* Write a read-write driver for a Unix-style filesystem with proper inodes
and directory entries.  Copy data from your CDROM filesystem into the Unix
filesystem, which you can now save across boot sessions.

* Implement loadable programs.  Using either a filesystem or just a raw disk
image file, load a user-level program compiled outside the kernel,
and execute it.  The simplest way is to compile straight to binary format
with a load address of 0x8000000, and then fall through process_switch().
A more sophisticated way is to create an A.OUT or ELF format executable,
and then write the code to load that into the kernel.

* Create a display system for multiple processes.  Building on the bitmapped
graphics module, create a set of virtual displays that allow each process
to display independently without interfering with each other.

* Port the system to 64-bit mode.  Start by modifying kernelcore.S to make
a second jump from 32 to 64 bit mode, then update all of the types and
kernel structures to accommodate the new mode.

* Create a driver for the PCI bus, which enumerates all devices and their configurations.

* Create an AHCI driver for SATA disks.

* Create an OHCI or EHCI driver for USB devices.

## Machine Organization

The following memory organization is used throughout the code.
To the extent possible, these definitions have been collected
in memorylayout.h, but changing any of them may require some
some significant code surgery.

### Physical Memory Layout

```
0000 0000 Reserved area for initial interrupt vector and BIOS data.
0000 7C00 (BOOTBLOCK_START) 512 bytes where bootblock.S is initially loaded.
0000 fff0 (INTERUPT_STACK_TOP) Initial location of kernel/interrupt stack
          until paging and user processes allocate their own stacks.
0001 0000 (KERNEL_START) Start of kernel code and data in kernelcore.S
0010 0000 (ALLOC_MEMORY_START)  Start of memory pages managed by memory.c
???? ???? Location of the video buffer, determined by video BIOS at runtime.
          Care must be taken in memory allocation and pagetable setup
          to avoid stomping on this area.
```

### Virtual Memory Layout

```
0000 0000 First 2GB of VM space for all processes is directly mapped to
          physical memory in kernel mode.  That way, kernel space is
          inaccessible in user mode, but kernel code can run correctly
          with paging activated.
8000 0000 (PROCESS_ENTRY_POINT) The upper 2GB of VM space for all processes
          is private to that process.  Each page of VM here is mapped to
          physical page allocated by memory.c.
ffff fff0 (PROCESS_STACK_INIT) The high end of the user space is designated
          for the user level stack, which grows down towards the middle
          of memory.
```

### Segmentation

```
		Start	Length		Type	Privilege
Segment 1	0	4GB		CODE	SUPERVISOR
Segment 2	0	4GB		DATA	SUPERVISOR
Segment 3	0	4GB		CODE	USER
Segment 4	0	4GB		DATA	USER
Segment 5	tss	sizeof(tss)	TSS	SUPERVISOR
```

For technical reasons, the X86 requires that segmentation be turned on whenever paging is used.  A few key places in interrupt handling and system startup require the use of segments.  So, we set up four segments that span all of memory, and use the one with the desired privilege wherever a segment is required.  The fifth segement identifies the Task State Structure (TSS) which is where the initial setup information for entering protected mode is stored.

## References

The [OS Development Wiki][1] is an excellent source of sample code and general orientation toward the topics that you need to learn. However, to get the details of each element right, you will need to dig into the source documents for each component of the system:

The [Intel CPU Manuals][2] are the definitive CPU reference.  Especially focus on the first five chapters of the * System Programming Guide * to fully understand the virtual memory model and exception handling.

The `doc` directory contains a few hard to find PDF specifications.
While they describe older hardware, standardized hardware has a high
degree of backwards compatibility, so these techniques continue to work.

[ATA Attachment-3 Interface][3] describes the interface
to traditional ATA disks, including both I/O registers and commands
interpreted by the disk.

[PhoenixBIOS Programmer's Guide][4] describes the BIOS
functions available in the early boot environment, while the system is
still in 16-bit mode.  These are used by bootblock.S and kernelcore.S
to load data from disk and initialize the system.

[VESA BIOS Extension (VBE)][5] describes extensions
to the BIOS interrupts specifically for setting and querying graphics
modes.  This is used by kernelcore.S just before jumping to protected mode.

[Super I/O Controller][6] describes a typical chipset
that provides basic support devices such as keyboard, serial ports,
timer, and real-time clock.  These can be accessed in any processor mode.
While your machine may not have this *specific* chipset, the basic
functions are widely compatible.

[1] http://wiki.osdev.org
[2] http://www.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html
[3] doc/ata-3-std.pdf
[4] doc/phoenix-bios.pdf
[5] doc/vesa-bios.pdf
[6] doc/super-io.pdf
