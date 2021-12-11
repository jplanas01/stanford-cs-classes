/*
 * pathstore.c  - Store pathnames for indexing
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>

#include "index.h"
#include "fileops.h"
#include "pathstore.h"
#include "assign1/chksumfile.h"

typedef struct PathstoreElement {
    char *pathname;
    struct PathstoreElement *nextElement;
    int checksum_loaded;
    unsigned char checksum[CHKSUMFILE_SIZE]; // Should be unsigned to ensure
                                             // accurate memory copies.
} PathstoreElement;

static uint64_t numdifferentfiles = 0;
static uint64_t numsamefiles = 0;
static uint64_t numdiffchecksum = 0;
static uint64_t numdups = 0;
static uint64_t numcompares = 0;
static uint64_t numstores = 0;

static int SameFileIsInStore(Pathstore *store, char *pathname);
static int IsSameFile(Pathstore *store, char *pathname1, char *pathname2);

Pathstore* Pathstore_create(void *fshandle) {
    Pathstore *store = malloc(sizeof(Pathstore));
    if (store == NULL)
        return NULL;

    store->elementList = NULL;
    store->fshandle = fshandle;
    return store;
}

/**
 * Free up all the sources allocated for a pathstore.
 */
void Pathstore_destory(Pathstore *store) {
    PathstoreElement *e = store->elementList;

    while (e) {
        PathstoreElement *next = e->nextElement;
        free(e->pathname);
        free(e);
        e = next;
    }
    free(store);
}

/**
 * Store a pathname in the pathname store.
 */
char *Pathstore_path(Pathstore *store, char *pathname, int discardDuplicateFiles) {
    numstores++;
    if (discardDuplicateFiles) {
        if (SameFileIsInStore(store,pathname)) {
            numdups++;
            return NULL;
        }
    }

    PathstoreElement *e = malloc(sizeof(PathstoreElement));
    if (e == NULL) {
        return NULL;
    }

    e->checksum_loaded = 0;
    e->pathname = strdup(pathname);
    if (e->pathname == NULL) {
        free(e);
        return NULL;
    }
    e->nextElement = store->elementList;
    store->elementList = e;
    return e->pathname;
}

/**
 * Is this file the same as any other one in the store
 */
static int SameFileIsInStore(Pathstore *store, char *pathname) {
    PathstoreElement *e = store->elementList;
    while (e) {
        if (IsSameFile(store, pathname, e->pathname)) {
            return 1;  // In store already
        }
        e = e->nextElement;
    }
    return 0; // Not found in store
}

/* Find an element in the Pathstore by its pathname, store a pointer to that
 * element if it was found. Returns 0 if found, -1 otherwise.
 */
static int find_file_by_pathname(Pathstore *store, char *pathname, PathstoreElement **result) {
    PathstoreElement *e = store->elementList;
    while (e) {
        if (strcmp(pathname, e->pathname) == 0) {
            *result = e;
            return 0;
        }
        e = e->nextElement;
    }
    return -1; // Not found in store
}

/* Returns a pointer to the checksum of a file in the store specified by the
 * given pathname.
 * Assumes pathname is a valid path.
 * Returns a pointer to the checksum or NULL if an error ocurred.
 */
static unsigned char *get_checksum(Pathstore *store, char *pathname) {
    /* For each path, check if we have it in the store; if we do, load the
     * checksum. Otherwise, calculate the checksum by adding it to the path
     * store.
     */
    char *err;
    PathstoreElement *e = NULL;
    if (find_file_by_pathname(store, pathname, &e) < 0) {
        // Can get away with not checking for duplicates because we already
        // know the path isn't in the store
        err = Pathstore_path(store, pathname, 0);
        if (err == NULL) {
            return NULL; // Name storing failed, panic
        }
        find_file_by_pathname(store, pathname, &e);
    }

    // Cache checksum if necessary
    if (!e->checksum_loaded) {
        struct unixfilesystem *fs = (struct unixfilesystem *) (store->fshandle);
        int err = chksumfile_bypathname(fs, pathname, &(e->checksum)); 
        if (err < 0) {
            fprintf(stderr,"Can't checksum path %s while storing\n", pathname);
            return NULL;
        }
        e->checksum_loaded = 1;
    }
    return e->checksum;
}

/**
 * Do the two pathnames refer to a file with the same contents.
 */
static int IsSameFile(Pathstore *store, char *pathname1, char *pathname2) {
    struct unixfilesystem *fs = (struct unixfilesystem *) (store->fshandle);
    numcompares++;
    if (strcmp(pathname1, pathname2) == 0) {
        return 1; // Same pathname must be same file.
    }

    unsigned char *chksum1, *chksum2;

    chksum1 = get_checksum(store, pathname1);
    chksum2 = get_checksum(store, pathname2);
    if (chksum1 == NULL || chksum2 == NULL) {
        return 0; // Error loading checksums
    }

    if (chksumfile_compare(chksum1, chksum2) == 0) {
        numdiffchecksum++;
        return 0;  // Checksum mismatch, not the same file
    }

    // Checksums match, do a content comparison
    int fd1 = Fileops_open(pathname1);
    if (fd1 < 0) {
        fprintf(stderr, "Can't open path %s\n", pathname1);
        return 0;
    }

    int fd2 = Fileops_open(pathname2);
    if (fd2 < 0) {
        Fileops_close(fd1);
        fprintf(stderr, "Can't open path %s\n", pathname2);
        return 0;
    }

    int ch1, ch2;
    do {
        ch1 = Fileops_getchar(fd1);
        ch2 = Fileops_getchar(fd2);

        if (ch1 != ch2) {
            break; // Mismatch - exit loop with ch1 != ch2
        }
    } while (ch1 != -1);

    // if files match then ch1 == ch2 == -1
    Fileops_close(fd1);
    Fileops_close(fd2);

    if (ch1 == ch2) {
        numsamefiles++;
    } else {
        numdifferentfiles++;
    }

    return ch1 == ch2;
}

void Pathstore_Dumpstats(FILE *file) {
    fprintf(file,
            "Pathstore:  %"PRIu64" stores, %"PRIu64" duplicates\n"
            "Pathstore2: %"PRIu64" compares, %"PRIu64" checksumdiff, "
            "%"PRIu64" comparesuccess, %"PRIu64" comparefail\n",
            numstores, numdups, numcompares, numdiffchecksum,
            numsamefiles, numdifferentfiles);
}
