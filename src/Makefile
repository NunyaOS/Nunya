OBJECTS=kernelcore.o main.o console.o $(MEMORY_OBJS) keyboard.o clock.o interrupt.o pic.o ata.o string.o graphics.o font.o syscall.o syscall_handler.o process.o mutex.o list.o pagetable.o rtc.o disk.o math.o window.o cmd_line.o $(TEST_OBJS) iso.o fs_terminal_commands.o

MEMORY_OBJS=memory_raw.o kmalloc.o
TEST_OBJS=module_tests.o testing.o tests.o

KERNEL_CCFLAGS=-Wall -c -ffreestanding -m32 -march=i386
KERNEL_LDFLAGS=-m elf_i386

all: basekernel.iso test_iso.iso

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

test_iso.iso: userproc
	mkdir ./test_iso && mv ./userproc ./test_iso/userproc && genisoimage -o ../test_iso.iso ./test_iso/ && rm -r ./test_iso

userproc: test_userproc.o syscall.h syscall.o
	ld ${KERNEL_LDFLAGS} -Ttext 0x80000000 -s --oformat binary test_userproc.o syscall.o -o $@

%.o: %.c
	gcc ${KERNEL_CCFLAGS} $< -o $@

%.o: %.S
	gcc ${KERNEL_CCFLAGS} $< -o $@

clean:
	rm -rf basekernel.iso basekernel.img bootblock userproc kernel ./test_iso *.o
