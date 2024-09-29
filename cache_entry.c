//----HEADER---------------------------------------------------------------------------------------
// Author:      Sam Rolfe
// Date:        September 2024
// Script:      cache_entry.c
// Usage:       Implementation file for cache
//*************************************************************************************************
#include "cache_entry.h"

#define _POSIX_C_SOURCE 199309L   // POSIX compliance for CLOK_REALTIME
#define DEFAULT_MAX_AGE 60*60     // Set max-age to 1 hour by default

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// ----GLOBAL VARIABLES----------------------------------------------------------------------------


//----FUNCTIONS------------------------------------------------------------------------------------
// Given pointer to server response, create CacheEntry
// with fields populated appropriately. Return pointer
// to populated CacheEntry to caller
CacheEntry *CacheEntry_create(char* url, unsigned char* server_response, size_t *server_response_size) {
    CacheEntry *cache_entry = malloc(sizeof(CacheEntry));
    strcpy(cache_entry->url, url);
    cache_entry->server_response = server_response;
    cache_entry->server_response_size = *server_response_size;
    cache_entry->max_age = get_max_age(server_response);
    clock_gettime(CLOCK_REALTIME, &(cache_entry->time_added));
    printf("Creating new cache entry. max-age: %d\n", cache_entry->max_age);
    return cache_entry;
}

// Given server response, return max-age of present in
// header, else return DEFAULT_MAX_AGE
int get_max_age(unsigned char *server_response) {
    // Search for Cache-Control in response header.
    // If not present, return default
    const char *char_control = strstr(server_response, "cache-control");
    if(char_control == NULL) {
        return DEFAULT_MAX_AGE;
    }
    const char *max_age = strstr(char_control, "max-age=");
    if(max_age == NULL) {
        return DEFAULT_MAX_AGE;
    }

    // If present, convert to int and return
    max_age += strlen("max_age=");
    int max_age_value = atoi(max_age);
    return(max_age_value);
}

// Given cache and index of cache_entry, return
// true if valid and false if stale
bool cache_entry_valid(CacheEntry* cache_entry) {
    int age = get_age(cache_entry);
    int max_age = cache_entry->max_age;
    if(age >= max_age) {
        return false;
    } else {
        return true;
    }
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


//----MAIN-----------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
