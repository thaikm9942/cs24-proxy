/*
 * hash.c - A package using a basic, non-dynamic Hash Table that contains an
 * array of queue pointers as a LRU cache to store websites.
 *
 * Each time the cache is initialized, a hash table containing a TABLE_SIZE
 * size array of queue pointers is initialized. This hash table keeps track of
 * all stored keys and values of the HTTP websites as well as the cache size.
 *
 * The hash table contains 3 basic functions: insert, remove and get. Everytime
 * a new element is inserted, its key is hashed and both the key and value are
 * stored as a node_t struct (see queue.c for explanation) within its respective
 * queue. The queue (which is a pointer containing the pointers to the head node
 * and the tail node) is updated accordingly. When remove is called, an element
 * is removed from the hash table based on LRU policy. This is achieved by
 * updating the timestamp in each node_t every time it is accessed. The node_t
 * with the least recent timestamp is removed and freed, and the queue is
 * updated accordingly. When get is called using a key, the queue corresponding
 * to the key hash id is iterated over until its matching node_t is found.
 * To prevent race conditions, a copy of the buffer_t value is returned instead
 * of the actual value.
 *
 * If the maximum cache size is exceeded when insert is attempted, then the hash
 * table automatically removes elements until there is enough space for caching.
 *
 * To create a thread-safe cache, a read-writer lock is used for the 3 functions
 * insert, get and create.
 *
 * This implementation is (hopefully) correct, thread-safe, utilize O(1)
 * lookup with a slightly slow insert and remove.
 */

#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"

#define HASH_NUMBER 37
#define TABLE_SIZE 67

/* This defines a NULL index to be used in searching for LRU node. If a
 * NULL index is returned, this means there was no minimum node.
 */
#define NULL_INDEX 9999

/* Defines the maximum cache size for the proxy cache */
#define MAX_CACHE_SIZE 1048756

struct hash_t {
  // This is an array of queue pointers for each bucket
  queue_t** queue_arr;
  // A read-write lock used to lock
  pthread_rwlock_t table_lock;
  // Keeps track of the number of buckets
  size_t buckets;
  // Keeps track of the cache size
  size_t cache_size;
};

// Constructor for a hash_t that also acts as a cache
hash_t *hash_init(void) {
  hash_t *hash_table = malloc(sizeof(hash_t));
  assert(hash_table != NULL);
  queue_t **queue_arr = malloc(TABLE_SIZE * sizeof(queue_t*));
  for (size_t i = 0; i < TABLE_SIZE; i++) {
    queue_arr[i] = queue_init();
  }
  hash_table->queue_arr = queue_arr;
  hash_table->buckets = TABLE_SIZE;
  hash_table->cache_size = 0;
  pthread_rwlock_init(&hash_table->table_lock, NULL);
  return hash_table;
}

size_t get_cache_size(hash_t* hash_table) {
  return hash_table->cache_size;
}

// Frees the hash table by calling queue_free on each bucket and destroying
// the lock
void hash_free(hash_t* hash_table) {
  if (!hash_table) {
    return;
  }
  for (size_t i = 0; i < hash_table->buckets; i++) {
    queue_free(hash_table->queue_arr[i]);
  }
  pthread_rwlock_destroy(&hash_table->table_lock);
  free(hash_table->queue_arr);
  free(hash_table);
}

// Returns the hash number of a given key
size_t get_hash_code(char* str) {
  size_t hash_total = 0;
  for (int i = strlen(str) - 1; i >= 0; i--) {
    hash_total += HASH_NUMBER * (hash_total + (size_t) str[i]);
  }
  return hash_total;
}

// Returns the hash id of a given key
size_t get_hash_id(hash_t* hash_table, char* key) {
  size_t hash_id = get_hash_code(key) % hash_table->buckets;
  return hash_id;
}

// Checks if the hash table contains a key
bool contains(hash_t* hash_table, char* key) {
  return get(hash_table, key) != NULL;
}

// Makes a new copy of the buffer_t before returning the copy from get to avoid
// race condition
buffer_t *copy_buffer(buffer_t* buf) {
  buffer_t* copy = buffer_create(buffer_length(buf));
  buffer_append_bytes(copy, buffer_data(buf), buffer_length(buf));
  return copy;
}

// Returns the value associated with the key
buffer_t* get(hash_t* hash_table, char* key) {
  size_t node_id = get_hash_id(hash_table, key);
  // Critical section
  pthread_rwlock_rdlock(&hash_table->table_lock);
  node_t* node = queue_get(hash_table->queue_arr[node_id], key);
  buffer_t* buffer = get_value(node);
  if (!get_value(node)) {
    pthread_rwlock_unlock(&hash_table->table_lock);
    return NULL;
  }
  pthread_rwlock_unlock(&hash_table->table_lock);
  return copy_buffer(buffer);
}

/* Returns the least recent node from the hashtable by checking each queue
 * for the least min node, and then comparing all of the min nodes with each
 * other for the min node. If there is no min_node (i.e, queue is empty),
 * then return the NULL_INDEX
 */
size_t find_least_recent_node_hash(hash_t* hash_table) {
  node_t* min_node_hash = NULL;
  size_t queue_idx = NULL_INDEX;
  for (size_t i = 0; i < hash_table->buckets; i++) {
    node_t* min_node_queue = find_least_recent_node(hash_table->queue_arr[i]);
    if (!min_node_hash && min_node_queue != NULL) {
      min_node_hash = min_node_queue;
      queue_idx = i;
    }
    if (min_node_queue != NULL && \
        get_timestamp(min_node_queue) < get_timestamp(min_node_hash)) {
      min_node_hash = min_node_queue;
      queue_idx = i;
    }
  }
  return queue_idx;
}

/* This function removes a LRU node from the hash_table by first finding
 * the correct queue to remove from, calling queue_remove on that queue,
 * and decrementing the cache size by the removed element size
 */
void hash_remove(hash_t* hash_table) {
  size_t queue_idx_to_remove = find_least_recent_node_hash(hash_table);
  // If queue_idx is NULL_INDEX, there is nothing to remove
  if (queue_idx_to_remove == NULL_INDEX) {
    return;
  }
  // Critical section
  pthread_rwlock_wrlock(&hash_table->table_lock);
  size_t removed = queue_remove(hash_table->queue_arr[queue_idx_to_remove]);
  hash_table->cache_size -= removed;
  pthread_rwlock_unlock(&hash_table->table_lock);
}

/* Inserts a node into the hash table based on its hash number. If the
 * cache is already full, keep removing elements until there is enough space
 * to insert.
 */
void insert(hash_t* hash_table, char* key, buffer_t* value) {
  node_t* new_node = node_init(key, value);
  size_t node_id = get_hash_id(hash_table, key);
  // If the added buffer size cause the cache size to overflow, then remove
  // until there's enough space
  while (hash_table->cache_size + buffer_length(value) > MAX_CACHE_SIZE) {
    hash_remove(hash_table);
  }
  // Critical section
  pthread_rwlock_wrlock(&hash_table->table_lock);
  enqueue(hash_table->queue_arr[node_id], new_node);
  hash_table->cache_size += buffer_length(value);
  pthread_rwlock_unlock(&hash_table->table_lock);
}
