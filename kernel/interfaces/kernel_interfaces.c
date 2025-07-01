/**
 * MTOS Kernel Interface Registry Implementation
 * Registration and management of virtual interfaces
 */

#include "kernel_interfaces.h"

// Global kernel registry
kernel_registry_t g_kernel_registry = {0};

// External declarations for all implementations
extern physical_allocator_ops_t bitmap_allocator_ops;
extern physical_allocator_ops_t buddy_allocator_ops;
extern scheduler_ops_t round_robin_scheduler_ops;
extern scheduler_ops_t priority_scheduler_ops;
extern ipc_transport_ops_t message_queue_ipc_ops;
extern ipc_transport_ops_t shared_memory_ipc_ops;

// Registration functions
int register_physical_allocator(physical_allocator_ops_t *ops) {
    if (!ops) return -1;
    g_kernel_registry.physical_allocator = ops;
    return 0;
}

int register_virtual_memory(virtual_memory_ops_t *ops) {
    if (!ops) return -1;
    g_kernel_registry.virtual_memory = ops;
    return 0;
}

int register_heap_allocator(heap_allocator_ops_t *ops) {
    if (!ops) return -1;
    g_kernel_registry.heap_allocator = ops;
    return 0;
}

int register_scheduler(scheduler_ops_t *ops) {
    if (!ops) return -1;
    g_kernel_registry.scheduler = ops;
    return 0;
}

int register_process_loader(process_loader_ops_t *ops) {
    if (!ops) return -1;
    g_kernel_registry.process_loader = ops;
    return 0;
}

int register_ipc_transport(ipc_transport_ops_t *ops) {
    if (!ops) return -1;
    g_kernel_registry.ipc_transport = ops;
    return 0;
}

int register_device_driver(device_driver_ops_t *ops) {
    if (!ops) return -1;
    // Simple implementation - just store first driver
    // In real implementation, would manage array of drivers
    return 0;
}

// Component switching
int switch_component(const char *component_type, const char *component_name) {
    if (!component_type || !component_name) return -1;
    
    // Simple string comparison - in real implementation would use proper comparison
    // For educational purposes, we'll implement basic switching
    
    if (component_type[0] == 'p' && component_type[1] == 'h') { // "physical_allocator"
        if (component_name[0] == 'b' && component_name[1] == 'i') { // "bitmap"
            return register_physical_allocator(&bitmap_allocator_ops);
        } else if (component_name[0] == 'b' && component_name[1] == 'u') { // "buddy"
            return register_physical_allocator(&buddy_allocator_ops);
        }
    } else if (component_type[0] == 's') { // "scheduler"
        if (component_name[0] == 'r') { // "round_robin"
            return register_scheduler(&round_robin_scheduler_ops);
        } else if (component_name[0] == 'p') { // "priority"
            return register_scheduler(&priority_scheduler_ops);
        }
    } else if (component_type[0] == 'i') { // "ipc_transport"
        if (component_name[0] == 'm') { // "message_queue"
            return register_ipc_transport(&message_queue_ipc_ops);
        } else if (component_name[0] == 's') { // "shared_memory"
            return register_ipc_transport(&shared_memory_ipc_ops);
        }
    }
    
    return -1; // Unknown component
}

// Initialize default components
void init_kernel_registry(void) {
    // Register default implementations
    register_physical_allocator(&bitmap_allocator_ops);
    register_scheduler(&round_robin_scheduler_ops);
    register_ipc_transport(&message_queue_ipc_ops);
}

// Print registered components
void print_registered_components(void) {
    // Define printf if not available
    #ifndef printf
    extern int printf(const char *format, ...);
    #endif
    
    printf("MTOS REGISTERED COMPONENTS:\n");
    
    if (g_kernel_registry.physical_allocator) {
        printf("  Physical Allocator: %s - %s\n", 
               g_kernel_registry.physical_allocator->name,
               g_kernel_registry.physical_allocator->description);
    }
    
    if (g_kernel_registry.scheduler) {
        printf("  Scheduler: %s - %s\n",
               g_kernel_registry.scheduler->name,
               g_kernel_registry.scheduler->description);
    }
    
    if (g_kernel_registry.ipc_transport) {
        printf("  IPC Transport: %s - %s\n",
               g_kernel_registry.ipc_transport->name,
               g_kernel_registry.ipc_transport->description);
    }
    
    if (g_kernel_registry.virtual_memory) {
        printf("  Virtual Memory: %s - %s\n",
               g_kernel_registry.virtual_memory->name,
               g_kernel_registry.virtual_memory->description);
    }
    
    if (g_kernel_registry.heap_allocator) {
        printf("  Heap Allocator: %s - %s\n",
               g_kernel_registry.heap_allocator->name,
               g_kernel_registry.heap_allocator->description);
    }
}
