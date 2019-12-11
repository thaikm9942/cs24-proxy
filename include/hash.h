#ifndef HASH_H
#define HASH_H

#include <stddef.h>
#include <stdint.h>
#include "queue.h"

/* An unresizahle hash table with default TABLE_SIZE */
typedef struct hash_t hash_t;

// Initializes a new hash table with default TABLE_SIZE by initializing
// the queue for each bucket; sets the number of buckets to be TABLE_SIZE,
// and initializes a read-write lock
hash_t *hash_init(void);
// Returns the cache size
size_t get_cache_size(hash_t* hash_table);
// Frees a given hash_t pointer
void hash_free(hash_t* hash_table);
// Returns the hash number of a given key
size_t get_hash_code(char* str);
// Returns the hash id of a given key
size_t get_hash_id(hash_t* hash_table, char* key);
// Checks if the hash table contains a key
bool contains(hash_t* hash_table, char* key);
// Returns the value associated with a key from the hash_table
buffer_t* get(hash_t* hash_table, char* key);
// Removes an element from the hash table
void hash_remove(hash_t* hash_table);
// Inserts a node element into the hash table given a key and a value
void insert(hash_t* hash_table, char* key, buffer_t* value);

#endif // HASH_H
