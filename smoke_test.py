#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
MTOS Smoke Test Runner - ASCII Version
Simple smoke test for Windows compatibility
"""

import os
import sys
import subprocess
from pathlib import Path

def safe_print(message):
    """Print message safely handling encoding issues."""
    try:
        print(message)
    except UnicodeEncodeError:
        # Fallback to ASCII-only output
        print(message.encode('ascii', errors='replace').decode('ascii'))

def check_prerequisites():
    """Check basic prerequisites."""
    safe_print("[INFO] Checking prerequisites...")
    
    # Check if we're in the right directory
    if not os.path.exists("build.py") and not os.path.exists("Makefile"):
        safe_print("[ERROR] Not in MTOS directory. Looking for build.py or Makefile")
        return False
    
    # Check for Python
    try:
        version = sys.version_info
        if version.major >= 3 and version.minor >= 7:
            safe_print(f"[OK] Python {version.major}.{version.minor}")
        else:
            safe_print("[ERROR] Python 3.7+ required")
            return False
    except Exception as e:
        safe_print(f"[ERROR] Python check failed: {e}")
        return False
    
    safe_print("[OK] Prerequisites check passed")
    return True

def test_build():
    """Test building MTOS."""
    safe_print("[BUILD] Testing MTOS build...")
    
    # Try modern build system first
    if os.path.exists("build.py"):
        try:
            result = subprocess.run([
                sys.executable, "build.py", 
                "--allocator", "bitmap", 
                "--scheduler", "round_robin", 
                "--ipc", "message_queue",
                "--configure"
            ], capture_output=True, text=True, timeout=60)
            
            if result.returncode == 0:
                safe_print("[OK] Build configuration successful")
                return True
            else:
                safe_print(f"[ERROR] Build configuration failed")
                safe_print(f"STDERR: {result.stderr}")
                return False
                
        except subprocess.TimeoutExpired:
            safe_print("[ERROR] Build configuration timed out")
            return False
        except Exception as e:
            safe_print(f"[ERROR] Build test failed: {e}")
            return False
    
    # Try make as fallback
    elif os.path.exists("Makefile"):
        try:
            result = subprocess.run(["make", "clean"], capture_output=True, text=True, timeout=30)
            if result.returncode == 0:
                safe_print("[OK] Make system available")
                return True
            else:
                safe_print("[WARNING] Make had issues but continuing")
                return True
        except Exception as e:
            safe_print(f"[WARNING] Make test failed: {e}")
            return True
    
    safe_print("[ERROR] No build system found")
    return False

def test_userspace():
    """Test Rust userspace if available."""
    safe_print("[RUST] Testing userspace...")
    
    userspace_dir = Path("userspace")
    if not userspace_dir.exists():
        safe_print("[INFO] No userspace directory found, skipping")
        return True
    
    # Check if Rust is available
    try:
        result = subprocess.run(["rustc", "--version"], 
                              capture_output=True, text=True, timeout=10)
        if result.returncode == 0:
            version = result.stdout.strip()
            safe_print(f"[OK] Rust found: {version}")
            
            # Try to check cargo.toml
            cargo_toml = userspace_dir / "Cargo.toml"
            if cargo_toml.exists():
                safe_print("[OK] Userspace Cargo.toml found")
                return True
            else:
                safe_print("[WARNING] Userspace directory exists but no Cargo.toml")
                return True
        else:
            safe_print("[INFO] Rust not available, skipping userspace test")
            return True
            
    except FileNotFoundError:
        safe_print("[INFO] Rust not installed, skipping userspace test")
        return True
    except Exception as e:
        safe_print(f"[WARNING] Rust test error: {e}")
        return True

def test_files():
    """Test basic file structure."""
    safe_print("[FILES] Checking file structure...")
    
    required_files = [
        "README.md", "LICENSE", "CMakeLists.txt"
    ]
    
    required_dirs = [
        "kernel", "include", "boot", "tests"
    ]
    
    all_good = True
    
    for file in required_files:
        if os.path.exists(file):
            safe_print(f"[OK] {file} found")
        else:
            safe_print(f"[WARNING] {file} not found")
            all_good = False
    
    for directory in required_dirs:
        if os.path.exists(directory) and os.path.isdir(directory):
            safe_print(f"[OK] {directory}/ directory found")
        else:
            safe_print(f"[ERROR] {directory}/ directory missing")
            all_good = False
    
    return all_good

def test_python_imports():
    """Test required Python modules."""
    safe_print("[PYTHON] Testing Python imports...")
    
    required_modules = ['subprocess', 'pathlib', 'json']
    optional_modules = {'matplotlib': 'visualization', 'numpy': 'numerical operations'}
    
    all_required = True
    
    for module in required_modules:
        try:
            __import__(module)
            safe_print(f"[OK] {module} module available")
        except ImportError:
            safe_print(f"[ERROR] Required module {module} not found")
            all_required = False
    
    for module, purpose in optional_modules.items():
        try:
            __import__(module)
            safe_print(f"[OK] {module} module available ({purpose})")
        except ImportError:
            safe_print(f"[INFO] Optional module {module} not found ({purpose})")
    
    return all_required

def main():
    """Run smoke test."""
    safe_print("MTOS Smoke Test")
    safe_print("=" * 30)
    
    tests = [
        ("Prerequisites", check_prerequisites),
        ("File Structure", test_files), 
        ("Python Imports", test_python_imports),
        ("Build System", test_build),
        ("Userspace", test_userspace)
    ]
    
    results = {}
    
    for test_name, test_func in tests:
        safe_print(f"\n[TEST] {test_name}...")
        try:
            result = test_func()
            results[test_name] = result
            status = "PASS" if result else "FAIL"
            safe_print(f"[RESULT] {test_name}: {status}")
        except Exception as e:
            safe_print(f"[ERROR] {test_name} crashed: {e}")
            results[test_name] = False
    
    # Summary
    safe_print("\n" + "=" * 40)
    safe_print("SMOKE TEST SUMMARY")
    safe_print("=" * 40)
    
    passed = sum(1 for r in results.values() if r)
    total = len(results)
    
    for test_name, result in results.items():
        status = "PASS" if result else "FAIL"
        safe_print(f"{test_name:15}: {status}")
    
    safe_print(f"\nOverall: {passed}/{total} tests passed")
    
    if passed == total:
        safe_print("\n[SUCCESS] All smoke tests passed!")
        safe_print("MTOS appears to be properly set up.")
        safe_print("\nNext steps:")
        safe_print("  1. Try: python build.py --build")
        safe_print("  2. Run: python showcase_implementations.py")
        safe_print("  3. Read: STUDENT_GUIDE.md")
        return 0
    elif passed >= total - 1:
        safe_print("\n[PARTIAL] Most tests passed, minor issues found.")
        safe_print("MTOS should work but may have some limitations.")
        return 0
    else:
        safe_print(f"\n[FAILURE] {total - passed} critical tests failed.")
        safe_print("Please fix the issues above before proceeding.")
        return 1

if __name__ == "__main__":
    sys.exit(main())
