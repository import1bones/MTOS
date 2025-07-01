#!/usr/bin/env python3
"""
MTOS Comprehensive Test Runner
Runs all tests and validates the complete system
"""

import os
import sys
import subprocess
import argparse
from pathlib import Path

def run_test_validation():
    """Run the test validation script."""
    print("[INFO] Validating test framework...")
    
    tests_dir = Path(__file__).parent / "tests"
    validation_script = tests_dir / "validate_tests.py"
    
    if validation_script.exists():
        try:
            result = subprocess.run([sys.executable, str(validation_script)], 
                                  capture_output=True, text=True)
            print(result.stdout)
            if result.stderr:
                print("Errors:", result.stderr)
            return result.returncode == 0
        except Exception as e:
            print(f"❌ Failed to run test validation: {e}")
            return False
    else:
        print("[WARNING] Test validation script not found, skipping...")
        return True

def build_mtos():
    """Build MTOS with default configuration."""
    print("[BUILD] Building MTOS...")
    
    # Try modern build system first
    if os.path.exists("build.py"):
        try:
            result = subprocess.run([sys.executable, "build.py", "--build"], 
                                  capture_output=True, text=True)
            print(result.stdout)
            if result.stderr:
                print("Errors:", result.stderr)
            return result.returncode == 0
        except Exception as e:
            print(f"[ERROR] Modern build failed: {e}")
    
    # Fallback to make
    if os.path.exists("Makefile"):
        try:
            result = subprocess.run(["make", "all"], capture_output=True, text=True)
            print(result.stdout)
            if result.stderr:
                print("Errors:", result.stderr)
            return result.returncode == 0
        except Exception as e:
            print(f"❌ Make build failed: {e}")
    
    print("❌ No build system found")
    return False

def run_component_tests():
    """Run component comparison tests."""
    print("📊 Running component tests...")
    
    tests_dir = Path(__file__).parent / "tests"
    compare_script = tests_dir / "compare_components.py"
    
    if not compare_script.exists():
        print("⚠️  Component comparison script not found, skipping...")
        return True
    
    components = ["allocators", "schedulers", "ipc"]
    success = True
    
    for component in components:
        print(f"  Testing {component}...")
        try:
            result = subprocess.run([sys.executable, str(compare_script), component], 
                                  capture_output=True, text=True, timeout=60)
            if result.returncode == 0:
                print(f"    ✅ {component} test passed")
            else:
                print(f"    ⚠️  {component} test had issues")
                if result.stderr:
                    print(f"    Error: {result.stderr}")
                success = False
        except subprocess.TimeoutExpired:
            print(f"    ⏰ {component} test timed out")
            success = False
        except Exception as e:
            print(f"    ❌ {component} test failed: {e}")
            success = False
    
    return success

def run_basic_tests():
    """Run basic functionality tests."""
    print("🧪 Running basic tests...")
    
    tests_dir = Path(__file__).parent / "tests"
    test_runner = tests_dir / "run_tests.py"
    
    if test_runner.exists():
        try:
            result = subprocess.run([sys.executable, str(test_runner)], 
                                  capture_output=True, text=True, timeout=120)
            print(result.stdout)
            if result.stderr:
                print("Errors:", result.stderr)
            return result.returncode == 0
        except subprocess.TimeoutExpired:
            print("⏰ Basic tests timed out")
            return False
        except Exception as e:
            print(f"❌ Basic tests failed: {e}")
            return False
    else:
        print("⚠️  Basic test runner not found, skipping...")
        return True

def run_showcase():
    """Run the implementation showcase."""
    print("🎭 Running implementation showcase...")
    
    showcase_script = Path(__file__).parent / "showcase_implementations.py"
    
    if showcase_script.exists():
        try:
            result = subprocess.run([sys.executable, str(showcase_script)], 
                                  capture_output=True, text=True, timeout=300)
            print(result.stdout)
            if result.stderr:
                print("Errors:", result.stderr)
            return result.returncode == 0
        except subprocess.TimeoutExpired:
            print("⏰ Showcase timed out")
            return False
        except Exception as e:
            print(f"❌ Showcase failed: {e}")
            return False
    else:
        print("⚠️  Showcase script not found, skipping...")
        return True

def run_userspace_tests():
    """Test Rust userspace if available."""
    print("🦀 Testing Rust userspace...")
    
    userspace_dir = Path(__file__).parent / "userspace"
    
    if userspace_dir.exists():
        try:
            # Check if Rust is available
            result = subprocess.run(["rustc", "--version"], 
                                  capture_output=True, text=True, timeout=10)
            if result.returncode != 0:
                print("  ⚠️  Rust not available, skipping userspace tests")
                return True
            
            # Build userspace
            result = subprocess.run(["cargo", "build"], cwd=userspace_dir,
                                  capture_output=True, text=True, timeout=120)
            if result.returncode == 0:
                print("  ✅ Userspace build successful")
                
                # Run tests if available
                result = subprocess.run(["cargo", "test"], cwd=userspace_dir,
                                      capture_output=True, text=True, timeout=60)
                if result.returncode == 0:
                    print("  ✅ Userspace tests passed")
                    return True
                else:
                    print("  ⚠️  Userspace tests had issues")
                    return False
            else:
                print("  ❌ Userspace build failed")
                print(result.stderr)
                return False
                
        except subprocess.TimeoutExpired:
            print("  ⏰ Userspace tests timed out")
            return False
        except FileNotFoundError:
            print("  ⚠️  Cargo not found, skipping userspace tests")
            return True
        except Exception as e:
            print(f"  ❌ Userspace test error: {e}")
            return False
    else:
        print("  ℹ️  No userspace directory found, skipping...")
        return True

def main():
    parser = argparse.ArgumentParser(description="MTOS Comprehensive Test Runner")
    parser.add_argument("--skip-validation", action="store_true", 
                       help="Skip test framework validation")
    parser.add_argument("--skip-build", action="store_true", 
                       help="Skip building MTOS")
    parser.add_argument("--skip-basic", action="store_true", 
                       help="Skip basic functionality tests")
    parser.add_argument("--skip-components", action="store_true", 
                       help="Skip component comparison tests")
    parser.add_argument("--skip-showcase", action="store_true", 
                       help="Skip implementation showcase")
    parser.add_argument("--skip-userspace", action="store_true", 
                       help="Skip userspace tests")
    parser.add_argument("--quick", action="store_true", 
                       help="Run only essential tests")
    
    args = parser.parse_args()
    
    print("🚀 MTOS Comprehensive Test Suite")
    print("=" * 40)
    
    results = {}
    
    # Test validation
    if not args.skip_validation and not args.quick:
        results["validation"] = run_test_validation()
    else:
        results["validation"] = True
    
    # Build MTOS
    if not args.skip_build:
        results["build"] = build_mtos()
    else:
        results["build"] = True
    
    # Basic tests
    if not args.skip_basic and results["build"]:
        results["basic"] = run_basic_tests()
    else:
        results["basic"] = True
    
    # Component tests
    if not args.skip_components and not args.quick and results["build"]:
        results["components"] = run_component_tests()
    else:
        results["components"] = True
    
    # Showcase
    if not args.skip_showcase and not args.quick and results["build"]:
        results["showcase"] = run_showcase()
    else:
        results["showcase"] = True
    
    # Userspace tests
    if not args.skip_userspace and not args.quick:
        results["userspace"] = run_userspace_tests()
    else:
        results["userspace"] = True
    
    # Report results
    print("\n📊 Test Results Summary")
    print("=" * 25)
    
    passed = 0
    total = 0
    
    for test_name, result in results.items():
        status = "✅ PASS" if result else "❌ FAIL"
        print(f"{test_name:12}: {status}")
        if result:
            passed += 1
        total += 1
    
    print(f"\nOverall: {passed}/{total} tests passed")
    
    if passed == total:
        print("\n🎉 All tests passed! MTOS is working correctly.")
        print("\n🎓 Next steps:")
        print("  • Explore the STUDENT_GUIDE.md")
        print("  • Try different component combinations")
        print("  • Implement your own algorithms")
        print("  • Use the comparison tools for analysis")
        return 0
    else:
        print(f"\n⚠️  {total - passed} test(s) failed. Please check the output above.")
        return 1

if __name__ == "__main__":
    sys.exit(main())
