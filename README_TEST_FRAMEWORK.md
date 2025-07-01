# MTOS Test Framework

A comprehensive test framework for MTOS (Mute OS) using QEMU virtualization.

## Overview

This test framework provides automated testing capabilities for the MTOS simple operating system. It uses QEMU to virtualize the OS and Python scripts to orchestrate and monitor the testing process.

## Features

### Test Categories
- **Boot Tests**: Verify bootloader and OS initialization
- **Memory Tests**: Test memory operations, ELF loading, and memory management
- **Integration Tests**: Full system testing and stability checks

### Framework Capabilities
- Automated QEMU process management
- Cross-platform support (Windows, Linux, macOS)
- Detailed logging and debugging output
- Timeout handling and error detection
- Pattern-based output monitoring
- Performance and stability testing

## Quick Start

### 1. Setup Environment
```bash
python setup_tests.py
```

### 2. Build the OS
```bash
make clean && make
```

### 3. Run Tests
```bash
# Run all tests
make test

# Run specific test suites
make test-boot
make test-memory

# Manual QEMU testing
make run
make debug
```

## File Structure

```
MTOS/
├── Makefile              # Build system
├── setup_tests.py        # Setup verification
├── boot/
│   ├── boot.S           # Bootloader assembly
│   ├── boot.ld          # Linker script
│   └── main.c           # Bootloader C code
├── include/
│   ├── elf.h            # ELF format definitions
│   ├── types.h          # Type definitions
│   └── x86.h            # x86 specific functions
├── tests/
│   ├── README.md        # Detailed test documentation
│   ├── run_tests.py     # Main test runner
│   ├── test_boot.py     # Boot sequence tests
│   ├── test_memory.py   # Memory operation tests
│   ├── test_integration.py # Integration tests
│   └── test_config.py   # Test configuration
└── build/               # Build output directory
    ├── mtos.img         # Final OS image
    └── ...              # Other build artifacts
```

## Prerequisites

- **QEMU**: For OS virtualization
- **GCC toolchain**: For building the OS
- **Python 3.6+**: For running the test framework
- **Make**: For build automation

### Installation

**Windows:**
- Install QEMU from https://www.qemu.org/download/
- Install MinGW-w64 or Visual Studio Build Tools
- Ensure tools are in PATH

**Linux:**
```bash
sudo apt-get update
sudo apt-get install qemu-system-x86 gcc binutils make python3
```

**macOS:**
```bash
brew install qemu gcc make python3
```

## Testing Process

### 1. Boot Tests
- Real mode to protected mode transition
- GDT (Global Descriptor Table) setup
- Basic CPU initialization
- Memory detection

### 2. Memory Tests
- ELF header validation
- Program loading
- Memory access patterns
- Stack operations
- Segment configuration

### 3. Integration Tests
- Complete boot cycle
- Performance measurement
- System stability
- Error handling

## Build System

The Makefile provides several targets:

```bash
# Build targets
make all          # Build complete OS image
make clean        # Clean build artifacts

# Test targets
make test         # Run all tests
make test-boot    # Boot tests only
make test-memory  # Memory tests only
make test-all     # Comprehensive testing

# QEMU targets
make run          # Run OS in QEMU
make debug        # Run with debugger support

# Development
make dump         # Disassemble bootloader
```

## Test Output

Tests generate detailed logs:
- `qemu.log` - General QEMU output
- `gdt.log` - GDT setup debugging
- `memory.log` - Memory operations
- `full_boot.log` - Complete boot trace

## Debugging

### QEMU Debug Options
- CPU state monitoring: `-d cpu`
- Memory access tracing: `-d mmu`
- Guest error detection: `-d guest_errors`
- Instruction tracing: `-d in_asm`

### GDB Integration
```bash
# Terminal 1: Start QEMU with GDB support
make debug

# Terminal 2: Connect GDB
gdb
(gdb) target remote :1234
(gdb) set architecture i8086
(gdb) break *0x7c00
(gdb) continue
```

## Extending the Framework

### Adding New Tests

1. Create a test class:
```python
class MyTest(TestCase):
    def __init__(self):
        super().__init__("Test Name", "Description")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        # Test implementation
        return True
```

2. Add to test suite:
```python
self.tests = [MyTest(), ...]
```

### Custom Test Scripts
Create new test files following the pattern in `tests/` directory.

## Common Issues

### QEMU Not Found
Ensure QEMU is installed and in PATH. Run `python setup_tests.py` to verify.

### Build Failures
Check that GCC toolchain is properly installed:
```bash
gcc --version
as --version
ld --version
```

### Test Timeouts
Increase timeout values in test configuration for slower systems.

### Permission Issues
Ensure write permissions for build and test output directories.

## Contributing

When adding new features:
1. Add corresponding tests
2. Update documentation
3. Verify cross-platform compatibility
4. Test with different QEMU versions

## Technical Details

### Memory Layout
The OS follows the standard x86 memory layout:
- 0x7C00: Boot sector load address
- 0x10000: ELF header location
- Real mode → Protected mode transition

### Boot Process
1. BIOS loads boot sector at 0x7C00
2. Boot sector enables A20 line
3. Sets up GDT and switches to protected mode
4. Loads and parses ELF kernel
5. Jumps to kernel entry point

### Test Framework Architecture
- **QEMUTestRunner**: Manages QEMU processes
- **TestCase**: Base class for individual tests
- **TestSuite**: Organizes and runs test collections
- **Output monitoring**: Pattern-based result detection

This framework provides a solid foundation for testing and developing the MTOS operating system.
