#!/usr/bin/env python3
"""
Quick setup verification for MTOS test framework
"""

import os
import sys
import subprocess

def check_prerequisites():
    """Check if all prerequisites are available."""
    print("MTOS Test Framework Setup Check")
    print("=" * 40)
    
    # Check Python version
    if sys.version_info < (3, 6):
        print("❌ Python 3.6+ required, found:", sys.version)
        return False
    else:
        print("✅ Python version:", sys.version.split()[0])
    
    # Check QEMU
    try:
        result = subprocess.run(['qemu-system-i386', '--version'], 
                              capture_output=True, text=True, timeout=10)
        if result.returncode == 0:
            version = result.stdout.split('\n')[0]
            print("✅ QEMU found:", version)
        else:
            print("❌ QEMU not working properly")
            return False
    except FileNotFoundError:
        print("❌ QEMU not found in PATH")
        print("   Please install QEMU:")
        print("   - Windows: https://www.qemu.org/download/")
        print("   - Linux: sudo apt-get install qemu-system-x86")
        print("   - macOS: brew install qemu")
        return False
    except subprocess.TimeoutExpired:
        print("❌ QEMU timeout")
        return False
    
    # Check build tools (basic check)
    tools = ['gcc', 'as', 'ld']
    for tool in tools:
        try:
            result = subprocess.run([tool, '--version'], 
                                  capture_output=True, text=True, timeout=5)
            if result.returncode == 0:
                print(f"✅ {tool} found")
            else:
                print(f"⚠️  {tool} might not be working properly")
        except FileNotFoundError:
            print(f"❌ {tool} not found")
            print("   Please install build tools (gcc, binutils)")
            return False
        except subprocess.TimeoutExpired:
            print(f"⚠️  {tool} timeout")
    
    return True

def create_test_directories():
    """Create necessary test directories."""
    dirs = ['build', 'test_output', 'test_logs']
    for dir_name in dirs:
        if not os.path.exists(dir_name):
            os.makedirs(dir_name)
            print(f"✅ Created directory: {dir_name}")
        else:
            print(f"✅ Directory exists: {dir_name}")

def main():
    """Main setup verification."""
    if not check_prerequisites():
        print("\n❌ Setup incomplete. Please install missing prerequisites.")
        return 1
    
    create_test_directories()
    
    print("\n🚀 MTOS test framework setup is complete!")
    print("\nNext steps:")
    print("1. Build the OS: make clean && make")
    print("2. Run tests: make test")
    print("3. Manual run: make run")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
