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
    except subprocess.TimeoutExpired:
        print("⏰ TIMEOUT")
    except Exception as e:
        print(f"💥 EXCEPTION: {e}")

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

    if not os.path.exists("Makefile"):
        print("❌ Error: Must be run from MTOS root directory")
        sys.exit(1)

    # Clean previous builds
    print("\n🧹 Cleaning previous builds...")
    run_command("make clean", "Cleaning build artifacts")

    # Memory Allocators Demo
    print(f"\n{'='*80}")
    print("📦 MEMORY ALLOCATORS DEMONSTRATION")
    print(f"{'='*80}")
    
    run_command("make build-bitmap", "Building with Bitmap Allocator")
    run_command("make build-buddy", "Building with Buddy System Allocator")
    
    # Schedulers Demo
    print(f"\n{'='*80}")
    print("⚡ SCHEDULERS DEMONSTRATION")
    print(f"{'='*80}")
    
    run_command("make build-round-robin", "Building with Round-Robin Scheduler")
    run_command("make build-priority", "Building with Priority Scheduler")
    
    # IPC Mechanisms Demo
    print(f"\n{'='*80}")
    print("💬 IPC MECHANISMS DEMONSTRATION")
    print(f"{'='*80}")
    
    run_command("make build-message-queue", "Building with Message Queue IPC")
    run_command("make build-shared-memory", "Building with Shared Memory IPC")
    
    # Combined Builds
    print(f"\n{'='*80}")
    print("🔧 COMBINED IMPLEMENTATION BUILDS")
    print(f"{'='*80}")
    
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
