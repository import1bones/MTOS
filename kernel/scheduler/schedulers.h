/**
 * MTOS Example Schedulers
 * Educational implementations for students to study and compare
 */

#ifndef MTOS_SCHEDULERS_H
#define MTOS_SCHEDULERS_H

#include "../interfaces/kernel_interfaces.h"

/**
 * =============================================================================
 * ROUND ROBIN SCHEDULER (Simple and Fair)
 * =============================================================================
 */

#define MAX_PROCESSES 64

typedef struct round_robin_state {
    process_t *processes[MAX_PROCESSES];  // Process queue
    size_t process_count;                 // Number of processes
    size_t current_index;                 // Current process index
    uint32_t time_slice;                  // Time slice in milliseconds
    uint32_t total_context_switches;     // Statistics
} round_robin_state_t;

extern scheduler_ops_t round_robin_scheduler_ops;

/**
 * =============================================================================
 * PRIORITY SCHEDULER (Multi-level priority with aging)
 * =============================================================================
 */

#define MAX_PRIORITY 31
#define MIN_PRIORITY 0
#define DEFAULT_PRIORITY 15
#define AGING_INTERVAL 100
#define AGING_BOOST 1

typedef struct priority_queue {
    process_t *head;
    process_t *tail;
    size_t count;
} priority_queue_t;

typedef struct priority_scheduler_state {
    priority_queue_t ready_queues[MAX_PRIORITY + 1];
    process_t *current_process;
    process_t *blocked_processes;
    uint32_t total_processes;
    uint32_t context_switches;
    uint32_t total_wait_time;
    uint32_t current_tick;
    bool initialized;
} priority_scheduler_state_t;

extern scheduler_ops_t priority_scheduler_ops;

/**
 * =============================================================================
 * COMPLETELY FAIR SCHEDULER (CFS-inspired)
 * =============================================================================
 */

typedef struct cfs_process_info {
    process_t *process;
    uint64_t virtual_runtime;            // Virtual runtime in nanoseconds
    int nice_value;                      // Nice value (-20 to +19)
    uint32_t weight;                     // Weight for calculations
    uint64_t last_scheduled;             // Last time scheduled
} cfs_process_info_t;

typedef struct cfs_scheduler_state {
    cfs_process_info_t processes[MAX_PROCESSES];
    size_t process_count;
    cfs_process_info_t *current;
    uint64_t min_virtual_runtime;        // Minimum vruntime in system
    uint32_t time_slice_ns;              // Current time slice
} cfs_scheduler_state_t;

extern scheduler_ops_t cfs_scheduler_ops;

/**
 * =============================================================================
 * REAL-TIME SCHEDULER (Rate Monotonic)
 * =============================================================================
 */

typedef struct rt_task_info {
    process_t *process;
    uint32_t period_ms;                  // Task period
    uint32_t deadline_ms;                // Task deadline
    uint32_t execution_time_ms;          // Worst-case execution time
    uint32_t next_deadline;              // Next absolute deadline
    bool is_periodic;                    // Periodic vs sporadic
} rt_task_info_t;

typedef struct rt_scheduler_state {
    rt_task_info_t tasks[MAX_PROCESSES];
    size_t task_count;
    rt_task_info_t *current_task;
    uint32_t system_time;                // Current system time
    uint32_t missed_deadlines;           // Statistics
} rt_scheduler_state_t;

extern scheduler_ops_t rt_scheduler_ops;

/**
 * =============================================================================
 * MULTILEVEL FEEDBACK QUEUE (Educational classic)
 * =============================================================================
 */

#define MLFQ_LEVELS 4

typedef struct mlfq_level {
    process_t *processes[MAX_PROCESSES];
    size_t count;
    size_t head;
    size_t tail;
    uint32_t time_slice_ms;              // Time slice for this level
    uint32_t aging_threshold;            // Time before promotion
} mlfq_level_t;

typedef struct mlfq_scheduler_state {
    mlfq_level_t levels[MLFQ_LEVELS];
    process_t *current_process;
    uint32_t current_level;
    uint32_t current_time_used;
    uint32_t total_promotions;           // Statistics
    uint32_t total_demotions;
} mlfq_scheduler_state_t;

extern scheduler_ops_t mlfq_scheduler_ops;

/**
 * =============================================================================
 * LOTTERY SCHEDULER (Proportional Share)
 * =============================================================================
 */

typedef struct lottery_ticket {
    process_t *process;
    uint32_t tickets;                    // Number of lottery tickets
    uint32_t total_runtime;              // Total runtime for fairness
} lottery_ticket_t;

typedef struct lottery_scheduler_state {
    lottery_ticket_t processes[MAX_PROCESSES];
    size_t process_count;
    uint32_t total_tickets;              // Total tickets in system
    uint32_t random_seed;                // For ticket selection
    process_t *current_process;
} lottery_scheduler_state_t;

extern scheduler_ops_t lottery_scheduler_ops;

/**
 * =============================================================================
 * STUDENT TEMPLATE SCHEDULER
 * =============================================================================
 */

typedef struct student_scheduler_template {
    scheduler_ops_t base;
    
    // Student data structures
    void *student_data;
    
    // Educational helpers
    void (*explain_algorithm)(void);
    void (*visualize_queues)(void);
    void (*run_simulation)(int num_processes, int time_units);
    
    // Performance metrics
    uint32_t (*get_average_turnaround)(void);
    uint32_t (*get_average_waiting_time)(void);
    uint32_t (*get_average_response_time)(void);
    
    // Debugging
    void (*print_debug_info)(void);
    bool (*validate_invariants)(void);
} student_scheduler_template_t;

/**
 * =============================================================================
 * SCHEDULER BENCHMARK AND COMPARISON
 * =============================================================================
 */

typedef struct scheduler_metrics {
    uint32_t avg_turnaround_time;
    uint32_t avg_waiting_time;
    uint32_t avg_response_time;
    uint32_t context_switches;
    uint32_t cpu_utilization;            // Percentage
    uint32_t fairness_index;             // Jain's fairness index
} scheduler_metrics_t;

typedef struct scheduler_workload {
    const char *name;
    process_t *processes;
    size_t process_count;
    uint32_t simulation_time;
} scheduler_workload_t;

// Standard workloads for testing
extern scheduler_workload_t cpu_bound_workload;
extern scheduler_workload_t io_bound_workload;
extern scheduler_workload_t mixed_workload;
extern scheduler_workload_t realtime_workload;

/**
 * =============================================================================
 * EDUCATIONAL FUNCTIONS
 * =============================================================================
 */

// Initialize all example schedulers
void init_example_schedulers(void);

// Compare schedulers with different workloads
scheduler_metrics_t benchmark_scheduler(scheduler_ops_t *scheduler, 
                                       scheduler_workload_t *workload);

// Educational visualization
void visualize_scheduling_timeline(scheduler_ops_t *scheduler, 
                                 scheduler_workload_t *workload, 
                                 uint32_t time_units);

// Interactive scheduler comparison
void interactive_scheduler_comparison(void);

// Generate scheduler performance report
void generate_scheduler_report(scheduler_ops_t **schedulers, 
                             size_t scheduler_count,
                             scheduler_workload_t **workloads,
                             size_t workload_count);

// Educational explanations
void explain_scheduling_algorithms(void);
void demonstrate_priority_inversion(void);
void demonstrate_convoy_effect(void);
void demonstrate_starvation(void);

#endif // MTOS_SCHEDULERS_H
