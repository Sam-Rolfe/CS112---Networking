//----HEADER---------------------------------------------------------------------------------------
// Author:      Sam Rolfe
// Date:        September 2024
// Script:      cache_entry.h 
// Usage:       Header file for cache_entry
//*************************************************************************************************
#ifndef CACHE_ENTRY_H
#define CACHE_ENTRY_H

#define URL_MAX_CHARACTERS 100                // Max size 100 characters
#define HTTP_RESPONSE_MAX_SIZE 10*1024*1024   // Max size 10 MB

// ----GLOBAL VARIABLES----------------------------------------------------------------------------
typedef struct CacheEntry{
     char url[URL_MAX_CHARACTERS+1];
     unsigned char http_response[HTTP_RESPONSE_MAX_SIZE];
} CacheEntry;

//----FUNCTIONS------------------------------------------------------------------------------------


//----MAIN-----------------------------------------------------------------------------------------

#endif
//-------------------------------------------------------------------------------------------------
