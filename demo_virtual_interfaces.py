#!/usr/bin/env python3
"""
MTOS Virtual Interface Demo
Interactive demonstration of the pluggable component system
"""

import os
import sys
import time
import subprocess
from typing import Dict, List

class MTOSDemo:
    """Interactive demo of MTOS virtual interface system."""
    
    def __init__(self):
        self.components = {
            'allocators': ['bitmap', 'buddy'],
            'schedulers': ['round_robin', 'priority'],
            'ipc': ['message_queue', 'shared_memory']
        }
        
        self.available_components = {
            'allocators': {
                'bitmap': 'Simple bitmap-based allocator (educational)',
                'buddy': 'Binary buddy system with coalescing'
            },
            'schedulers': {
                'round_robin': 'Time-sliced fair scheduling',
                'priority': 'Multi-level priority with aging'
            },
            'ipc': {
                'message_queue': 'Classic message passing with queues',
                'shared_memory': 'High-performance shared memory regions'
            }
        }
        
        self.current_config = {
            'allocator': 'bitmap',
            'scheduler': 'round_robin',
            'ipc': 'message_queue'
        }
    
    def print_banner(self):
        """Print the demo banner."""
        print("=" * 60)
        print("🎓 MTOS Virtual Interface System Demo")
        print("   Educational Operating System with Pluggable Components")
        print("=" * 60)
        print()
    
    def print_menu(self):
        """Print the main menu."""
        print("📋 Main Menu:")
        print("1. 🧠 Explore Memory Allocators")
        print("2. ⚙️  Explore Process Schedulers") 
        print("3. 💬 Explore IPC Mechanisms")
        print("4. 🏗️  Build Custom Configuration")
        print("5. 📊 Compare Component Performance")
        print("6. 🎯 Educational Exercises")
        print("7. ❓ Learn About Virtual Interfaces")
        print("8. 🚪 Exit")
        print()
    
    def explain_virtual_interfaces(self):
        """Explain the virtual interface concept."""
        print("\n🔌 Virtual Interface System Explained")
        print("-" * 40)
        print("""
The virtual interface system in MTOS allows you to:

💡 CONCEPT: Function Pointers as Contracts
  Every OS component (memory allocator, scheduler, etc.) implements
  the same interface - a set of function pointers that define what
  the component can do.

🔄 RUNTIME SWAPPING:
  You can switch between different implementations at build time
  or even at runtime, without changing the rest of the kernel.

📚 EDUCATIONAL BENEFITS:
  • Compare algorithms side-by-side
  • Focus on one area without worrying about others  
  • Measure real performance differences
  • Experiment safely with new ideas

🏗️ EXAMPLE: Memory Allocator Interface
  
  typedef struct {
      const char *name;
      uint32_t (*alloc_page)(void);      // All allocators implement this
      void (*free_page)(uint32_t addr);  // Common interface
      size_t (*get_free_pages)(void);    // Same function signature
  } allocator_ops_t;
  
  // Different implementations:
  allocator_ops_t bitmap_allocator = { .name = "bitmap", ... };
  allocator_ops_t buddy_allocator = { .name = "buddy", ... };
  
  // Kernel uses any allocator through same interface:
  uint32_t page = current_allocator->alloc_page();

🎯 WHY THIS MATTERS:
  • Real OS kernels use similar patterns (Linux VFS, Windows I/O Manager)
  • Learn design patterns used in production systems
  • Understand abstraction and modularity
  • Practice comparing algorithms objectively
""")
        input("\nPress Enter to continue...")
    
    def explore_allocators(self):
        """Explore memory allocators."""
        print("\n🧠 Memory Allocator Explorer")
        print("-" * 30)
        
        allocators_info = {
            'bitmap': {
                'description': 'Simple bitmap-based allocator',
                'complexity': 'O(n) allocation, O(1) deallocation',
                'pros': 'Simple, easy to understand, low memory overhead',
                'cons': 'Linear search time, external fragmentation',
                'best_for': 'Educational purposes, simple systems',
                'available': True
            },
            'buddy': {
                'description': 'Binary buddy system with coalescing',
                'complexity': 'O(log n) allocation and deallocation',
                'pros': 'Fast allocation, reduces fragmentation, coalescable',
                'cons': 'Internal fragmentation, more complex',
                'best_for': 'General purpose systems, kernel allocators',
                'available': True
            },
            'slab': {
                'description': 'Object-oriented cache allocator',
                'complexity': 'O(1) for cached objects',
                'pros': 'Very fast, cache-friendly, reduces fragmentation',
                'cons': 'Memory overhead, complex initialization',
                'best_for': 'Frequent allocation of same-sized objects',
                'available': False
            },
            'first_fit': {
                'description': 'First-fit heap allocator',
                'complexity': 'O(n) allocation, O(1) deallocation',
                'pros': 'Simple implementation, good for learning',
                'cons': 'External fragmentation, slow allocation',
                'best_for': 'Educational demonstrations',
                'available': False
            }
        }
        
        for name, info in allocators_info.items():
            status = "✅ Available" if info['available'] else "📝 Exercise"
            print(f"\n📦 {name.upper()} ALLOCATOR ({status}):")
            print(f"   Description: {info['description']}")
            print(f"   Complexity: {info['complexity']}")
            print(f"   Pros: {info['pros']}")
            print(f"   Cons: {info['cons']}")
            print(f"   Best for: {info['best_for']}")
        
        print(f"\nCurrent allocator: {self.current_config['allocator']}")
        choice = input("\nChoose allocator (bitmap/buddy/slab/first_fit) or press Enter to skip: ")
        if choice in self.components['allocators']:
            self.current_config['allocator'] = choice
            print(f"✅ Switched to {choice} allocator")
    
    def explore_schedulers(self):
        """Explore process schedulers."""
        print("\n⚙️ Process Scheduler Explorer")
        print("-" * 30)
        
        schedulers_info = {
            'round_robin': {
                'description': 'Time-sliced round-robin scheduling',
                'fairness': 'Perfect fairness (equal time slices)',
                'responsiveness': 'Good for interactive workloads',
                'complexity': 'O(1) scheduling decisions',
                'best_for': 'Interactive systems, fair resource sharing'
            },
            'priority': {
                'description': 'Priority-based preemptive scheduling',
                'fairness': 'Based on priorities (can cause starvation)',
                'responsiveness': 'Excellent for high-priority tasks',
                'complexity': 'O(1) with priority queues',
                'best_for': 'Real-time systems, differentiated service'
            },
            'cfs': {
                'description': 'Completely Fair Scheduler (Linux-inspired)',
                'fairness': 'Virtual runtime ensures long-term fairness',
                'responsiveness': 'Good balance of fairness and responsiveness',
                'complexity': 'O(log n) with red-black tree',
                'best_for': 'General-purpose desktop/server systems'
            },
            'mlfq': {
                'description': 'Multi-Level Feedback Queue',
                'fairness': 'Adaptive fairness based on behavior',
                'responsiveness': 'Favors interactive processes',
                'complexity': 'O(1) per level, adaptive complexity',
                'best_for': 'Mixed workloads, unknown process behavior'
            }
        }
        
        for name, info in schedulers_info.items():
            print(f"\n🔄 {name.upper()} SCHEDULER:")
            print(f"   Description: {info['description']}")
            print(f"   Fairness: {info['fairness']}")
            print(f"   Responsiveness: {info['responsiveness']}")
            print(f"   Complexity: {info['complexity']}")
            print(f"   Best for: {info['best_for']}")
        
        print(f"\nCurrent scheduler: {self.current_config['scheduler']}")
        choice = input("\nChoose scheduler (round_robin/priority/cfs/mlfq) or press Enter to skip: ")
        if choice in self.components['schedulers']:
            self.current_config['scheduler'] = choice
            print(f"✅ Switched to {choice} scheduler")
    
    def explore_ipc(self):
        """Explore IPC mechanisms."""
        print("\n💬 IPC Mechanism Explorer")
        print("-" * 25)
        
        ipc_info = {
            'message_queue': {
                'description': 'Asynchronous message passing',
                'latency': 'Medium (copying overhead)',
                'throughput': 'Medium',
                'security': 'Good (isolated message buffers)',
                'best_for': 'General communication, reliability'
            },
            'shared_memory': {
                'description': 'Direct memory sharing between processes',
                'latency': 'Very low (no copying)',
                'throughput': 'Very high',
                'security': 'Low (direct memory access)',
                'best_for': 'High-performance data sharing'
            },
            'pipe': {
                'description': 'Unix-style byte stream communication',
                'latency': 'Low',
                'throughput': 'High',
                'security': 'Medium (kernel-mediated)',
                'best_for': 'Stream processing, shell commands'
            },
            'capability': {
                'description': 'Capability-based secure communication',
                'latency': 'Higher (security checks)',
                'throughput': 'Medium',
                'security': 'Very high (fine-grained access control)',
                'best_for': 'Security-critical systems'
            }
        }
        
        for name, info in ipc_info.items():
            print(f"\n📡 {name.upper()} IPC:")
            print(f"   Description: {info['description']}")
            print(f"   Latency: {info['latency']}")
            print(f"   Throughput: {info['throughput']}")
            print(f"   Security: {info['security']}")
            print(f"   Best for: {info['best_for']}")
        
        print(f"\nCurrent IPC: {self.current_config['ipc']}")
        choice = input("\nChoose IPC (message_queue/shared_memory/pipe/capability) or press Enter to skip: ")
        if choice in self.components['ipc']:
            self.current_config['ipc'] = choice
            print(f"✅ Switched to {choice} IPC")
    
    def build_custom_configuration(self):
        """Build OS with custom component configuration."""
        print("\n🏗️ Custom Configuration Builder")
        print("-" * 35)
        
        print("Current configuration:")
        print(f"  📦 Memory Allocator: {self.current_config['allocator']}")
        print(f"  ⚙️  Process Scheduler: {self.current_config['scheduler']}")
        print(f"  💬 IPC Mechanism: {self.current_config['ipc']}")
        
        build = input("\nBuild OS with this configuration? (y/n): ")
        if build.lower() == 'y':
            print("\n🔨 Building MTOS with custom configuration...")
            print("This would run:")
            print(f"make ALLOCATOR={self.current_config['allocator']} " +
                  f"SCHEDULER={self.current_config['scheduler']} " +
                  f"IPC={self.current_config['ipc']} all")
            
            # Simulate build process
            for i in range(5):
                print("." * (i + 1), end="\r")
                time.sleep(0.5)
            
            print("\n✅ Build complete! (simulated)")
            print(f"📀 OS image: build/mtos_{self.current_config['allocator']}_" +
                  f"{self.current_config['scheduler']}_{self.current_config['ipc']}.img")
    
    def compare_performance(self):
        """Compare component performance."""
        print("\n📊 Performance Comparison Tool")
        print("-" * 30)
        
        print("Available comparisons:")
        print("1. 🧠 Memory Allocator Performance")
        print("2. ⚙️  Scheduler Fairness and Latency")
        print("3. 💬 IPC Throughput and Security")
        print("4. 🔄 All Components Overview")
        
        choice = input("\nChoose comparison (1-4): ")
        
        if choice == '1':
            print("\n🧠 Memory Allocator Comparison")
            print("This would run: python tests/compare_components.py allocators --visualize")
            print("\nKey metrics compared:")
            print("• Allocation speed (ops/second)")
            print("• Memory fragmentation (%)")
            print("• Memory overhead (%)")
            print("• Worst-case allocation time")
            
        elif choice == '2':
            print("\n⚙️ Scheduler Comparison")
            print("This would run: python tests/compare_components.py schedulers --visualize")
            print("\nKey metrics compared:")
            print("• Average response time")
            print("• Fairness index")
            print("• CPU utilization")
            print("• Context switch overhead")
            
        elif choice == '3':
            print("\n💬 IPC Comparison")
            print("This would run: python tests/compare_components.py ipc --visualize")
            print("\nKey metrics compared:")
            print("• Message latency (microseconds)")
            print("• Throughput (MB/second)")
            print("• CPU overhead (%)")
            print("• Security properties")
            
        elif choice == '4':
            print("\n🔄 Complete System Analysis")
            print("This would generate comprehensive reports showing:")
            print("• How component choices affect overall system performance")
            print("• Interaction effects between components")
            print("• Recommended configurations for different use cases")
    
    def educational_exercises(self):
        """Show educational exercises."""
        print("\n🎯 Educational Exercises")
        print("-" * 25)
        
        exercises = [
            {
                'title': 'Memory Allocator Fragmentation Study',
                'description': 'Compare how different allocators handle fragmentation',
                'steps': [
                    '1. Build OS with bitmap allocator',
                    '2. Run memory stress test',
                    '3. Measure fragmentation',
                    '4. Repeat with buddy allocator',
                    '5. Analyze differences'
                ],
                'learning': 'Understanding internal vs external fragmentation'
            },
            {
                'title': 'Scheduler Fairness Analysis',
                'description': 'Study how different schedulers treat processes',
                'steps': [
                    '1. Create workload with CPU-bound and I/O-bound processes',
                    '2. Run with round-robin scheduler',
                    '3. Measure response times and fairness',
                    '4. Compare with CFS scheduler',
                    '5. Analyze fairness metrics'
                ],
                'learning': 'Tradeoffs between fairness and performance'
            },
            {
                'title': 'IPC Security vs Performance',
                'description': 'Explore the security-performance tradeoff in IPC',
                'steps': [
                    '1. Benchmark shared memory IPC (fast, less secure)',
                    '2. Benchmark capability-based IPC (secure, slower)',
                    '3. Measure latency differences',
                    '4. Analyze security implications',
                    '5. Design hybrid approach'
                ],
                'learning': 'Security often comes at a performance cost'
            }
        ]
        
        for i, exercise in enumerate(exercises, 1):
            print(f"\n📝 Exercise {i}: {exercise['title']}")
            print(f"   Goal: {exercise['description']}")
            print(f"   Learning objective: {exercise['learning']}")
            print("   Steps:")
            for step in exercise['steps']:
                print(f"     {step}")
    
    def run_demo(self):
        """Run the interactive demo."""
        self.print_banner()
        
        while True:
            self.print_menu()
            choice = input("Choose an option (1-8): ")
            
            if choice == '1':
                self.explore_allocators()
            elif choice == '2':
                self.explore_schedulers()
            elif choice == '3':
                self.explore_ipc()
            elif choice == '4':
                self.build_custom_configuration()
            elif choice == '5':
                self.compare_performance()
            elif choice == '6':
                self.educational_exercises()
            elif choice == '7':
                self.explain_virtual_interfaces()
            elif choice == '8':
                print("\n👋 Thanks for exploring MTOS!")
                print("   To continue learning:")
                print("   • Read STUDENT_GUIDE.md for detailed information")
                print("   • Try building with: make demo-allocators")
                print("   • Run comparisons with: make compare-schedulers")
                break
            else:
                print("❌ Invalid choice. Please try again.")
            
            input("\nPress Enter to return to main menu...")


def main():
    """Main demo entry point."""
    demo = MTOSDemo()
    demo.run_demo()


if __name__ == "__main__":
    main()
