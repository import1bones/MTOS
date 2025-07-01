#!/usr/bin/env python3
"""
MTOS Multi-Platform Deployment Script
Automated deployment for Windows, Linux, macOS, and embedded targets
"""

import argparse
import json
import subprocess
import sys
from pathlib import Path
from typing import Dict, List, Optional

class MTOSDeployer:
    """Cross-platform deployment system for MTOS."""
    
    def __init__(self):
        self.script_dir = Path(__file__).parent.absolute()
        self.build_script = self.script_dir / "build.py"
        
        # Deployment configurations
        self.targets = {
            "windows-x86": {
                "platform": "windows",
                "arch": "i386",
                "toolchain": "mingw",
                "package_formats": ["zip", "nsis"]
            },
            "windows-x64": {
                "platform": "windows", 
                "arch": "x86_64",
                "toolchain": "mingw",
                "package_formats": ["zip", "nsis"]
            },
            "linux-x86": {
                "platform": "linux",
                "arch": "i386", 
                "toolchain": "gcc",
                "package_formats": ["tar.gz", "deb", "rpm"]
            },
            "linux-x64": {
                "platform": "linux",
                "arch": "x86_64",
                "toolchain": "gcc", 
                "package_formats": ["tar.gz", "deb", "rpm"]
            },
            "linux-arm": {
                "platform": "linux",
                "arch": "arm",
                "toolchain": "arm-none-eabi-gcc",
                "package_formats": ["tar.gz"]
            },
            "linux-aarch64": {
                "platform": "linux",
                "arch": "aarch64",
                "toolchain": "aarch64-linux-gnu-gcc",
                "package_formats": ["tar.gz"]
            },
            "macos-x64": {
                "platform": "macos",
                "arch": "x86_64",
                "toolchain": "clang",
                "package_formats": ["tar.gz", "bundle"]
            },
            "macos-arm64": {
                "platform": "macos",
                "arch": "aarch64", 
                "toolchain": "clang",
                "package_formats": ["tar.gz", "bundle"]
            },
            "embedded-arm": {
                "platform": "embedded",
                "arch": "arm",
                "toolchain": "arm-none-eabi-gcc",
                "package_formats": ["bin"]
            }
        }
        
        # Component configurations to build
        self.component_configs = [
            {"allocator": "bitmap", "scheduler": "round_robin", "ipc": "message_queue"},
            {"allocator": "buddy", "scheduler": "priority", "ipc": "shared_memory"},
            {"allocator": "bitmap", "scheduler": "priority", "ipc": "message_queue"},
            {"allocator": "buddy", "scheduler": "round_robin", "ipc": "shared_memory"}
        ]
    
    def deploy_target(self, target_name: str, config: Dict, build_type: str = "Release") -> bool:
        """Deploy MTOS for a specific target."""
        print(f"🚀 Deploying {target_name} ({config['platform']}/{config['arch']})...")
        
        target_dir = self.script_dir / "deploy" / target_name
        target_dir.mkdir(parents=True, exist_ok=True)
        
        success_count = 0
        total_count = len(self.component_configs)
        
        for i, components in enumerate(self.component_configs):
            component_name = f"{components['allocator']}-{components['scheduler']}-{components['ipc']}"
            print(f"  📦 Building configuration {i+1}/{total_count}: {component_name}")
            
            # Build this configuration
            build_success = self.build_configuration(
                target_name, config, components, build_type, component_name
            )
            
            if build_success:
                success_count += 1
                print(f"    ✅ {component_name} built successfully")
            else:
                print(f"    ❌ {component_name} build failed")
        
        if success_count == total_count:
            print(f"✅ All configurations for {target_name} built successfully")
            
            # Create packages
            package_success = self.create_packages(target_name, config)
            return package_success
        else:
            print(f"⚠️  {success_count}/{total_count} configurations built for {target_name}")
            return False
    
    def build_configuration(self, target_name: str, config: Dict, components: Dict, 
                          build_type: str, component_name: str) -> bool:
        """Build a specific component configuration."""
        try:
            # Build using the build script
            build_cmd = [
                sys.executable, str(self.build_script),
                "--clean",
                "--target-platform", config["platform"],
                "--target-arch", config["arch"],
                "--build-type", build_type,
                "--allocator", components["allocator"],
                "--scheduler", components["scheduler"],
                "--ipc", components["ipc"],
                "--build",
                "--no-tests"  # Skip tests for deployment builds
            ]
            
            result = subprocess.run(build_cmd, cwd=self.script_dir, 
                                  capture_output=True, text=True)
            
            if result.returncode == 0:
                # Copy build artifacts to deployment directory
                self.copy_artifacts(target_name, component_name)
                return True
            else:
                print(f"      Build error: {result.stderr}")
                return False
                
        except Exception as e:
            print(f"      Exception during build: {e}")
            return False
    
    def copy_artifacts(self, target_name: str, component_name: str):
        """Copy build artifacts to deployment directory."""
        source_dir = self.script_dir / "build"
        target_dir = self.script_dir / "deploy" / target_name / component_name
        target_dir.mkdir(parents=True, exist_ok=True)
        
        # Copy main OS image
        artifacts = ["mtos.img", "kernel.bin", "bootloader.bin"]
        
        for artifact in artifacts:
            source_file = source_dir / artifact
            if source_file.exists():
                import shutil
                shutil.copy2(source_file, target_dir / artifact)
        
        # Copy userspace binaries if they exist
        userspace_dir = self.script_dir / "userspace" / "target" / "release"
        if userspace_dir.exists():
            userspace_target = target_dir / "userspace"
            userspace_target.mkdir(exist_ok=True)
            
            for binary in userspace_dir.glob("*"):
                if binary.is_file() and not binary.suffix:
                    import shutil
                    shutil.copy2(binary, userspace_target / binary.name)
    
    def create_packages(self, target_name: str, config: Dict) -> bool:
        """Create deployment packages for the target."""
        print(f"  📦 Creating packages for {target_name}...")
        
        target_dir = self.script_dir / "deploy" / target_name
        packages_dir = self.script_dir / "packages"
        packages_dir.mkdir(exist_ok=True)
        
        for package_format in config["package_formats"]:
            try:
                if package_format == "zip":
                    self.create_zip_package(target_name, target_dir, packages_dir)
                elif package_format == "tar.gz":
                    self.create_tarball_package(target_name, target_dir, packages_dir)
                elif package_format == "deb":
                    self.create_deb_package(target_name, target_dir, packages_dir)
                elif package_format == "rpm":
                    self.create_rpm_package(target_name, target_dir, packages_dir)
                elif package_format == "nsis":
                    self.create_nsis_package(target_name, target_dir, packages_dir)
                elif package_format == "bundle":
                    self.create_macos_bundle(target_name, target_dir, packages_dir)
                elif package_format == "bin":
                    self.create_binary_package(target_name, target_dir, packages_dir)
                    
                print(f"    ✅ Created {package_format} package")
            except Exception as e:
                print(f"    ❌ Failed to create {package_format} package: {e}")
        
        return True
    
    def create_zip_package(self, target_name: str, source_dir: Path, packages_dir: Path):
        """Create ZIP package."""
        import zipfile
        
        package_path = packages_dir / f"mtos-{target_name}.zip"
        
        with zipfile.ZipFile(package_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
            for file_path in source_dir.rglob('*'):
                if file_path.is_file():
                    arcname = file_path.relative_to(source_dir)
                    zipf.write(file_path, arcname)
    
    def create_tarball_package(self, target_name: str, source_dir: Path, packages_dir: Path):
        """Create tarball package."""
        import tarfile
        
        package_path = packages_dir / f"mtos-{target_name}.tar.gz"
        
        with tarfile.open(package_path, 'w:gz') as tar:
            tar.add(source_dir, arcname=f"mtos-{target_name}")
    
    def create_deb_package(self, target_name: str, source_dir: Path, packages_dir: Path):
        """Create Debian package (simplified)."""
        # This would require more complex packaging
        print(f"    ℹ️  DEB packaging not fully implemented")
    
    def create_rpm_package(self, target_name: str, source_dir: Path, packages_dir: Path):
        """Create RPM package (simplified)."""
        # This would require rpmbuild
        print(f"    ℹ️  RPM packaging not fully implemented")
    
    def create_nsis_package(self, target_name: str, source_dir: Path, packages_dir: Path):
        """Create NSIS installer (simplified)."""
        # This would require NSIS
        print(f"    ℹ️  NSIS packaging not fully implemented")
    
    def create_macos_bundle(self, target_name: str, source_dir: Path, packages_dir: Path):
        """Create macOS app bundle (simplified)."""
        # This would create .app bundle structure
        print(f"    ℹ️  macOS bundle not fully implemented")
    
    def create_binary_package(self, target_name: str, source_dir: Path, packages_dir: Path):
        """Create raw binary package for embedded targets."""
        import shutil
        
        # Copy the main OS image as a binary
        for config_dir in source_dir.iterdir():
            if config_dir.is_dir():
                img_file = config_dir / "mtos.img"
                if img_file.exists():
                    package_path = packages_dir / f"mtos-{target_name}-{config_dir.name}.bin"
                    shutil.copy2(img_file, package_path)
    
    def deploy_all_targets(self, build_type: str = "Release") -> bool:
        """Deploy MTOS for all supported targets."""
        print("🌍 Deploying MTOS for all supported targets...")
        print("=" * 50)
        
        successful_targets = []
        failed_targets = []
        
        for target_name, config in self.targets.items():
            print(f"\n🎯 Target: {target_name}")
            print("-" * 30)
            
            success = self.deploy_target(target_name, config, build_type)
            
            if success:
                successful_targets.append(target_name)
            else:
                failed_targets.append(target_name)
        
        print(f"\n📊 Deployment Summary:")
        print(f"✅ Successful: {len(successful_targets)}")
        print(f"❌ Failed: {len(failed_targets)}")
        
        if successful_targets:
            print(f"\nSuccessful targets:")
            for target in successful_targets:
                print(f"  • {target}")
        
        if failed_targets:
            print(f"\nFailed targets:")
            for target in failed_targets:
                print(f"  • {target}")
        
        return len(failed_targets) == 0
    
    def clean_deployment(self):
        """Clean deployment artifacts."""
        print("🧹 Cleaning deployment artifacts...")
        
        import shutil
        
        deploy_dir = self.script_dir / "deploy"
        packages_dir = self.script_dir / "packages"
        
        if deploy_dir.exists():
            shutil.rmtree(deploy_dir)
        
        if packages_dir.exists():
            shutil.rmtree(packages_dir)
        
        print("✅ Deployment artifacts cleaned")

def main():
    parser = argparse.ArgumentParser(description="MTOS Multi-Platform Deployment")
    parser.add_argument("--target", help="Specific target to deploy")
    parser.add_argument("--all", action="store_true", help="Deploy all targets")
    parser.add_argument("--clean", action="store_true", help="Clean deployment artifacts")
    parser.add_argument("--build-type", default="Release", 
                       choices=["Debug", "Release", "MinSizeRel"],
                       help="Build type")
    parser.add_argument("--list-targets", action="store_true", 
                       help="List available targets")
    
    args = parser.parse_args()
    
    deployer = MTOSDeployer()
    
    if args.list_targets:
        print("Available deployment targets:")
        for target_name, config in deployer.targets.items():
            print(f"  • {target_name}: {config['platform']}/{config['arch']}")
        return 0
    
    if args.clean:
        deployer.clean_deployment()
        return 0
    
    if args.all:
        success = deployer.deploy_all_targets(args.build_type)
        return 0 if success else 1
    
    if args.target:
        if args.target in deployer.targets:
            config = deployer.targets[args.target]
            success = deployer.deploy_target(args.target, config, args.build_type)
            return 0 if success else 1
        else:
            print(f"❌ Unknown target: {args.target}")
            print("Use --list-targets to see available targets")
            return 1
    
    # Default: show help
    parser.print_help()
    return 0

if __name__ == "__main__":
    sys.exit(main())
