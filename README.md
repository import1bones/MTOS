# MTOS (Modular Teaching Operating System)
Educational operating system with virtual interfaces and swappable implementations

## 🎯 Overview

MTOS is designed as an **educational operating system** that demonstrates different algorithms and approaches through a **virtual interface system**. Students can easily swap between different implementations of:

- **Memory Allocators**: Bitmap vs Buddy System
- **Process Schedulers**: Round-Robin vs Priority-based
- **IPC Mechanisms**: Message Queues vs Shared Memory

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────┐
│                  VIRTUAL INTERFACES                    │
├─────────────────┬─────────────────┬─────────────────────┤
│ Memory Mgmt     │   Scheduler     │       IPC           │
│                 │                 │                     │
│ ✅ Bitmap       │ ✅ Round Robin  │ ✅ Message Queue    │
│ ✅ Buddy        │ ✅ Priority     │ ✅ Shared Memory    │
│ 📝 Slab         │ 📝 CFS          │ 📝 Pipes            │
│ 📝 First-Fit    │ 📝 MLFQ         │ 📝 Capabilities     │
└─────────────────┴─────────────────┴─────────────────────┘
```

## 🚀 Quick Start

### 1. Setup Environment
```bash
# Check prerequisites and install dependencies
python setup_tests.py
```

### 2. Build with Different Implementations
```bash
# Build with bitmap allocator and round-robin scheduler
make MEMORY_ALLOCATOR=bitmap SCHEDULER=round_robin all

# Build with buddy allocator and priority scheduler  
make MEMORY_ALLOCATOR=buddy SCHEDULER=priority all

# Build all variants
make demo-all
```

### 3. Test and Compare
```bash
# Test boot sequence
make test-boot

# Compare memory allocators
python tests/compare_components.py allocators --visualize

# Compare schedulers
python tests/compare_components.py schedulers --visualize
```

## 📚 Available Implementations

### Memory Allocators
- **✅ Bitmap Allocator**: Simple, educational, linear search
- **✅ Buddy System**: Binary buddy with coalescing, efficient for power-of-2 sizes

### Process Schedulers  
- **✅ Round Robin**: Time-sliced, fair scheduling with configurable quantum
- **✅ Priority Scheduler**: Multi-level priority with aging to prevent starvation

### IPC Mechanisms
- **✅ Message Queues**: Classic message passing with configurable queues
- **✅ Shared Memory**: High-performance communication with basic synchronization

## 🎓 Educational Features

### Interactive Comparison
```bash
# Compare all memory allocators
make demo-allocators

# Compare all schedulers  
make demo-schedulers

# Compare all IPC mechanisms
make demo-ipc
```

### Performance Analysis
```bash
# Generate detailed performance reports
python tests/compare_components.py allocators --output report.txt

# Visualize scheduler fairness
python tests/compare_components.py schedulers --visualize
```

### Student Exercises
- Implement new algorithms (Slab allocator, CFS scheduler, etc.)
- Compare performance characteristics
- Analyze trade-offs between implementations
- Create custom benchmarks

## 🔧 Development
## 🔧 Development

### Build System
```bash
# Build specific components
make build-bitmap        # Bitmap allocator
make build-buddy          # Buddy allocator
make build-round-robin    # Round-robin scheduler
make build-priority       # Priority scheduler
make build-message-queue  # Message queue IPC
make build-shared-memory  # Shared memory IPC

# Build all combinations
make build-all-variants
```

### Testing Framework
```bash
# Run comprehensive tests
make test

# Test specific components
make test-memory ALLOCATOR=buddy
make test-scheduler SCHED=priority

# Integration testing
make test-integration
```

## 📖 Documentation

- **[Student Guide](STUDENT_GUIDE.md)**: Complete tutorial for students
- **[Test Framework](README_TEST_FRAMEWORK.md)**: Testing and benchmarking guide  
- **[API Reference](kernel/interfaces/kernel_interfaces.h)**: Virtual interface documentation

## 🎯 Learning Objectives

Students will learn:
- **Algorithm Comparison**: Direct performance and behavior comparison
- **System Design**: How abstraction enables modularity
- **Trade-offs**: Understanding the pros/cons of different approaches
- **Implementation**: Writing OS components with real constraints

## 🤝 Contributing

1. **Add New Implementations**: Use existing ones as templates
2. **Improve Documentation**: Add comments and educational content
3. **Create Benchmarks**: Design tests for specific scenarios
4. **Educational Content**: Develop exercises and learning materials

## Memory Structure
know *memory structure* is important for system,which memory can be allocated,which can't.this is a job of system.

	+----------+ <-0X FFFF FFFC
	|up to 4GB |
	|	       |
	\/\/\/\/\/\/

	/\/\/\/\/\/\
	|          |
	|extend RAM|
	+----------+ <-0X 0010 0000
	+----------+ <-0X 000F FFFC
	|BIOS   ROM|
	+----------+ <-0X 000F 0000
	+----------+ <-0X 000E FFFC
	|16 bits   |
	|devices,  |
	|expansion |
	|ROMs      |
	+----------+ <-0X 000C 0000
	+----------+ <-0X 000B FFFC
	|   VGA    |
	|  Display |
	+----------+ <-0X 000A 0000
	+----------+ <-0X 0009 FFFC
	|    LOW   |
	|  Memory  |
	+----------+ <-0X 0000 0000
	|<-4bytes->| HEAD Address of 4 bytes(32 bits)

physical address = 16 * segment + offset

## Testing Framework

MTOS now includes a comprehensive test framework using QEMU for automated testing.

### Quick Test Setup

1. **Install Prerequisites**:
   ```bash
   # Check setup
   python setup_tests.py
   ```

2. **Build and Test**:
   ```bash
   # Build the OS
   make clean && make
   
   # Run all tests
   make test
   
   # Run specific test suites
   make test-boot    # Boot sequence tests
   make test-memory  # Memory operation tests
   ```

3. **Manual Testing**:
   ```bash
   # Run OS in QEMU
   make run
   
   # Debug with GDB support
   make debug
   ```

### Test Framework Features

- **Automated QEMU Testing**: Full OS boot and execution testing
- **Cross-Platform**: Works on Windows, Linux, and macOS
- **Multiple Test Suites**: Boot, memory, and integration tests
- **Detailed Logging**: Comprehensive debug output and error tracking
- **CI/CD Ready**: GitHub Actions workflow included

### Test Categories

- **Boot Tests**: Bootloader functionality, GDT setup, protected mode transition
- **Memory Tests**: ELF loading, memory access patterns, stack operations
- **Integration Tests**: Full system testing, performance, and stability

For detailed testing documentation, see [README_TEST_FRAMEWORK.md](README_TEST_FRAMEWORK.md) and [tests/README.md](tests/README.md).
