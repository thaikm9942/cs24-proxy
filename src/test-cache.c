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
  /* Hashtable Test*/
  char* key1 = "a";
  buffer_t *buf1 = buffer_create(DEFAULT_CAPACITY);
  buffer_append_char(buf1, 'd');
  buffer_append_char(buf1, 'e');

  // Initializes a new cache
  hash_t* cache = hash_init();

  // Insert a single key and buffer into the cache
  insert(cache, key1, buf1);

  // Test contains. Key1 and buf1 should be in cache now
  assert(contains(cache, key1));

  // Test that cache_size is same size as buf1 size
  assert(get_cache_size(cache) == buffer_length(buf1));


  // Test that get returns the correct buffer string
  assert(strcmp(buffer_string(get(cache, key1)), "de") == 0);

  // Test hash_free, should pass and give no leaks or errors
  hash_free(cache);

  printf("Cache tests passsed\n");
}
