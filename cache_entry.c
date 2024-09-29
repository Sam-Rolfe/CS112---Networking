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

//----MAIN-----------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
