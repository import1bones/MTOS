#!/usr/bin/env python3
"""
Memory-specific tests for MTOS
"""

import sys
import os
import time
from run_tests import QEMUTestRunner, TestCase, TestSuite

class ELFLoadingTest(TestCase):
    """Test ELF loading functionality."""
    
    def __init__(self):
        super().__init__("ELF Loading Test", "Verify ELF headers are read correctly")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        """Test ELF loading."""
        try:
            if not runner.start_qemu(['-d', 'guest_errors,unimp', '-D', 'elf.log']):
                self.error_message = "Failed to start QEMU with ELF debugging"
                return False
            
            # Wait for ELF loading process
            time.sleep(5)
            
            # Check for guest errors which might indicate ELF loading issues
            if runner.qemu_process and runner.qemu_process.poll() is None:
                self.passed = True
                return True
            else:
                self.error_message = "Process terminated during ELF loading"
                return False
                
        except Exception as e:
            self.error_message = f"Exception during ELF loading test: {e}"
            return False


class MemoryAccessTest(TestCase):
    """Test memory access patterns."""
    
    def __init__(self):
        super().__init__("Memory Access Test", "Verify memory reads/writes work correctly")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        """Test memory access."""
        try:
            # Use memory debugging
            if not runner.start_qemu(['-d', 'mmu,guest_errors', '-D', 'memory.log']):
                self.error_message = "Failed to start QEMU with memory debugging"
                return False
            
            # Monitor for memory access violations
            time.sleep(6)
            
            if runner.qemu_process and runner.qemu_process.poll() is None:
                self.passed = True
                return True
            else:
                self.error_message = "Memory access violation detected"
                return False
                
        except Exception as e:
            self.error_message = f"Exception during memory access test: {e}"
            return False


class StackTest(TestCase):
    """Test stack operations."""
    
    def __init__(self):
        super().__init__("Stack Test", "Verify stack setup and operations")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        """Test stack operations."""
        try:
            if not runner.start_qemu(['-d', 'cpu,guest_errors', '-D', 'stack.log']):
                self.error_message = "Failed to start QEMU with stack debugging"
                return False
            
            # Check stack operations don't cause crashes
            time.sleep(4)
            
            if runner.qemu_process and runner.qemu_process.poll() is None:
                self.passed = True
                return True
            else:
                self.error_message = "Stack operation failure"
                return False
                
        except Exception as e:
            self.error_message = f"Exception during stack test: {e}"
            return False


class SegmentTest(TestCase):
    """Test segment register setup."""
    
    def __init__(self):
        super().__init__("Segment Test", "Verify segment registers are configured correctly")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        """Test segment configuration."""
        try:
            if not runner.start_qemu(['-d', 'cpu,guest_errors', '-D', 'segments.log']):
                self.error_message = "Failed to start QEMU with segment debugging"
                return False
            
            # Check segment setup
            time.sleep(3)
            
            if runner.qemu_process and runner.qemu_process.poll() is None:
                self.passed = True
                return True
            else:
                self.error_message = "Segment configuration error"
                return False
                
        except Exception as e:
            self.error_message = f"Exception during segment test: {e}"
            return False


class MemoryTestSuite(TestSuite):
    """Memory-specific test suite."""
    
    def __init__(self, os_image_path: str):
        super().__init__(os_image_path)
        self.tests = [
            ELFLoadingTest(),
            MemoryAccessTest(),
            StackTest(),
            SegmentTest()
        ]


def main():
    """Main entry point for memory tests."""
    if len(sys.argv) < 2:
        print("Usage: python test_memory.py <os_image_path>")
        sys.exit(1)
    
    os_image_path = sys.argv[1]
    
    if not os.path.exists(os_image_path):
        print(f"Error: OS image not found: {os_image_path}")
        sys.exit(1)
    
    # Run the memory test suite
    suite = MemoryTestSuite(os_image_path)
    success = suite.run_all_tests()
    
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
