# MTOS Test Framework

This directory contains a comprehensive test framework for the MTOS (Mute OS) operating system using QEMU.

## Test Framework Components

### Core Framework
- `run_tests.py` - Main test framework and runner
- `test_config.py` - Configuration and environment setup

### Test Suites
- `test_boot.py` - Boot sequence testing
- `test_memory.py` - Memory operations testing  
- `test_integration.py` - Integration and stability testing

## Prerequisites

1. **QEMU** - Required for running the tests
   - Windows: Download from https://www.qemu.org/download/
   - Linux: `sudo apt-get install qemu-system-x86`
   - macOS: `brew install qemu`

2. **Python 3.6+** - For running the test framework

## Quick Start

1. Build the OS image first:
```bash
make clean && make
```

2. Run all tests:
```bash
make test
```

3. Or run specific test suites:
```bash
make test-boot    # Boot sequence tests
make test-memory  # Memory operation tests
```

## Manual Testing

### Setup Test Environment
```bash
python tests/test_config.py
```

### Run Individual Test Suites
```bash
# All tests
python tests/run_tests.py build/mtos.img

# Boot tests only
python tests/test_boot.py build/mtos.img

# Memory tests only
python tests/test_memory.py build/mtos.img

# Integration tests
python tests/test_integration.py build/mtos.img
```

### Manual QEMU Testing
```bash
# Run OS in QEMU
make run

# Debug with GDB
make debug
```

## Test Types

### Boot Tests
- **Detailed Boot Test**: Verifies complete boot sequence
- **GDT Test**: Checks Global Descriptor Table setup
- **Protected Mode Test**: Validates transition to 32-bit protected mode

### Memory Tests
- **ELF Loading Test**: Verifies ELF header parsing and loading
- **Memory Access Test**: Checks memory read/write operations
- **Stack Test**: Validates stack setup and operations
- **Segment Test**: Verifies segment register configuration

### Integration Tests
- **Full Boot Cycle**: Complete boot sequence testing
- **Performance Test**: Boot time and performance measurement
- **Stability Test**: Extended runtime stability testing

## Test Framework Features

### QEMU Integration
- Automatic QEMU process management
- Output capture and monitoring
- Timeout handling
- Cross-platform support

### Test Monitoring
- Pattern-based output matching
- Error detection and reporting
- Detailed logging and debugging
- Test result summarization

### Debugging Support
- QEMU debug output capture
- CPU state monitoring
- Memory access tracing
- Disk I/O monitoring

## Output Files

Tests generate several output files in the current directory:
- `qemu.log` - General QEMU debug output
- `gdt.log` - GDT setup debugging
- `pmode.log` - Protected mode transition
- `elf.log` - ELF loading debugging
- `memory.log` - Memory access tracing
- `stack.log` - Stack operation debugging
- `segments.log` - Segment register debugging
- `full_boot.log` - Complete boot cycle trace

## Extending the Framework

### Adding New Tests

1. Create a new test class inheriting from `TestCase`:
```python
class MyCustomTest(TestCase):
    def __init__(self):
        super().__init__("My Test", "Description of my test")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        # Implement test logic
        try:
            if not runner.start_qemu(['additional', 'args']):
                self.error_message = "Failed to start QEMU"
                return False
            
            # Test logic here
            # Use runner.wait_for_output() to monitor output
            
            self.passed = True
            return True
        except Exception as e:
            self.error_message = f"Test failed: {e}"
            return False
```

2. Add the test to a test suite:
```python
class MyTestSuite(TestSuite):
    def __init__(self, os_image_path: str):
        super().__init__(os_image_path)
        self.tests = [
            MyCustomTest(),
            # ... other tests
        ]
```

### Custom QEMU Arguments

Tests can specify custom QEMU arguments for specific debugging needs:
```python
runner.start_qemu([
    '-d', 'cpu,guest_errors',  # Debug flags
    '-D', 'debug.log',         # Debug log file
    '-trace', 'pattern*',      # Tracing
    '-monitor', 'stdio'        # Monitor interface
])
```

## Troubleshooting

### Common Issues

1. **QEMU not found**
   - Ensure QEMU is installed and in PATH
   - Run `python tests/test_config.py` to check

2. **Tests timing out**
   - Increase timeout values in test configuration
   - Check QEMU debug logs for issues

3. **Permission errors**
   - Ensure test directories are writable
   - Run with appropriate permissions

4. **Build failures**
   - Ensure build tools are installed (gcc, gas, ld)
   - Check Makefile for correct paths

### Debug Tips

1. Check QEMU logs for detailed error information
2. Use `make debug` for interactive debugging
3. Increase test timeouts for slow systems
4. Run individual test suites to isolate issues

## Platform-Specific Notes

### Windows
- Use PowerShell or Command Prompt
- Ensure QEMU is in PATH or use full path
- May need to adjust timeout values

### Linux
- Install qemu-system-x86 package
- Ensure user has access to /dev/kvm if available
- Use sudo if permission issues occur

### macOS
- Install via Homebrew: `brew install qemu`
- May need to allow QEMU in System Preferences
- Intel and Apple Silicon both supported
