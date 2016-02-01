[![Build Status](https://travis-ci.org/NunyaOS/Nunya.svg?branch=master)](https://travis-ci.org/NunyaOS/Nunya)

# NunyaOS, an operating system. 

NunyaOS is an ongoing project, an experimental operating system whose overarching goal is hierarchy and process containment. Security and permission are central tenets of the system, which led to its name: Nunya, short for "None of your business!"


## How to Run NunyaOS

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
