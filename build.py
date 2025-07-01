#!/usr/bin/env python3
"""
MTOS Multi-Platform Build Script
Automated build system for cross-platform deployment
"""

import os
import sys
import subprocess
import argparse
import json
import shutil
import platform
from pathlib import Path
from typing import Dict, List, Optional

class MTOSBuilder:
    """Cross-platform build system for MTOS."""
    
    def __init__(self):
        self.script_dir = Path(__file__).parent.absolute()
        self.build_dir = self.script_dir / "build"
        self.install_dir = self.script_dir / "install"
        
        # Platform detection
        self.host_platform = self.detect_platform()
        self.host_arch = platform.machine().lower()
        
        # Build configuration
        self.config = {
            "target_platform": self.host_platform,
            "target_arch": "i386",
            "build_type": "Debug",
            "memory_allocator": "bitmap",
            "scheduler": "round_robin", 
            "ipc_mechanism": "message_queue",
            "enable_tests": True,
            "enable_docs": True,
            "enable_container": False,
            "parallel_jobs": os.cpu_count() or 1
        }
        
        # Tool paths
        self.tools = {}
        
    def detect_platform(self) -> str:
        """Detect the current platform."""
        system = platform.system().lower()
        if system == "windows":
            return "windows"
        elif system == "darwin":
            return "macos"
        elif system == "linux":
            return "linux"
        else:
            raise RuntimeError(f"Unsupported platform: {system}")
    
    def find_tools(self) -> bool:
        """Find required build tools."""
        required_tools = {
            "cmake": ["cmake", "--version"],
            "make": ["make", "--version"] if self.host_platform != "windows" else ["ninja", "--version"],
            "gcc": ["gcc", "--version"],
            "python": [sys.executable, "--version"]
        }
        
        optional_tools = {
            "qemu": ["qemu-system-i386", "--version"],
            "docker": ["docker", "--version"],
            "doxygen": ["doxygen", "--version"],
            "git": ["git", "--version"]
        }
        
        print("🔍 Checking for required tools...")
        missing_tools = []
        
        for tool, cmd in required_tools.items():
            try:
                result = subprocess.run(cmd, capture_output=True, text=True, timeout=10)
                if result.returncode == 0:
                    self.tools[tool] = cmd[0]
                    print(f"  ✅ {tool}: {cmd[0]}")
                else:
                    missing_tools.append(tool)
                    print(f"  ❌ {tool}: Not found")
            except (subprocess.TimeoutExpired, FileNotFoundError):
                missing_tools.append(tool)
                print(f"  ❌ {tool}: Not found")
        
        # Check optional tools
        print("🔍 Checking for optional tools...")
        for tool, cmd in optional_tools.items():
            try:
                result = subprocess.run(cmd, capture_output=True, text=True, timeout=10)
                if result.returncode == 0:
                    self.tools[tool] = cmd[0]
                    print(f"  ✅ {tool}: {cmd[0]}")
                else:
                    print(f"  ⚠️  {tool}: Not found (optional)")
            except (subprocess.TimeoutExpired, FileNotFoundError):
                print(f"  ⚠️  {tool}: Not found (optional)")
        
        # Check Rust toolchain
        self.check_rust_toolchain()
        
        if missing_tools:
            print(f"\n❌ Missing required tools: {', '.join(missing_tools)}")
            print("Please install the missing tools and try again.")
            return False
        
        return True
    
    def setup_cross_compilation(self, target_platform: str, target_arch: str):
        """Setup cross-compilation environment."""
        if target_platform == self.host_platform and target_arch == self.host_arch:
            return  # Native compilation
        
        print(f"🔧 Setting up cross-compilation for {target_platform}/{target_arch}")
        
        # Install cross-compilation toolchains
        if target_arch == "i386" and self.host_platform == "linux":
            # Install 32-bit development libraries
            self.run_command(["sudo", "apt-get", "install", "-y", "gcc-multilib", "libc6-dev-i386"])
        elif target_arch == "arm":
            # Install ARM cross-compiler
            if self.host_platform == "linux":
                self.run_command(["sudo", "apt-get", "install", "-y", "gcc-arm-none-eabi"])
            elif self.host_platform == "macos":
                self.run_command(["brew", "install", "arm-none-eabi-gcc"])
    
    def configure_cmake(self) -> bool:
        """Configure the build with CMake."""
        print("⚙️  Configuring build with CMake...")
        
        # Create build directory
        self.build_dir.mkdir(exist_ok=True)
        
        # CMake arguments
        cmake_args = [
            self.tools["cmake"],
            "-S", str(self.script_dir),
            "-B", str(self.build_dir),
            f"-DCMAKE_BUILD_TYPE={self.config['build_type']}",
            f"-DTARGET_PLATFORM={self.config['target_platform']}",
            f"-DTARGET_ARCH={self.config['target_arch']}",
            f"-DMEMORY_ALLOCATOR={self.config['memory_allocator']}",
            f"-DSCHEDULER={self.config['scheduler']}",
            f"-DIPC_MECHANISM={self.config['ipc_mechanism']}",
            f"-DMTOS_ENABLE_TESTS={'ON' if self.config['enable_tests'] else 'OFF'}",
            f"-DMTOS_ENABLE_DOCS={'ON' if self.config['enable_docs'] else 'OFF'}",
            f"-DCMAKE_INSTALL_PREFIX={self.install_dir}"
        ]
        
        # Platform-specific generator
        if self.host_platform == "windows":
            if "ninja" in self.tools:
                cmake_args.extend(["-G", "Ninja"])
            else:
                cmake_args.extend(["-G", "MinGW Makefiles"])
        else:
            cmake_args.extend(["-G", "Unix Makefiles"])
        
        try:
            result = subprocess.run(cmake_args, cwd=self.script_dir, check=True)
            print("✅ CMake configuration successful")
            return True
        except subprocess.CalledProcessError as e:
            print(f"❌ CMake configuration failed: {e}")
            return False
    
    def build(self) -> bool:
        """Build the project."""
        print("🔨 Building MTOS...")
        
        build_args = [
            self.tools["cmake"],
            "--build", str(self.build_dir),
            "--target", "mtos",
            "--parallel", str(self.config["parallel_jobs"])
        ]
        
        try:
            result = subprocess.run(build_args, cwd=self.script_dir, check=True)
            print("✅ Kernel build successful")
            
            # Build userspace applications
            userspace_success = self.build_userspace()
            
            if userspace_success:
                print("✅ Complete build successful")
                return True
            else:
                print("⚠️  Kernel built successfully, but userspace build failed")
                return True  # Don't fail entire build for userspace issues
        except subprocess.CalledProcessError as e:
            print(f"❌ Build failed: {e}")
            return False
    
    def test(self) -> bool:
        """Run tests."""
        if not self.config["enable_tests"]:
            print("⚠️  Tests disabled")
            return True
        
        print("🧪 Running tests...")
        
        try:
            # Run CMake tests
            test_args = [self.tools["cmake"], "--build", str(self.build_dir), "--target", "test"]
            subprocess.run(test_args, cwd=self.script_dir, check=True)
            
            # Run Python test framework
            python_tests = [
                sys.executable, "tests/run_tests.py",
                "--build-dir", str(self.build_dir)
            ]
            subprocess.run(python_tests, cwd=self.script_dir, check=True)
            
            # Run userspace tests
            userspace_test_success = self.test_userspace()
            
            if userspace_test_success:
                print("✅ All tests passed")
                return True
            else:
                print("⚠️  Kernel tests passed, but userspace tests had issues")
                return True  # Don't fail for userspace test issues
        except subprocess.CalledProcessError as e:
            print(f"❌ Tests failed: {e}")
            return False
    
    def package(self) -> bool:
        """Create distribution packages."""
        print("📦 Creating packages...")
        
        try:
            # Install to staging directory
            install_args = [
                self.tools["cmake"],
                "--build", str(self.build_dir),
                "--target", "install"
            ]
            subprocess.run(install_args, cwd=self.script_dir, check=True)
            
            # Create packages
            package_args = [
                self.tools["cmake"],
                "--build", str(self.build_dir),
                "--target", "package"
            ]
            subprocess.run(package_args, cwd=self.script_dir, check=True)
            
            print("✅ Packages created")
            return True
        except subprocess.CalledProcessError as e:
            print(f"❌ Packaging failed: {e}")
            return False
    
    def clean(self):
        """Clean build artifacts."""
        print("🧹 Cleaning build artifacts...")
        
        if self.build_dir.exists():
            shutil.rmtree(self.build_dir)
        if self.install_dir.exists():
            shutil.rmtree(self.install_dir)
        
        print("✅ Clean complete")
    
    def run_in_qemu(self):
        """Run MTOS in QEMU emulator."""
        if "qemu" not in self.tools:
            print("❌ QEMU not found. Cannot run emulation.")
            return False
        
        print("🚀 Running MTOS in QEMU...")
        
        image_path = self.build_dir / "mtos.img"
        if not image_path.exists():
            print("❌ OS image not found. Please build first.")
            return False
        
        qemu_args = [
            self.tools["qemu"],
            "-drive", f"format=raw,file={image_path}",
            "-no-reboot",
            "-no-shutdown"
        ]
        
        try:
            subprocess.run(qemu_args, cwd=self.script_dir)
            return True
        except KeyboardInterrupt:
            print("\n⏹️  QEMU stopped by user")
            return True
        except subprocess.CalledProcessError as e:
            print(f"❌ QEMU failed: {e}")
            return False
    
    def build_container(self):
        """Build MTOS in a container for reproducible builds."""
        if "docker" not in self.tools:
            print("❌ Docker not found. Cannot build container.")
            return False
        
        print("🐳 Building MTOS in container...")
        
        # Create Dockerfile
        dockerfile_content = self.generate_dockerfile()
        dockerfile_path = self.script_dir / "Dockerfile.build"
        
        with open(dockerfile_path, "w") as f:
            f.write(dockerfile_content)
        
        try:
            # Build container image
            build_args = [
                self.tools["docker"], "build",
                "-f", str(dockerfile_path),
                "-t", "mtos-builder",
                str(self.script_dir)
            ]
            subprocess.run(build_args, check=True)
            
            # Run build in container
            run_args = [
                self.tools["docker"], "run",
                "--rm",
                "-v", f"{self.script_dir}:/workspace",
                "mtos-builder"
            ]
            subprocess.run(run_args, check=True)
            
            print("✅ Container build complete")
            return True
        except subprocess.CalledProcessError as e:
            print(f"❌ Container build failed: {e}")
            return False
    
    def generate_dockerfile(self) -> str:
        """Generate Dockerfile for reproducible builds."""
        return """
FROM ubuntu:22.04

# Install build dependencies
RUN apt-get update && apt-get install -y \\
    gcc gcc-multilib \\
    cmake make ninja-build \\
    python3 python3-pip \\
    qemu-system-x86 \\
    doxygen graphviz \\
    git \\
    && rm -rf /var/lib/apt/lists/*

# Install Python packages
RUN pip3 install matplotlib numpy

# Set working directory
WORKDIR /workspace

# Default build command
CMD ["python3", "build.py", "--all"]
"""
    
    def check_rust_toolchain(self) -> bool:
        """Check if Rust toolchain is available for userspace development."""
        try:
            result = subprocess.run(["rustc", "--version"], capture_output=True, text=True, timeout=10)
            if result.returncode == 0:
                self.tools["rustc"] = "rustc"
                print(f"  ✅ Rust: {result.stdout.strip()}")
                
                # Check for cargo
                cargo_result = subprocess.run(["cargo", "--version"], capture_output=True, text=True, timeout=10)
                if cargo_result.returncode == 0:
                    self.tools["cargo"] = "cargo"
                    print(f"  ✅ Cargo: {cargo_result.stdout.strip()}")
                    return True
                else:
                    print("  ⚠️  Cargo not found")
                    return False
            else:
                print("  ⚠️  Rust not working properly")
                return False
        except (subprocess.TimeoutExpired, FileNotFoundError):
            print("  ℹ️  Rust not found (optional for userspace development)")
            return False
    
    def build_userspace(self) -> bool:
        """Build Rust userspace applications."""
        userspace_dir = self.script_dir / "userspace"
        
        if not userspace_dir.exists():
            print("ℹ️  No userspace directory found, skipping userspace build")
            return True
        
        if "cargo" not in self.tools:
            print("⚠️  Cargo not found, skipping userspace build")
            return True
        
        print("🦀 Building Rust userspace applications...")
        
        try:
            # Build userspace applications
            cargo_args = [
                self.tools["cargo"], "build",
                "--release"
            ]
            
            # Add target architecture if cross-compiling
            if self.config["target_arch"] != self.host_arch:
                rust_target = self.get_rust_target()
                if rust_target:
                    cargo_args.extend(["--target", rust_target])
            
            result = subprocess.run(cargo_args, cwd=userspace_dir, check=True)
            print("✅ Userspace applications built successfully")
            return True
        except subprocess.CalledProcessError as e:
            print(f"❌ Userspace build failed: {e}")
            return False
    
    def get_rust_target(self) -> Optional[str]:
        """Get Rust target triple for cross-compilation."""
        target_map = {
            ("linux", "i386"): "i686-unknown-linux-gnu",
            ("linux", "x86_64"): "x86_64-unknown-linux-gnu",
            ("linux", "arm"): "armv7-unknown-linux-gnueabi",
            ("linux", "aarch64"): "aarch64-unknown-linux-gnu",
            ("windows", "i386"): "i686-pc-windows-gnu",
            ("windows", "x86_64"): "x86_64-pc-windows-gnu",
            ("macos", "x86_64"): "x86_64-apple-darwin",
            ("macos", "aarch64"): "aarch64-apple-darwin",
        }
        
        key = (self.config["target_platform"], self.config["target_arch"])
        return target_map.get(key)
    
    def test_userspace(self) -> bool:
        """Test Rust userspace applications."""
        userspace_dir = self.script_dir / "userspace"
        
        if not userspace_dir.exists() or "cargo" not in self.tools:
            return True
        
        print("🧪 Testing Rust userspace applications...")
        
        try:
            # Run cargo tests
            test_args = [self.tools["cargo"], "test"]
            subprocess.run(test_args, cwd=userspace_dir, check=True)
            
            # Run cargo clippy for additional checks
            clippy_args = [self.tools["cargo"], "clippy", "--", "-D", "warnings"]
            subprocess.run(clippy_args, cwd=userspace_dir, check=True)
            
            print("✅ Userspace tests passed")
            return True
        except subprocess.CalledProcessError as e:
            print(f"⚠️  Userspace tests failed: {e}")
            return False  # Don't fail the entire build for userspace test failures
    
    def format_userspace(self) -> bool:
        """Format Rust userspace code."""
        userspace_dir = self.script_dir / "userspace"
        
        if not userspace_dir.exists() or "cargo" not in self.tools:
            return True
        
        print("🎨 Formatting Rust userspace code...")
        
        try:
            format_args = [self.tools["cargo"], "fmt"]
            subprocess.run(format_args, cwd=userspace_dir, check=True)
            print("✅ Userspace code formatted")
            return True
        except subprocess.CalledProcessError as e:
            print(f"⚠️  Userspace formatting failed: {e}")
            return True  # Don't fail build for formatting issues
    
    def run_command(self, cmd: List[str], **kwargs) -> subprocess.CompletedProcess:
        """Run a shell command with error handling."""
        try:
            return subprocess.run(cmd, check=True, **kwargs)
        except subprocess.CalledProcessError as e:
            print(f"Command failed: {' '.join(cmd)}")
            raise
    
    def print_status(self):
        """Print current build status and configuration."""
        print("📊 MTOS Build Status")
        print("=" * 50)
        print(f"Host Platform: {self.host_platform}/{self.host_arch}")
        print(f"Target Platform: {self.config['target_platform']}/{self.config['target_arch']}")
        print(f"Build Type: {self.config['build_type']}")
        print(f"Configuration: {self.config['memory_allocator']} + {self.config['scheduler']} + {self.config['ipc_mechanism']}")
        print(f"Build Directory: {self.build_dir}")
        print(f"Install Directory: {self.install_dir}")
        print()
        
        # Check if built
        image_path = self.build_dir / "mtos.img"
        if image_path.exists():
            size = image_path.stat().st_size
            print(f"✅ OS Image: {image_path} ({size} bytes)")
        else:
            print("❌ OS Image: Not built")
        
        print()

def main():
    parser = argparse.ArgumentParser(description="MTOS Multi-Platform Build System")
    
    # Build actions
    parser.add_argument("--configure", action="store_true", help="Configure build")
    parser.add_argument("--build", action="store_true", help="Build MTOS")
    parser.add_argument("--test", action="store_true", help="Run tests")
    parser.add_argument("--package", action="store_true", help="Create packages")
    parser.add_argument("--clean", action="store_true", help="Clean build")
    parser.add_argument("--all", action="store_true", help="Configure, build, test, and package")
    parser.add_argument("--run", action="store_true", help="Run in QEMU")
    parser.add_argument("--container", action="store_true", help="Build in container")
    parser.add_argument("--status", action="store_true", help="Show build status")
    
    # Configuration options
    parser.add_argument("--target-platform", choices=["windows", "linux", "macos", "embedded"],
                       help="Target platform")
    parser.add_argument("--target-arch", choices=["i386", "x86_64", "arm", "aarch64"],
                       help="Target architecture")
    parser.add_argument("--build-type", choices=["Debug", "Release", "MinSizeRel"],
                       help="Build type")
    parser.add_argument("--allocator", choices=["bitmap", "buddy"],
                       help="Memory allocator")
    parser.add_argument("--scheduler", choices=["round_robin", "priority"],
                       help="Process scheduler")
    parser.add_argument("--ipc", choices=["message_queue", "shared_memory"],
                       help="IPC mechanism")
    parser.add_argument("--no-tests", action="store_true", help="Disable tests")
    parser.add_argument("--no-docs", action="store_true", help="Disable documentation")
    parser.add_argument("-j", "--jobs", type=int, help="Number of parallel jobs")
    
    args = parser.parse_args()
    
    # Create builder
    builder = MTOSBuilder()
    
    # Apply configuration from command line
    if args.target_platform:
        builder.config["target_platform"] = args.target_platform
    if args.target_arch:
        builder.config["target_arch"] = args.target_arch
    if args.build_type:
        builder.config["build_type"] = args.build_type
    if args.allocator:
        builder.config["memory_allocator"] = args.allocator
    if args.scheduler:
        builder.config["scheduler"] = args.scheduler
    if args.ipc:
        builder.config["ipc_mechanism"] = args.ipc
    if args.no_tests:
        builder.config["enable_tests"] = False
    if args.no_docs:
        builder.config["enable_docs"] = False
    if args.jobs:
        builder.config["parallel_jobs"] = args.jobs
    
    # Check tools
    if not builder.find_tools():
        sys.exit(1)
    
    # Handle actions
    try:
        if args.status:
            builder.print_status()
            return
        
        if args.clean:
            builder.clean()
            return
        
        if args.container:
            builder.build_container()
            return
        
        if args.all:
            builder.setup_cross_compilation(builder.config["target_platform"], 
                                           builder.config["target_arch"])
            builder.configure_cmake()
            builder.build()
            if builder.config["enable_tests"]:
                builder.test()
            builder.package()
        else:
            if args.configure:
                builder.setup_cross_compilation(builder.config["target_platform"], 
                                               builder.config["target_arch"])
                builder.configure_cmake()
            
            if args.build:
                builder.build()
            
            if args.test:
                builder.test()
            
            if args.package:
                builder.package()
        
        if args.run:
            builder.run_in_qemu()
    
    except KeyboardInterrupt:
        print("\n⏹️  Build stopped by user")
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ Build failed: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
