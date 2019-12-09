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

/* Defines the maximum cache size for the proxy cache */
#define MAX_CACHE_SIZE 1048756
// #define MAX_CACHE_SIZE 50000


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

void hash_remove(hash_t* hash_table, char* key) {
  size_t node_id = get_hash_id(hash_table, key);
  pthread_rwlock_wrlock(&hash_table->table_lock);
  size_t removed = queue_remove(hash_table->queue_arr[node_id]);
  printf("removed size: %zu\n", removed);
  hash_table->cache_size -= removed;
  printf("current cache size: %zu\n", get_cache_size(hash_table));
  pthread_rwlock_unlock(&hash_table->table_lock);
}

void insert(hash_t* hash_table, char* key, buffer_t* value) {
  node_t* new_node = node_init(key, value);
  size_t node_id = get_hash_id(hash_table, key);
  // If the added buffer size cause the cache size to overflow, then remove
  // until there's enough space
  while (hash_table->cache_size + buffer_length(value) > MAX_CACHE_SIZE) {
    // printf("removing from cache because it's full\n");
    hash_remove(hash_table, key);
  }
  pthread_rwlock_wrlock(&hash_table->table_lock);
  enqueue(hash_table->queue_arr[node_id], new_node);
  hash_table->cache_size += buffer_length(value);
  printf("current cache size: %zu\n", get_cache_size(hash_table));
  pthread_rwlock_unlock(&hash_table->table_lock);
}
