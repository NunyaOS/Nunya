OBJECTS = kernelcore.o main.o console.o $(MEMORY_OBJS) keyboard.o clock.o interrupt.o pic.o ata.o string.o font.o syscall.o syscall_handler.o mutex.o list.o pagetable.o rtc.o disk.o math.o cmd_line.o $(TEST_OBJS) iso.o fs_terminal_commands.o
OBJECTS += $(DEBUG_OBJS)
OBJECTS += $(MOUSE_OBJS)
OBJECTS += $(FS_OBJS)
OBJECTS += $(WINDOW_OBJS)
OBJECTS += $(PROCESS_OBJS)

MOUSE_OBJS = mouse.o ps2.o
MEMORY_OBJS = memory_raw.o kmalloc.o
TEST_OBJS = module_tests.o testing.o tests.o
DEBUG_OBJS = debug_kernel.o
FS_OBJS = fs.o syscall_handler_fs.o
WINDOW_OBJS = window.o graphics.o syscall_handler_window.o window_manager.o
PROCESS_OBJS = syscall_handler_process.o syscall_handler_permissions.o process.o permissions_capabilities.o

BINARIES = bin/test_userproc_even.nun bin/test_window.nun
LIB_INCLUDE_PATH = ./include

KERNEL_CCFLAGS = -Wall -c -ffreestanding -m32 -march=i386
KERNEL_LDFLAGS = -m elf_i386

all: nunya.iso files.iso

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

%.o: %.c
	gcc ${KERNEL_CCFLAGS} -I ${LIB_INCLUDE_PATH} $< -o $@

%.o: %.S
	gcc ${KERNEL_CCFLAGS} $< -o $@

%.s: %.c
	gcc -Wall -S -ffreestanding -m32 -march=i386 -I ${LIB_INCLUDE_PATH} $< -o $@

files.iso: ${BINARIES}
	mv $^ ../files/bin/ && genisoimage -o ../files.iso ../files/

bin/%.nun: bin/%.o
	ld ${KERNEL_LDFLAGS} -Ttext 0x80000000 -s --oformat binary $< syscall.o -o $@

bin/%.o: bin/%.c
	gcc ${KERNEL_CCFLAGS} -I ${LIB_INCLUDE_PATH} $< -o $@

clean:
	rm -rf nunya.iso nunya.img bootblock kernel *.o ../files.iso *.nun
