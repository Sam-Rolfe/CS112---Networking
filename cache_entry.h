//----HEADER---------------------------------------------------------------------------------------
// Author:      Sam Rolfe
// Date:        September 2024
// Script:      cache_entry.h 
// Usage:       Header file for cache_entry
//*************************************************************************************************
#ifndef CACHE_ENTRY_H
#define CACHE_ENTRY_H

#include <time.h>
#include <stdbool.h>

#define URL_MAX_CHARACTERS 100                // Max size 100 characters
#define HTTP_RESPONSE_MAX_SIZE 10*1024*1024   // Max size 10 MB

// ----GLOBAL VARIABLES----------------------------------------------------------------------------
typedef struct CacheEntry{
    char url[URL_MAX_CHARACTERS+1];
    unsigned char *server_response;
    size_t server_response_size;
    struct timespec time_added;
    int max_age;
} CacheEntry;

//----FUNCTIONS------------------------------------------------------------------------------------

CacheEntry *CacheEntry_create(char* url, unsigned char* server_response, size_t *server_response_size);
int get_max_age(unsigned char *server_response);
int get_age(CacheEntry *cached_entry);
bool cache_entry_valid(CacheEntry* cache_entry);
void timespec_diff(struct timespec start, struct timespec end, struct timespec *diff);

//----MAIN-----------------------------------------------------------------------------------------

#endif
//-------------------------------------------------------------------------------------------------
