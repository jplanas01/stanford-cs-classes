#ifndef _CACHEMEM_H_
#define _CACHEMEM_H_
#include "assign1/diskimg.h"
/**
 * The main export of the cachemem module is the memory for the cache
 * pointed to by the following global variables:
 *
 * cacheMemSizeInKB - The size of the cache memory in kiloytes. 
 * cacheMemPtr      - Starting address of the cache memory. 
 */

extern int cacheMemSizeInKB;
extern void *cacheMemPtr;

#define CACHEMEM_MAX_SIZE (64*1024*1024)
#define FD_CACHE_LENGTH 4

int CacheMem_Init(int sizeInKB);

// This struct makes it easier to handle the caches, since instead of having
// arrays within arrays, it's an array of structs.
typedef struct sector_t {
    unsigned char data[DISKIMG_SECTOR_SIZE];
} sector_t;

typedef struct cache_data_t {
    int sector_num;
    int valid_bytes;
} cache_data_t;
#endif // _CACHEMEM_H_
