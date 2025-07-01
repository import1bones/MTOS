# MOS
Mute OS
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
