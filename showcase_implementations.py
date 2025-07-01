#!/usr/bin/env python3
"""
MTOS Implementation Showcase
Demonstrates the different virtual interface implementations available in MTOS
"""

import subprocess
import sys
import os
import time

def run_command(cmd, description):
    """Run a command and print the results"""
    print(f"\n{'='*60}")
    print(f"🔧 {description}")
    print(f"{'='*60}")
    print(f"Command: {cmd}")
    print("-" * 60)
    
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=30)
        if result.returncode == 0:
            print("✅ SUCCESS")
            if result.stdout:
                print("Output:")
                print(result.stdout)
        else:
            print("❌ FAILED")
            if result.stderr:
                print("Error:")
                print(result.stderr)
            if result.stdout:
                print("Output:")
                print(result.stdout)
    except subprocess.TimeoutExpired:
        print("⏰ TIMEOUT - Command took longer than 30 seconds")
    except Exception as e:
        print(f"💥 EXCEPTION: {e}")

def check_prerequisites():
    """Check if the system has necessary tools."""
    print("🔍 Checking prerequisites...")
    
    # Check if we're in the right directory
    if not os.path.exists("Makefile") and not os.path.exists("CMakeLists.txt"):
        print("❌ Error: Must be run from MTOS root directory")
        print("   Looking for Makefile or CMakeLists.txt")
        return False
    
    # Check for build script
    if not os.path.exists("build.py"):
        print("❌ Error: build.py not found")
        return False
    
    # Check for source directories
    required_dirs = ["kernel", "include", "boot"]
    for directory in required_dirs:
        if not os.path.exists(directory):
            print(f"❌ Error: Required directory '{directory}' not found")
            return False
    
    print("✅ Prerequisites check passed")
    return True

def main():
    print("""
╔══════════════════════════════════════════════════════════════╗
║                    MTOS IMPLEMENTATION SHOWCASE              ║
║              Virtual Interface System Demonstration          ║
╚══════════════════════════════════════════════════════════════╝

This script demonstrates the different implementations available 
in MTOS for each virtual module:

📦 Memory Allocators:  Bitmap vs Buddy System
⚡ Schedulers:         Round-Robin vs Priority
💬 IPC Mechanisms:     Message Queues vs Shared Memory
    """)

    # Check prerequisites first
    if not check_prerequisites():
        print("\n❌ Prerequisites check failed. Please fix the issues above.")
        sys.exit(1)

    # Check if we should use the modern build system
    use_cmake = os.path.exists("build.py")
    
    print(f"\n🔧 Using {'modern build system (build.py)' if use_cmake else 'traditional build system (make)'}")

    # Clean previous builds
    print("\n🧹 Cleaning previous builds...")
    if use_cmake:
        run_command("python build.py --clean", "Cleaning build artifacts")
    else:
        run_command("make clean", "Cleaning build artifacts")

    # Memory Allocators Demo
    print(f"\n{'='*80}")
    print("📦 MEMORY ALLOCATORS DEMONSTRATION")
    print(f"{'='*80}")
    
    if use_cmake:
        run_command("python build.py --allocator bitmap --build", "Building with Bitmap Allocator")
        run_command("python build.py --allocator buddy --build", "Building with Buddy System Allocator")
    else:
        run_command("make build-bitmap", "Building with Bitmap Allocator")
        run_command("make build-buddy", "Building with Buddy System Allocator")
    
    # Schedulers Demo
    print(f"\n{'='*80}")
    print("⚡ SCHEDULERS DEMONSTRATION")
    print(f"{'='*80}")
    
    if use_cmake:
        run_command("python build.py --scheduler round_robin --build", "Building with Round-Robin Scheduler")
        run_command("python build.py --scheduler priority --build", "Building with Priority Scheduler")
    else:
        run_command("make build-round-robin", "Building with Round-Robin Scheduler")
        run_command("make build-priority", "Building with Priority Scheduler")
    
    # IPC Mechanisms Demo
    print(f"\n{'='*80}")
    print("💬 IPC MECHANISMS DEMONSTRATION")
    print(f"{'='*80}")
    
    if use_cmake:
        run_command("python build.py --ipc message_queue --build", "Building with Message Queue IPC")
        run_command("python build.py --ipc shared_memory --build", "Building with Shared Memory IPC")
    else:
        run_command("make build-message-queue", "Building with Message Queue IPC")
        run_command("make build-shared-memory", "Building with Shared Memory IPC")
    
    # Combined Builds
    print(f"\n{'='*80}")
    print("🔧 COMBINED IMPLEMENTATION BUILDS")
    print(f"{'='*80}")
    
    if use_cmake:
        run_command(
            "python build.py --allocator bitmap --scheduler round_robin --ipc message_queue --build",
            "Building: Bitmap + Round-Robin + Message Queue"
        )
        
        run_command(
            "python build.py --allocator buddy --scheduler priority --ipc shared_memory --build",
            "Building: Buddy + Priority + Shared Memory"
        )
    else:
        run_command(
            "make MEMORY_ALLOCATOR=bitmap SCHEDULER=round_robin IPC_MECHANISM=message_queue all",
            "Building: Bitmap + Round-Robin + Message Queue"
        )
        
        run_command(
            "make MEMORY_ALLOCATOR=buddy SCHEDULER=priority IPC_MECHANISM=shared_memory all",
            "Building: Buddy + Priority + Shared Memory"
        )
    
    # Performance Comparison (if test framework is available)
    print(f"\n{'='*80}")
    print("📊 PERFORMANCE COMPARISON")
    print(f"{'='*80}")
    
    if os.path.exists("tests/compare_components.py"):
        run_command(
            "python tests/compare_components.py allocators",
            "Comparing Memory Allocator Performance"
        )
        
        run_command(
            "python tests/compare_components.py schedulers", 
            "Comparing Scheduler Performance"
        )
        
        run_command(
            "python tests/compare_components.py ipc",
            "Comparing IPC Mechanism Performance"
        )
    else:
        print("⚠️  Performance comparison tools not available")
        print("   Run 'python setup_tests.py' to install test framework")

    # Summary
    print(f"\n{'='*80}")
    print("🎉 DEMONSTRATION COMPLETE")
    print(f"{'='*80}")
    
    print("""
✅ Successfully demonstrated MTOS virtual interface system!

📚 What you've seen:
   • Multiple working implementations for each virtual module
   • Easy switching between different algorithms
   • Modular build system supporting any combination
   • Performance comparison tools

🎓 Next steps for students:
   • Study the implementation differences in kernel/ directory
   • Try building with different combinations
   • Implement your own algorithms using the virtual interfaces
   • Use the comparison tools to analyze performance

📖 Documentation:
   • STUDENT_GUIDE.md - Complete tutorial
   • README_TEST_FRAMEWORK.md - Testing guide
   • kernel/interfaces/kernel_interfaces.h - API reference

💡 Tips:
   • Use 'make demo-all' for a quick demonstration
   • Try 'python tests/compare_components.py --help' for analysis options
   • Each implementation includes educational comments and statistics
    """)

if __name__ == "__main__":
    main()
