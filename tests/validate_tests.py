#!/usr/bin/env python3
"""
MTOS Test Validation Script
Ensures the test framework is properly configured and working
"""

import os
import sys
import subprocess
import platform
from pathlib import Path

class TestValidator:
    """Validates the MTOS test framework setup."""
    
    def __init__(self):
        self.script_dir = Path(__file__).parent.absolute()
        self.mtos_root = self.script_dir.parent
        self.issues = []
        self.warnings = []
    
    def check_python_requirements(self):
        """Check Python version and basic requirements."""
        print("🐍 Checking Python requirements...")
        
        # Check Python version
        if sys.version_info < (3, 7):
            self.issues.append("Python 3.7+ required")
        else:
            print(f"  ✅ Python {sys.version.split()[0]}")
        
        # Check for required modules
        required_modules = ['subprocess', 'threading', 'queue', 're', 'json']
        for module in required_modules:
            try:
                __import__(module)
                print(f"  ✅ {module} module available")
            except ImportError:
                self.issues.append(f"Required module '{module}' not found")
        
        # Check for optional modules
        optional_modules = {
            'matplotlib': 'pip install matplotlib',
            'numpy': 'pip install numpy'
        }
        
        for module, install_cmd in optional_modules.items():
            try:
                __import__(module)
                print(f"  ✅ {module} module available")
            except ImportError:
                self.warnings.append(f"Optional module '{module}' not found. Install with: {install_cmd}")
    
    def check_qemu_installation(self):
        """Check if QEMU is properly installed."""
        print("🖥️  Checking QEMU installation...")
        
        # Get platform-specific QEMU binary name
        system = platform.system()
        qemu_binaries = {
            'Windows': ['qemu-system-i386.exe', 'qemu-system-i386'],
            'Linux': ['qemu-system-i386'],
            'Darwin': ['qemu-system-i386']  # macOS
        }
        
        binaries_to_check = qemu_binaries.get(system, ['qemu-system-i386'])
        
        qemu_found = False
        for binary in binaries_to_check:
            try:
                result = subprocess.run(
                    [binary, '--version'],
                    capture_output=True,
                    text=True,
                    timeout=10
                )
                if result.returncode == 0:
                    version_info = result.stdout.split('\n')[0]
                    print(f"  ✅ QEMU found: {version_info}")
                    qemu_found = True
                    break
            except (FileNotFoundError, subprocess.TimeoutExpired):
                continue
        
        if not qemu_found:
            self.issues.append("QEMU not found. Please install QEMU.")
            print("  ❌ QEMU not found")
            
            # Provide installation instructions
            install_instructions = {
                'Windows': 'Download from https://www.qemu.org/download/#windows or use chocolatey: choco install qemu',
                'Linux': 'sudo apt-get install qemu-system-x86 (Ubuntu/Debian) or sudo yum install qemu (RHEL/CentOS)',
                'Darwin': 'brew install qemu'
            }
            
            instruction = install_instructions.get(system, 'Visit https://www.qemu.org/download/')
            print(f"  ℹ️  Installation: {instruction}")
    
    def check_build_system(self):
        """Check if the build system is properly configured."""
        print("🔧 Checking build system...")
        
        # Check for build files
        build_files = ['Makefile', 'CMakeLists.txt', 'build.py']
        for build_file in build_files:
            file_path = self.mtos_root / build_file
            if file_path.exists():
                print(f"  ✅ {build_file} found")
            else:
                self.warnings.append(f"Build file '{build_file}' not found")
        
        # Check for source directories
        required_dirs = ['kernel', 'include', 'boot', 'tests']
        for dir_name in required_dirs:
            dir_path = self.mtos_root / dir_name
            if dir_path.exists() and dir_path.is_dir():
                print(f"  ✅ {dir_name}/ directory found")
            else:
                self.issues.append(f"Required directory '{dir_name}' not found")
        
        # Check for userspace (optional)
        userspace_dir = self.mtos_root / 'userspace'
        if userspace_dir.exists():
            print(f"  ✅ userspace/ directory found")
            
            # Check for Rust
            try:
                result = subprocess.run(['rustc', '--version'], capture_output=True, text=True, timeout=10)
                if result.returncode == 0:
                    version_info = result.stdout.strip()
                    print(f"  ✅ Rust found: {version_info}")
                else:
                    self.warnings.append("Rust userspace directory found but rustc not available")
            except (FileNotFoundError, subprocess.TimeoutExpired):
                self.warnings.append("Rust userspace directory found but rustc not available")
        else:
            self.warnings.append("userspace/ directory not found (optional)")
    
    def check_test_files(self):
        """Check if test files are properly configured."""
        print("🧪 Checking test files...")
        
        test_files = [
            'run_tests.py',
            'test_config.py',
            'test_boot.py',
            'test_memory.py',
            'test_integration.py',
            'compare_components.py'
        ]
        
        for test_file in test_files:
            file_path = self.script_dir / test_file
            if file_path.exists():
                print(f"  ✅ {test_file} found")
                
                # Basic syntax check
                try:
                    with open(file_path, 'r') as f:
                        content = f.read()
                        compile(content, str(file_path), 'exec')
                    print(f"  ✅ {test_file} syntax OK")
                except SyntaxError as e:
                    self.issues.append(f"Syntax error in {test_file}: {e}")
                except Exception as e:
                    self.warnings.append(f"Issue with {test_file}: {e}")
            else:
                self.issues.append(f"Test file '{test_file}' not found")
    
    def check_build_artifacts(self):
        """Check if build artifacts exist (if built)."""
        print("📦 Checking build artifacts...")
        
        build_dir = self.mtos_root / 'build'
        if build_dir.exists():
            print(f"  ✅ build/ directory found")
            
            # Check for OS image
            os_image = build_dir / 'mtos.img'
            if os_image.exists():
                print(f"  ✅ OS image found: {os_image}")
                print(f"    Size: {os_image.stat().st_size} bytes")
            else:
                self.warnings.append("OS image not found. Run 'python build.py --build' to create it.")
        else:
            self.warnings.append("build/ directory not found. Run 'python build.py --build' to create it.")
    
    def run_basic_test(self):
        """Run a basic test to verify the framework works."""
        print("🚀 Running basic test...")
        
        # Check if we can import test modules
        try:
            sys.path.insert(0, str(self.script_dir))
            from test_config import TestConfig
            print("  ✅ Test configuration module loaded")
            
            # Check QEMU availability through test config
            if TestConfig.check_qemu_available():
                print("  ✅ QEMU accessible through test config")
            else:
                self.warnings.append("QEMU not accessible through test config")
                
        except ImportError as e:
            self.issues.append(f"Cannot import test configuration: {e}")
        except Exception as e:
            self.warnings.append(f"Issue with test configuration: {e}")
    
    def generate_setup_script(self):
        """Generate a setup script to fix common issues."""
        setup_script_content = '''#!/usr/bin/env python3
"""
MTOS Test Setup Script
Automatically sets up the test environment
"""

import subprocess
import sys
import platform

def install_python_deps():
    """Install Python dependencies."""
    deps = ['matplotlib', 'numpy']
    for dep in deps:
        try:
            subprocess.check_call([sys.executable, '-m', 'pip', 'install', dep])
            print(f"✅ Installed {dep}")
        except subprocess.CalledProcessError:
            print(f"❌ Failed to install {dep}")

def install_qemu():
    """Provide QEMU installation instructions."""
    system = platform.system()
    
    instructions = {
        'Windows': 'choco install qemu or download from https://www.qemu.org/download/#windows',
        'Linux': 'sudo apt-get install qemu-system-x86 (Ubuntu/Debian)',
        'Darwin': 'brew install qemu'
    }
    
    print(f"📥 To install QEMU on {system}:")
    print(f"   {instructions.get(system, 'Visit https://www.qemu.org/download/')}")

if __name__ == "__main__":
    print("🔧 MTOS Test Environment Setup")
    print("=" * 35)
    
    print("Installing Python dependencies...")
    install_python_deps()
    
    print("\\nQEMU installation:")
    install_qemu()
    
    print("\\n✅ Setup complete! Run 'python validate_tests.py' to verify.")
'''
        
        setup_script_path = self.script_dir / 'setup_test_environment.py'
        with open(setup_script_path, 'w') as f:
            f.write(setup_script_content)
        
        print(f"📝 Generated setup script: {setup_script_path}")
    
    def validate(self):
        """Run all validation checks."""
        print("🔍 MTOS Test Framework Validation")
        print("=" * 40)
        print()
        
        self.check_python_requirements()
        print()
        
        self.check_qemu_installation()
        print()
        
        self.check_build_system()
        print()
        
        self.check_test_files()
        print()
        
        self.check_build_artifacts()
        print()
        
        self.run_basic_test()
        print()
        
        # Generate report
        print("📊 Validation Results")
        print("=" * 20)
        
        if not self.issues and not self.warnings:
            print("🎉 All checks passed! Test framework is ready to use.")
            print()
            print("🚀 Next steps:")
            print("  1. Build MTOS: python build.py --build")
            print("  2. Run tests: python tests/run_tests.py")
            print("  3. Compare components: python tests/compare_components.py allocators")
            return True
        
        if self.issues:
            print("❌ Issues found:")
            for issue in self.issues:
                print(f"  • {issue}")
            print()
        
        if self.warnings:
            print("⚠️  Warnings:")
            for warning in self.warnings:
                print(f"  • {warning}")
            print()
        
        if self.issues:
            print("🔧 Please fix the issues above before running tests.")
            self.generate_setup_script()
            return False
        else:
            print("✅ No critical issues found. Warnings are optional.")
            print()
            print("🚀 You can proceed with testing:")
            print("  1. Build MTOS: python build.py --build")
            print("  2. Run tests: python tests/run_tests.py")
            return True

def main():
    validator = TestValidator()
    success = validator.validate()
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())
