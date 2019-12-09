#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "queue.h"

struct node_t {
  // The string depicting the site
  char* key;
  // A byte array storing the byte values
  buffer_t* value;
  // Timestamp to keep track of the last time accessed
  clock_t timestamp;
  // Pointer to previous node
  struct node_t *prev;
  // Pointer to next node
  struct node_t *next;
};

// Construct for a node_t
node_t* node_init(char* key, buffer_t* value) {
  node_t *node = malloc(sizeof(node_t));
  assert (node != NULL);
  node->key = key;
  node->value = value;
  node->prev = NULL;
  node->next = NULL;
  node->timestamp = 0;
  return node;
}

// Frees a node_t pointer
void node_free(node_t* node) {
  if (!node) {
    return;
  }
  buffer_free(node->value);
  free(node);
}

// Constructor for a queue_t
queue_t* queue_init(void) {
  queue_t* queue = malloc(sizeof(queue_t));
  assert (queue != NULL);
  queue->head = NULL;
  queue->tail = NULL;
  return queue;
}

// Frees a queue_t pointer
void queue_free(queue_t* queue) {
  if (!queue->head) {
    free(queue);
    return;
  }
  node_t* curr = queue->head;
  while (curr->next != NULL) {
    curr = curr->next;
    node_free(curr->prev);
  }
  node_free(curr);
  free(queue);
}

/* Returns the buffer_t of an associated node */
buffer_t* get_value(node_t* node) {
  if (!node) {
    return NULL;
  }
  return node->value;
}

/* Returns the timestamp of a node */
clock_t get_timestamp(node_t* node) {
  return node->timestamp;
}

/* Returns whether or not the queue contains a node with the given key */
bool queue_contains(queue_t* queue, char* key) {
  return queue_get(queue, key) != NULL;
}

/* Returns the node from the queue given a key */
node_t* queue_get(queue_t* queue, char* key) {
  // If queue is empty, then returns NULL
  if (!queue->head) {
    return NULL;
  }
  node_t* curr = queue->head;
  curr->timestamp = clock();
  // If the head's key matches the given key, then return current node
  if (strcmp(curr->key, key) == 0) {
    return curr;
  }
  /* Else, loop through the queue and checks if any of the node's key matches
   * the given key
   */
  while (curr->next != NULL) {
    curr = curr->next;
    curr->timestamp = clock();
    if (strcmp(curr->key, key) == 0) {
      return curr;
    }
  }
  return NULL;
}

void enqueue(queue_t* queue, node_t* node) {
  node_t* tail = queue->tail;
  /* If head is NULL, then sets head and tail to the new node
   * and updates timestamp.
   */
  if(!queue->head) {
    queue->head = node;
    queue->tail = node;
    node->timestamp = clock();
  }
  /* Else, updates the queue accordingly and updates the timestamp. */
  else {
    node->prev = tail;
    tail->next = node;
    queue->tail = node;
    node->timestamp = clock();
  }
}

/* Returns the least recent node by iterating through the queue and finding
 * the minimum timestamp. Returns NULL if the queue is empty.
 */
node_t* find_least_recent_node(queue_t* queue) {
  node_t* min_node = NULL;
  if(!queue->head) {
    return NULL;
  }
  else {
    // Loops over queue and check minimum timestamp
    node_t* curr = queue->head;
    clock_t min_time = curr->timestamp;
    min_node = curr;
    while (curr->next != NULL) {
      curr = curr->next;
      if (curr->timestamp < min_time) {
        min_time = curr->timestamp;
        min_node = curr;
      }
    }
  }
  return min_node;
}

/* Randomly evicts a LRU element from the queue. */
size_t queue_remove(queue_t* queue) {
  node_t* head = queue->head;
  node_t* tail = queue->tail;
  node_t* min_node = find_least_recent_node(queue);
  // If queue is empty, then return 0.
  if (!min_node) {
    return 0;
  }
  size_t buf_length;
  // If min_node is the only element in the queue, then free it
  if (min_node == head && !head->next) {
    buf_length = buffer_length(head->value);
    queue->head = NULL;
    node_free(head);
  }
  // If min_node is an element in the queue and there is a next pointer
  else if (min_node == head && head->next != NULL) {
    buf_length = buffer_length(head->value);
    // Retrieves the next element
    node_t* next = head->next;
    // Sets next pointer of current head to NULL
    head->next = NULL;
    // Sets prev pointer of next to NULL
    next->prev = NULL;
    // Sets new head to next element
    queue->head = next;
    // Frees the head
    node_free(head);
  }
  // If min_node is the tail (or last element)
  else if (min_node == tail) {
    buf_length = buffer_length(tail->value);
    // Retrieves previous element
    node_t* prev = tail->prev;
    // Sets the next pointer of previous element to NULL
    prev->next = NULL;
    // Sets prev pointer of the tail to NULL
    tail->prev = NULL;
    // Sets new tail to previous element
    queue->tail = prev;
    // Frees the tail
    node_free(tail);
  }
  // If min_node has both a next and prev pointer
  else {
    buf_length = buffer_length(min_node->value);
    // Retrieves both next and prev element
    node_t* next = min_node->next;
    node_t* prev = min_node->prev;
    prev->next = next;
    next->prev = prev;
    // Resets the next and prev pointers of the min_node
    min_node->next = NULL;
    min_node->prev = NULL;
    node_free(min_node);
  }
  return buf_length;
}

/* Functions used to test the node and queue implementation */
/* Returns the next node of a node, if there is one. */
node_t* get_next_node(node_t* node) {
  if (!node->next) {
    return NULL;
  }
  return node->next;
}

/* Returns the previous node of a node, if there is one. */
node_t* get_prev_node(node_t* node) {
  if (!node->prev) {
    return NULL;
  }
  return node->prev;
}
