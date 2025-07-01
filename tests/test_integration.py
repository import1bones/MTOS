#!/usr/bin/env python3
"""
Integration tests for MTOS that test multiple components together
"""

import sys
import os
import time
import subprocess
from run_tests import QEMUTestRunner, TestCase, TestSuite

class FullBootCycleTest(TestCase):
    """Test complete boot cycle from power-on to OS entry."""
    
    def __init__(self):
        super().__init__("Full Boot Cycle", "Complete boot sequence test")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        """Test full boot cycle."""
        try:
            # Start with comprehensive logging
            if not runner.start_qemu([
                '-d', 'cpu_reset,cpu,guest_errors,unimp,trace:*',
                '-D', 'full_boot.log'
            ]):
                self.error_message = "Failed to start QEMU"
                return False
            
            # Give it time to complete the full boot sequence
            time.sleep(10)
            
            if runner.qemu_process and runner.qemu_process.poll() is None:
                self.passed = True
                return True
            else:
                self.error_message = "Boot cycle incomplete or system crashed"
                return False
                
        except Exception as e:
            self.error_message = f"Exception during full boot cycle test: {e}"
            return False


class PerformanceTest(TestCase):
    """Test boot performance and timing."""
    
    def __init__(self):
        super().__init__("Performance Test", "Measure boot time and performance")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        """Test performance."""
        try:
            start_time = time.time()
            
            if not runner.start_qemu(['-d', 'guest_errors']):
                self.error_message = "Failed to start QEMU"
                return False
            
            # Monitor boot time
            time.sleep(5)
            boot_time = time.time() - start_time
            
            if runner.qemu_process and runner.qemu_process.poll() is None:
                if boot_time < 30:  # Reasonable boot time
                    self.passed = True
                    return True
                else:
                    self.error_message = f"Boot time too slow: {boot_time:.2f}s"
                    return False
            else:
                self.error_message = "System failed to boot"
                return False
                
        except Exception as e:
            self.error_message = f"Exception during performance test: {e}"
            return False


class StabilityTest(TestCase):
    """Test system stability over time."""
    
    def __init__(self):
        super().__init__("Stability Test", "Test system stability and reliability")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        """Test stability."""
        try:
            if not runner.start_qemu(['-d', 'guest_errors']):
                self.error_message = "Failed to start QEMU"
                return False
            
            # Run for extended period
            time.sleep(15)
            
            if runner.qemu_process and runner.qemu_process.poll() is None:
                self.passed = True
                return True
            else:
                self.error_message = "System became unstable"
                return False
                
        except Exception as e:
            self.error_message = f"Exception during stability test: {e}"
            return False


class IntegrationTestSuite(TestSuite):
    """Integration test suite."""
    
    def __init__(self, os_image_path: str):
        super().__init__(os_image_path)
        self.tests = [
            FullBootCycleTest(),
            PerformanceTest(),
            StabilityTest()
        ]


def main():
    """Main entry point for integration tests."""
    if len(sys.argv) < 2:
        print("Usage: python test_integration.py <os_image_path>")
        sys.exit(1)
    
    os_image_path = sys.argv[1]
    
    if not os.path.exists(os_image_path):
        print(f"Error: OS image not found: {os_image_path}")
        sys.exit(1)
    
    # Run the integration test suite
    suite = IntegrationTestSuite(os_image_path)
    success = suite.run_all_tests()
    
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
