/**
 * MTOS Bitmap Memory Allocator Implementation
 * Simple bitmap-based physical page allocator for educational purposes
 */

#include "../interfaces/kernel_interfaces.h"

// Bitmap allocator state
static struct {
    uint32_t *bitmap;           // Bitmap of free/used pages
    uint32_t total_pages;       // Total number of pages
    uint32_t free_pages;        // Number of free pages
    uint32_t start_addr;        // Start of managed memory
    uint32_t page_size;         // Page size (4KB)
    uint32_t last_allocated;    // Last allocated page (for next-fit optimization)
} bitmap_state;

#define PAGE_SIZE 4096
#define PAGES_PER_WORD 32

// Helper functions
static inline void set_page_used(uint32_t page_num) {
    uint32_t word_idx = page_num / PAGES_PER_WORD;
    uint32_t bit_idx = page_num % PAGES_PER_WORD;
    bitmap_state.bitmap[word_idx] |= (1U << bit_idx);
}

static inline void set_page_free(uint32_t page_num) {
    uint32_t word_idx = page_num / PAGES_PER_WORD;
    uint32_t bit_idx = page_num % PAGES_PER_WORD;
    bitmap_state.bitmap[word_idx] &= ~(1U << bit_idx);
}

static inline bool is_page_free(uint32_t page_num) {
    uint32_t word_idx = page_num / PAGES_PER_WORD;
    uint32_t bit_idx = page_num % PAGES_PER_WORD;
    return !(bitmap_state.bitmap[word_idx] & (1U << bit_idx));
}

// Find first free page starting from given page
static uint32_t find_free_page(uint32_t start_page) {
    for (uint32_t page = start_page; page < bitmap_state.total_pages; page++) {
        if (is_page_free(page)) {
            return page;
        }
    }
    
    // Wrap around to beginning
    for (uint32_t page = 0; page < start_page; page++) {
        if (is_page_free(page)) {
            return page;
        }
    }
    
    return 0xFFFFFFFF; // No free pages
}

// Implementation functions
static int bitmap_init(uint32_t start_addr, uint32_t end_addr) {
    bitmap_state.start_addr = start_addr;
    bitmap_state.page_size = PAGE_SIZE;
    bitmap_state.total_pages = (end_addr - start_addr) / PAGE_SIZE;
    bitmap_state.free_pages = bitmap_state.total_pages;
    bitmap_state.last_allocated = 0;
    
    // Calculate bitmap size (one bit per page)
    uint32_t bitmap_size = (bitmap_state.total_pages + PAGES_PER_WORD - 1) / PAGES_PER_WORD;
    
    // Place bitmap at the beginning of managed memory
    bitmap_state.bitmap = (uint32_t*)start_addr;
    
    // Initialize bitmap to all free (zeros)
    for (uint32_t i = 0; i < bitmap_size; i++) {
        bitmap_state.bitmap[i] = 0;
    }
    
    // Mark bitmap pages as used
    uint32_t bitmap_pages = (bitmap_size * sizeof(uint32_t) + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t i = 0; i < bitmap_pages; i++) {
        set_page_used(i);
        bitmap_state.free_pages--;
    }
    
    return 0;
}

static uint32_t bitmap_alloc_page(void) {
    if (bitmap_state.free_pages == 0) {
        return 0; // Out of memory
    }
    
    uint32_t page_num = find_free_page(bitmap_state.last_allocated);
    if (page_num == 0xFFFFFFFF) {
        return 0; // No free pages found
    }
    
    set_page_used(page_num);
    bitmap_state.free_pages--;
    bitmap_state.last_allocated = page_num;
    
    return bitmap_state.start_addr + (page_num * PAGE_SIZE);
}

static uint32_t bitmap_alloc_pages(size_t count) {
    if (count == 0 || bitmap_state.free_pages < count) {
        return 0;
    }
    
    // Find contiguous free pages
    for (uint32_t start_page = 0; start_page <= bitmap_state.total_pages - count; start_page++) {
        bool found = true;
        
        // Check if 'count' consecutive pages are free
        for (size_t i = 0; i < count; i++) {
            if (!is_page_free(start_page + i)) {
                found = false;
                break;
            }
        }
        
        if (found) {
            // Allocate all pages
            for (size_t i = 0; i < count; i++) {
                set_page_used(start_page + i);
                bitmap_state.free_pages--;
            }
            return bitmap_state.start_addr + (start_page * PAGE_SIZE);
        }
    }
    
    return 0; // No contiguous block found
}

static void bitmap_free_page(uint32_t paddr) {
    if (paddr < bitmap_state.start_addr) {
        return; // Invalid address
    }
    
    uint32_t page_num = (paddr - bitmap_state.start_addr) / PAGE_SIZE;
    if (page_num >= bitmap_state.total_pages) {
        return; // Invalid address
    }
    
    if (is_page_free(page_num)) {
        return; // Already free
    }
    
    set_page_free(page_num);
    bitmap_state.free_pages++;
}

static void bitmap_free_pages(uint32_t paddr, size_t count) {
    for (size_t i = 0; i < count; i++) {
        bitmap_free_page(paddr + (i * PAGE_SIZE));
    }
}

static size_t bitmap_get_free_pages(void) {
    return bitmap_state.free_pages;
}

static size_t bitmap_get_total_pages(void) {
    return bitmap_state.total_pages;
}

static void bitmap_print_stats(void) {
    printf("BITMAP ALLOCATOR STATISTICS:\n");
    printf("  Total pages: %u\n", bitmap_state.total_pages);
    printf("  Free pages: %u\n", bitmap_state.free_pages);
    printf("  Used pages: %u\n", bitmap_state.total_pages - bitmap_state.free_pages);
    printf("  Utilization: %.1f%%\n", 
           100.0 * (bitmap_state.total_pages - bitmap_state.free_pages) / bitmap_state.total_pages);
    printf("  Last allocated page: %u\n", bitmap_state.last_allocated);
}

static uint32_t bitmap_alloc_aligned(size_t size, size_t alignment) {
    size_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    size_t align_pages = (alignment + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint32_t start_page = 0; start_page <= bitmap_state.total_pages - pages_needed; start_page += align_pages) {
        bool found = true;
        
        // Check if aligned block is free
        for (size_t i = 0; i < pages_needed; i++) {
            if (!is_page_free(start_page + i)) {
                found = false;
                break;
            }
        }
        
        if (found) {
            // Allocate aligned block
            for (size_t i = 0; i < pages_needed; i++) {
                set_page_used(start_page + i);
                bitmap_state.free_pages--;
            }
            return bitmap_state.start_addr + (start_page * PAGE_SIZE);
        }
    }
    
    return 0;
}

static bool bitmap_is_available(uint32_t paddr) {
    if (paddr < bitmap_state.start_addr) {
        return false;
    }
    
    uint32_t page_num = (paddr - bitmap_state.start_addr) / PAGE_SIZE;
    if (page_num >= bitmap_state.total_pages) {
        return false;
    }
    
    return is_page_free(page_num);
}

// Export the bitmap allocator interface
physical_allocator_ops_t bitmap_allocator_ops = {
    .name = "bitmap",
    .description = "Simple bitmap-based page allocator with linear search",
    .init = bitmap_init,
    .alloc_page = bitmap_alloc_page,
    .alloc_pages = bitmap_alloc_pages,
    .free_page = bitmap_free_page,
    .free_pages = bitmap_free_pages,
    .get_free_pages = bitmap_get_free_pages,
    .get_total_pages = bitmap_get_total_pages,
    .print_stats = bitmap_print_stats,
    .alloc_aligned = bitmap_alloc_aligned,
    .is_available = bitmap_is_available
};
