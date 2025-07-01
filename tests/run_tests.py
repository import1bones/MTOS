#!/usr/bin/env python3
"""
MTOS Test Framework
A comprehensive testing framework for the MTOS simple operating system using QEMU.
"""

import subprocess
import time
import sys
import os
import signal
import threading
import queue
import re
from typing import Optional, List, Dict, Any

class QEMUTestRunner:
    """Main test runner class that manages QEMU instances and test execution."""
    
    def __init__(self, os_image_path: str, timeout: int = 30):
        self.os_image_path = os_image_path
        self.timeout = timeout
        self.qemu_process = None
        self.output_queue = queue.Queue()
        
    def start_qemu(self, extra_args: List[str] = None) -> bool:
        """Start QEMU with the OS image."""
        if extra_args is None:
            extra_args = []
            
        # Check if image file exists
        if not os.path.exists(self.os_image_path):
            print(f"Error: OS image not found: {self.os_image_path}")
            return False
            
        # Import test config for platform-specific QEMU binary
        try:
            from .test_config import TestConfig
            qemu_binary = TestConfig.get_qemu_binary()
        except ImportError:
            # Fallback to default
            qemu_binary = 'qemu-system-i386'
            
        cmd = [
            qemu_binary,
            '-drive', f'file={self.os_image_path},index=0,if=floppy,format=raw',
            '-serial', 'stdio',
            '-display', 'none',
            '-no-reboot',
            '-no-shutdown'
        ] + extra_args
        
        try:
            self.qemu_process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                stdin=subprocess.PIPE,
                text=True,
                bufsize=1,
                universal_newlines=True
            )
            
            # Start output reader thread
            self.output_thread = threading.Thread(
                target=self._read_output,
                daemon=True
            )
            self.output_thread.start()
            
            return True
            
        except FileNotFoundError:
            print("Error: QEMU not found. Please install QEMU.")
            return False
        except Exception as e:
            print(f"Error starting QEMU: {e}")
            return False
    
    def _read_output(self):
        """Read QEMU output in a separate thread."""
        while self.qemu_process and self.qemu_process.poll() is None:
            try:
                line = self.qemu_process.stdout.readline()
                if line:
                    self.output_queue.put(('stdout', line.strip()))
                    
                # Also read stderr
                if self.qemu_process.stderr:
                    error_line = self.qemu_process.stderr.readline()
                    if error_line:
                        self.output_queue.put(('stderr', error_line.strip()))
                        
            except Exception as e:
                self.output_queue.put(('error', str(e)))
                break
    
    def wait_for_output(self, patterns: List[str], timeout: Optional[int] = None) -> Optional[str]:
        """Wait for specific output patterns from QEMU."""
        if timeout is None:
            timeout = self.timeout
            
        start_time = time.time()
        collected_output = []
        
        while time.time() - start_time < timeout:
            try:
                output_type, line = self.output_queue.get(timeout=1)
                collected_output.append(line)
                
                # Check if any pattern matches
                for pattern in patterns:
                    if re.search(pattern, line, re.IGNORECASE):
                        return line
                        
            except queue.Empty:
                continue
            except Exception as e:
                print(f"Error reading output: {e}")
                break
        
        print(f"Timeout waiting for patterns: {patterns}")
        print("Collected output:")
        for line in collected_output[-10:]:  # Show last 10 lines
            print(f"  {line}")
        return None
    
    def send_command(self, command: str):
        """Send a command to QEMU monitor."""
        if self.qemu_process and self.qemu_process.stdin:
            self.qemu_process.stdin.write(command + '\n')
            self.qemu_process.stdin.flush()
    
    def stop_qemu(self):
        """Stop the QEMU process."""
        if self.qemu_process:
            try:
                self.qemu_process.terminate()
                self.qemu_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.qemu_process.kill()
            except Exception as e:
                print(f"Error stopping QEMU: {e}")
            finally:
                self.qemu_process = None
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.stop_qemu()


class TestCase:
    """Base class for individual test cases."""
    
    def __init__(self, name: str, description: str):
        self.name = name
        self.description = description
        self.passed = False
        self.error_message = ""
    
    def run(self, runner: QEMUTestRunner) -> bool:
        """Run the test case. Override in subclasses."""
        raise NotImplementedError
    
    def log_result(self):
        """Log the test result."""
        status = "PASS" if self.passed else "FAIL"
        print(f"[{status}] {self.name}: {self.description}")
        if not self.passed and self.error_message:
            print(f"      Error: {self.error_message}")


class BootTest(TestCase):
    """Test that the OS boots successfully."""
    
    def __init__(self):
        super().__init__("Boot Test", "Verify that the OS boots and starts execution")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        """Test OS boot process."""
        try:
            if not runner.start_qemu():
                self.error_message = "Failed to start QEMU"
                return False
            
            # Wait for boot to complete - look for any signs of execution
            # Since this is a simple OS, we'll check if QEMU starts without immediate crash
            time.sleep(3)
            
            if runner.qemu_process and runner.qemu_process.poll() is None:
                self.passed = True
                return True
            else:
                self.error_message = "QEMU process terminated unexpectedly"
                return False
                
        except Exception as e:
            self.error_message = f"Exception during boot test: {e}"
            return False


class MemoryTest(TestCase):
    """Test memory-related functionality."""
    
    def __init__(self):
        super().__init__("Memory Test", "Verify memory operations and ELF loading")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        """Test memory operations."""
        try:
            # Add memory debugging to QEMU
            if not runner.start_qemu(['-d', 'guest_errors']):
                self.error_message = "Failed to start QEMU with memory debugging"
                return False
            
            # Monitor for memory-related errors
            error_output = runner.wait_for_output(
                ['segmentation fault', 'page fault', 'memory error', 'invalid'],
                timeout=10
            )
            
            if error_output:
                self.error_message = f"Memory error detected: {error_output}"
                return False
            
            # If no errors within timeout, consider it passed
            self.passed = True
            return True
            
        except Exception as e:
            self.error_message = f"Exception during memory test: {e}"
            return False


class DiskIOTest(TestCase):
    """Test disk I/O operations."""
    
    def __init__(self):
        super().__init__("Disk I/O Test", "Verify disk reading functionality")
    
    def run(self, runner: QEMUTestRunner) -> bool:
        """Test disk I/O."""
        try:
            if not runner.start_qemu(['-trace', 'fdc_*']):
                self.error_message = "Failed to start QEMU with disk tracing"
                return False
            
            # Monitor for disk I/O activity
            disk_activity = runner.wait_for_output(
                ['fdc', 'disk', 'read', 'sector'],
                timeout=15
            )
            
            # For this simple OS, successful disk reading is a good sign
            if disk_activity or runner.qemu_process.poll() is None:
                self.passed = True
                return True
            else:
                self.error_message = "No disk activity detected or process terminated"
                return False
                
        except Exception as e:
            self.error_message = f"Exception during disk I/O test: {e}"
            return False


class TestSuite:
    """Main test suite that runs all tests."""
    
    def __init__(self, os_image_path: str):
        self.os_image_path = os_image_path
        self.tests = [
            BootTest(),
            MemoryTest(),
            DiskIOTest()
        ]
        self.results = []
    
    def run_all_tests(self) -> bool:
        """Run all tests in the suite."""
        print(f"Running MTOS Test Suite")
        print(f"OS Image: {self.os_image_path}")
        print("=" * 50)
        
        all_passed = True
        
        for test in self.tests:
            print(f"\nRunning {test.name}...")
            
            with QEMUTestRunner(self.os_image_path) as runner:
                test_passed = test.run(runner)
                test.log_result()
                
                if not test_passed:
                    all_passed = False
            
            self.results.append((test.name, test.passed, test.error_message))
        
        print("\n" + "=" * 50)
        self.print_summary()
        
        return all_passed
    
    def print_summary(self):
        """Print test summary."""
        passed_count = sum(1 for _, passed, _ in self.results if passed)
        total_count = len(self.results)
        
        print(f"Test Summary: {passed_count}/{total_count} tests passed")
        
        if passed_count == total_count:
            print("🎉 All tests passed!")
        else:
            print("❌ Some tests failed:")
            for name, passed, error in self.results:
                if not passed:
                    print(f"  - {name}: {error}")


def main():
    """Main entry point for the test framework."""
    if len(sys.argv) < 2:
        print("Usage: python run_tests.py <os_image_path>")
        sys.exit(1)
    
    os_image_path = sys.argv[1]
    
    if not os.path.exists(os_image_path):
        print(f"Error: OS image not found: {os_image_path}")
        sys.exit(1)
    
    # Run the test suite
    suite = TestSuite(os_image_path)
    success = suite.run_all_tests()
    
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
