/**
 * MTOS Priority-based Process Scheduler Implementation
 * Multi-level priority scheduler with aging to prevent starvation
 */

#include "../../include/types.h"
#include "../interfaces/kernel_interfaces.h"

// Priority scheduler constants
#define MAX_PRIORITY 31
#define MIN_PRIORITY 0
#define DEFAULT_PRIORITY 15
#define AGING_INTERVAL 100  // Ticks before aging occurs
#define AGING_BOOST 1       // Priority boost when aging

// Process structure (simplified for educational purposes)
struct process {
    uint32_t pid;
    uint32_t priority;
    uint32_t original_priority;
    uint32_t age;               // How long since last run
    uint32_t time_slice;
    uint32_t remaining_slice;
    bool is_running;
    bool is_blocked;
    struct process *next;
    struct process *prev;
};

// Priority queue for each priority level
typedef struct priority_queue {
    process_t *head;
    process_t *tail;
    size_t count;
} priority_queue_t;

// Priority scheduler state
static struct {
    priority_queue_t ready_queues[MAX_PRIORITY + 1];
    process_t *current_process;
    process_t *blocked_processes;
    uint32_t total_processes;
    uint32_t context_switches;
    uint32_t total_wait_time;
    uint32_t current_tick;
    bool initialized;
} priority_state;

// Define printf if not available
#ifndef printf
extern int printf(const char *format, ...);
#endif

// Helper functions for queue management
static void init_queue(priority_queue_t *queue) {
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
}

static void enqueue_process(priority_queue_t *queue, process_t *proc) {
    proc->next = NULL;
    proc->prev = queue->tail;
    
    if (queue->tail) {
        queue->tail->next = proc;
    } else {
        queue->head = proc;
    }
    
    queue->tail = proc;
    queue->count++;
}

static process_t* dequeue_process(priority_queue_t *queue) {
    if (!queue->head) {
        return NULL;
    }
    
    process_t *proc = queue->head;
    queue->head = proc->next;
    
    if (queue->head) {
        queue->head->prev = NULL;
    } else {
        queue->tail = NULL;
    }
    
    proc->next = NULL;
    proc->prev = NULL;
    queue->count--;
    
    return proc;
}

static void remove_process_from_queue(priority_queue_t *queue, process_t *proc) {
    if (proc->prev) {
        proc->prev->next = proc->next;
    } else {
        queue->head = proc->next;
    }
    
    if (proc->next) {
        proc->next->prev = proc->prev;
    } else {
        queue->tail = proc->prev;
    }
    
    proc->next = NULL;
    proc->prev = NULL;
    queue->count--;
}

// Find the highest priority non-empty queue
static int find_highest_priority(void) {
    for (int priority = MAX_PRIORITY; priority >= MIN_PRIORITY; priority--) {
        if (priority_state.ready_queues[priority].count > 0) {
            return priority;
        }
    }
    return -1;  // No ready processes
}

// Age processes to prevent starvation
static void age_processes(void) {
    for (int priority = MIN_PRIORITY; priority < MAX_PRIORITY; priority++) {
        priority_queue_t *queue = &priority_state.ready_queues[priority];
        process_t *proc = queue->head;
        
        while (proc) {
            process_t *next = proc->next;
            proc->age++;
            
            // If process has been waiting too long, boost its priority
            if (proc->age >= AGING_INTERVAL && priority < MAX_PRIORITY) {
                remove_process_from_queue(queue, proc);
                proc->priority = priority + AGING_BOOST;
                if (proc->priority > MAX_PRIORITY) {
                    proc->priority = MAX_PRIORITY;
                }
                proc->age = 0;
                enqueue_process(&priority_state.ready_queues[proc->priority], proc);
            }
            
            proc = next;
        }
    }
}

// Calculate dynamic time slice based on priority
static uint32_t calculate_time_slice(uint32_t priority) {
    // Higher priority gets longer time slice
    return 10 + (priority * 2);  // 10-72 ticks based on priority
}

// Implementation functions
static int priority_init(void) {
    if (priority_state.initialized) {
        return 0;  // Already initialized
    }
    
    // Initialize all priority queues
    for (int i = 0; i <= MAX_PRIORITY; i++) {
        init_queue(&priority_state.ready_queues[i]);
    }
    
    priority_state.current_process = NULL;
    priority_state.blocked_processes = NULL;
    priority_state.total_processes = 0;
    priority_state.context_switches = 0;
    priority_state.total_wait_time = 0;
    priority_state.current_tick = 0;
    priority_state.initialized = true;
    
    return 0;
}

static void priority_shutdown(void) {
    // Clean up all queues
    for (int i = 0; i <= MAX_PRIORITY; i++) {
        priority_state.ready_queues[i].head = NULL;
        priority_state.ready_queues[i].tail = NULL;
        priority_state.ready_queues[i].count = 0;
    }
    
    priority_state.current_process = NULL;
    priority_state.blocked_processes = NULL;
    priority_state.total_processes = 0;
    priority_state.initialized = false;
}

static void priority_add_process(process_t *proc) {
    if (!proc) return;
    
    // Set default values if not set
    if (proc->priority > MAX_PRIORITY) {
        proc->priority = DEFAULT_PRIORITY;
    }
    proc->original_priority = proc->priority;
    proc->age = 0;
    proc->time_slice = calculate_time_slice(proc->priority);
    proc->remaining_slice = proc->time_slice;
    proc->is_running = false;
    proc->is_blocked = false;
    
    // Add to appropriate priority queue
    enqueue_process(&priority_state.ready_queues[proc->priority], proc);
    priority_state.total_processes++;
}

static void priority_remove_process(process_t *proc) {
    if (!proc) return;
    
    // Remove from current queue
    if (!proc->is_blocked && !proc->is_running) {
        remove_process_from_queue(&priority_state.ready_queues[proc->priority], proc);
    }
    
    if (priority_state.current_process == proc) {
        priority_state.current_process = NULL;
    }
    
    priority_state.total_processes--;
}

static process_t* priority_get_next(void) {
    int highest_priority = find_highest_priority();
    if (highest_priority < 0) {
        return NULL;  // No ready processes
    }
    
    return dequeue_process(&priority_state.ready_queues[highest_priority]);
}

static void priority_schedule(void) {
    process_t *next_process = priority_get_next();
    
    if (next_process != priority_state.current_process) {
        // Context switch needed
        if (priority_state.current_process) {
            priority_state.current_process->is_running = false;
            // Add back to queue if not blocked
            if (!priority_state.current_process->is_blocked) {
                enqueue_process(&priority_state.ready_queues[priority_state.current_process->priority], 
                               priority_state.current_process);
            }
        }
        
        priority_state.current_process = next_process;
        if (next_process) {
            next_process->is_running = true;
            next_process->remaining_slice = next_process->time_slice;
            next_process->age = 0;  // Reset age when scheduled
        }
        priority_state.context_switches++;
    }
}

static void priority_yield(void) {
    if (priority_state.current_process) {
        priority_state.current_process->remaining_slice = 0;  // Force reschedule
    }
    priority_schedule();
}

static void priority_block(process_t *proc) {
    if (!proc) return;
    
    proc->is_blocked = true;
    
    if (proc == priority_state.current_process) {
        priority_state.current_process = NULL;
        priority_schedule();
    } else {
        // Remove from ready queue
        remove_process_from_queue(&priority_state.ready_queues[proc->priority], proc);
    }
    
    // Add to blocked list
    proc->next = priority_state.blocked_processes;
    priority_state.blocked_processes = proc;
}

static void priority_unblock(process_t *proc) {
    if (!proc || !proc->is_blocked) return;
    
    // Remove from blocked list
    if (priority_state.blocked_processes == proc) {
        priority_state.blocked_processes = proc->next;
    } else {
        process_t *current = priority_state.blocked_processes;
        while (current && current->next != proc) {
            current = current->next;
        }
        if (current) {
            current->next = proc->next;
        }
    }
    
    proc->is_blocked = false;
    proc->next = NULL;
    
    // Restore original priority (remove aging effects)
    proc->priority = proc->original_priority;
    proc->age = 0;
    
    // Add back to ready queue
    enqueue_process(&priority_state.ready_queues[proc->priority], proc);
}

static void priority_timer_tick(void) {
    priority_state.current_tick++;
    
    // Update current process time slice
    if (priority_state.current_process) {
        if (priority_state.current_process->remaining_slice > 0) {
            priority_state.current_process->remaining_slice--;
        }
        
        // Check if time slice expired
        if (priority_state.current_process->remaining_slice == 0) {
            priority_schedule();
        }
    }
    
    // Age processes periodically
    if (priority_state.current_tick % AGING_INTERVAL == 0) {
        age_processes();
    }
    
    // Update wait times for statistics
    for (int priority = MIN_PRIORITY; priority <= MAX_PRIORITY; priority++) {
        priority_state.total_wait_time += priority_state.ready_queues[priority].count;
    }
}

static uint32_t priority_get_time_slice(process_t *proc) {
    if (!proc) return 0;
    return proc->time_slice;
}

static void priority_set_priority(process_t *proc, int new_priority) {
    if (!proc || new_priority < MIN_PRIORITY || new_priority > MAX_PRIORITY) {
        return;
    }
    
    // Remove from current queue if not running or blocked
    if (!proc->is_running && !proc->is_blocked) {
        remove_process_from_queue(&priority_state.ready_queues[proc->priority], proc);
        proc->priority = new_priority;
        proc->original_priority = new_priority;
        proc->time_slice = calculate_time_slice(new_priority);
        enqueue_process(&priority_state.ready_queues[proc->priority], proc);
    } else {
        // Just update the priority for later use
        proc->priority = new_priority;
        proc->original_priority = new_priority;
        proc->time_slice = calculate_time_slice(new_priority);
    }
}

static int priority_get_priority(process_t *proc) {
    return proc ? (int)proc->priority : -1;
}

static void priority_print_stats(void) {
    printf("PRIORITY SCHEDULER STATISTICS:\n");
    printf("  Total processes: %u\n", priority_state.total_processes);
    printf("  Context switches: %u\n", priority_state.context_switches);
    printf("  Current tick: %u\n", priority_state.current_tick);
    
    if (priority_state.current_tick > 0) {
        printf("  Average wait time: %.2f ticks\n", 
               (float)priority_state.total_wait_time / priority_state.current_tick);
    }
    
    printf("\n  Ready processes by priority:\n");
    for (int priority = MAX_PRIORITY; priority >= MIN_PRIORITY; priority--) {
        if (priority_state.ready_queues[priority].count > 0) {
            printf("    Priority %d: %zu processes\n", 
                   priority, priority_state.ready_queues[priority].count);
        }
    }
    
    if (priority_state.current_process) {
        printf("\n  Current process: PID %u, Priority %u, Remaining slice: %u\n",
               priority_state.current_process->pid,
               priority_state.current_process->priority,
               priority_state.current_process->remaining_slice);
    }
}

static uint32_t priority_get_context_switches(void) {
    return priority_state.context_switches;
}

static uint32_t priority_get_avg_wait_time(void) {
    if (priority_state.current_tick == 0) {
        return 0;
    }
    return priority_state.total_wait_time / priority_state.current_tick;
}

// Export the priority scheduler interface
scheduler_ops_t priority_scheduler_ops = {
    .name = "priority",
    .description = "Multi-level priority scheduler with aging and dynamic time slices",
    .init = priority_init,
    .shutdown = priority_shutdown,
    .add_process = priority_add_process,
    .remove_process = priority_remove_process,
    .get_next = priority_get_next,
    .schedule = priority_schedule,
    .yield = priority_yield,
    .block = priority_block,
    .unblock = priority_unblock,
    .timer_tick = priority_timer_tick,
    .get_time_slice = priority_get_time_slice,
    .set_priority = priority_set_priority,
    .get_priority = priority_get_priority,
    .print_stats = priority_print_stats,
    .get_context_switches = priority_get_context_switches,
    .get_avg_wait_time = priority_get_avg_wait_time
};
