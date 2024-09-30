//----HEADER---------------------------------------------------------------------------------------
// Author:      Sam Rolfe
// Date:        September 2024
// Script:      cache.c
// Usage:       Implementation file for cache
//*************************************************************************************************
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cache.h"
// #include "proxy.c"

#define HTTP_HEADER_MAX_SIZE 2000 

// ----GLOBAL VARIABLES----------------------------------------------------------------------------


//----FUNCTIONS------------------------------------------------------------------------------------
// Initialize new cache
Cache *cache_create(void) {
    Cache *cache = malloc(sizeof(Cache));
    cache->size = 0;
    cache->capacity = MAX_CACHE_SIZE;
    printf("Cache capacity: %d\n", cache->capacity);
    for(int i = 0; i < cache->capacity; i++) {
        cache->entries[i] = NULL;
    }
    return cache;
}

// Given cache pointer and server response, create a 
// new cache entry, add to cache, and update cache accordingly
void cache_insert(Cache* cache, char* url, unsigned char *server_response, size_t *server_response_size) {
    // Create cache entry
    CacheEntry *cache_entry = CacheEntry_create(url, server_response, server_response_size);
    // Check whether cache is full
    bool cache_at_capacity = (cache->capacity == cache->size);
    if(cache_at_capacity) {
        printf("Entering cache eviction protocol\n");
        cache_eviction_protocol(cache);
    }
    // Insert cache entry into cache
    printf("Inserting response for %s at cache_entries[%d]\n", url, cache->size);
    cache->entries[cache->size] = cache_entry;
    cache->size += 1;
    printf("Updating cache size to: %d\n", cache->size);
    printf("Cache capacity: %d\n", cache->capacity);
}

// Identify node to be evicted and evict
void cache_eviction_protocol(Cache* cache){
    // Evict any stale item
    int cache_entry_eviction_index = -1;
    for(int i = 0; i < cache->size; i++) {
        if(!cache_entry_valid(cache->entries[i])) {
            cache_entry_eviction_index = i;
        }
    }
    if(cache_entry_eviction_index >= 0) {
        printf("Evicting cache_entries[%d]: (stale)\n", cache_entry_eviction_index);
    }

    // If no items are stale, remove the least-recently accessed
    // cache entry
    if(cache_entry_eviction_index < 0) {
        int cache_entry_oldest_access_time_index = 0;
        struct timespec oldest_access_time = cache->entries[0]->time_accessed;
        struct timespec current_access_time;
        for(int i = 0; i < cache->size; i++) {
            current_access_time = cache->entries[i]->time_accessed;
            if(is_older(current_access_time, oldest_access_time)) {
                oldest_access_time = current_access_time;
                cache_entry_oldest_access_time_index = i;
            }
        }
        cache_entry_eviction_index = cache_entry_oldest_access_time_index;
        printf("Evicting cache_entries[%d]: (oldest)\n", cache_entry_eviction_index);
    }

    // Evict cache entry identified by eviction protocol
    evict(cache, cache_entry_eviction_index);
}

// Return true if time1 is older than time2, else return false
bool is_older(struct timespec time1, struct timespec time2){
    if((time1.tv_sec < time2.tv_sec) || 
       ((time1.tv_sec == time2.tv_sec) && (time1.tv_nsec < time2.tv_nsec))) {
        return true;
    } else {
        return false;
    }
}


void cache_free(Cache* cache) {
    for(int i = 0; i < cache->size; i++) {
        free(cache->entries[i]);
    }
    free(cache);
}

// Given cache and url, determine whether matching
// cache entry exists in cache. If so, return index
// of matching entry. Else, return -1
int get_cache_entry_index(Cache* cache, char *url) {
    int cache_entry_index = -1;
    for(int i = 0; i < cache->size; i++) {
        char *cached_url = cache->entries[i]->url;
        // printf("Searched-for URL: %s\n", url);
        // printf("Cached URL (index %d): %s\n", i, cached_url);
        // printf("strlen(searched-for URL): %d\n", strlen(url));
        // printf("strlen(cached_URL): %d\n", strlen(cached_url));
        if(strcmp(url, cached_url) == 0) {
            cache_entry_index = i;
            return cache_entry_index;
        }
    }
    return cache_entry_index;
}

// Determine whether request is present in cache.
// If request is present and valid, return true.
// If request is present and stale, evict and return false. 
// If request is not present, return false.
bool cache_check(Cache* cache, char *url) {
    int cache_entry_index = get_cache_entry_index(cache, url);
    if(cache_entry_index < 0) {
        printf("Cached entry not found\n");
        return false;
    } 
    if(cache_entry_index >= 0) {
        printf("Found cached entry at index :%d\n", cache_entry_index);
    }
    CacheEntry *cache_entry = cache->entries[cache_entry_index];
    bool is_valid = cache_entry_valid(cache_entry);
    if(!is_valid) {
        printf("Cached entry is stale. Evicting...\n");
        evict(cache, cache_entry_index);
        return false;
    }

    return true;
}

// Given cache and index of entry to evict, 
// evict entry and update cache accordingly
void evict(Cache* cache, int cache_entry_index) {
    free(cache->entries[cache_entry_index]);
    for(int i = cache_entry_index; i < (cache->size - 1); i++) {
        cache->entries[i] = cache->entries[i+1];
    }
    cache->size -= 1;
}


// Given a cache and the URL of a cache entry known to exist in the cache, 
// return a copy of the response stored in the cache entry with the age field
// added to the header. Update the retrieval field and assign the size of the 
// server response to the server_response_size pointer passed in
unsigned char *cache_retrieval(Cache *cache, char *url, size_t *server_response_size){
    // Retrieve the cached entry and update time last accessed
    int n = get_cache_entry_index(cache, url);
    if(n < 0) {
        printf("Error, cached item should exist, but is missing. Exitting...\n");
        exit(EXIT_FAILURE);
    }
    CacheEntry *cached_entry = cache->entries[n];
    clock_gettime(CLOCK_REALTIME, &(cached_entry->time_accessed));

    // Modify response to incorporate age
    unsigned char *server_response_with_age = add_age_header(cached_entry, server_response_size);

    // Return response (with age) and modified response size
    return server_response_with_age;
}

// Given cached entry, return copy of cached response 
// with "Age" field incorporated
unsigned char *add_age_header(CacheEntry *cached_entry, size_t *server_response_size) {
    // Get age
    int age = get_age(cached_entry);
    
    // Create age header
    char *age_header = malloc(20);
    sprintf(age_header, "\r\nAge: %d", age);
    // printf("Age header: %s\n", age_header);
    // printf("Length of age string: %d\n", strlen(age_header));

    // Locate the end of the HTTP headers marked by "\r\n\r\n"
    unsigned char *server_response = cached_entry->server_response;
    unsigned char *server_response_header_end = (unsigned char *)strstr((const char *)server_response, "\r\n\r\n");

    // Copy contents of original server response into new buffer with
    // age header added
    size_t header_length = server_response_header_end - server_response;
    size_t body_length   = cached_entry->server_response_size - header_length;

    unsigned char *server_response_with_age = malloc(cached_entry->server_response_size + strlen(age_header));
    
    memcpy(server_response_with_age, server_response, header_length);
    memcpy(server_response_with_age + header_length, age_header, strlen(age_header));
    memcpy(server_response_with_age + header_length + strlen(age_header), server_response_header_end, body_length);

    // Return new buffer with age header added
    *server_response_size = cached_entry->server_response_size + strlen(age_header);
    return server_response_with_age;
}





//-------------------------------------------------------------------------------------------------
