/**
 * MTOS Example Memory Allocators
 * Educational implementations for students to study and improve
 */

#ifndef MTOS_MEMORY_ALLOCATORS_H
#define MTOS_MEMORY_ALLOCATORS_H

#include "../interfaces/kernel_interfaces.h"

/**
 * =============================================================================
 * BITMAP PHYSICAL ALLOCATOR (Simple but educational)
 * =============================================================================
 */

// Bitmap allocator state
typedef struct bitmap_allocator_state {
    uint32_t *bitmap;           // Bitmap of free/used pages
    uint32_t total_pages;       // Total number of pages
    uint32_t free_pages;        // Number of free pages
    uint32_t start_addr;        // Start of managed memory
    uint32_t page_size;         // Page size (usually 4KB)
} bitmap_allocator_state_t;

// Bitmap allocator implementation
extern physical_allocator_ops_t bitmap_allocator_ops;

/**
 * =============================================================================
 * BUDDY SYSTEM ALLOCATOR (Binary buddy system with coalescing)
 * =============================================================================
 */

#define MAX_BUDDY_ORDER 20  // Support up to 4MB blocks

typedef struct buddy_block {
    struct buddy_block *next;
    struct buddy_block *prev;
    bool is_free;
    uint8_t order;            // log2 of block size in pages
} buddy_block_t;

typedef struct buddy_allocator_state {
    buddy_block_t *free_lists[MAX_BUDDY_ORDER + 1];
    buddy_block_t *block_metadata;
    uintptr_t memory_start;
    size_t total_pages;
    size_t allocated_pages;
    size_t allocation_count;
} buddy_allocator_state_t;

// Buddy allocator implementation
extern physical_allocator_ops_t buddy_allocator_ops;

/**
 * =============================================================================
 * SIMPLE HEAP ALLOCATORS
 * =============================================================================
 */

/**
 * First-Fit Allocator (Simple but educational)
 */
typedef struct heap_block {
    size_t size;               // Size of this block
    bool is_free;              // Whether block is free
    struct heap_block *next;   // Next block in list
    uint32_t magic;            // Corruption detection
} heap_block_t;

typedef struct first_fit_state {
    heap_block_t *head;        // First block in heap
    void *heap_start;          // Start of heap region
    size_t heap_size;          // Total heap size
    size_t free_size;          // Available free space
} first_fit_state_t;

extern heap_allocator_ops_t first_fit_allocator_ops;

/**
 * Slab Allocator (Cache-friendly)
 */
#define MAX_SLAB_CLASSES 16

typedef struct slab_class {
    size_t object_size;        // Size of objects in this slab
    void **free_list;          // List of free objects
    void *slab_start;          // Start of slab memory
    size_t objects_per_slab;   // Number of objects per slab
    size_t free_count;         // Number of free objects
} slab_class_t;

typedef struct slab_allocator_state {
    slab_class_t classes[MAX_SLAB_CLASSES];
    size_t num_classes;
    void *fallback_heap;       // For large allocations
} slab_allocator_state_t;

extern heap_allocator_ops_t slab_allocator_ops;

/**
 * =============================================================================
 * EDUCATIONAL VIRTUAL MEMORY IMPLEMENTATIONS
 * =============================================================================
 */

/**
 * Simple Two-Level Paging
 */
typedef struct simple_paging_state {
    uint32_t *kernel_page_dir;  // Kernel page directory
    uint32_t current_page_dir;  // Current page directory
    uint32_t next_table_addr;   // Next available page table address
} simple_paging_state_t;

extern virtual_memory_ops_t simple_paging_ops;

/**
 * Advanced Paging with Copy-on-Write
 */
typedef struct advanced_paging_state {
    uint32_t *page_directories; // Array of page directories
    uint32_t *ref_counts;       // Reference counts for COW
    uint32_t num_address_spaces;
} advanced_paging_state_t;

extern virtual_memory_ops_t advanced_paging_ops;

/**
 * =============================================================================
 * STUDENT EXERCISE TEMPLATES
 * =============================================================================
 */

/**
 * Template for student implementations
 * Students fill in the function pointers
 */
typedef struct student_allocator_template {
    physical_allocator_ops_t base;
    
    // Student data structures go here
    void *student_data;
    
    // Helper functions for students
    void (*debug_print)(void);
    bool (*validate_state)(void);
    void (*stress_test)(void);
} student_allocator_template_t;

/**
 * =============================================================================
 * BENCHMARK AND TESTING FRAMEWORK
 * =============================================================================
 */

typedef struct allocator_benchmark {
    const char *test_name;
    uint32_t (*run_test)(physical_allocator_ops_t *allocator);
    void (*print_results)(uint32_t result);
} allocator_benchmark_t;

// Standard benchmarks for comparison
extern allocator_benchmark_t allocation_speed_test;
extern allocator_benchmark_t fragmentation_test;
extern allocator_benchmark_t stress_test;

/**
 * =============================================================================
 * INITIALIZATION FUNCTIONS
 * =============================================================================
 */

// Initialize all example allocators
void init_example_allocators(void);

// Benchmark comparison
void compare_allocators(physical_allocator_ops_t **allocators, size_t count);

// Educational helpers
void explain_allocator_differences(void);
void run_allocator_tutorial(void);

#endif // MTOS_MEMORY_ALLOCATORS_H
