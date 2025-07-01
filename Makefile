# MTOS Build System
CC = gcc
AS = gas
LD = ld
OBJCOPY = objcopy
OBJDUMP = objdump

# Compiler flags
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -I.
ASFLAGS = --32
LDFLAGS = -m elf_i386

# Directories
BOOTDIR = boot
INCLUDEDIR = include
TESTDIR = tests
BUILDDIR = build

# Source files
BOOT_ASM = $(BOOTDIR)/boot.S
BOOT_C = $(BOOTDIR)/main.c
HEADERS = $(wildcard $(INCLUDEDIR)/*.h)

# Object files
BOOT_OBJ = $(BUILDDIR)/boot.o
MAIN_OBJ = $(BUILDDIR)/main.o

# Output files
BOOTLOADER = $(BUILDDIR)/bootloader.bin
OS_IMAGE = $(BUILDDIR)/mtos.img

# Default target
all: $(OS_IMAGE)

# Create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Compile boot.S
$(BOOT_OBJ): $(BOOT_ASM) $(HEADERS) | $(BUILDDIR)
	$(AS) $(ASFLAGS) -o $@ $<

# Compile main.c
$(MAIN_OBJ): $(BOOT_C) $(HEADERS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Link bootloader
$(BOOTLOADER): $(BOOT_OBJ) $(MAIN_OBJ) | $(BUILDDIR)
	$(LD) $(LDFLAGS) -T $(BOOTDIR)/boot.ld -o $(BUILDDIR)/bootloader.elf $^
	$(OBJCOPY) -S -O binary $(BUILDDIR)/bootloader.elf $@

# Create OS image
$(OS_IMAGE): $(BOOTLOADER) | $(BUILDDIR)
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=$< of=$@ conv=notrunc

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR)

# Test targets
test: $(OS_IMAGE)
	python $(TESTDIR)/run_tests.py

test-boot: $(OS_IMAGE)
	python $(TESTDIR)/test_boot.py

test-memory: $(OS_IMAGE)
	python $(TESTDIR)/test_memory.py

test-all: test

# QEMU targets
run: $(OS_IMAGE)
	qemu-system-i386 -drive file=$<,index=0,if=floppy,format=raw -monitor stdio

debug: $(OS_IMAGE)
	qemu-system-i386 -drive file=$<,index=0,if=floppy,format=raw -monitor stdio -s -S

# Development helpers
dump: $(BUILDDIR)/bootloader.elf
	$(OBJDUMP) -d $<

.PHONY: all clean test test-boot test-memory test-all run debug dump
