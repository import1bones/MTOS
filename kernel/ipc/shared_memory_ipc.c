/**
 * MTOS Shared Memory IPC Implementation
 * High-performance IPC using shared memory regions with synchronization
 */

#include "../../include/types.h"
#include "../interfaces/kernel_interfaces.h"

// Shared memory constants
#define MAX_SHARED_REGIONS 64
#define MAX_PROCESSES_PER_REGION 8
#define SHARED_REGION_SIZE 4096  // 4KB regions
#define MAX_MESSAGE_SIZE 1024

// Shared memory region structure
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

// Message structure for shared memory
typedef struct shared_message {
    uint32_t sender_id;
    uint32_t receiver_id;
    uint32_t message_id;
    size_t size;
    uint8_t data[MAX_MESSAGE_SIZE];
    bool valid;
} shared_message_t;

// Shared memory IPC state
static struct {
    shared_region_t regions[MAX_SHARED_REGIONS];
    uint32_t next_region_id;
    size_t active_regions;
    uint32_t total_messages_sent;
    uint32_t total_messages_received;
    bool initialized;
} shm_state;

// Define printf if not available
#ifndef printf
extern int printf(const char *format, ...);
#endif

// Simple spinlock implementation
static void acquire_lock(volatile bool *lock) {
    while (*lock) {
        // Busy wait (in real OS, would use proper synchronization)
    }
    *lock = true;
}

static void release_lock(volatile bool *lock) {
    *lock = false;
}

// Find free region slot
static int find_free_region(void) {
    for (int i = 0; i < MAX_SHARED_REGIONS; i++) {
        if (!shm_state.regions[i].in_use) {
            return i;
        }
    }
    return -1;
}

// Find region by participants
static shared_region_t* find_region_by_participants(uint32_t sender_id, uint32_t receiver_id) {
    for (int i = 0; i < MAX_SHARED_REGIONS; i++) {
        shared_region_t *region = &shm_state.regions[i];
        if (!region->in_use) continue;
        
        bool sender_found = false, receiver_found = false;
        for (size_t j = 0; j < region->participant_count; j++) {
            if (region->participants[j] == sender_id) sender_found = true;
            if (region->participants[j] == receiver_id) receiver_found = true;
        }
        
        if (sender_found && receiver_found) {
            return region;
        }
    }
    return NULL;
}

// Check if process has permission to access region
static bool check_region_permission(shared_region_t *region, uint32_t process_id, uint32_t required_permission) {
    if (!region || !region->in_use) return false;
    
    // Check if process is a participant
    for (size_t i = 0; i < region->participant_count; i++) {
        if (region->participants[i] == process_id) {
            return (region->permissions & required_permission) != 0;
        }
    }
    
    return false;
}

// Allocate memory for shared region (simplified)
static void* allocate_shared_memory(size_t size) {
    // In a real OS, this would use special shared memory allocation
    // For educational purposes, we'll use a simple allocation
    static uint8_t shared_pool[MAX_SHARED_REGIONS * SHARED_REGION_SIZE];
    static size_t pool_offset = 0;
    
    if (pool_offset + size > sizeof(shared_pool)) {
        return NULL;
    }
    
    void *ptr = &shared_pool[pool_offset];
    pool_offset += size;
    return ptr;
}

// Implementation functions
static int shm_init(void) {
    if (shm_state.initialized) {
        return 0;
    }
    
    // Initialize all regions
    for (int i = 0; i < MAX_SHARED_REGIONS; i++) {
        shared_region_t *region = &shm_state.regions[i];
        region->region_id = 0;
        region->creator_id = 0;
        region->participant_count = 0;
        region->memory = NULL;
        region->size = 0;
        region->permissions = 0;
        region->in_use = false;
        region->lock = false;
        region->read_index = 0;
        region->write_index = 0;
        region->has_data = false;
        
        for (int j = 0; j < MAX_PROCESSES_PER_REGION; j++) {
            region->participants[j] = 0;
        }
    }
    
    shm_state.next_region_id = 1;
    shm_state.active_regions = 0;
    shm_state.total_messages_sent = 0;
    shm_state.total_messages_received = 0;
    shm_state.initialized = true;
    
    return 0;
}

static void shm_shutdown(void) {
    // Clean up all regions
    for (int i = 0; i < MAX_SHARED_REGIONS; i++) {
        shm_state.regions[i].in_use = false;
        shm_state.regions[i].memory = NULL;
    }
    
    shm_state.active_regions = 0;
    shm_state.initialized = false;
}

static int shm_create_channel(uint32_t sender_id, uint32_t receiver_id) {
    // Check if channel already exists
    shared_region_t *existing = find_region_by_participants(sender_id, receiver_id);
    if (existing) {
        return existing->region_id;
    }
    
    // Find free region
    int region_index = find_free_region();
    if (region_index < 0) {
        return -1;  // No free regions
    }
    
    shared_region_t *region = &shm_state.regions[region_index];
    
    // Allocate shared memory
    region->memory = allocate_shared_memory(SHARED_REGION_SIZE);
    if (!region->memory) {
        return -1;
    }
    
    // Initialize region
    region->region_id = shm_state.next_region_id++;
    region->creator_id = sender_id;
    region->participants[0] = sender_id;
    region->participants[1] = receiver_id;
    region->participant_count = 2;
    region->size = SHARED_REGION_SIZE;
    region->permissions = 0x3;  // Read and write for all participants
    region->in_use = true;
    region->lock = false;
    region->read_index = 0;
    region->write_index = 0;
    region->has_data = false;
    
    // Clear the shared memory
    for (size_t i = 0; i < SHARED_REGION_SIZE; i++) {
        ((uint8_t*)region->memory)[i] = 0;
    }
    
    shm_state.active_regions++;
    return region->region_id;
}

static void shm_destroy_channel(int channel_id) {
    for (int i = 0; i < MAX_SHARED_REGIONS; i++) {
        shared_region_t *region = &shm_state.regions[i];
        if (region->in_use && region->region_id == (uint32_t)channel_id) {
            region->in_use = false;
            region->memory = NULL;  // Don't actually free in this simple implementation
            shm_state.active_regions--;
            break;
        }
    }
}

static int shm_send_message(int channel_id, const ipc_message_t *msg) {
    if (!msg) return -1;
    
    // Find the region
    shared_region_t *region = NULL;
    for (int i = 0; i < MAX_SHARED_REGIONS; i++) {
        if (shm_state.regions[i].in_use && shm_state.regions[i].region_id == (uint32_t)channel_id) {
            region = &shm_state.regions[i];
            break;
        }
    }
    
    if (!region || !check_region_permission(region, msg->sender_id, 0x2)) {  // Write permission
        return -1;
    }
    
    acquire_lock(&region->lock);
    
    // Check if there's space (simplified - only one message at a time)
    if (region->has_data) {
        release_lock(&region->lock);
        return -1;  // Buffer full
    }
    
    // Copy message to shared memory
    shared_message_t *shared_msg = (shared_message_t*)region->memory;
    shared_msg->sender_id = msg->sender_id;
    shared_msg->receiver_id = msg->receiver_id;
    shared_msg->message_id = msg->message_id;
    shared_msg->size = msg->size < MAX_MESSAGE_SIZE ? msg->size : MAX_MESSAGE_SIZE;
    
    for (size_t i = 0; i < shared_msg->size; i++) {
        shared_msg->data[i] = msg->data[i];
    }
    
    shared_msg->valid = true;
    region->has_data = true;
    
    release_lock(&region->lock);
    
    shm_state.total_messages_sent++;
    return 0;
}

static int shm_receive_message(int channel_id, ipc_message_t *msg) {
    if (!msg) return -1;
    
    // Find the region
    shared_region_t *region = NULL;
    for (int i = 0; i < MAX_SHARED_REGIONS; i++) {
        if (shm_state.regions[i].in_use && shm_state.regions[i].region_id == (uint32_t)channel_id) {
            region = &shm_state.regions[i];
            break;
        }
    }
    
    if (!region) {
        return -1;
    }
    
    acquire_lock(&region->lock);
    
    if (!region->has_data) {
        release_lock(&region->lock);
        return -1;  // No messages
    }
    
    // Copy message from shared memory
    shared_message_t *shared_msg = (shared_message_t*)region->memory;
    if (!shared_msg->valid) {
        release_lock(&region->lock);
        return -1;
    }
    
    msg->sender_id = shared_msg->sender_id;
    msg->receiver_id = shared_msg->receiver_id;
    msg->message_id = shared_msg->message_id;
    msg->size = shared_msg->size;
    
    for (size_t i = 0; i < shared_msg->size; i++) {
        msg->data[i] = shared_msg->data[i];
    }
    
    // Mark message as consumed
    shared_msg->valid = false;
    region->has_data = false;
    
    release_lock(&region->lock);
    
    shm_state.total_messages_received++;
    return 0;
}

static int shm_try_receive(int channel_id, ipc_message_t *msg) {
    // Same as receive_message for this simple implementation
    return shm_receive_message(channel_id, msg);
}

static bool shm_can_send(int channel_id) {
    for (int i = 0; i < MAX_SHARED_REGIONS; i++) {
        shared_region_t *region = &shm_state.regions[i];
        if (region->in_use && region->region_id == (uint32_t)channel_id) {
            return !region->has_data;  // Can send if no data waiting
        }
    }
    return false;
}

static bool shm_has_messages(int channel_id) {
    for (int i = 0; i < MAX_SHARED_REGIONS; i++) {
        shared_region_t *region = &shm_state.regions[i];
        if (region->in_use && region->region_id == (uint32_t)channel_id) {
            return region->has_data;
        }
    }
    return false;
}

static size_t shm_get_queue_size(int channel_id) {
    // For this simple implementation, queue size is either 0 or 1
    return shm_has_messages(channel_id) ? 1 : 0;
}

static bool shm_check_permission(uint32_t sender_id, uint32_t receiver_id) {
    // In a real OS, this would check security policies
    // For educational purposes, allow all communications
    return true;
}

static void shm_grant_capability(uint32_t grantor, uint32_t grantee, uint32_t rights) {
    // Find regions where grantor is creator and add grantee with rights
    for (int i = 0; i < MAX_SHARED_REGIONS; i++) {
        shared_region_t *region = &shm_state.regions[i];
        if (region->in_use && region->creator_id == grantor && 
            region->participant_count < MAX_PROCESSES_PER_REGION) {
            
            // Check if grantee is already a participant
            bool already_participant = false;
            for (size_t j = 0; j < region->participant_count; j++) {
                if (region->participants[j] == grantee) {
                    already_participant = true;
                    break;
                }
            }
            
            if (!already_participant) {
                region->participants[region->participant_count++] = grantee;
                region->permissions |= rights;
            }
        }
    }
}

static void shm_print_stats(void) {
    printf("SHARED MEMORY IPC STATISTICS:\n");
    printf("  Active regions: %zu\n", shm_state.active_regions);
    printf("  Messages sent: %u\n", shm_state.total_messages_sent);
    printf("  Messages received: %u\n", shm_state.total_messages_received);
    printf("  Next region ID: %u\n", shm_state.next_region_id);
    
    printf("\n  Active regions:\n");
    for (int i = 0; i < MAX_SHARED_REGIONS; i++) {
        shared_region_t *region = &shm_state.regions[i];
        if (region->in_use) {
            printf("    Region %u: %zu participants, %zu bytes, %s\n",
                   region->region_id,
                   region->participant_count,
                   region->size,
                   region->has_data ? "has data" : "empty");
        }
    }
}

// Export the shared memory IPC interface
ipc_transport_ops_t shared_memory_ipc_ops = {
    .name = "shared_memory",
    .description = "High-performance IPC using shared memory regions with basic synchronization",
    .init = shm_init,
    .shutdown = shm_shutdown,
    .create_channel = shm_create_channel,
    .destroy_channel = shm_destroy_channel,
    .send_message = shm_send_message,
    .receive_message = shm_receive_message,
    .try_receive = shm_try_receive,
    .can_send = shm_can_send,
    .has_messages = shm_has_messages,
    .get_queue_size = shm_get_queue_size,
    .check_permission = shm_check_permission,
    .grant_capability = shm_grant_capability,
    .print_stats = shm_print_stats
};
