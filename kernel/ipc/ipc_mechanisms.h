/**
 * MTOS Example IPC Mechanisms
 * Educational implementations for students to study different communication models
 */

#ifndef MTOS_IPC_MECHANISMS_H
#define MTOS_IPC_MECHANISMS_H

#include "../interfaces/kernel_interfaces.h"

// Common constants
#define MAX_PROCESSES 64

/**
 * =============================================================================
 * MESSAGE QUEUE IPC (Classic message passing)
 * =============================================================================
 */

#define MAX_MESSAGE_SIZE 4096
#define MAX_QUEUE_DEPTH 64

typedef struct message_queue_entry {
    ipc_message_t message;
    struct message_queue_entry *next;
    uint32_t timestamp;
} message_queue_entry_t;

typedef struct message_queue {
    message_queue_entry_t *head;
    message_queue_entry_t *tail;
    size_t count;
    size_t max_size;
    uint32_t sender_id;
    uint32_t receiver_id;
    bool is_blocking;                    // Blocking vs non-blocking
} message_queue_t;

typedef struct message_queue_ipc_state {
    message_queue_t queues[MAX_PROCESSES];
    size_t queue_count;
    uint32_t total_messages_sent;
    uint32_t total_messages_dropped;
} message_queue_ipc_state_t;

extern ipc_transport_ops_t message_queue_ipc_ops;

/**
 * =============================================================================
 * SHARED MEMORY IPC (High-performance communication)
 * =============================================================================
 */

#define MAX_SHARED_REGIONS 64
#define MAX_PROCESSES_PER_REGION 8
#define SHARED_REGION_SIZE 4096

typedef struct shared_region {
    uint32_t region_id;
    uint32_t creator_id;
    uint32_t participants[MAX_PROCESSES_PER_REGION];
    size_t participant_count;
    void *memory;
    size_t size;
    uint32_t permissions;
    bool in_use;
    
    // Synchronization
    volatile bool lock;
    volatile uint32_t read_index;
    volatile uint32_t write_index;
    volatile bool has_data;
} shared_region_t;

typedef struct shared_memory_ipc_state {
    shared_region_t regions[MAX_SHARED_REGIONS];
    uint32_t next_region_id;
    size_t active_regions;
    uint32_t total_messages_sent;
    uint32_t total_messages_received;
    bool initialized;
} shared_memory_ipc_state_t;

extern ipc_transport_ops_t shared_memory_ipc_ops;

/**
 * =============================================================================
 * PIPE IPC (Unix-style pipes)
 * =============================================================================
 */

#define PIPE_BUFFER_SIZE 4096

typedef struct pipe {
    uint8_t buffer[PIPE_BUFFER_SIZE];
    size_t read_pos;
    size_t write_pos;
    size_t data_size;
    uint32_t reader_id;
    uint32_t writer_id;
    bool is_named;                       // Named vs anonymous pipe
    char name[64];                       // Pipe name (if named)
} pipe_t;

typedef struct pipe_ipc_state {
    pipe_t pipes[MAX_PROCESSES];
    size_t pipe_count;
    uint32_t total_bytes_transferred;
    uint32_t total_pipe_operations;
} pipe_ipc_state_t;

extern ipc_transport_ops_t pipe_ipc_ops;

/**
 * =============================================================================
 * CAPABILITY-BASED IPC (Security-focused)
 * =============================================================================
 */

typedef struct capability {
    uint32_t capability_id;
    uint32_t object_id;                  // Object this capability refers to
    uint32_t permissions;                // READ, WRITE, EXECUTE, etc.
    uint32_t owner_process;              // Process that owns this capability
    bool is_transferable;                // Can be transferred to other processes
    uint32_t expiration_time;            // Time when capability expires
} capability_t;

typedef struct capability_channel {
    uint32_t channel_id;
    capability_t required_capability;    // Capability needed to use channel
    message_queue_t message_queue;       // Underlying message queue
    uint32_t access_count;               // Number of times accessed
} capability_channel_t;

typedef struct capability_ipc_state {
    capability_t capabilities[MAX_PROCESSES * 4];  // Multiple caps per process
    size_t capability_count;
    capability_channel_t channels[MAX_PROCESSES];
    size_t channel_count;
    uint32_t next_capability_id;
    uint32_t capability_violations;      // Security violations
} capability_ipc_state_t;

extern ipc_transport_ops_t capability_ipc_ops;

/**
 * =============================================================================
 * ACTOR MODEL IPC (Modern message passing)
 * =============================================================================
 */

typedef struct actor_mailbox {
    uint32_t actor_id;
    message_queue_t inbox;
    uint32_t processed_messages;
    uint32_t max_queue_size;
    bool is_system_actor;                // System vs user actor
} actor_mailbox_t;

typedef struct actor_system_state {
    actor_mailbox_t actors[MAX_PROCESSES];
    size_t actor_count;
    uint32_t next_actor_id;
    uint32_t total_messages_processed;
    uint32_t actor_failures;             // Failed message deliveries
} actor_system_state_t;

extern ipc_transport_ops_t actor_model_ipc_ops;

/**
 * =============================================================================
 * REMOTE PROCEDURE CALL (RPC) IPC
 * =============================================================================
 */

typedef struct rpc_procedure {
    uint32_t procedure_id;
    char name[64];
    uint32_t param_count;
    uint32_t param_types[8];             // Parameter types
    uint32_t return_type;
    uint32_t provider_process;           // Process that provides this procedure
} rpc_procedure_t;

typedef struct rpc_call {
    uint32_t call_id;
    uint32_t procedure_id;
    uint32_t caller_process;
    uint32_t provider_process;
    void *parameters;
    size_t param_size;
    void *return_value;
    size_t return_size;
    bool is_async;                       // Synchronous vs asynchronous call
} rpc_call_t;

typedef struct rpc_ipc_state {
    rpc_procedure_t procedures[256];
    size_t procedure_count;
    rpc_call_t active_calls[64];
    size_t active_call_count;
    uint32_t next_call_id;
    uint32_t total_rpc_calls;
    uint32_t failed_calls;
} rpc_ipc_state_t;

extern ipc_transport_ops_t rpc_ipc_ops;

/**
 * =============================================================================
 * STUDENT TEMPLATE IPC
 * =============================================================================
 */

typedef struct student_ipc_template {
    ipc_transport_ops_t base;
    
    // Student data structures
    void *student_data;
    
    // Educational helpers
    void (*explain_mechanism)(void);
    void (*demonstrate_use_case)(void);
    void (*show_performance_characteristics)(void);
    
    // Performance metrics
    uint32_t (*get_latency_us)(void);
    uint32_t (*get_throughput_mbps)(void);
    uint32_t (*get_cpu_overhead_percent)(void);
    
    // Security analysis
    void (*analyze_security_properties)(void);
    void (*demonstrate_vulnerabilities)(void);
    
    // Debugging
    void (*trace_message_flow)(void);
    void (*validate_state_consistency)(void);
} student_ipc_template_t;

/**
 * =============================================================================
 * IPC BENCHMARK AND COMPARISON
 * =============================================================================
 */

typedef struct ipc_performance_metrics {
    uint32_t avg_latency_us;             // Average message latency
    uint32_t max_latency_us;             // Maximum message latency
    uint32_t throughput_msgs_per_sec;    // Messages per second
    uint32_t cpu_overhead_percent;       // CPU overhead percentage
    uint32_t memory_overhead_kb;         // Memory overhead in KB
    uint32_t failed_operations;          // Failed operations count
} ipc_performance_metrics_t;

typedef struct ipc_test_scenario {
    const char *name;
    uint32_t message_count;
    size_t message_size;
    uint32_t sender_count;
    uint32_t receiver_count;
    bool is_bidirectional;
    uint32_t think_time_us;              // Time between messages
} ipc_test_scenario_t;

// Standard test scenarios
extern ipc_test_scenario_t latency_test;
extern ipc_test_scenario_t throughput_test;
extern ipc_test_scenario_t scalability_test;
extern ipc_test_scenario_t stress_test;

/**
 * =============================================================================
 * EDUCATIONAL FUNCTIONS
 * =============================================================================
 */

// Initialize all IPC mechanisms
void init_example_ipc_mechanisms(void);

// Benchmark IPC mechanisms
ipc_performance_metrics_t benchmark_ipc(ipc_transport_ops_t *ipc_ops,
                                       ipc_test_scenario_t *scenario);

// Educational demonstrations
void demonstrate_ipc_tradeoffs(void);
void demonstrate_synchronization_issues(void);
void demonstrate_deadlock_scenarios(void);
void demonstrate_security_models(void);

// Interactive IPC exploration
void interactive_ipc_comparison(void);
void simulate_distributed_system(void);

// Performance analysis
void analyze_ipc_scalability(ipc_transport_ops_t **mechanisms, 
                           size_t mechanism_count);
void generate_ipc_performance_report(void);

// Security analysis
void analyze_ipc_security_properties(void);
void demonstrate_capability_model_benefits(void);

// Real-world examples
void demonstrate_microkernel_communication(void);
void simulate_client_server_model(void);
void simulate_producer_consumer_pattern(void);

#endif // MTOS_IPC_MECHANISMS_H
