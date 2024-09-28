//----HEADER---------------------------------------------------------------------------------------
// Author:      Sam Rolfe
// Date:        September 2024
// Script:      cache.c
// Usage:       Implementation file for cache
//*************************************************************************************************
#include <stdlib.h>
#include <stdio.h>

#include "cache.h"

// ----GLOBAL VARIABLES----------------------------------------------------------------------------


//----FUNCTIONS------------------------------------------------------------------------------------
// Initialize new cache
Cache *create_cache() {
    Cache *cache = malloc(sizeof(Cache));
    cache->size = 0;
    cache->capacity = MAX_CACHE_SIZE;
    for(int i =0; i < cache->capacity; i++) {
        cache->entries[i] = NULL;
    }
    return cache;
}


//-------------------------------------------------------------------------------------------------
