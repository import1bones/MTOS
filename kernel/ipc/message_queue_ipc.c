/**
 * MTOS Message Queue IPC Implementation
 * Classic message passing with configurable queues and blocking/non-blocking modes
 */

#include "../../include/types.h"
#include "../interfaces/kernel_interfaces.h"

// Message queue constants
#define MAX_CHANNELS 32
#define MAX_QUEUE_DEPTH 16
#define MESSAGE_TIMEOUT 1000  // Ticks

// Message queue entry
typedef struct message_entry {
    ipc_message_t message;
    struct message_entry *next;
    uint32_t timestamp;
} message_entry_t;

// Message channel structure
typedef struct message_channel {
    uint32_t channel_id;
    uint32_t sender_id;
    uint32_t receiver_id;
    message_entry_t *queue_head;
    message_entry_t *queue_tail;
    size_t queue_size;
    size_t max_queue_size;
    bool is_blocking;
    bool in_use;
    uint32_t messages_sent;
    uint32_t messages_received;
    uint32_t messages_dropped;
} message_channel_t;

// Message queue IPC state
static struct {
    message_channel_t channels[MAX_CHANNELS];
    uint32_t next_channel_id;
    size_t active_channels;
    uint32_t current_tick;
    bool initialized;
    
    // Memory pool for message entries
    message_entry_t entry_pool[MAX_CHANNELS * MAX_QUEUE_DEPTH];
    bool entry_used[MAX_CHANNELS * MAX_QUEUE_DEPTH];
    size_t next_entry_index;
} mq_state;

// Define printf if not available
#ifndef printf
extern int printf(const char *format, ...);
#endif

// Memory management for message entries
static message_entry_t* alloc_message_entry(void) {
    for (size_t i = 0; i < MAX_CHANNELS * MAX_QUEUE_DEPTH; i++) {
        size_t index = (mq_state.next_entry_index + i) % (MAX_CHANNELS * MAX_QUEUE_DEPTH);
        if (!mq_state.entry_used[index]) {
            mq_state.entry_used[index] = true;
            mq_state.next_entry_index = (index + 1) % (MAX_CHANNELS * MAX_QUEUE_DEPTH);
            return &mq_state.entry_pool[index];
        }
    }
    return NULL;  // Pool exhausted
}

static void free_message_entry(message_entry_t *entry) {
    if (!entry) return;
    
    size_t index = entry - mq_state.entry_pool;
    if (index < MAX_CHANNELS * MAX_QUEUE_DEPTH) {
        mq_state.entry_used[index] = false;
    }
}

// Helper functions
static message_channel_t* find_channel_by_id(int channel_id) {
    for (size_t i = 0; i < MAX_CHANNELS; i++) {
        if (mq_state.channels[i].in_use && mq_state.channels[i].channel_id == (uint32_t)channel_id) {
            return &mq_state.channels[i];
        }
    }
    return NULL;
}

static message_channel_t* find_channel_by_participants(uint32_t sender_id, uint32_t receiver_id) {
    for (size_t i = 0; i < MAX_CHANNELS; i++) {
        message_channel_t *ch = &mq_state.channels[i];
        if (ch->in_use && ch->sender_id == sender_id && ch->receiver_id == receiver_id) {
            return ch;
        }
    }
    return NULL;
}

static int find_free_channel(void) {
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (!mq_state.channels[i].in_use) {
            return i;
        }
    }
    return -1;
}

static void enqueue_message(message_channel_t *channel, message_entry_t *entry) {
    entry->next = NULL;
    entry->timestamp = mq_state.current_tick;
    
    if (channel->queue_tail) {
        channel->queue_tail->next = entry;
    } else {
        channel->queue_head = entry;
    }
    
    channel->queue_tail = entry;
    channel->queue_size++;
}

static message_entry_t* dequeue_message(message_channel_t *channel) {
    if (!channel->queue_head) {
        return NULL;
    }
    
    message_entry_t *entry = channel->queue_head;
    channel->queue_head = entry->next;
    
    if (!channel->queue_head) {
        channel->queue_tail = NULL;
    }
    
    channel->queue_size--;
    entry->next = NULL;
    
    return entry;
}

// Implementation functions
static int mq_init(void) {
    if (mq_state.initialized) {
        return 0;
    }
    
    // Initialize channels
    for (int i = 0; i < MAX_CHANNELS; i++) {
        message_channel_t *ch = &mq_state.channels[i];
        ch->channel_id = 0;
        ch->sender_id = 0;
        ch->receiver_id = 0;
        ch->queue_head = NULL;
        ch->queue_tail = NULL;
        ch->queue_size = 0;
        ch->max_queue_size = MAX_QUEUE_DEPTH;
        ch->is_blocking = true;
        ch->in_use = false;
        ch->messages_sent = 0;
        ch->messages_received = 0;
        ch->messages_dropped = 0;
    }
    
    // Initialize memory pool
    for (size_t i = 0; i < MAX_CHANNELS * MAX_QUEUE_DEPTH; i++) {
        mq_state.entry_used[i] = false;
    }
    
    mq_state.next_channel_id = 1;
    mq_state.active_channels = 0;
    mq_state.current_tick = 0;
    mq_state.next_entry_index = 0;
    mq_state.initialized = true;
    
    return 0;
}

static void mq_shutdown(void) {
    // Clean up all channels
    for (int i = 0; i < MAX_CHANNELS; i++) {
        message_channel_t *ch = &mq_state.channels[i];
        
        // Free all messages in queue
        while (ch->queue_head) {
            message_entry_t *entry = dequeue_message(ch);
            free_message_entry(entry);
        }
        
        ch->in_use = false;
    }
    
    mq_state.active_channels = 0;
    mq_state.initialized = false;
}

static int mq_create_channel(uint32_t sender_id, uint32_t receiver_id) {
    // Check if channel already exists
    message_channel_t *existing = find_channel_by_participants(sender_id, receiver_id);
    if (existing) {
        return existing->channel_id;
    }
    
    // Find free channel slot
    int channel_index = find_free_channel();
    if (channel_index < 0) {
        return -1;  // No free channels
    }
    
    message_channel_t *ch = &mq_state.channels[channel_index];
    ch->channel_id = mq_state.next_channel_id++;
    ch->sender_id = sender_id;
    ch->receiver_id = receiver_id;
    ch->queue_head = NULL;
    ch->queue_tail = NULL;
    ch->queue_size = 0;
    ch->max_queue_size = MAX_QUEUE_DEPTH;
    ch->is_blocking = true;
    ch->in_use = true;
    ch->messages_sent = 0;
    ch->messages_received = 0;
    ch->messages_dropped = 0;
    
    mq_state.active_channels++;
    return ch->channel_id;
}

static void mq_destroy_channel(int channel_id) {
    message_channel_t *ch = find_channel_by_id(channel_id);
    if (!ch) return;
    
    // Free all messages in queue
    while (ch->queue_head) {
        message_entry_t *entry = dequeue_message(ch);
        free_message_entry(entry);
    }
    
    ch->in_use = false;
    mq_state.active_channels--;
}

static int mq_send_message(int channel_id, const ipc_message_t *msg) {
    if (!msg) return -1;
    
    message_channel_t *ch = find_channel_by_id(channel_id);
    if (!ch) return -1;
    
    // Check if queue is full
    if (ch->queue_size >= ch->max_queue_size) {
        ch->messages_dropped++;
        return -1;  // Queue full
    }
    
    // Allocate message entry
    message_entry_t *entry = alloc_message_entry();
    if (!entry) {
        ch->messages_dropped++;
        return -1;  // No memory
    }
    
    // Copy message
    entry->message = *msg;
    entry->next = NULL;
    entry->timestamp = mq_state.current_tick;
    
    // Add to queue
    enqueue_message(ch, entry);
    ch->messages_sent++;
    
    return 0;
}

static int mq_receive_message(int channel_id, ipc_message_t *msg) {
    if (!msg) return -1;
    
    message_channel_t *ch = find_channel_by_id(channel_id);
    if (!ch) return -1;
    
    // Try to get message from queue
    message_entry_t *entry = dequeue_message(ch);
    if (!entry) {
        return -1;  // No messages
    }
    
    // Copy message
    *msg = entry->message;
    ch->messages_received++;
    
    // Free the entry
    free_message_entry(entry);
    
    return 0;
}

static int mq_try_receive(int channel_id, ipc_message_t *msg) {
    // Same as receive_message for this implementation
    return mq_receive_message(channel_id, msg);
}

static bool mq_can_send(int channel_id) {
    message_channel_t *ch = find_channel_by_id(channel_id);
    if (!ch) return false;
    
    return ch->queue_size < ch->max_queue_size;
}

static bool mq_has_messages(int channel_id) {
    message_channel_t *ch = find_channel_by_id(channel_id);
    if (!ch) return false;
    
    return ch->queue_size > 0;
}

static size_t mq_get_queue_size(int channel_id) {
    message_channel_t *ch = find_channel_by_id(channel_id);
    if (!ch) return 0;
    
    return ch->queue_size;
}

static bool mq_check_permission(uint32_t sender_id, uint32_t receiver_id) {
    // For educational purposes, allow all communications
    return true;
}

static void mq_grant_capability(uint32_t grantor, uint32_t grantee, uint32_t rights) {
    // In a real implementation, this would manage access control lists
    // For educational purposes, we'll just acknowledge the call
}

static void mq_print_stats(void) {
    printf("MESSAGE QUEUE IPC STATISTICS:\n");
    printf("  Active channels: %zu\n", mq_state.active_channels);
    printf("  Current tick: %u\n", mq_state.current_tick);
    
    uint32_t total_sent = 0, total_received = 0, total_dropped = 0;
    size_t total_queued = 0;
    
    for (int i = 0; i < MAX_CHANNELS; i++) {
        message_channel_t *ch = &mq_state.channels[i];
        if (ch->in_use) {
            total_sent += ch->messages_sent;
            total_received += ch->messages_received;
            total_dropped += ch->messages_dropped;
            total_queued += ch->queue_size;
        }
    }
    
    printf("  Total messages sent: %u\n", total_sent);
    printf("  Total messages received: %u\n", total_received);
    printf("  Total messages dropped: %u\n", total_dropped);
    printf("  Total messages queued: %zu\n", total_queued);
    
    if (total_sent > 0) {
        printf("  Delivery rate: %.1f%%\n", 
               100.0 * total_received / total_sent);
        printf("  Drop rate: %.1f%%\n", 
               100.0 * total_dropped / (total_sent + total_dropped));
    }
    
    printf("\n  Active channels:\n");
    for (int i = 0; i < MAX_CHANNELS; i++) {
        message_channel_t *ch = &mq_state.channels[i];
        if (ch->in_use) {
            printf("    Channel %u: %u->%u, %zu/%zu messages, %u sent, %u received\n",
                   ch->channel_id, ch->sender_id, ch->receiver_id,
                   ch->queue_size, ch->max_queue_size,
                   ch->messages_sent, ch->messages_received);
        }
    }
}

// Utility functions
void mq_set_blocking_mode(int channel_id, bool blocking) {
    message_channel_t *ch = find_channel_by_id(channel_id);
    if (ch) {
        ch->is_blocking = blocking;
    }
}

void mq_set_queue_size(int channel_id, size_t max_size) {
    message_channel_t *ch = find_channel_by_id(channel_id);
    if (ch && max_size <= MAX_QUEUE_DEPTH) {
        ch->max_queue_size = max_size;
    }
}

void mq_tick(void) {
    mq_state.current_tick++;
    
    // Could implement message timeouts here
    // For now, just increment the tick counter
}

// Export the message queue IPC interface
ipc_transport_ops_t message_queue_ipc_ops = {
    .name = "message_queue",
    .description = "Classic message passing with configurable queues and flow control",
    .init = mq_init,
    .shutdown = mq_shutdown,
    .create_channel = mq_create_channel,
    .destroy_channel = mq_destroy_channel,
    .send_message = mq_send_message,
    .receive_message = mq_receive_message,
    .try_receive = mq_try_receive,
    .can_send = mq_can_send,
    .has_messages = mq_has_messages,
    .get_queue_size = mq_get_queue_size,
    .check_permission = mq_check_permission,
    .grant_capability = mq_grant_capability,
    .print_stats = mq_print_stats
};
