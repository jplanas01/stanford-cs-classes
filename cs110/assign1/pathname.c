
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/* Function: pathname_lookup
 * -------------------------
 *  Fetch the inode associated with the specified pathname.
 *
 *  Assumes fs is properly initialized and pathname is absolute and a valid C
 *  string.
 *
 *  Returns the inode number for the file, or -1 if an error ocurred (disk could
 *  not be read, file not found, etc.)
 */
int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    // Make sure only handling absolute paths
    if (pathname[0] != '/') {
        return -1;
    }

    //char *path = strdup(pathname);
    char path[strlen(pathname) + 1];
    strcpy(path, pathname);

    char *pch = path + 1;
    char *token = path + 1;
    int inode_num = ROOT_INUMBER; // Start at root directory
    int result = -1;
    struct direntv6 entry;

    /* Make the initial filename the empty string so that it'll match the root
     * directory.
     */
    memset(&entry, '\0', sizeof(struct direntv6));

    /* Crawl over '/'-delimited filenames to find specified file.
     */
    while (pch != NULL && *token != '\0') {
        token = strsep(&pch, "/");
        result = directory_findname(fs, token, inode_num, &entry);
        if (result != -1) {
            inode_num = entry.d_inumber;
        } else {
            break;
        }
    }

    // Ensure matched file has the right name
    if (strncmp(entry.d_name, strrchr(pathname, '/') + 1, 14) == 0) {
        result = inode_num;
    } else {
        result = -1;
    }

    //free(path);
    return result;
}
