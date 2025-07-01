# MTOS Multi-Platform Deployment Guide

## Overview

MTOS provides a comprehensive multi-platform build and deployment system that supports building and distributing the educational operating system across Windows, Linux, macOS, and embedded targets.

## Supported Platforms

### Desktop Platforms
- **Windows**: x86, x86_64
- **Linux**: x86, x86_64, ARM, AArch64  
- **macOS**: x86_64, Apple Silicon (ARM64)

### Embedded Platforms
- **ARM**: Cortex-A series processors
- **Custom**: Extensible for other architectures

## Quick Start

### Using the Deployment Script

```bash
# Deploy to all supported platforms
python deploy.py --all

# Deploy to specific target
python deploy.py --target linux-x64

# List available targets
python deploy.py --list-targets

# Clean deployment artifacts
python deploy.py --clean
```

### Using the Build Script

```bash
# Cross-compile for specific platform
python build.py --target-platform linux --target-arch arm --build

# Build with specific components
python build.py --allocator buddy --scheduler priority --ipc shared_memory --build

# Package for distribution
python build.py --package
```

## Deployment Targets

| Target | Platform | Architecture | Toolchain | Package Formats |
|--------|----------|--------------|-----------|-----------------|
| `windows-x86` | Windows | i386 | MinGW | ZIP, NSIS |
| `windows-x64` | Windows | x86_64 | MinGW | ZIP, NSIS |
| `linux-x86` | Linux | i386 | GCC | TAR.GZ, DEB, RPM |
| `linux-x64` | Linux | x86_64 | GCC | TAR.GZ, DEB, RPM |
| `linux-arm` | Linux | ARM | arm-none-eabi-gcc | TAR.GZ |
| `linux-aarch64` | Linux | AArch64 | aarch64-linux-gnu-gcc | TAR.GZ |
| `macos-x64` | macOS | x86_64 | Clang | TAR.GZ, Bundle |
| `macos-arm64` | macOS | ARM64 | Clang | TAR.GZ, Bundle |
| `embedded-arm` | Embedded | ARM | arm-none-eabi-gcc | BIN |

## Component Configurations

The deployment system builds multiple component combinations:

1. **Educational Default**: `bitmap` + `round_robin` + `message_queue`
2. **Performance Optimized**: `buddy` + `priority` + `shared_memory`
3. **Mixed Configurations**: Various combinations for comparison

## Build Requirements

### Base Requirements
- **Python 3.9+**: Build orchestration
- **CMake 3.16+**: Cross-platform build system
- **Git**: Source code management

### Platform-Specific Tools

#### Windows
```powershell
# Install with Chocolatey
choco install cmake ninja mingw

# Or install with vcpkg/Visual Studio
```

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get install -y \
    gcc gcc-multilib g++ \
    make cmake ninja-build \
    binutils binutils-dev \
    gcc-arm-none-eabi \
    gcc-aarch64-linux-gnu \
    qemu-system-x86
```

#### macOS
```bash
# Install with Homebrew
brew install cmake ninja qemu
```

### Rust Userspace (Optional)
```bash
# Install Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

# Add cross-compilation targets
rustup target add i686-unknown-linux-gnu
rustup target add x86_64-unknown-linux-gnu
rustup target add armv7-unknown-linux-gnueabi
rustup target add aarch64-unknown-linux-gnu
```

## Docker-Based Deployment

### Using Pre-built Containers

```bash
# Build all configurations in container
docker build -t mtos-builder .
docker run --rm -v $(pwd):/workspace mtos-builder ./build-matrix.sh

# Educational demo environment
docker build -t mtos-demo --target mtos-demo .
docker run -it --rm mtos-demo
```

### Custom Container Builds

```bash
# CI/CD container
docker build -t mtos-ci --target mtos-ci .
docker run --rm mtos-ci ./ci-build.sh

# Cross-compilation containers
docker build -t mtos-arm --target mtos-arm .
docker build -t mtos-x86_64 --target mtos-x86_64 .
```

## CI/CD Integration

### GitHub Actions

The project includes comprehensive GitHub Actions workflows:

- **Multi-platform CI**: Builds and tests on Windows, Linux, macOS
- **Cross-compilation**: ARM, AArch64, embedded targets
- **Code Quality**: Static analysis, formatting, security checks
- **Documentation**: Automated doc generation and deployment
- **Release**: Automated package creation and distribution

### Configuration

```yaml
# Example workflow step
- name: Multi-platform build
  run: |
    python deploy.py --all --build-type Release
```

## Package Distribution

### Package Contents

Each deployment package includes:

- **Kernel Binaries**: `kernel.bin`, `bootloader.bin`
- **OS Images**: `mtos.img` (bootable disk image)
- **Userspace Applications**: Rust binaries (if built)
- **Documentation**: Student guides, API docs
- **Build Tools**: Scripts for rebuilding and customization

### Educational Package

The educational package (`MTOS-Educational-Complete.tar.gz`) contains:

- Complete source code
- All implementations and examples
- Testing framework
- Documentation and guides
- Build and deployment scripts
- Container configurations

## Customization

### Adding New Targets

1. **Update `deploy.py`**:
```python
"custom-target": {
    "platform": "custom",
    "arch": "custom_arch",
    "toolchain": "custom-gcc",
    "package_formats": ["tar.gz"]
}
```

2. **Update CMake configuration**:
```cmake
# Add custom toolchain support
if(TARGET_ARCH STREQUAL "custom_arch")
    set(CMAKE_C_COMPILER "custom-gcc")
    # ... additional settings
endif()
```

3. **Test the new target**:
```bash
python deploy.py --target custom-target
```

### Adding Component Implementations

1. **Create implementation** in appropriate directory
2. **Register in virtual interface system**
3. **Update build configuration**
4. **Add to deployment matrix**

## Testing Deployments

### QEMU Testing
```bash
# Test deployed image
qemu-system-i386 -drive format=raw,file=deploy/linux-x86/mtos.img

# With debug support
qemu-system-i386 -drive format=raw,file=deploy/linux-x86/mtos.img -s -S
```

### Automated Testing
```bash
# Run deployment tests
python tests/test_deployment.py

# Validate all packages
python tests/validate_packages.py
```

## Educational Use

### For Students

1. **Download** educational package
2. **Extract** and explore source code
3. **Build** with different configurations
4. **Compare** implementations using tools
5. **Extend** with custom components

### For Instructors

1. **Deploy** to lab environments
2. **Customize** for specific lessons
3. **Monitor** student progress
4. **Distribute** assignments

## Troubleshooting

### Common Issues

#### Missing Toolchains
```bash
# Check available tools
python build.py --check-tools

# Install missing dependencies
python setup_tests.py
```

#### Cross-compilation Failures
```bash
# Verify target support
python deploy.py --list-targets

# Check toolchain installation
arm-none-eabi-gcc --version
```

#### Package Creation Errors
```bash
# Clean and rebuild
python deploy.py --clean
python deploy.py --target linux-x64
```

### Debug Mode

```bash
# Enable verbose output
export MTOS_DEBUG=1
python deploy.py --target linux-x64
```

## Performance Optimization

### Build Optimization
- Use `Release` build type for deployment
- Enable link-time optimization (LTO)
- Strip debug symbols for production

### Deployment Optimization
- Parallel builds: `--parallel N`
- Incremental builds: avoid `--clean` when possible
- Container caching: Use multi-stage builds

## Security Considerations

### Build Environment
- Use containerized builds for isolation
- Verify toolchain authenticity
- Scan dependencies for vulnerabilities

### Distribution
- Sign packages with GPG
- Provide checksums for verification
- Use secure distribution channels

## Support and Community

- **Documentation**: `/docs` directory
- **Issues**: GitHub issue tracker
- **Discussions**: GitHub discussions
- **Educational Support**: Student guides and examples

---

**Note**: This deployment system is designed for educational use. For production deployments, additional security and testing measures should be implemented.
