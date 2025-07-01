# MTOS Modular Build System
CC = gcc
AS = gas
LD = ld
OBJCOPY = objcopy
OBJDUMP = objdump

# Compiler flags
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -I.
ASFLAGS = --32
LDFLAGS = -m elf_i386

# Directories
BOOTDIR = boot
INCLUDEDIR = include
KERNELDIR = kernel
TESTDIR = tests
BUILDDIR = build
RUSTDIR = userspace

# Source files
BOOT_ASM = $(BOOTDIR)/boot.S
BOOT_C = $(BOOTDIR)/main.c
HEADERS = $(wildcard $(INCLUDEDIR)/*.h) $(wildcard $(KERNELDIR)/**/*.h)

# Kernel modules (include all implementations)
KERNEL_INTERFACES = $(KERNELDIR)/interfaces/kernel_interfaces.c
KERNEL_MEMORY_BITMAP = $(KERNELDIR)/memory/bitmap_allocator.c
KERNEL_MEMORY_BUDDY = $(KERNELDIR)/memory/buddy_allocator.c
KERNEL_SCHEDULER_RR = $(KERNELDIR)/scheduler/round_robin_scheduler.c
KERNEL_SCHEDULER_PRIORITY = $(KERNELDIR)/scheduler/priority_scheduler.c
KERNEL_IPC_MSGQUEUE = $(KERNELDIR)/ipc/message_queue_ipc.c
KERNEL_IPC_SHMEM = $(KERNELDIR)/ipc/shared_memory_ipc.c

# All kernel source files
KERNEL_SOURCES = $(KERNEL_INTERFACES) $(KERNEL_MEMORY_BITMAP) $(KERNEL_MEMORY_BUDDY) \
                 $(KERNEL_SCHEDULER_RR) $(KERNEL_SCHEDULER_PRIORITY) \
                 $(KERNEL_IPC_MSGQUEUE) $(KERNEL_IPC_SHMEM)

# Object files
BOOT_OBJ = $(BUILDDIR)/boot.o
MAIN_OBJ = $(BUILDDIR)/main.o

# Kernel object files (build all implementations)
KERNEL_OBJS = $(BUILDDIR)/kernel_interfaces.o \
              $(BUILDDIR)/bitmap_allocator.o \
              $(BUILDDIR)/buddy_allocator.o \
              $(BUILDDIR)/round_robin_scheduler.o \
              $(BUILDDIR)/priority_scheduler.o \
              $(BUILDDIR)/message_queue_ipc.o \
              $(BUILDDIR)/shared_memory_ipc.o

# Output files
BOOTLOADER = $(BUILDDIR)/bootloader.bin
KERNEL_ELF = $(BUILDDIR)/kernel.elf
OS_IMAGE = $(BUILDDIR)/mtos.img

# Module selection (can be overridden)
MEMORY_ALLOCATOR ?= bitmap
SCHEDULER ?= round_robin
IPC_MECHANISM ?= message_queue

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

# Compile kernel interfaces
$(BUILDDIR)/kernel_interfaces.o: $(KERNEL_INTERFACES) $(HEADERS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile memory allocators
$(BUILDDIR)/bitmap_allocator.o: $(KERNEL_MEMORY_BITMAP) $(HEADERS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/buddy_allocator.o: $(KERNEL_MEMORY_BUDDY) $(HEADERS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile schedulers
$(BUILDDIR)/round_robin_scheduler.o: $(KERNEL_SCHEDULER_RR) $(HEADERS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/priority_scheduler.o: $(KERNEL_SCHEDULER_PRIORITY) $(HEADERS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile IPC mechanisms
$(BUILDDIR)/message_queue_ipc.o: $(KERNEL_IPC_MSGQUEUE) $(HEADERS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/shared_memory_ipc.o: $(KERNEL_IPC_SHMEM) $(HEADERS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Link bootloader
$(BOOTLOADER): $(BOOT_OBJ) $(MAIN_OBJ) | $(BUILDDIR)
	$(LD) $(LDFLAGS) -T $(BOOTDIR)/boot.ld -o $(BUILDDIR)/bootloader.elf $^
	$(OBJCOPY) -S -O binary $(BUILDDIR)/bootloader.elf $@

# Create kernel (minimal for now)
$(KERNEL_ELF): $(KERNEL_OBJS) | $(BUILDDIR)
	$(LD) $(LDFLAGS) -T $(KERNELDIR)/kernel.ld -o $@ $^

# Create OS image
$(OS_IMAGE): $(BOOTLOADER) | $(BUILDDIR)
	dd if=/dev/zero of=$@ bs=512 count=2880 2>/dev/null || dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=$< of=$@ conv=notrunc 2>/dev/null || dd if=$< of=$@ conv=notrunc

# Modular build targets
build-with-allocator:
	$(MAKE) MEMORY_ALLOCATOR=$(ALLOCATOR) all

build-with-scheduler:
	$(MAKE) SCHEDULER=$(SCHED) all

build-with-ipc:
	$(MAKE) IPC_MECHANISM=$(IPC) all

# Educational build targets
build-bitmap: 
	$(MAKE) MEMORY_ALLOCATOR=bitmap all

build-buddy:
	$(MAKE) MEMORY_ALLOCATOR=buddy all

build-round-robin:
	$(MAKE) SCHEDULER=round_robin all

build-priority:
	$(MAKE) SCHEDULER=priority all

build-message-queue:
	$(MAKE) IPC_MECHANISM=message_queue all

build-shared-memory:
	$(MAKE) IPC_MECHANISM=shared_memory all

# Build all combinations
build-all-variants:
	@echo "Building all implementation variants..."
	$(MAKE) MEMORY_ALLOCATOR=bitmap SCHEDULER=round_robin IPC_MECHANISM=message_queue all
	@echo "Built: Bitmap + Round Robin + Message Queue"
	$(MAKE) MEMORY_ALLOCATOR=buddy SCHEDULER=priority IPC_MECHANISM=shared_memory all
	@echo "Built: Buddy + Priority + Shared Memory"
	@echo "All variants built successfully!"

# Component testing
test-allocator:
	$(MAKE) build-$(ALLOCATOR)
	python $(TESTDIR)/test_memory.py $(OS_IMAGE)

test-scheduler:
	$(MAKE) build-$(SCHED)
	python $(TESTDIR)/test_scheduler.py $(OS_IMAGE)

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR)

# Test targets
test: $(OS_IMAGE)
	python $(TESTDIR)/run_tests.py $<

test-boot: $(OS_IMAGE)
	python $(TESTDIR)/test_boot.py $<

test-memory: $(OS_IMAGE)
	python $(TESTDIR)/test_memory.py $<

test-all: test

# Educational targets
demo-allocators:
	@echo "Building OS with different allocators..."
	$(MAKE) build-bitmap
	@echo "✅ Bitmap allocator built"
	$(MAKE) build-buddy
	@echo "✅ Buddy allocator built"
	@echo "Run 'make compare-allocators' to compare performance"

demo-schedulers:
	@echo "Building OS with different schedulers..."
	$(MAKE) build-round-robin
	@echo "✅ Round-robin scheduler built"
	$(MAKE) build-priority
	@echo "✅ Priority scheduler built"
	@echo "Run 'make compare-schedulers' to compare performance"

demo-ipc:
	@echo "Building OS with different IPC mechanisms..."
	$(MAKE) build-message-queue
	@echo "✅ Message queue IPC built"
	$(MAKE) build-shared-memory
	@echo "✅ Shared memory IPC built"
	@echo "Run 'make compare-ipc' to compare performance"

demo-all:
	@echo "Building all available implementations..."
	$(MAKE) demo-allocators
	$(MAKE) demo-schedulers
	$(MAKE) demo-ipc
	@echo "🎉 All implementations built successfully!"

compare-allocators:
	python $(TESTDIR)/compare_components.py allocators

compare-schedulers:
	python $(TESTDIR)/compare_components.py schedulers

compare-ipc:
	python $(TESTDIR)/compare_components.py ipc

# QEMU targets
run: $(OS_IMAGE)
	qemu-system-i386 -drive file=$<,index=0,if=floppy,format=raw -monitor stdio

debug: $(OS_IMAGE)
	qemu-system-i386 -drive file=$<,index=0,if=floppy,format=raw -monitor stdio -s -S

run-with-allocator: build-with-allocator
	qemu-system-i386 -drive file=$(OS_IMAGE),index=0,if=floppy,format=raw -monitor stdio

# Development helpers
dump: $(BUILDDIR)/bootloader.elf
	$(OBJDUMP) -d $<

# Educational reports
education-report:
	@echo "MTOS Educational Build System"
	@echo "============================"
	@echo "Available allocators: bitmap, buddy, slab, first_fit"
	@echo "Available schedulers: round_robin, priority, cfs, rt, mlfq, lottery"
	@echo "Available IPC: message_queue, shared_memory, pipe, capability, actor, rpc"
	@echo ""
	@echo "Example usage:"
	@echo "  make ALLOCATOR=buddy test-allocator"
	@echo "  make SCHED=cfs test-scheduler"
	@echo "  make demo-allocators"
	@echo "  make compare-schedulers"

# Help target
help: education-report
	@echo ""
	@echo "Build targets:"
	@echo "  all                 - Build complete OS image"
	@echo "  clean               - Clean build artifacts"
	@echo ""
	@echo "Modular builds:"
	@echo "  build-bitmap        - Build with bitmap allocator"
	@echo "  build-buddy         - Build with buddy allocator"
	@echo "  build-round-robin   - Build with round-robin scheduler"
	@echo "  build-priority      - Build with priority scheduler"
	@echo "  build-cfs           - Build with CFS scheduler"
	@echo ""
	@echo "Testing:"
	@echo "  test                - Run all tests"
	@echo "  test-boot           - Boot tests only"
	@echo "  test-memory         - Memory tests only"
	@echo "  ALLOCATOR=X test-allocator - Test specific allocator"
	@echo "  SCHED=X test-scheduler     - Test specific scheduler"
	@echo ""
	@echo "Educational demos:"
	@echo "  demo-allocators     - Build all allocator variants"
	@echo "  demo-schedulers     - Build all scheduler variants"
	@echo "  compare-allocators  - Performance comparison"
	@echo "  compare-schedulers  - Performance comparison"
	@echo ""
	@echo "QEMU:"
	@echo "  run                 - Run OS in QEMU"
	@echo "  debug               - Run with GDB support"
	@echo "  run-with-allocator  - Run with specific allocator"

.PHONY: all clean test test-boot test-memory test-all run debug dump help education-report demo-allocators demo-schedulers compare-allocators compare-schedulers compare-ipc
