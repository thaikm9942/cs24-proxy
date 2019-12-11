#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>
#include <stdint.h>
#include "buffer.h"

typedef struct node_t node_t;

/* A queue_t struct contains a pointer to the head and the tail. This is defined
 * so the hash table implementation can access.
 */
typedef struct queue_t {
  node_t* head;
  node_t* tail;
} queue_t;

// Initializes a new node with a given key and a given value
node_t* node_init(char* key, buffer_t* value);
// Frees the given node
void node_free(node_t* node);
// Returns the timestamp of a node
clock_t get_timestamp(node_t* node);
// Initializes a new queue with NULL head and tail pointers
queue_t* queue_init();
// Frees a given queue pointer by iterating over every node and calling node_free
void queue_free(queue_t* queue);
// Returns the value of an associated node
buffer_t* get_value(node_t* node);
// Returns whether or not the queue contains a node with the given key
bool queue_contains(queue_t* queue, char* key);
// Returns a node associated with a key from the queue
node_t* queue_get(queue_t* queue, char* key);
// Adds a new node to the end of the queue and updates the timestamp
void enqueue(queue_t* queue, node_t* node);
// Returns the least recent node by iterating through the queue and finding
// the minimum timestamp
node_t* find_least_recent_node(queue_t* queue);
// Removes the least recently used element in a queue by checking each timestamp
// and returns the buffer size of the value removed.
size_t queue_remove(queue_t* queue);

// Functions used to test the node and queue implementation
node_t* get_next_node(node_t* node);
node_t* get_prev_node(node_t* node);


#endif // QUEUE_H
