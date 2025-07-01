# MTOS Student Developer Guide
## Virtual Interface System for Educational OS Development

Welcome to MTOS (Mute OS) - an educational operating system designed with **virtual interfaces** that allow you to experiment with different algorithms and implementations.

## 🎯 Educational Philosophy

MTOS uses a **virtual interface system** where every major OS component is abstracted behind well-defined interfaces. This allows you to:

- **Swap algorithms easily** - Compare different memory allocators, schedulers, etc.
- **Focus on specific areas** - Study one component without worrying about others
- **Measure performance** - Built-in benchmarking and comparison tools
- **Learn through experimentation** - See how different approaches affect system behavior

## 🏗️ Architecture Overview

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Rust Apps     │    │   Rust Apps     │    │   Rust Apps     │
│   (Userspace)   │    │   (Userspace)   │    │   (Userspace)   │
└─────────┬───────┘    └─────────┬───────┘    └─────────┬───────┘
          │                      │                      │
          └──────────────────────┼──────────────────────┘
                                 │
    ┌─────────────────────────────┼─────────────────────────────┐
    │              SYSTEM CALL INTERFACE                      │
    └─────────────────────────────┼─────────────────────────────┘
                                 │
    ┌─────────────────────────────┼─────────────────────────────┐
    │                    C MICROKERNEL                        │
    │  ┌─────────────┐  ┌──────────────┐  ┌─────────────────┐ │
    │  │   Memory    │  │  Scheduler   │  │      IPC        │ │
    │  │ Management  │  │             │  │                 │ │
    │  │             │  │              │  │                 │ │
    │  │ Virtual     │  │ Virtual      │  │ Virtual         │ │
    │  │ Interface   │  │ Interface    │  │ Interface       │ │
    │  └─────────────┘  └──────────────┘  └─────────────────┘ │
    └─────────────────────────────────────────────────────────┘
```

## 🔌 Virtual Interface System

### Core Interfaces

Every major OS component implements a **virtual interface** (vtable):

#### 1. **Memory Management**
```c
// Physical Memory Allocator Interface
typedef struct physical_allocator_ops {
    const char *name;                    // "bitmap", "buddy", "slab"
    int (*init)(uint32_t start, uint32_t end);
    uint32_t (*alloc_page)(void);
    void (*free_page)(uint32_t addr);
    // ... more functions
} physical_allocator_ops_t;
```

#### 2. **Process Scheduler**
```c
// Scheduler Interface
typedef struct scheduler_ops {
    const char *name;                    // "round_robin", "cfs", "priority"
    void (*schedule)(void);
    void (*add_process)(process_t *proc);
    process_t* (*get_next)(void);
    // ... more functions
} scheduler_ops_t;
```

#### 3. **IPC Mechanism**
```c
// IPC Transport Interface
typedef struct ipc_transport_ops {
    const char *name;                    // "message_queue", "shared_memory"
    int (*send_message)(int channel, const message_t *msg);
    int (*receive_message)(int channel, message_t *msg);
    // ... more functions
} ipc_transport_ops_t;
```

### Component Registration

Components register themselves with the kernel:

```c
// Register your custom allocator
register_physical_allocator(&my_custom_allocator_ops);

// Switch to different scheduler at runtime
switch_component("scheduler", "cfs");
```

## 📚 Available Implementations

MTOS now includes **multiple working implementations** for each virtual module:

### Memory Allocators

| Algorithm | Characteristics | Best For | Implementation |
|-----------|----------------|----------|---------------|
| **Bitmap** | Simple, educational | Learning basics | ✅ Available |
| **Buddy System** | Balanced performance, coalescing | General purpose | ✅ Available |
| **Slab** | Cache-friendly | Object allocation | 📝 Exercise |
| **First-Fit** | Minimal overhead | Simple systems | 📝 Exercise |

### Process Schedulers

| Algorithm | Characteristics | Best For | Implementation |
|-----------|----------------|----------|---------------|
| **Round Robin** | Fair, predictable | Interactive systems | ✅ Available |
| **Priority** | Responsive, aging | Real-time tasks | ✅ Available |
| **CFS** | Linux-like fairness | General workloads | 📝 Exercise |
| **MLFQ** | Adaptive | Mixed workloads | 📝 Exercise |
| **Lottery** | Proportional share | Resource allocation | 📝 Exercise |

### IPC Mechanisms

| Mechanism | Characteristics | Best For | Implementation |
|-----------|----------------|----------|---------------|
| **Message Queues** | Reliable, ordered | General communication | ✅ Available |
| **Shared Memory** | High throughput | Data sharing | ✅ Available |
| **Pipes** | Unix-style | Stream processing | 📝 Exercise |
| **Capabilities** | Secure | Security-critical | 📝 Exercise |

## 🛠️ Development Workflow

### 1. **Choose Your Focus Area**

Pick what you want to study:
```bash
# Study memory allocators
make demo-allocators

# Study schedulers  
make demo-schedulers

# Compare IPC mechanisms
make compare-ipc
```

### 2. **Build with Specific Components**

```bash
# Build OS with buddy allocator
make ALLOCATOR=buddy all

# Build with CFS scheduler
make SCHEDULER=cfs all

# Build with capability-based IPC
make IPC=capability all
```

### 3. **Run Benchmarks**

```bash
# Compare allocator performance
python tests/compare_components.py allocators --visualize

# Compare scheduler fairness
python tests/compare_components.py schedulers --visualize

# Analyze IPC latency
python tests/compare_components.py ipc --visualize
```

## 🧪 Creating Your Own Implementation

### Step 1: Define Your Algorithm

Create a new file `kernel/memory/my_allocator.c`:

```c
#include "../interfaces/kernel_interfaces.h"

// Your allocator state
typedef struct my_allocator_state {
    // Your data structures here
    void *heap_start;
    size_t heap_size;
    // ... custom fields
} my_allocator_state_t;

static my_allocator_state_t g_my_allocator;

// Implement the interface functions
static int my_allocator_init(uint32_t start, uint32_t end) {
    // Your initialization code
    g_my_allocator.heap_start = (void*)start;
    g_my_allocator.heap_size = end - start;
    return 0;
}

static uint32_t my_allocator_alloc_page(void) {
    // Your allocation algorithm
    // Return physical address of allocated page
}

static void my_allocator_free_page(uint32_t addr) {
    // Your deallocation algorithm
}

// Define the interface
physical_allocator_ops_t my_allocator_ops = {
    .name = "my_algorithm",
    .description = "My custom allocation algorithm",
    .init = my_allocator_init,
    .alloc_page = my_allocator_alloc_page,
    .free_page = my_allocator_free_page,
    // ... implement other functions
};
```

### Step 2: Register Your Implementation

In `kernel/memory/init.c`:

```c
void init_memory_allocators(void) {
    // Register all available allocators
    register_physical_allocator(&bitmap_allocator_ops);
    register_physical_allocator(&buddy_allocator_ops);
    register_physical_allocator(&my_allocator_ops);  // Your allocator
}
```

### Step 3: Test Your Implementation

```bash
# Build with your allocator
make ALLOCATOR=my_algorithm all

# Test it
make ALLOCATOR=my_algorithm test-allocator

# Compare with others
python tests/compare_components.py allocators
```

## 📊 Educational Analysis Tools

### Performance Comparison

Generate detailed performance reports:

```bash
# Compare all memory allocators
python tests/compare_components.py allocators --output allocator_report.txt

# Generate visualizations
python tests/compare_components.py schedulers --visualize
```

### Interactive Exploration

```bash
# Interactive scheduler comparison
python tests/interactive_scheduler_demo.py

# Memory allocator visualization
python tests/memory_visualizer.py
```

### Custom Benchmarks

Create your own test scenarios:

```python
# Custom benchmark in tests/my_benchmark.py
from compare_components import AllocatorBenchmark

class MyCustomBenchmark(AllocatorBenchmark):
    def run_benchmark(self, allocator_name, os_image):
        # Your custom test logic
        return {"my_metric": test_result}
```

## 🎓 Learning Exercises

### Exercise 1: Allocator Analysis
1. Run all allocator implementations
2. Compare fragmentation patterns
3. Analyze which works best for different allocation patterns
4. **Challenge**: Implement a hybrid allocator

### Exercise 2: Scheduler Fairness
1. Create workloads with different process types
2. Measure fairness with different schedulers  
3. Identify starvation scenarios
4. **Challenge**: Design a scheduler for real-time tasks

### Exercise 3: IPC Performance
1. Benchmark IPC mechanisms with different message sizes
2. Analyze latency vs throughput tradeoffs
3. Study security implications
4. **Challenge**: Design a zero-copy IPC mechanism

### Exercise 4: System Integration
1. Choose optimal components for specific use cases:
   - **Web server**: High concurrency, I/O heavy
   - **Game engine**: Low latency, real-time
   - **Database**: Memory intensive, consistent performance
   - **Embedded system**: Low memory, predictable timing

## 🔬 Advanced Features

### Runtime Component Switching

Switch algorithms at runtime for comparison:

```c
// Switch from bitmap to buddy allocator
switch_component("physical_allocator", "buddy");

// Switch from round-robin to CFS
switch_component("scheduler", "cfs");
```

### Performance Monitoring

Built-in performance counters:

```c
// Get allocator statistics
allocator_stats_t stats = PHYS_ALLOC()->get_stats();
printf("Free pages: %zu\n", stats.free_pages);
printf("Fragmentation: %f%%\n", stats.fragmentation);

// Get scheduler metrics
scheduler_metrics_t metrics = SCHEDULER()->get_metrics();
printf("Avg response time: %u ms\n", metrics.avg_response_time);
```

### Educational Visualizations

Generate graphs and charts:

```bash
# Memory allocation timeline
python tools/memory_timeline.py

# Scheduler gantt chart
python tools/scheduler_gantt.py

# IPC message flow diagram
python tools/ipc_flow_diagram.py
```

## 🚀 Getting Started

1. **Setup Environment**:
   ```bash
   python setup_tests.py
   ```

2. **Build and Test**:
   ```bash
   make clean && make
   make test
   ```

3. **Try Different Components**:
   ```bash
   make demo-allocators
   make compare-schedulers
   ```

4. **Create Your First Implementation**:
   - Copy an existing allocator as a template
   - Modify the algorithm
   - Test and benchmark your changes

5. **Explore and Learn**:
   - Use the visualization tools
   - Read the generated reports
   - Experiment with different combinations

## 💡 Tips for Success

1. **Start Simple**: Begin with bitmap allocator or round-robin scheduler
2. **Use Debugging**: Enable verbose logging to understand behavior
3. **Measure Everything**: Use the built-in benchmarking tools
4. **Compare Actively**: Always compare your implementation with others
5. **Read the Code**: Study existing implementations to learn patterns
6. **Ask Questions**: Use the debugging and validation functions

## 📖 Further Reading

- **Operating System Concepts** by Silberschatz (Theoretical background)
- **Modern Operating Systems** by Tanenbaum (Implementation details)
- **The Design and Implementation of the FreeBSD Operating System** (Real-world examples)
- **Linux Kernel Development** by Love (Advanced implementation techniques)

## 🤝 Contributing

When you create interesting implementations:

1. Document your algorithm's characteristics
2. Add educational comments explaining design decisions
3. Include benchmark results and analysis
4. Consider contributing back to help other students

---

**Happy OS Development!** 🎉

The virtual interface system in MTOS makes it easy to focus on the algorithms and concepts that interest you most, while providing the tools to measure and understand the tradeoffs involved in operating system design.
