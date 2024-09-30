//----HEADER---------------------------------------------------------------------------------------
// Author:      Sam Rolfe
// Date:        September 2024
// Script:      cache.h 
// Usage:       Header file for cache
//*************************************************************************************************
#ifndef CACHE_H
#define CACHE_H

#include "cache_entry.h"
#include <stdbool.h>

// ----GLOBAL VARIABLES----------------------------------------------------------------------------
#define MAX_CACHE_SIZE 4

// ----STRUCT--------------------------------------------------------------------------------------

typedef struct Cache{
    CacheEntry *entries[MAX_CACHE_SIZE];
    int size;
    int capacity;
} Cache;

//----FUNCTIONS------------------------------------------------------------------------------------

Cache *cache_create(void);
void cache_free(Cache* cache);
void cache_insert(Cache* cache, char* url, unsigned char *server_response, size_t *server_response_size);
bool cache_check(Cache* cache, char *url);
int get_cache_entry_index(Cache* cache, char *url);
unsigned char *cache_retrieval(Cache *cache, char *url, size_t *server_response_size);
unsigned char *add_age_header(CacheEntry *cached_entry, size_t *server_response_size);
void evict(Cache* cache, int cache_entry_index);
void cache_eviction_protocol(Cache* cache);
bool is_older(struct timespec time1, struct timespec time2);

//----MAIN-----------------------------------------------------------------------------------------

#endif
//-------------------------------------------------------------------------------------------------
