#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>
#include <assert.h>
#include "diskimg.h"
#include "disksim.h"
#include "debug.h"
#include <stdlib.h>
#include "cachemem.h"

#include <string.h>

static uint64_t numreads, numwrites;

static size_t cache_entries;
static cache_data_t *cached_sectors;
static sector_t *cache_data;

/** 
 * Opens a disk image for I/O.  Returns an open file descriptor, or -1 if
 * unsuccessful  
 */
int diskimg_open(char *pathname, int readOnly) {
    cache_entries = (cacheMemSizeInKB * 1024 - sizeof(sector_t) * FD_CACHE_LENGTH)/ DISKIMG_SECTOR_SIZE;
    cached_sectors = malloc(cache_entries * sizeof(cache_data_t));
    cache_data = (sector_t *)((char *)cacheMemPtr + sizeof(sector_t) * FD_CACHE_LENGTH);

    for (size_t i = 0; i < cache_entries; i++) {
        cached_sectors[i].sector_num = -1;
    }

  return disksim_open(pathname, readOnly);
}

/**
 * Returns the size of the disk image in bytes, or -1 if unsuccessful.
 */
int diskimg_getsize(int fd) {
  return disksim_getsize(fd);
}

/**
 * Read the specified sector from the disk.  Return number of bytes read, or -1
 * on error.
 */
int diskimg_readsector(int fd, int sectorNum, void *buf) {
  numreads++;

  size_t bucket = sectorNum % cache_entries;

  // Cache hit
  if (sectorNum == cached_sectors[bucket].sector_num) {
      memcpy(buf, &cache_data[bucket].data, cached_sectors[bucket].valid_bytes);
      return cached_sectors[bucket].valid_bytes;
  }

  int result = disksim_readsector(fd, sectorNum, buf); 
  if (result > -1) {
      memcpy(&cache_data[bucket].data, buf, result);
      cached_sectors[bucket].sector_num = sectorNum;
      cached_sectors[bucket].valid_bytes = result;
  }
  return result;
}

/**
 * Writes the specified sector to the disk.  Return number of bytes written,
 * -1 on error.
 */
int diskimg_writesector(int fd, int sectorNum, void *buf) {
  numwrites++;
  return disksim_writesector(fd, sectorNum, buf);
}

/**
 * Cleans up from a previous diskimg_open() call.  Returns 0 on success, -1 on
 * error
 */
int diskimg_close(int fd) {
    free(cached_sectors);
  return disksim_close(fd);
}

void diskimg_dumpstats(FILE *file) {
  fprintf(file, "Diskimg: %"PRIu64" reads, %"PRIu64" writes\n",
          numreads, numwrites);
}
