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
    for(int i =0; i < cache->capacity; i++) {
        cache->entries[i] = NULL;
    }
    return cache;
}

// Given cache pointer and server response, create a 
// new cache entry, add to cache, and update cache accordingly
void cache_insert(Cache* cache, char* url, unsigned char *server_response, size_t *server_response_size) {
    CacheEntry *cache_entry = CacheEntry_create(url, server_response, server_response_size);
    cache->entries[cache->size] = cache_entry;
    cache->size += 1;
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
    int n = get_cache_entry_index(cache, url);
    if(n < 0) {
        printf("Cached entry not found\n");
        return false;
    } else {
        printf("Cached entry found at index: %d\n", n);
        return true;
    }
}

// Given a cache and the URL of a cache entry known to exist in the cache, 
// return a pointer to the response stored in the cache entry, update
// the retrieval field, and assign the size of the server response
// to the server_response_size pointer passed in
unsigned char *cache_retrieval(Cache *cache, char *url, size_t *server_response_size){
    // Retrieve the cached entry
    int n = get_cache_entry_index(cache, url);
    if(n < 0) {
        printf("Error, cached item should exist, but is missing. Exitting...\n");
        exit(EXIT_FAILURE);
    }
    CacheEntry *cached_entry = cache->entries[n];

    // Modify response to incorporate age
    unsigned char *server_response_with_age = add_age_header(cached_entry, server_response_size);
    // TODO: Add retrieved status

    // Return response (with age) and modified response size
    return server_response_with_age;
}

// Given cached entry, modify the cached response to incorporate
// a new "Age" field
unsigned char *add_age_header(CacheEntry *cached_entry, size_t *server_response_size) {
    // Get age
    int age = get_age(cached_entry);
    
    // Create age header
    char *age_header = malloc(20);
    sprintf(age_header, "\r\nAge: %d", age);
    printf("Age header: %s\n", age_header);
    printf("Length of age string: %d\n", strlen(age_header));

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

    // Free the original server response
    // free(cached_entry->server_response);
    // cached_entry->server_response = server_response_with_age;
    *server_response_size = cached_entry->server_response_size + strlen(age_header);
    return server_response_with_age;
}

// Given cached entry, return current age in seconds as integer
int get_age(CacheEntry *cached_entry) {
    // Calculate age
    struct timespec time_added = cached_entry->time_added;
    struct timespec time_current;
    struct timespec time_diff;
    // Calculate time difference
    clock_gettime(CLOCK_REALTIME, &time_current);
    timespec_diff(time_added, time_current, &time_diff);
    int age = time_diff.tv_sec;
    return(age);
}

// Given two timespec structures, store the difference in timespec diff
void timespec_diff(struct timespec start, struct timespec end, struct timespec *diff) {
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        diff->tv_sec = end.tv_sec - start.tv_sec - 1;
        diff->tv_nsec = 1000000000L + end.tv_nsec - start.tv_nsec;
    } else {
        diff->tv_sec = end.tv_sec - start.tv_sec;
        diff->tv_nsec = end.tv_nsec - start.tv_nsec;
    }
}




//-------------------------------------------------------------------------------------------------
