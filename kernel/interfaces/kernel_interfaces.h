/**
 * MTOS Virtual Interface System
 * Core abstractions for pluggable kernel components
 * Educational OS with swappable algorithms
 */

#ifndef MTOS_KERNEL_INTERFACES_H
#define MTOS_KERNEL_INTERFACES_H

#include "../../include/types.h"

// Forward declarations
typedef struct process process_t;
typedef struct memory_region memory_region_t;

/**
 * IPC Message Structure
 */
#define IPC_MAX_MESSAGE_SIZE 4096

typedef struct ipc_message {
    uint32_t sender_id;
    uint32_t receiver_id;
    uint32_t message_id;
    uint32_t type;
    size_t size;
    uint8_t data[IPC_MAX_MESSAGE_SIZE];
    uint32_t timestamp;
    uint32_t flags;
} ipc_message_t;

/**
 * =============================================================================
 * MEMORY MANAGEMENT INTERFACES
 * =============================================================================
 */

/**
 * Physical Memory Allocator Interface
 * Students can implement: Bitmap, Buddy System, Free List, etc.
 */
typedef struct physical_allocator_ops {
    const char *name;
    const char *description;
    
    // Initialize the allocator
    int (*init)(uint32_t start_addr, uint32_t end_addr);
    
    // Allocate/free physical pages
    uint32_t (*alloc_page)(void);
    uint32_t (*alloc_pages)(size_t count);
    void (*free_page)(uint32_t paddr);
    void (*free_pages)(uint32_t paddr, size_t count);
    
    // Statistics and debugging
    size_t (*get_free_pages)(void);
    size_t (*get_total_pages)(void);
    void (*print_stats)(void);
    
    // Advanced features (optional)
    uint32_t (*alloc_aligned)(size_t size, size_t alignment);
    bool (*is_available)(uint32_t paddr);
} physical_allocator_ops_t;

/**
 * Virtual Memory Manager Interface
 * Students can implement: Simple paging, Multi-level paging, IOMMU, etc.
 */
typedef struct virtual_memory_ops {
    const char *name;
    const char *description;
    
    // Page table management
    int (*init)(void);
    uint32_t (*create_address_space)(void);
    void (*destroy_address_space)(uint32_t page_dir);
    void (*switch_address_space)(uint32_t page_dir);
    
    // Memory mapping
    int (*map_page)(uint32_t vaddr, uint32_t paddr, uint32_t flags);
    int (*unmap_page)(uint32_t vaddr);
    uint32_t (*get_physical)(uint32_t vaddr);
    
    // Memory regions
    int (*map_region)(uint32_t vstart, uint32_t pstart, size_t size, uint32_t flags);
    void (*unmap_region)(uint32_t vstart, size_t size);
    
    // Page fault handling
    int (*handle_page_fault)(uint32_t fault_addr, uint32_t error_code);
    
    // Statistics
    void (*print_mappings)(uint32_t page_dir);
} virtual_memory_ops_t;

/**
 * Heap Allocator Interface
 * Students can implement: First-fit, Best-fit, Slab, SLOB, etc.
 */
typedef struct heap_allocator_ops {
    const char *name;
    const char *description;
    
    // Basic allocation
    int (*init)(void *heap_start, size_t heap_size);
    void* (*malloc)(size_t size);
    void* (*calloc)(size_t count, size_t size);
    void* (*realloc)(void *ptr, size_t size);
    void (*free)(void *ptr);
    
    // Aligned allocation
    void* (*aligned_alloc)(size_t alignment, size_t size);
    
    // Statistics and debugging
    size_t (*get_free_size)(void);
    size_t (*get_used_size)(void);
    size_t (*get_total_size)(void);
    void (*print_heap_info)(void);
    bool (*validate_heap)(void);
    
    // Advanced features
    void* (*malloc_atomic)(size_t size);  // Non-movable allocation
    void (*defragment)(void);
} heap_allocator_ops_t;

/**
 * =============================================================================
 * PROCESS MANAGEMENT INTERFACES
 * =============================================================================
 */

/**
 * Process Scheduler Interface
 * Students can implement: Round-robin, CFS, Priority, Real-time, etc.
 */
typedef struct scheduler_ops {
    const char *name;
    const char *description;
    
    // Scheduler lifecycle
    int (*init)(void);
    void (*shutdown)(void);
    
    // Process management
    void (*add_process)(process_t *proc);
    void (*remove_process)(process_t *proc);
    process_t* (*get_next)(void);
    
    // Scheduling decisions
    void (*schedule)(void);
    void (*yield)(void);
    void (*block)(process_t *proc);
    void (*unblock)(process_t *proc);
    
    // Time management
    void (*timer_tick)(void);
    uint32_t (*get_time_slice)(process_t *proc);
    
    // Priority and policy
    void (*set_priority)(process_t *proc, int priority);
    int (*get_priority)(process_t *proc);
    
    // Statistics
    void (*print_stats)(void);
    uint32_t (*get_context_switches)(void);
    uint32_t (*get_avg_wait_time)(void);
} scheduler_ops_t;

/**
 * Process Loader Interface
 * Students can implement: ELF, PE, Custom formats, JIT, etc.
 */
typedef struct process_loader_ops {
    const char *name;
    const char *description;
    
    // Format detection
    bool (*can_load)(const void *binary_data, size_t size);
    
    // Loading process
    int (*load_process)(const void *binary_data, size_t size, process_t *proc);
    void (*unload_process)(process_t *proc);
    
    // Entry point and segments
    uint32_t (*get_entry_point)(const void *binary_data);
    int (*get_segments)(const void *binary_data, memory_region_t **segments, size_t *count);
    
    // Relocation and linking
    int (*relocate)(process_t *proc, uint32_t base_addr);
    int (*resolve_symbols)(process_t *proc);
    
    // Format-specific info
    void (*print_info)(const void *binary_data);
} process_loader_ops_t;

/**
 * =============================================================================
 * INTER-PROCESS COMMUNICATION INTERFACES
 * =============================================================================
 */

/**
 * IPC Transport Interface
 * Students can implement: Message queues, Shared memory, Pipes, etc.
 */
typedef struct ipc_transport_ops {
    const char *name;
    const char *description;
    
    // Transport lifecycle
    int (*init)(void);
    void (*shutdown)(void);
    
    // Communication channels
    int (*create_channel)(uint32_t sender_id, uint32_t receiver_id);
    void (*destroy_channel)(int channel_id);
    
    // Message operations
    int (*send_message)(int channel_id, const ipc_message_t *msg);
    int (*receive_message)(int channel_id, ipc_message_t *msg);
    int (*try_receive)(int channel_id, ipc_message_t *msg);
    
    // Flow control
    bool (*can_send)(int channel_id);
    bool (*has_messages)(int channel_id);
    size_t (*get_queue_size)(int channel_id);
    
    // Capabilities and security
    bool (*check_permission)(uint32_t sender_id, uint32_t receiver_id);
    void (*grant_capability)(uint32_t grantor, uint32_t grantee, uint32_t rights);
    
    // Statistics
    void (*print_stats)(void);
} ipc_transport_ops_t;

/**
 * =============================================================================
 * I/O AND DEVICE INTERFACES
 * =============================================================================
 */

/**
 * Device Driver Interface
 * Students can implement: Polling, Interrupt-driven, DMA, etc.
 */
typedef struct device_driver_ops {
    const char *name;
    const char *description;
    uint32_t device_type;
    
    // Driver lifecycle
    int (*probe)(uint32_t device_id);
    int (*init)(uint32_t device_id);
    void (*shutdown)(uint32_t device_id);
    
    // I/O operations
    ssize_t (*read)(uint32_t device_id, void *buffer, size_t size, offset_t offset);
    ssize_t (*write)(uint32_t device_id, const void *buffer, size_t size, offset_t offset);
    int (*ioctl)(uint32_t device_id, uint32_t cmd, void *arg);
    
    // Async operations
    int (*read_async)(uint32_t device_id, void *buffer, size_t size, void (*callback)(int status));
    int (*write_async)(uint32_t device_id, const void *buffer, size_t size, void (*callback)(int status));
    
    // Power management
    int (*suspend)(uint32_t device_id);
    int (*resume)(uint32_t device_id);
    
    // Status and debugging
    uint32_t (*get_status)(uint32_t device_id);
    void (*print_info)(uint32_t device_id);
} device_driver_ops_t;

/**
 * =============================================================================
 * SUBSYSTEM REGISTRATION INTERFACE
 * =============================================================================
 */

/**
 * Kernel Subsystem Registry
 * Allows runtime swapping of algorithms
 */
typedef struct kernel_registry {
    // Memory management
    physical_allocator_ops_t *physical_allocator;
    virtual_memory_ops_t *virtual_memory;
    heap_allocator_ops_t *heap_allocator;
    
    // Process management
    scheduler_ops_t *scheduler;
    process_loader_ops_t *process_loader;
    
    // IPC
    ipc_transport_ops_t *ipc_transport;
    
    // Device drivers (array)
    device_driver_ops_t **device_drivers;
    size_t num_drivers;
} kernel_registry_t;

// Global registry
extern kernel_registry_t g_kernel_registry;

/**
 * =============================================================================
 * REGISTRATION FUNCTIONS
 * =============================================================================
 */

// Memory management registration
int register_physical_allocator(physical_allocator_ops_t *ops);
int register_virtual_memory(virtual_memory_ops_t *ops);
int register_heap_allocator(heap_allocator_ops_t *ops);

// Process management registration
int register_scheduler(scheduler_ops_t *ops);
int register_process_loader(process_loader_ops_t *ops);

// IPC registration
int register_ipc_transport(ipc_transport_ops_t *ops);

// Device driver registration
int register_device_driver(device_driver_ops_t *ops);

// Utility functions
void print_registered_components(void);
int switch_component(const char *component_type, const char *component_name);

/**
 * =============================================================================
 * CONVENIENCE MACROS FOR STUDENTS
 * =============================================================================
 */

// Easy access to current implementations
#define PHYS_ALLOC() (g_kernel_registry.physical_allocator)
#define VIRT_MEM() (g_kernel_registry.virtual_memory)
#define HEAP_ALLOC() (g_kernel_registry.heap_allocator)
#define SCHEDULER() (g_kernel_registry.scheduler)
#define IPC_TRANSPORT() (g_kernel_registry.ipc_transport)

// Quick allocation calls
#define kmalloc(size) HEAP_ALLOC()->malloc(size)
#define kfree(ptr) HEAP_ALLOC()->free(ptr)
#define alloc_page() PHYS_ALLOC()->alloc_page()
#define free_page(addr) PHYS_ALLOC()->free_page(addr)

#endif // MTOS_KERNEL_INTERFACES_H
