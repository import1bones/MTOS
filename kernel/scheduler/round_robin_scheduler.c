/**
 * MTOS Round-Robin Scheduler Implementation
 * Classic time-sliced round-robin scheduler with configurable quantum
 */

#include "../../include/types.h"
#include "../interfaces/kernel_interfaces.h"

// Round-robin scheduler constants
#define DEFAULT_TIME_QUANTUM 20  // Default time slice in ticks
#define MIN_TIME_QUANTUM 1
#define MAX_TIME_QUANTUM 1000

// Process queue structure
typedef struct rr_process_node {
    process_t *process;
    struct rr_process_node *next;
    struct rr_process_node *prev;
    uint32_t wait_time;  // Time spent waiting
} rr_process_node_t;

// Round-robin scheduler state
static struct {
    rr_process_node_t *ready_queue_head;
    rr_process_node_t *ready_queue_tail;
    rr_process_node_t *blocked_processes;
    process_t *current_process;
    uint32_t time_quantum;
    uint32_t remaining_quantum;
    uint32_t process_count;
    uint32_t context_switches;
    uint32_t total_wait_time;
    uint32_t current_tick;
    bool initialized;
} rr_state;

// Define printf if not available
#ifndef printf
extern int printf(const char *format, ...);
#endif

// Helper functions for process node management
static rr_process_node_t* create_process_node(process_t *proc) {
    // Simple static allocation for educational purposes
    static rr_process_node_t node_pool[64];
    static size_t node_index = 0;
    
    if (node_index >= 64) {
        return NULL;  // Pool exhausted
    }
    
    rr_process_node_t *node = &node_pool[node_index++];
    node->process = proc;
    node->next = NULL;
    node->prev = NULL;
    node->wait_time = 0;
    
    return node;
}

static void add_to_ready_queue(rr_process_node_t *node) {
    if (!node) return;
    
    node->next = NULL;
    node->prev = rr_state.ready_queue_tail;
    
    if (rr_state.ready_queue_tail) {
        rr_state.ready_queue_tail->next = node;
    } else {
        rr_state.ready_queue_head = node;
    }
    
    rr_state.ready_queue_tail = node;
}

static rr_process_node_t* remove_from_ready_queue(void) {
    if (!rr_state.ready_queue_head) {
        return NULL;
    }
    
    rr_process_node_t *node = rr_state.ready_queue_head;
    rr_state.ready_queue_head = node->next;
    
    if (rr_state.ready_queue_head) {
        rr_state.ready_queue_head->prev = NULL;
    } else {
        rr_state.ready_queue_tail = NULL;
    }
    
    node->next = NULL;
    node->prev = NULL;
    
    return node;
}

static void remove_node_from_queue(rr_process_node_t *node) {
    if (!node) return;
    
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        rr_state.ready_queue_head = node->next;
    }
    
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        rr_state.ready_queue_tail = node->prev;
    }
    
    node->next = NULL;
    node->prev = NULL;
}

static rr_process_node_t* find_process_node(process_t *proc) {
    // Search in ready queue
    rr_process_node_t *current = rr_state.ready_queue_head;
    while (current) {
        if (current->process == proc) {
            return current;
        }
        current = current->next;
    }
    
    // Search in blocked processes
    current = rr_state.blocked_processes;
    while (current) {
        if (current->process == proc) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// Implementation functions
static int rr_init(void) {
    if (rr_state.initialized) {
        return 0;
    }
    
    rr_state.ready_queue_head = NULL;
    rr_state.ready_queue_tail = NULL;
    rr_state.blocked_processes = NULL;
    rr_state.current_process = NULL;
    rr_state.time_quantum = DEFAULT_TIME_QUANTUM;
    rr_state.remaining_quantum = 0;
    rr_state.process_count = 0;
    rr_state.context_switches = 0;
    rr_state.total_wait_time = 0;
    rr_state.current_tick = 0;
    rr_state.initialized = true;
    
    return 0;
}

static void rr_shutdown(void) {
    rr_state.ready_queue_head = NULL;
    rr_state.ready_queue_tail = NULL;
    rr_state.blocked_processes = NULL;
    rr_state.current_process = NULL;
    rr_state.process_count = 0;
    rr_state.initialized = false;
}

static void rr_add_process(process_t *proc) {
    if (!proc) return;
    
    rr_process_node_t *node = create_process_node(proc);
    if (!node) return;
    
    add_to_ready_queue(node);
    rr_state.process_count++;
}

static void rr_remove_process(process_t *proc) {
    if (!proc) return;
    
    rr_process_node_t *node = find_process_node(proc);
    if (!node) return;
    
    // Remove from appropriate queue
    if (node->process == rr_state.current_process) {
        rr_state.current_process = NULL;
        rr_state.remaining_quantum = 0;
    } else {
        remove_node_from_queue(node);
    }
    
    rr_state.process_count--;
}

static process_t* rr_get_next(void) {
    rr_process_node_t *node = remove_from_ready_queue();
    return node ? node->process : NULL;
}

static void rr_schedule(void) {
    process_t *next_process = NULL;
    
    // If current process exhausted quantum or no current process, get next
    if (!rr_state.current_process || rr_state.remaining_quantum == 0) {
        // If current process still has time but quantum expired, put it back
        if (rr_state.current_process && rr_state.remaining_quantum == 0) {
            rr_process_node_t *node = create_process_node(rr_state.current_process);
            if (node) {
                add_to_ready_queue(node);
            }
        }
        
        next_process = rr_get_next();
        
        if (next_process != rr_state.current_process) {
            rr_state.current_process = next_process;
            rr_state.remaining_quantum = rr_state.time_quantum;
            rr_state.context_switches++;
        }
    }
}

static void rr_yield(void) {
    if (rr_state.current_process) {
        // Force the current process to give up its remaining time slice
        rr_process_node_t *node = create_process_node(rr_state.current_process);
        if (node) {
            add_to_ready_queue(node);
        }
        
        rr_state.current_process = NULL;
        rr_state.remaining_quantum = 0;
    }
    
    rr_schedule();
}

static void rr_block(process_t *proc) {
    if (!proc) return;
    
    if (proc == rr_state.current_process) {
        rr_state.current_process = NULL;
        rr_state.remaining_quantum = 0;
        rr_schedule();
    } else {
        // Remove from ready queue
        rr_process_node_t *node = find_process_node(proc);
        if (node) {
            remove_node_from_queue(node);
            
            // Add to blocked list
            node->next = rr_state.blocked_processes;
            if (rr_state.blocked_processes) {
                rr_state.blocked_processes->prev = node;
            }
            rr_state.blocked_processes = node;
        }
    }
}

static void rr_unblock(process_t *proc) {
    if (!proc) return;
    
    // Find in blocked list
    rr_process_node_t *node = rr_state.blocked_processes;
    while (node && node->process != proc) {
        node = node->next;
    }
    
    if (!node) return;
    
    // Remove from blocked list
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        rr_state.blocked_processes = node->next;
    }
    
    if (node->next) {
        node->next->prev = node->prev;
    }
    
    // Reset node linkage and add to ready queue
    node->next = NULL;
    node->prev = NULL;
    node->wait_time = 0;  // Reset wait time
    add_to_ready_queue(node);
}

static void rr_timer_tick(void) {
    rr_state.current_tick++;
    
    // Decrement remaining quantum for current process
    if (rr_state.current_process && rr_state.remaining_quantum > 0) {
        rr_state.remaining_quantum--;
    }
    
    // Update wait times for processes in ready queue
    rr_process_node_t *current = rr_state.ready_queue_head;
    while (current) {
        current->wait_time++;
        rr_state.total_wait_time++;
        current = current->next;
    }
    
    // Check if we need to schedule
    if (rr_state.remaining_quantum == 0) {
        rr_schedule();
    }
}

static uint32_t rr_get_time_slice(process_t *proc) {
    // All processes get the same time slice in round-robin
    return rr_state.time_quantum;
}

static void rr_set_priority(process_t *proc, int priority) {
    // Round-robin doesn't use priorities, but we can store it for compatibility
    // In a real implementation, we might adjust time quantum based on priority
}

static int rr_get_priority(process_t *proc) {
    // Round-robin treats all processes equally
    return 0;  // Default priority
}

static void rr_print_stats(void) {
    printf("ROUND-ROBIN SCHEDULER STATISTICS:\n");
    printf("  Total processes: %u\n", rr_state.process_count);
    printf("  Context switches: %u\n", rr_state.context_switches);
    printf("  Time quantum: %u ticks\n", rr_state.time_quantum);
    printf("  Current tick: %u\n", rr_state.current_tick);
    
    if (rr_state.current_tick > 0) {
        printf("  Average wait time: %.2f ticks\n", 
               (float)rr_state.total_wait_time / rr_state.current_tick);
    }
    
    // Count processes in ready queue
    uint32_t ready_count = 0;
    rr_process_node_t *current = rr_state.ready_queue_head;
    while (current) {
        ready_count++;
        current = current->next;
    }
    
    // Count blocked processes
    uint32_t blocked_count = 0;
    current = rr_state.blocked_processes;
    while (current) {
        blocked_count++;
        current = current->next;
    }
    
    printf("  Ready processes: %u\n", ready_count);
    printf("  Blocked processes: %u\n", blocked_count);
    
    if (rr_state.current_process) {
        printf("  Current process: Running, Remaining quantum: %u\n",
               rr_state.remaining_quantum);
    } else {
        printf("  Current process: None\n");
    }
    
    // Calculate fairness metric
    if (ready_count > 0) {
        printf("  Scheduler efficiency: %.1f%% (ideal: %.1f%%)\n",
               100.0 / (ready_count + 1),
               100.0 / rr_state.process_count);
    }
}

static uint32_t rr_get_context_switches(void) {
    return rr_state.context_switches;
}

static uint32_t rr_get_avg_wait_time(void) {
    if (rr_state.current_tick == 0) {
        return 0;
    }
    return rr_state.total_wait_time / rr_state.current_tick;
}

// Additional round-robin specific functions
void rr_set_time_quantum(uint32_t quantum) {
    if (quantum >= MIN_TIME_QUANTUM && quantum <= MAX_TIME_QUANTUM) {
        rr_state.time_quantum = quantum;
        // If currently running process, adjust remaining quantum proportionally
        if (rr_state.current_process && rr_state.remaining_quantum > 0) {
            rr_state.remaining_quantum = quantum;
        }
    }
}

uint32_t rr_get_time_quantum(void) {
    return rr_state.time_quantum;
}

// Export the round-robin scheduler interface
scheduler_ops_t round_robin_scheduler_ops = {
    .name = "round_robin",
    .description = "Classic time-sliced round-robin scheduler with configurable quantum",
    .init = rr_init,
    .shutdown = rr_shutdown,
    .add_process = rr_add_process,
    .remove_process = rr_remove_process,
    .get_next = rr_get_next,
    .schedule = rr_schedule,
    .yield = rr_yield,
    .block = rr_block,
    .unblock = rr_unblock,
    .timer_tick = rr_timer_tick,
    .get_time_slice = rr_get_time_slice,
    .set_priority = rr_set_priority,
    .get_priority = rr_get_priority,
    .print_stats = rr_print_stats,
    .get_context_switches = rr_get_context_switches,
    .get_avg_wait_time = rr_get_avg_wait_time
};
