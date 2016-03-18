OBJECTS=kernelcore.o main.o console.o $(MEMORY_OBJS) keyboard.o clock.o interrupt.o pic.o ata.o string.o graphics.o font.o fs_sys.o syscall.o syscall_handler.o process.o mutex.o list.o pagetable.o rtc.o disk.o math.o window.o cmd_line.o $(TEST_OBJS) iso.o fs_terminal_commands.o
OBJECTS+=$(DEBUG_OBJS)
OBJECTS+=$(MOUSE_OBJS)

MOUSE_OBJS = mouse.o ps2.o
MEMORY_OBJS=memory_raw.o kmalloc.o
TEST_OBJS=module_tests.o testing.o tests.o
DEBUG_OBJS=debug_kernel.o

KERNEL_CCFLAGS=-Wall -c -ffreestanding -m32 -march=i386
KERNEL_LDFLAGS=-m elf_i386

all: nunya.iso

debug: KERNEL_CCFLAGS += -DNUNYA_KDEBUG
debug: nunya.iso

nunya.iso: nunya.img
	genisoimage -J -R -o nunya.iso -b nunya.img nunya.img
	rm nunya.img

nunya.img: bootblock kernel
	cat bootblock kernel > nunya.img
	truncate nunya.img --size 1474560

bootblock: bootblock.o
	ld ${KERNEL_LDFLAGS} -Ttext 0 -s --oformat binary $< -o $@

kernel: ${OBJECTS}
	ld ${KERNEL_LDFLAGS} -Ttext 0x10000 -s --oformat binary ${OBJECTS} -o $@

test_iso.iso: read_file
	mkdir ./test_iso && mv ./read_file ./test_iso/read_file && cp ./gettysbu.txt ./test_iso/gettysbu.txt && genisoimage -o ../test_iso.iso ./test_iso/ && rm -r ./test_iso

read_file: read_file.o syscall.h syscall.o
	ld ${KERNEL_LDFLAGS} -Ttext 0x80000000 -s --oformat binary read_file.o syscall.o -o $@

%.o: %.c
	gcc ${KERNEL_CCFLAGS} $< -o $@

%.o: %.S
	gcc ${KERNEL_CCFLAGS} $< -o $@

%.s: %.c
	gcc -Wall -S -ffreestanding -m32 -march=i386 $< -o $@

clean:
	rm -rf nunya.iso nunya.img bootblock kernel *.o
