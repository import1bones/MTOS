#!/usr/bin/env python3
"""
Test configuration and utilities for MTOS testing framework
"""

import os
import subprocess
import platform

class TestConfig:
    """Configuration for the test framework."""
    
    # Default QEMU binary names for different platforms
    QEMU_BINARIES = {
        'Windows': 'qemu-system-i386.exe',
        'Linux': 'qemu-system-i386',
        'Darwin': 'qemu-system-i386'  # macOS
    }
    
    # Default test timeouts (in seconds)
    DEFAULT_TIMEOUT = 30
    BOOT_TIMEOUT = 45
    STABILITY_TIMEOUT = 60
    
    # QEMU default arguments
    DEFAULT_QEMU_ARGS = [
        '-no-reboot',
        '-no-shutdown',
        '-display', 'none'
    ]
    
    @classmethod
    def get_qemu_binary(cls):
        """Get the appropriate QEMU binary for the current platform."""
        system = platform.system()
        return cls.QEMU_BINARIES.get(system, 'qemu-system-i386')
    
    @classmethod
    def check_qemu_available(cls):
        """Check if QEMU is available on the system."""
        qemu_binary = cls.get_qemu_binary()
        try:
            result = subprocess.run(
                [qemu_binary, '--version'],
                capture_output=True,
                text=True,
                timeout=10
            )
            return result.returncode == 0
        except (FileNotFoundError, subprocess.TimeoutExpired):
            return False
    
    @classmethod
    def get_qemu_version(cls):
        """Get QEMU version information."""
        qemu_binary = cls.get_qemu_binary()
        try:
            result = subprocess.run(
                [qemu_binary, '--version'],
                capture_output=True,
                text=True,
                timeout=10
            )
            if result.returncode == 0:
                return result.stdout.split('\n')[0]
            return "Unknown"
        except (FileNotFoundError, subprocess.TimeoutExpired):
            return "Not available"


def setup_test_environment():
    """Set up the test environment and check prerequisites."""
    print("Setting up MTOS test environment...")
    
    # Check QEMU availability
    if not TestConfig.check_qemu_available():
        print("❌ QEMU not found!")
        print("Please install QEMU:")
        print("  Windows: Download from https://www.qemu.org/download/")
        print("  Linux:   sudo apt-get install qemu-system-x86")
        print("  macOS:   brew install qemu")
        return False
    
    print(f"✅ QEMU found: {TestConfig.get_qemu_version()}")
    
    # Create test output directories
    os.makedirs('test_output', exist_ok=True)
    os.makedirs('test_logs', exist_ok=True)
    
    print("✅ Test directories created")
    print("🚀 Test environment ready!")
    return True


if __name__ == "__main__":
    setup_test_environment()
