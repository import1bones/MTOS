# Multi-stage Dockerfile for cross-platform MTOS builds
# Supports building for different targets from any host platform

# =============================================================================
# Base development environment
# =============================================================================
FROM ubuntu:22.04 as base-devel

ENV DEBIAN_FRONTEND=noninteractive

# Install base development tools
RUN apt-get update && apt-get install -y \
    # Build essentials
    gcc gcc-multilib g++ \
    make cmake ninja-build \
    binutils binutils-dev \
    # Cross-compilation toolchains
    gcc-arm-none-eabi \
    gcc-aarch64-linux-gnu \
    gcc-i686-linux-gnu \
    # Development tools
    git curl wget \
    python3 python3-pip python3-venv \
    # Emulation and testing
    qemu-system-x86 qemu-system-arm \
    # Documentation
    doxygen graphviz \
    # Utilities
    file tree htop \
    && rm -rf /var/lib/apt/lists/*

# Install Rust for userspace development
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

# Add Rust targets for cross-compilation
RUN rustup target add i686-unknown-linux-gnu
RUN rustup target add x86_64-unknown-linux-gnu
RUN rustup target add armv7-unknown-linux-gnueabi
RUN rustup target add aarch64-unknown-linux-gnu

# Install Python packages for testing framework
RUN pip3 install --no-cache-dir \
    matplotlib \
    numpy \
    pytest \
    coverage \
    sphinx

# =============================================================================
# MTOS development environment
# =============================================================================
FROM base-devel as mtos-devel

# Create development user
RUN useradd -m -s /bin/bash -G sudo mtos && \
    echo "mtos:mtos" | chpasswd

# Set up workspace
WORKDIR /workspace
RUN chown mtos:mtos /workspace

USER mtos

# Copy source code
COPY --chown=mtos:mtos . /workspace/

# =============================================================================
# Build environment for CI/CD
# =============================================================================
FROM mtos-devel as mtos-ci

USER root

# Install additional CI tools
RUN apt-get update && apt-get install -y \
    lcov \
    valgrind \
    cppcheck \
    clang-format \
    && rm -rf /var/lib/apt/lists/*

# Install additional Python packages for CI
RUN pip3 install --no-cache-dir \
    black \
    flake8 \
    mypy \
    bandit

USER mtos

# Build script for CI
RUN echo '#!/bin/bash\n\
set -e\n\
echo "🏗️  MTOS CI Build Pipeline"\n\
echo "========================"\n\
\n\
# Run static analysis\n\
echo "📊 Running static analysis..."\n\
cppcheck --enable=all --error-exitcode=1 kernel/ || true\n\
\n\
# Format check\n\
echo "🎨 Checking code formatting..."\n\
find kernel/ -name "*.c" -o -name "*.h" | xargs clang-format --dry-run --Werror || true\n\
\n\
# Python code quality\n\
echo "🐍 Checking Python code..."\n\
python3 -m flake8 tests/ build.py || true\n\
\n\
# Rust code quality (if userspace exists)\n\
if [ -d "userspace" ]; then\n\
    echo "🦀 Checking Rust userspace code..."\n\
    cd userspace\n\
    cargo fmt --check || true\n\
    cargo clippy -- -D warnings || true\n\
    cd ..\n\
fi\n\
\n\
# Build all configurations\n\
echo "🔨 Building all configurations..."\n\
for allocator in bitmap buddy; do\n\
    for scheduler in round_robin priority; do\n\
        for ipc in message_queue shared_memory; do\n\
            echo "Building: $allocator + $scheduler + $ipc"\n\
            python3 build.py --clean\n\
            python3 build.py --allocator $allocator --scheduler $scheduler --ipc $ipc --build\n\
        done\n\
    done\n\
done\n\
\n\
# Build Rust userspace (if exists)\n\
if [ -d "userspace" ]; then\n\
    echo "🦀 Building Rust userspace applications..."\n\
    cd userspace\n\
    cargo build --release\n\
    cd ..\n\
fi\n\
\n\
# Run tests\n\
echo "🧪 Running comprehensive tests..."\n\
python3 build.py --test\n\
\n\
# Generate documentation\n\
echo "📚 Generating documentation..."\n\
python3 build.py --configure --no-tests\n\
cmake --build build --target docs\n\
\n\
echo "✅ CI pipeline completed successfully!"\n\
' > /workspace/ci-build.sh && chmod +x /workspace/ci-build.sh

# =============================================================================
# Cross-compilation environments
# =============================================================================

# ARM cross-compilation
FROM mtos-devel as mtos-arm

ENV CC=arm-none-eabi-gcc
ENV CXX=arm-none-eabi-g++
ENV AR=arm-none-eabi-ar
ENV STRIP=arm-none-eabi-strip

# ARM-specific build script
RUN echo '#!/bin/bash\n\
python3 build.py --target-platform embedded --target-arch arm --build\n\
' > /workspace/build-arm.sh && chmod +x /workspace/build-arm.sh

# x86_64 cross-compilation  
FROM mtos-devel as mtos-x86_64

# x86_64 build script
RUN echo '#!/bin/bash\n\
python3 build.py --target-arch x86_64 --build\n\
' > /workspace/build-x86_64.sh && chmod +x /workspace/build-x86_64.sh

# =============================================================================
# Educational demo environment
# =============================================================================
FROM mtos-devel as mtos-demo

# Install additional tools for demos
USER root
RUN apt-get update && apt-get install -y \
    firefox-esr \
    tigervnc-standalone-server \
    x11-apps \
    && rm -rf /var/lib/apt/lists/*

USER mtos

# Demo script
RUN echo '#!/bin/bash\n\
echo "🎓 MTOS Educational Demo Environment"\n\
echo "==================================="\n\
echo\n\
echo "Available commands:"\n\
echo "  ./demo-all.sh        - Build and demo all implementations"\n\
echo "  ./showcase.py        - Interactive component showcase"\n\
echo "  python3 build.py     - Build system with options"\n\
echo "  make run            - Run MTOS in QEMU (if using Makefile)"\n\
echo\n\
echo "Educational exercises:"\n\
echo "  1. Compare allocators: python3 tests/compare_components.py allocators"\n\
echo "  2. Benchmark schedulers: python3 tests/compare_components.py schedulers"\n\
echo "  3. Analyze IPC: python3 tests/compare_components.py ipc"\n\
echo\n\
echo "Documentation: /workspace/docs/"\n\
' > /workspace/demo-help.sh && chmod +x /workspace/demo-help.sh

# Create comprehensive demo script
RUN echo '#!/bin/bash\n\
set -e\n\
echo "🎓 MTOS Complete Educational Demo"\n\
echo "================================="\n\
\n\
# Build Rust userspace first (if available)\n\
if [ -d "userspace" ]; then\n\
    echo "🦀 Building Rust userspace applications..."\n\
    cd userspace\n\
    cargo build --release\n\
    cd ..\n\
    echo "✅ Userspace applications built successfully!"\n\
fi\n\
\n\
# Build all implementations\n\
echo "📦 Building all memory allocators..."\n\
python3 build.py --allocator bitmap --build\n\
python3 build.py --allocator buddy --build\n\
\n\
echo "⚡ Building all schedulers..."\n\
python3 build.py --scheduler round_robin --build\n\
python3 build.py --scheduler priority --build\n\
\n\
echo "💬 Building all IPC mechanisms..."\n\
python3 build.py --ipc message_queue --build\n\
python3 build.py --ipc shared_memory --build\n\
\n\
# Run comparisons\n\
echo "📊 Running performance comparisons..."\n\
python3 tests/compare_components.py allocators || true\n\
python3 tests/compare_components.py schedulers || true\n\
python3 tests/compare_components.py ipc || true\n\
\n\
# Generate reports\n\
echo "📋 Generating educational reports..."\n\
mkdir -p reports\n\
python3 tests/compare_components.py allocators --output reports/allocator_comparison.txt || true\n\
python3 tests/compare_components.py schedulers --output reports/scheduler_comparison.txt || true\n\
\n\
# List userspace binaries if available\n\
if [ -d "userspace/target/release" ]; then\n\
    echo "🦀 Available userspace applications:"\n\
    ls -la userspace/target/release/ | grep -E "^-.*x.*" | awk "{print \"  \" \$9}"\n\
fi\n\
\n\
echo "🎉 Demo complete! Check the reports/ directory for analysis."\n\
echo "Next steps:"\n\
echo "  1. Study the implementation differences in kernel/"\n\
echo "  2. Try implementing your own algorithms"\n\
echo "  3. Run individual components with: python3 build.py --allocator X --scheduler Y --ipc Z"\n\
if [ -d "userspace" ]; then\n\
    echo "  4. Explore Rust userspace applications in userspace/"\n\
    echo "  5. Build userspace apps with: cd userspace && cargo build"\n\
fi\n\
' > /workspace/demo-all.sh && chmod +x /workspace/demo-all.sh

# =============================================================================
# Production build environment
# =============================================================================
FROM base-devel as mtos-production

# Minimal production build
COPY . /workspace/
WORKDIR /workspace

# Optimized build script
RUN echo '#!/bin/bash\n\
python3 build.py --build-type Release --no-docs --build --package\n\
' > /workspace/build-production.sh && chmod +x /workspace/build-production.sh

# =============================================================================
# Default target for general use
# =============================================================================
FROM mtos-demo as default

# Set default command
CMD ["./demo-help.sh"]

# =============================================================================
# Multi-architecture build support
# =============================================================================

# Create build matrix script
RUN echo '#!/bin/bash\n\
# Build matrix for all supported configurations\n\
set -e\n\
\n\
ALLOCATORS="bitmap buddy"\n\
SCHEDULERS="round_robin priority"\n\
IPC_MECHANISMS="message_queue shared_memory"\n\
BUILD_TYPES="Debug Release"\n\
\n\
echo "🏭 MTOS Build Matrix"\n\
echo "=================="\n\
\n\
total=0\n\
success=0\n\
\n\
for allocator in $ALLOCATORS; do\n\
    for scheduler in $SCHEDULERS; do\n\
        for ipc in $IPC_MECHANISMS; do\n\
            for build_type in $BUILD_TYPES; do\n\
                config="$allocator+$scheduler+$ipc ($build_type)"\n\
                echo "Building: $config"\n\
                total=$((total + 1))\n\
                \n\
                if python3 build.py --clean --allocator $allocator --scheduler $scheduler --ipc $ipc --build-type $build_type --build --no-tests; then\n\
                    echo "✅ $config - SUCCESS"\n\
                    success=$((success + 1))\n\
                else\n\
                    echo "❌ $config - FAILED"\n\
                fi\n\
                echo\n\
            done\n\
        done\n\
    done\n\
done\n\
\n\
echo "📊 Build Matrix Results: $success/$total successful"\n\
\n\
if [ $success -eq $total ]; then\n\
    echo "🎉 All builds successful!"\n\
    exit 0\n\
else\n\
    echo "⚠️  Some builds failed."\n\
    exit 1\n\
fi\n\
' > /workspace/build-matrix.sh && chmod +x /workspace/build-matrix.sh
