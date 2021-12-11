#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define ENTS_PER_SECTOR (DISKIMG_SECTOR_SIZE / sizeof(struct direntv6))

/* Function: do_findname
 * ---------------------
 *  Searches through all the blocks of a directory to find the file with the
 *  specified name, if any.
 *
 *  Assumes fs amd dir_node is properly initialized, and that there is enough
 *  space in dir_ent to hold the result.
 *
 *  Returns 0 on success, -1 on error or file not found. The matching file
 *  directory entry is copied into dir_ent.
 */
static int do_findname(struct unixfilesystem *fs, struct inode *dir_node, int inum,
        const char *name, struct direntv6 *dir_ent) {
    unsigned int filesize = inode_getsize(dir_node);

    // Account for fact that have one block too many for blocks of size multiple
    // of 512
    unsigned int num_blocks;
    if (filesize % DISKIMG_SECTOR_SIZE == 0) {
        num_blocks = filesize / DISKIMG_SECTOR_SIZE; 
    } else {
        num_blocks = filesize / DISKIMG_SECTOR_SIZE + 1;
    }

    for (unsigned int block = 0; block < num_blocks; block++) {
        struct direntv6 entries[DISKIMG_SECTOR_SIZE / sizeof(struct direntv6)];
        int bytes = file_getblock(fs, inum, block, entries);

        if (bytes == -1) {
            return -1;
        }

        for (unsigned int i = 0; i < bytes / sizeof(struct direntv6); i++) {
            if (strncmp(name, entries[i].d_name, 14) == 0) {
                *dir_ent = entries[i];
                return 0;
            }
        }
    }
    return -1;
}

/* Function: directory_findname
 * ----------------------------
 *  Searches through the entries stored under a directory specified by an inode
 *  number in dirinumber for a specific filename.
 *
 *  Assumes fs is properly initialized, dirEnt has enough space to store the
 *  result.
 *
 *  Returns -1 on error or not found, 0 on success. The matching directory
 *  entry, if found, is copied into dirEnt.
 */
int directory_findname(struct unixfilesystem *fs, const char *name,
		       int dirinumber, struct direntv6 *dirEnt) {

    struct inode dir_node;
    if (inode_iget(fs, dirinumber, &dir_node) == -1) {
        return -1;
    }

    if ((dir_node.i_mode & IFDIR) == 0) {
        // Not a directory
        return -1;
    }

    return do_findname(fs, &dir_node, dirinumber, name, dirEnt);
}
