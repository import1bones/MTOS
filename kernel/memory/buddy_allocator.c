/**
 * MTOS Buddy System Memory Allocator Implementation
 * Binary buddy system for efficient memory allocation with minimal fragmentation
 */

#include "../../include/types.h"

// Include full interface after types
#include "../interfaces/kernel_interfaces.h"

// Define printf if not available
#ifndef printf
extern int printf(const char *format, ...);
#endif

// Forward declarations
static uint32_t buddy_alloc_pages(size_t count);
static void buddy_free_pages(uint32_t paddr, size_t count);

// Buddy allocator constants
#define MAX_ORDER 20  // Support up to 4MB blocks (2^20 * 4KB = 4GB)
#define MIN_ORDER 0   // Minimum block size is one page (4KB)
#define PAGE_SIZE 4096

// Buddy block structure
typedef struct buddy_block {
    struct buddy_block *next;
    struct buddy_block *prev;
    bool is_free;
    uint8_t order;  // log2 of block size in pages
} buddy_block_t;

// Buddy allocator state
static struct {
    uintptr_t memory_start;      // Start address as integer for arithmetic
    size_t total_size;
    size_t total_pages;
    buddy_block_t *free_lists[MAX_ORDER + 1];  // Free lists for each order
    buddy_block_t *block_metadata;             // Metadata for all blocks
    size_t allocated_pages;
    size_t allocation_count;
} buddy_state;

// Helper macros
#define PAGES_TO_SIZE(pages) ((pages) * PAGE_SIZE)
#define SIZE_TO_PAGES(size) (((size) + PAGE_SIZE - 1) / PAGE_SIZE)
#define BLOCK_SIZE(order) (1U << (order))
#define GET_BUDDY_INDEX(index, order) ((index) ^ (1U << (order)))

// Convert page index to block metadata pointer
static buddy_block_t* get_block(size_t page_index) {
    return &buddy_state.block_metadata[page_index];
}

// Convert block to page index
static size_t block_to_page_index(buddy_block_t *block) {
    return block - buddy_state.block_metadata;
}

// Initialize free list for an order
static void init_free_list(uint8_t order) {
    buddy_state.free_lists[order] = NULL;
}

// Add block to free list
static void add_to_free_list(buddy_block_t *block, uint8_t order) {
    block->order = order;
    block->is_free = true;
    block->next = buddy_state.free_lists[order];
    block->prev = NULL;
    
    if (buddy_state.free_lists[order]) {
        buddy_state.free_lists[order]->prev = block;
    }
    buddy_state.free_lists[order] = block;
}

// Remove block from free list
static void remove_from_free_list(buddy_block_t *block, uint8_t order) {
    if (block->prev) {
        block->prev->next = block->next;
    } else {
        buddy_state.free_lists[order] = block->next;
    }
    
    if (block->next) {
        block->next->prev = block->prev;
    }
    
    block->is_free = false;
    block->next = NULL;
    block->prev = NULL;
}

// Find the buddy of a block
static buddy_block_t* get_buddy(buddy_block_t *block, uint8_t order) {
    size_t page_index = block_to_page_index(block);
    size_t buddy_index = GET_BUDDY_INDEX(page_index, order);
    
    if (buddy_index >= buddy_state.total_pages) {
        return NULL;
    }
    
    return get_block(buddy_index);
}

// Split a block into two smaller blocks
static buddy_block_t* split_block(buddy_block_t *block, uint8_t target_order) {
    uint8_t current_order = block->order;
    
    while (current_order > target_order) {
        current_order--;
        
        // Get the buddy (right half)
        buddy_block_t *buddy = get_buddy(block, current_order);
        if (buddy) {
            add_to_free_list(buddy, current_order);
        }
        
        // Update the original block's order
        block->order = current_order;
    }
    
    return block;
}

// Merge a block with its buddy if possible
static buddy_block_t* merge_block(buddy_block_t *block) {
    uint8_t order = block->order;
    
    while (order < MAX_ORDER) {
        buddy_block_t *buddy = get_buddy(block, order);
        
        // Check if buddy exists, is free, and has the same order
        if (!buddy || !buddy->is_free || buddy->order != order) {
            break;
        }
        
        // Remove buddy from free list
        remove_from_free_list(buddy, order);
        
        // Determine which block becomes the merged block (lower address)
        if (buddy < block) {
            block = buddy;
        }
        
        order++;
        block->order = order;
    }
    
    return block;
}

// Find the smallest order that can fit the requested pages
static uint8_t get_order_for_pages(size_t pages) {
    uint8_t order = 0;
    size_t block_pages = 1;
    
    while (block_pages < pages && order <= MAX_ORDER) {
        order++;
        block_pages <<= 1;
    }
    
    return (order <= MAX_ORDER) ? order : 0xFF;  // 0xFF indicates failure
}

// Implementation functions
static int buddy_init(uint32_t start_addr, uint32_t end_addr) {
    buddy_state.memory_start = (uintptr_t)start_addr;
    buddy_state.total_size = end_addr - start_addr;
    buddy_state.total_pages = buddy_state.total_size / PAGE_SIZE;
    buddy_state.allocated_pages = 0;
    buddy_state.allocation_count = 0;
    
    // Initialize free lists
    for (int i = 0; i <= MAX_ORDER; i++) {
        init_free_list(i);
    }
    
    // Allocate metadata for all possible blocks
    size_t metadata_size = buddy_state.total_pages * sizeof(buddy_block_t);
    buddy_state.block_metadata = (buddy_block_t*)start_addr;
    
    // Initialize all block metadata
    for (size_t i = 0; i < buddy_state.total_pages; i++) {
        buddy_block_t *block = get_block(i);
        block->next = NULL;
        block->prev = NULL;
        block->is_free = false;
        block->order = 0;
    }
    
    // Calculate usable memory after metadata
    size_t metadata_pages = SIZE_TO_PAGES(metadata_size);
    size_t usable_pages = buddy_state.total_pages - metadata_pages;
    
    // Create initial free blocks
    size_t current_page = metadata_pages;
    while (current_page < buddy_state.total_pages) {
        // Find the largest block that fits
        uint8_t max_order = 0;
        size_t remaining_pages = buddy_state.total_pages - current_page;
        
        while ((1U << (max_order + 1)) <= remaining_pages && max_order < MAX_ORDER) {
            max_order++;
        }
        
        // Create and add the block
        buddy_block_t *block = get_block(current_page);
        add_to_free_list(block, max_order);
        
        current_page += BLOCK_SIZE(max_order);
    }
    
    return 0;
}

static uint32_t buddy_alloc_page(void) {
    return buddy_alloc_pages(1);
}

static uint32_t buddy_alloc_pages(size_t count) {
    if (count == 0) {
        return 0;
    }
    
    uint8_t order = get_order_for_pages(count);
    if (order == 0xFF) {
        return 0;  // Requested size too large
    }
    
    // Find a free block of sufficient size
    buddy_block_t *block = NULL;
    for (uint8_t search_order = order; search_order <= MAX_ORDER; search_order++) {
        if (buddy_state.free_lists[search_order]) {
            block = buddy_state.free_lists[search_order];
            remove_from_free_list(block, search_order);
            break;
        }
    }
    
    if (!block) {
        return 0;  // No suitable block found
    }
    
    // Split the block if necessary
    if (block->order > order) {
        block = split_block(block, order);
    }
    
    // Mark as allocated
    block->is_free = false;
    buddy_state.allocated_pages += BLOCK_SIZE(order);
    buddy_state.allocation_count++;
    
    // Calculate physical address
    size_t page_index = block_to_page_index(block);
    return (uint32_t)(buddy_state.memory_start + (page_index * PAGE_SIZE));
}

static void buddy_free_page(uint32_t paddr) {
    buddy_free_pages(paddr, 1);
}

static void buddy_free_pages(uint32_t paddr, size_t count) {
    if (paddr < buddy_state.memory_start) {
        return;  // Invalid address
    }
    
    size_t page_index = (paddr - buddy_state.memory_start) / PAGE_SIZE;
    if (page_index >= buddy_state.total_pages) {
        return;  // Invalid address
    }
    
    buddy_block_t *block = get_block(page_index);
    if (block->is_free) {
        return;  // Already free
    }
    
    // Update statistics
    buddy_state.allocated_pages -= BLOCK_SIZE(block->order);
    buddy_state.allocation_count--;
    
    // Merge with buddies and add to appropriate free list
    block = merge_block(block);
    add_to_free_list(block, block->order);
}

static size_t buddy_get_free_pages(void) {
    return buddy_state.total_pages - buddy_state.allocated_pages;
}

static size_t buddy_get_total_pages(void) {
    return buddy_state.total_pages;
}

static void buddy_print_stats(void) {
    printf("BUDDY ALLOCATOR STATISTICS:\n");
    printf("  Total pages: %zu\n", buddy_state.total_pages);
    printf("  Allocated pages: %zu\n", buddy_state.allocated_pages);
    printf("  Free pages: %zu\n", buddy_get_free_pages());
    printf("  Utilization: %.1f%%\n", 
           100.0 * buddy_state.allocated_pages / buddy_state.total_pages);
    printf("  Total allocations: %zu\n", buddy_state.allocation_count);
    
    printf("\n  Free blocks by order:\n");
    for (int order = 0; order <= MAX_ORDER; order++) {
        int count = 0;
        buddy_block_t *block = buddy_state.free_lists[order];
        while (block) {
            count++;
            block = block->next;
        }
        if (count > 0) {
            printf("    Order %d (%u pages): %d blocks\n", 
                   order, (unsigned int)BLOCK_SIZE(order), count);
        }
    }
}

static uint32_t buddy_alloc_aligned(size_t size, size_t alignment) {
    // For buddy allocator, all allocations are naturally aligned to their size
    // This is a simplified implementation - could be enhanced for arbitrary alignments
    size_t pages_needed = SIZE_TO_PAGES(size);
    uint8_t order = get_order_for_pages(pages_needed);
    
    if (order == 0xFF) {
        return 0;
    }
    
    // Buddy allocations are naturally aligned to their block size
    return buddy_alloc_pages(BLOCK_SIZE(order));
}

static bool buddy_is_available(uint32_t paddr) {
    if (paddr < buddy_state.memory_start) {
        return false;
    }
    
    size_t page_index = (paddr - buddy_state.memory_start) / PAGE_SIZE;
    if (page_index >= buddy_state.total_pages) {
        return false;
    }
    
    buddy_block_t *block = get_block(page_index);
    return block->is_free;
}

// Export the buddy allocator interface
physical_allocator_ops_t buddy_allocator_ops = {
    .name = "buddy",
    .description = "Binary buddy system allocator with efficient merging and splitting",
    .init = buddy_init,
    .alloc_page = buddy_alloc_page,
    .alloc_pages = buddy_alloc_pages,
    .free_page = buddy_free_page,
    .free_pages = buddy_free_pages,
    .get_free_pages = buddy_get_free_pages,
    .get_total_pages = buddy_get_total_pages,
    .print_stats = buddy_print_stats,
    .alloc_aligned = buddy_alloc_aligned,
    .is_available = buddy_is_available
};
