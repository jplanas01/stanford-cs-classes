#include <stdio.h>
#include <assert.h>

#include "file.h"
#include "inode.h"
#include "diskimg.h"

/* Function: file_getblock
 * -----------------------
 *  Copies the bytes stored in the file associated with inode inumber's blockNum
 *  block on disk into buf.
 *
 *  Assumes properly initialized fs and that buf will have at least 512 bytes of
 *  space.
 *
 * Returns the number of valid bytes copied into buf or -1 on error.
 */
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    struct inode file_node;
    if (inode_iget(fs, inumber, &file_node) == -1) {
        return -1;
    }

    int sector = inode_indexlookup(fs, &file_node, blockNum);
    if (sector == -1) {
        return -1;
    }

    if (diskimg_readsector(fs->dfd, sector, buf) == -1) {
        return -1;
    }
    
    int size = inode_getsize(&file_node);
    /* A block containing less than 512 bytes of data will only happen at the
     * file's last block. If counting this block we have more space allocated
     * than is valid, we must be at the last block and need to find the actual
     * used space.
     */
    if ((blockNum + 1) * DISKIMG_SECTOR_SIZE > size) {
        return inode_getsize(&file_node) % DISKIMG_SECTOR_SIZE;
    }
    return DISKIMG_SECTOR_SIZE;
}
