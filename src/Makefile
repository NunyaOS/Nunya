OBJECTS=kernelcore.o main.o console.o $(MEMORY_OBJS) keyboard.o clock.o interrupt.o pic.o ata.o string.o graphics.o font.o syscall.o syscall_handler.o process.o mutex.o list.o pagetable.o rtc.o disk.o math.o window.o cmd_line.o $(TEST_OBJS) iso.o fs_terminal_commands.o
OBJECTS+=$(DEBUG_OBJS)
OBJECTS+=$(MOUSE_OBJS)
OBJECTS+=$(PROCESS_OBJS)

MOUSE_OBJS = mouse.o ps2.o
MEMORY_OBJS=memory_raw.o kmalloc.o
TEST_OBJS=module_tests.o testing.o tests.o
PROCESS_OBJS = sys_process_lifecycle.o
DEBUG_OBJS=debug_kernel.o

KERNEL_CCFLAGS=-Wall -c -ffreestanding -m32 -march=i386
KERNEL_LDFLAGS=-m elf_i386

all: basekernel.iso files.iso

debug: KERNEL_CCFLAGS += -DNUNYA_KDEBUG
debug: basekernel.iso

basekernel.iso: basekernel.img
	genisoimage -J -R -o basekernel.iso -b basekernel.img basekernel.img
	rm basekernel.img

basekernel.img: bootblock kernel
	cat bootblock kernel > basekernel.img
	truncate basekernel.img --size 1474560

bootblock: bootblock.o
	ld ${KERNEL_LDFLAGS} -Ttext 0 -s --oformat binary $< -o $@

kernel: ${OBJECTS}
	ld ${KERNEL_LDFLAGS} -Ttext 0x10000 -s --oformat binary ${OBJECTS} -o $@

files.iso: bin/test_userproc_even.nun #this should be bin/%.nun but it doesn't work"
	mv bin/*.nun ../files/bin/ && genisoimage -o ../files.iso ../files/

bin/%.nun: bin/%.o
	ld ${KERNEL_LDFLAGS} -Ttext 0x80000000 -s --oformat binary $< syscall.o -o $@

bin/%.o: bin/%.c
	gcc ${KERNEL_CCFLAGS} $< -o $@

%.o: %.c
	gcc ${KERNEL_CCFLAGS} $< -o $@

%.o: %.S
	gcc ${KERNEL_CCFLAGS} $< -o $@

%.s: %.c
	gcc -Wall -S -ffreestanding -m32 -march=i386 $< -o $@

clean:
	rm -rf basekernel.iso basekernel.img bootblock kernel *.o files.iso *.nun
