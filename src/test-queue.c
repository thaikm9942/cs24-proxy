#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "queue.h"
#include "hash.h"

#define DEFAULT_CAPACITY 8

int main() {
  char* key1 = "a";
  buffer_t *buf1 = buffer_create(DEFAULT_CAPACITY);
  buffer_append_char(buf1, 'd');
  buffer_append_char(buf1, 'e');

  char* key2 = "b";
  buffer_t *buf2 = buffer_create(DEFAULT_CAPACITY);
  buffer_append_char(buf2, 'f');
  buffer_append_char(buf2, 'g');
  buffer_append_char(buf2, 'h');

  char* key3 = "c";
  buffer_t *buf3 = buffer_create(DEFAULT_CAPACITY);
  buffer_append_char(buf3, 'i');

  node_t* node1 = node_init(key1, buf1);
  node_t* node2 = node_init(key2, buf2);
  node_t* node3 = node_init(key3, buf3);

  /* Queue Tests*/
  // Initializes a queue
  queue_t* queue = queue_init();
  // Test removing an empty queue. Removing an empty queue always returns 0
  // for buf_length
  assert(queue_remove(queue) == 0);

  // Test enqueue
  // Inserts 2 elements
  enqueue(queue, node1);
  enqueue(queue, node2);

  // Tests that node1 is the head, and node2 is the tail
  assert(queue->head == node1);
  assert(queue->tail == node2);

  // Tests that queue_get returns node1 and node2 when given the respective keys.
  // On the other hand, queue_get should return NULL when given key3.
  assert(queue_get(queue, key1) == node1);
  assert(queue_get(queue, key2) == node2);
  assert(!queue_get(queue, key3));

  // Test contains. Since node1 and node2 are in the queue, both contains should
  // return TRUE. Since node3 is not in the queue, queue_contains returns FALSE
  assert(queue_contains(queue, key1));
  assert(queue_contains(queue, key2));
  assert(!queue_contains(queue, key3));

  // Test that the queue is aligning correctly
  assert(get_next_node(node2) == NULL);
  assert(get_prev_node(node1) == NULL);
  assert(get_next_node(node1) == node2);
  assert(get_prev_node(node2) == node1);

  // Inserts 3rd element
  enqueue(queue, node3);

  // Tests that node1 is the head, and node3 is the tail
  assert(queue->head == node1);
  assert(queue->tail == node3);

  // Test that queue_get correctly returns node3
  assert(queue_get(queue, key3) == node3);

  // Test queue_contains on node3
  assert(queue_contains(queue, key3));

  // Test that the queue is being updated correctly
  assert(get_next_node(node3) == NULL);
  assert(get_prev_node(node1) == NULL);
  assert(get_next_node(node1) == node2);
  assert(get_prev_node(node2) == node1);
  assert(get_next_node(node2) == node3);
  assert(get_prev_node(node3) == node2);

  // Test method to find least recent node. Should return node1 based on
  // we've ran
  assert(find_least_recent_node(queue) == node1);

  // Test removing. Since node1 is the least recent node, queue_remove should free
  // node1 and return the size of the buffer freed. In this case, should
  // return 2.
  assert(queue_remove(queue) == 2);

  // Node1 should not be in the queue anymore
  assert(!queue_contains(queue, key1));

  // queue_get should return NULL when searching for key1
  assert(queue_get(queue, key1) == NULL);

  // Order should be 2->3 now.
  // After removal, test that node2 is the head, and node3 is the tail
  assert(queue->head == node2);
  assert(queue->tail == node3);

  // Test that the queue is being updated correctly
  assert(get_next_node(node3) == NULL);
  assert(get_prev_node(node2) == NULL);
  assert(get_next_node(node2) == node3);
  assert(get_prev_node(node3) == node2);

  // Queue should still contains key2, updates timestamp so we can remove
  // node3 again.
  assert(queue_contains(queue, key2));

  // Test method to find least recent node. Should return node3 now
  assert(find_least_recent_node(queue) == node3);

  // Remove another node. This time it should be node3, which has buf_length 1.
  assert(queue_remove(queue) == 1);

  // Node3 should not be in the queue anymore
  assert(!queue_contains(queue, key3));

  // queue_get should return NULL when searching for key3
  assert(queue_get(queue, key3) == NULL);

  // Order should just be 2.
  // After removal, test that node2 is the head, and node3 is the tail
  assert(queue->head == node2);
  assert(queue->tail == node2);

  // Test that the queue is being updated correctly
  assert(get_next_node(node2) == NULL);
  assert(get_prev_node(node2) == NULL);

  key1 = "a";
  buf1 = buffer_create(DEFAULT_CAPACITY);
  buffer_append_char(buf1, 'd');
  buffer_append_char(buf1, 'e');

  key3 = "c";
  buf3 = buffer_create(DEFAULT_CAPACITY);
  buffer_append_char(buf3, 'i');

  node1 = node_init(key1, buf1);
  node3 = node_init(key3, buf3);

  // Inserts node1 and node3 again
  enqueue(queue, node1);
  enqueue(queue, node3);

  // Queue should now be 2->1->3
  // Now node2 is the head, and node3 is the tail
  assert(queue->head == node2);
  assert(queue->tail == node3);

  // Tests queue_get again.
  assert(queue_get(queue, key1) == node1);
  assert(queue_get(queue, key2) == node2);
  assert(queue_get(queue, key3) == node3);

  // Test queue_contains, but let node2 be the most recent one.
  assert(queue_contains(queue, key1));
  assert(queue_contains(queue, key3));
  assert(queue_contains(queue, key2));

  // Now, order of least recent should be 3->2->1
  // Test that the queue is aligning correctly
  assert(get_next_node(node2) == node1);
  assert(get_prev_node(node3) == node1);
  assert(get_next_node(node1) == node3);
  assert(get_prev_node(node2) == NULL);
  assert(get_next_node(node3) == NULL);
  assert(get_prev_node(node1) == node2);

  // Now that the order is 2->1->3.
  // Test removing the element in the middle (which is node1) in this case.

  // Test method to find least recent node. Should return node1 now.
  assert(find_least_recent_node(queue) == node1);

  // Remove another node. This time it should be node1, which has buf_length 2.
  assert(queue_remove(queue) == 2);

  // Node1 should not be in the queue anymore
  assert(!queue_contains(queue, key1));

  // queue_get should return NULL when searching for key3
  assert(queue_get(queue, key1) == NULL);

  // Order should be 2->3 after removal.
  // After removal, test that node2 is the head, and node3 is the tail
  assert(queue->head == node2);
  assert(queue->tail == node3);

  // Test that the queue is being updated correctly
  assert(get_next_node(node2) == node3);
  assert(get_prev_node(node3) == node2);
  assert(get_next_node(node3) == NULL);
  assert(get_prev_node(node2) == NULL);

  // Test queue_free, should not get any memory leaks or give any errors
  queue_free(queue);
  printf("Queue tests passed!\n");
}
