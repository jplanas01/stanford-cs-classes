#include <stdio.h>
#include <assert.h>

#include "inode.h"
#include "diskimg.h"

#include "ino.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_DIRECT_SIZE (DISKIMG_SECTOR_SIZE * 8) // 8 blocks * 512 bytes/block
#define INDIRECT_SECTORS (DISKIMG_SECTOR_SIZE / sizeof(uint16_t)) // Number of sectors in indirect blocks

/* Function inode_iget
 * -------------------
 *  Fetches the inode struct associated with a specific inumber (inode number)
 *  on the Unix filesystem specified by the struct fs and copies it to inp.
 *
 * This function assumes that fs has been initialized properly and that there is
 * enough space in inp to hold the struct.
 *
 *  Returns -1 on error and 0 on success.
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    // Check for invalid inode number
    if (inumber < ROOT_INUMBER || (unsigned int)inumber >
            (DISKIMG_SECTOR_SIZE/sizeof(struct inode)) * fs->superblock.s_isize)
    {
        return -1;
    }

    
    unsigned int sector_num = INODE_START_SECTOR + (inumber - ROOT_INUMBER) *
                              sizeof(struct inode) / DISKIMG_SECTOR_SIZE;
    unsigned int offset =  (inumber - ROOT_INUMBER) * sizeof(struct inode) %
                           DISKIMG_SECTOR_SIZE;
    char sector[DISKIMG_SECTOR_SIZE];
    struct inode *new_node = (struct inode *)((char *)sector + offset);

    diskimg_readsector(fs->dfd, sector_num, sector);

    if ((new_node->i_mode & IALLOC) == 0) {
        return -1;
    }
    
    //memcpy(inp, new_node, sizeof(struct inode));
    *inp = *new_node;
    return 0;
}

/* Function: single_indirect
 * -------------------------
 *  Gets the sector number associated with a specific block number (block_num)
 *  for the specified inode (stored in inp) in the filesystem specified by fs.
 *
 *  Assumes properly initialized fs and inp and that block_num is indeed in one
 *  of the indirect blocks.
 *
 *  Returns the sector number on disk that corresponds to the given block
 *  number.
 */
static int single_indirect(struct unixfilesystem *fs, struct inode *inp, int block_num) {
    unsigned int place = block_num / INDIRECT_SECTORS;
    unsigned int sector = inp->i_addr[place];
    unsigned int sector_num = block_num % INDIRECT_SECTORS;
    uint16_t blocks[INDIRECT_SECTORS];

    if (diskimg_readsector(fs->dfd, sector, &blocks) == -1) {
        return -1;
    }

    return blocks[sector_num];
}

/* Function: double_indirect
 * -------------------------
 *  The same as single_indirect, except for doubly indirect sector numbers.
 *  Assumptions and return values are the same.
 */

static int double_indirect(struct unixfilesystem *fs, struct inode *inp, int block_num) {
    uint16_t indirect_blocks[256];
    uint16_t sector_nums[256];
    const unsigned int base_block = INDIRECT_SECTORS * 7;
    unsigned int indirect_loc = (block_num - base_block) / (256);
    unsigned int sector = (block_num - base_block) % 256;
    
    if (diskimg_readsector(fs->dfd, inp->i_addr[7], &indirect_blocks) == -1) {
        return -1;
    }
    if (diskimg_readsector(fs->dfd, indirect_blocks[indirect_loc], &sector_nums) == -1) {
        return -1;
    }

    return sector_nums[sector];
}

int is_valid_blocknum(int block_num, struct inode *inp) {
    return (block_num * 512 <= inode_getsize(inp) && block_num >= 0);
}

/* Function: inode_indexlookup
 * ---------------------------
 *  Finds the sector number associated with a file's block number. Most of the
 *  heavy lifting is done in single_indirect and double_indirect. This function
 *  handles error checking, dispatching, and direct blocks.
 *
 *  Assumes properly initialized filesystem in fs and proper struct in inp.
 *
 *  Returns the sector number associated with the file's blockNum.
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
    // Check for invalid blockNum
    if (!is_valid_blocknum(blockNum, inp)) {
        return -1;
    }

    // File is small, handle directly
    if ((inp->i_mode & ILARG) == 0) {
        return inp->i_addr[blockNum];
    }

    unsigned int place = blockNum / 256;
    if (place < 7) {
        return single_indirect(fs, inp, blockNum);
    } else {
        return double_indirect(fs, inp, blockNum);
    }
}

/* Function: inode_getsize
 * -----------------------
 *  Returns the file size as specified in the two fields of inp.
 *  Assumes proper inp.
 */
int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}

