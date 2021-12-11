/**
 * fileops.c  -  This module provides an Unix like file absraction
 * on the assign1 file system access code
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>

#include "fileops.h"
#include "assign1/pathname.h"
#include "assign1/unixfilesystem.h"
#include "diskimg.h"
#include "assign1/inode.h"
#include "assign1/file.h"
#include "assign1/chksumfile.h"
#include "cachemem.h"

#define MAX_FILES 64

static uint64_t numopens = 0;
static uint64_t numreads = 0;
static uint64_t numgetchars = 0;
static uint64_t numisfiles = 0;

/**
 * Table of open files.
 */
static struct {
    char *pathname;    // absolute pathname NULL if slot is not used.
    int  cursor;       // Current position in the file
    int last_block;    // Number of last read file block
    int inumber;       // INode number for the file
    int is_alloc;      // Is 1 if the inumber is allocated
    int size;          // File size in bytes
} openFileTable[MAX_FILES];

static struct unixfilesystem *unixfs;
static sector_t *sector_cache;
static int cache_inums[8];

/**
 * Initialize the fileops module for the specified disk.
 */
void *Fileops_init(char *diskpath) {
    sector_cache = cacheMemPtr;
    memset(cache_inums, 0, sizeof(cache_inums));
    memset(openFileTable, 0, sizeof(openFileTable));
    memset(sector_cache, 0, sizeof(sector_t) * FD_CACHE_LENGTH);
    int fd = diskimg_open(diskpath, 1);
    if (fd < 0) {
        fprintf(stderr, "Can't open diskimagePath %s\n", diskpath);
        return NULL;
    }

    unixfs = unixfilesystem_init(fd);
    if (unixfs == NULL) {
        diskimg_close(fd);
        return NULL;
    }
    return unixfs;
}

/**
 * Open the specified absolute pathname for reading. Returns -1 on error;
 */
int Fileops_open(char *pathname) {
    numopens++;
    int inumber = pathname_lookup(unixfs,pathname);
    if (inumber < 0) {
        return -1; // File not found
    }

    int fd;
    for (fd = 0; fd < MAX_FILES; fd++) {
        if (openFileTable[fd].pathname == NULL) break;
    }
    if (fd >= MAX_FILES) {
        return -1;  // No open file slots
    }
    openFileTable[fd].pathname = strdup(pathname); // Save our own copy
    openFileTable[fd].cursor = 0;
    openFileTable[fd].last_block = -1;
    openFileTable[fd].inumber = -2;
    openFileTable[fd].is_alloc = 0;
    return fd;
}

/* Loads information about a file specified by its fd number into the open file
 * struct. Returns 0 if all the information was loaded, <0 otherwise.
 */
static int load_fd_info(int fd) {
    int inumber, err;
    struct inode in;

    inumber = pathname_lookup(unixfs, openFileTable[fd].pathname);
    openFileTable[fd].inumber = inumber;

    err = inode_iget(unixfs, inumber,&in);
    if (err < 0) {
        return err;
    }

    openFileTable[fd].is_alloc = (in.i_mode & IALLOC);
    openFileTable[fd].size = inode_getsize(&in);
    return 0;
}

/*
 * Fetch the next character from the file. Return -1 if at end of file.
 */
int Fileops_getchar(int fd) {
    int inumber, result;
    unsigned char buf[DISKIMG_SECTOR_SIZE];
    int bytesMoved;
    int err, size, allocated;
    int blockNo, blockOffset;

    numgetchars++;

    if (openFileTable[fd].pathname == NULL)
        return -1;  // fd not opened.


    if (openFileTable[fd].inumber == -2) {
        load_fd_info(fd);
    }
    inumber = openFileTable[fd].inumber;
    allocated = openFileTable[fd].is_alloc;
    size = openFileTable[fd].size;


    if (inumber < 0) {
        return inumber; // Can't find file
    }

    if (!allocated) {
        return -1;
    }

    if (openFileTable[fd].cursor >= size) return -1; // Finished with file

    blockNo = openFileTable[fd].cursor / DISKIMG_SECTOR_SIZE;
    blockOffset =  openFileTable[fd].cursor % DISKIMG_SECTOR_SIZE;
    int bucket = fd % FD_CACHE_LENGTH;

    if (blockNo != openFileTable[fd].last_block || cache_inums[bucket] != inumber) {
        bytesMoved = file_getblock(unixfs, inumber,blockNo,buf);
        if (bytesMoved < 0) {
            return -1;
        }
        memcpy(&sector_cache[bucket].data, buf, DISKIMG_SECTOR_SIZE);
        cache_inums[bucket] = inumber;
        result = (int)(buf[blockOffset]);
    } else {
        result = (int)(sector_cache[bucket].data[blockOffset]);
    }
    openFileTable[fd].last_block = blockNo;
    openFileTable[fd].cursor += 1;
    return result;
}

/**
 * Implement the Unix read system call. Number of bytes returned.  Return -1 on
 * err.
 */
int Fileops_read(int fd, char *buffer, int length) {
    numreads++;
    int i;
    for (i = 0; i < length; i++) {
        int ch = Fileops_getchar(fd);
        if (ch == -1) break;
        buffer[i] = ch;
    }
    return i;
}

/**
 * Return the current position in the file.
 */
int Fileops_tell(int fd) {
    if (openFileTable[fd].pathname == NULL)
        return -1;  // fd not opened.
    return openFileTable[fd].cursor;
}

/**
 * Close the files - return the resources
 */

int Fileops_close(int fd) {
    if (openFileTable[fd].pathname == NULL)
        return -1;  // fd not opened.
    free(openFileTable[fd].pathname);
    openFileTable[fd].pathname = NULL;
    return 0;
}

/**
 * Return true if specified pathname is a regular file.
 */
int Fileops_isfile(char *pathname) {
    numisfiles++;
    int inumber = pathname_lookup(unixfs, pathname);
    if (inumber < 0) {
        return 0;
    }

    struct inode in;
    int err = inode_iget(unixfs, inumber, &in);
    if (err < 0) return 0;

    if (!(in.i_mode & IALLOC) || ((in.i_mode & IFMT) != 0)) {
        // Not allocated or not a file
        return 0;
    }
    return 1; // Must be a file
}

void Fileops_Dumpstats(FILE *file) {
    fprintf(file,
            "Fileops: %"PRIu64" opens, %"PRIu64" reads, "
            "%"PRIu64" getchars, %"PRIu64 " isfiles\n",
            numopens, numreads, numgetchars, numisfiles);
}

