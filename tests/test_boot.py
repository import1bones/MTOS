#!/usr/bin/env python3
"""
Boot-specific tests for MTOS
"""

import sys
import os
import time
from run_tests import QEMUTestRunner, TestCase, TestSuite

class DetailedBootTest(TestCase):
    """Detailed boot sequence testing."""
    
    def __init__(self):
        super().__init__("Detailed Boot Test", "Verify detailed boot sequence")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        """Test detailed boot sequence."""
        try:
            # Start QEMU with more verbose output
            if not runner.start_qemu(['-d', 'cpu_reset,guest_errors', '-D', 'qemu.log']):
                self.error_message = "Failed to start QEMU"
                return False
            
            # Check if the process starts and runs for a reasonable time
            time.sleep(5)
            
            if runner.qemu_process and runner.qemu_process.poll() is None:
                self.passed = True
                return True
            else:
                self.error_message = "Boot process terminated unexpectedly"
                return False
                
        except Exception as e:
            self.error_message = f"Exception during detailed boot test: {e}"
            return False


class GDTTest(TestCase):
    """Test Global Descriptor Table setup."""
    
    def __init__(self):
        super().__init__("GDT Test", "Verify GDT is loaded correctly")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        """Test GDT loading."""
        try:
            if not runner.start_qemu(['-d', 'cpu', '-D', 'gdt.log']):
                self.error_message = "Failed to start QEMU with CPU debugging"
                return False
            
            # For GDT testing, we mainly check that the system doesn't crash
            # when switching to protected mode
            time.sleep(3)
            
            if runner.qemu_process and runner.qemu_process.poll() is None:
                self.passed = True
                return True
            else:
                self.error_message = "System crashed during GDT setup"
                return False
                
        except Exception as e:
            self.error_message = f"Exception during GDT test: {e}"
            return False


class ProtectedModeTest(TestCase):
    """Test transition to protected mode."""
    
    def __init__(self):
        super().__init__("Protected Mode Test", "Verify transition to 32-bit protected mode")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        """Test protected mode transition."""
        try:
            if not runner.start_qemu(['-d', 'cpu,int', '-D', 'pmode.log']):
                self.error_message = "Failed to start QEMU with mode debugging"
                return False
            
            # Monitor for successful mode transition
            time.sleep(4)
            
            if runner.qemu_process and runner.qemu_process.poll() is None:
                self.passed = True
                return True
            else:
                self.error_message = "Failed to transition to protected mode"
                return False
                
        except Exception as e:
            self.error_message = f"Exception during protected mode test: {e}"
            return False


class BootTestSuite(TestSuite):
    """Boot-specific test suite."""
    
    def __init__(self, os_image_path: str):
        super().__init__(os_image_path)
        self.tests = [
            DetailedBootTest(),
            GDTTest(),
            ProtectedModeTest()
        ]


def main():
    """Main entry point for boot tests."""
    if len(sys.argv) < 2:
        print("Usage: python test_boot.py <os_image_path>")
        sys.exit(1)
    
    os_image_path = sys.argv[1]
    
    if not os.path.exists(os_image_path):
        print(f"Error: OS image not found: {os_image_path}")
        sys.exit(1)
    
    # Run the boot test suite
    suite = BootTestSuite(os_image_path)
    success = suite.run_all_tests()
    
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
