#include "libTinyFS.h"
#include "tinyFS_errno.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void debug_inode_table() {
    printf("=== INODE TABLE ===\n");
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_directory[i].used) {
            printf("[INODE %d] Name: '%s' | Parent: %d | First Block: %d | Next Sibling: %d | Is Dir: %d\n",
                   i, inode_directory[i].inode.filename, inode_directory[i].inode.par_inode,
                   inode_directory[i].inode.first_block, inode_directory[i].inode.next_sibling,
                   inode_directory[i].inode.is_dir);
        }
    }
    printf("====================\n");
}

// allocates a new data block for file data.
static int allocate_data_block() {
    int total_blocks = DEFAULT_DISK_SIZE / BLOCKSIZE;
    if (next_free_data_block >= total_blocks) {
        return -1;
    }
    return next_free_data_block++;
}

// writes the inode from inode_directory[index] to disk
// the inode is stored in block (1 + index)
static int write_inode_to_disk(int index) {
    int block_no = index + 1;
    return writeBlock(0, block_no, &inode_directory[index].inode);
}

// reads all inode blocks from disk (blocks 1 to MAX_INODES) into inode_directory
static void load_inode_directory() {
    for (int i = 0; i < MAX_INODES; i++) {
        int block_no = i + 1;
        inode_t temp;
        if (readBlock(0, block_no, &temp) == 0 && temp.type == 2) {
            inode_directory[i].used = 1;
            inode_directory[i].inode = temp;
            inode_directory[i].block_number = block_no;
        } else {
            inode_directory[i].used = 0;
        }
    }
}

int find_inode_by_name(const char *path) {
    if (strcmp(path, "/") == 0) return 0; // Root inode is always at index 0

    char temp[256], *token;
    strncpy(temp, path, 255);
    temp[255] = '\0';

    int cur_inode = 0;  // Start from root directory

    token = strtok(temp, "/");
    while (token != NULL) {
        int found = -1;
        int cur_idx = inode_directory[cur_inode].inode.first_block; // First file/dir in current directory

        while (cur_idx != -1) {
            printf("[DEBUG] find_inode_by_name: Checking inode %d (%s) under parent '%s'\n",
                   cur_idx, inode_directory[cur_idx].inode.filename, inode_directory[cur_inode].inode.filename);

            if (strcmp(inode_directory[cur_idx].inode.filename, token) == 0) {
                found = cur_idx;
                break;
            }

            cur_idx = inode_directory[cur_idx].inode.next_sibling;  // **Check siblings**
        }

        if (found == -1) {
            printf("[DEBUG] find_inode_by_name: '%s' NOT found under '%s'\n",
                   token, inode_directory[cur_inode].inode.filename);
            return -1;
        }

        cur_inode = found;  // Move to next directory level
        token = strtok(NULL, "/");
    }

    printf("[DEBUG] find_inode_by_name: Resolved '%s' to inode %d\n", path, cur_inode);
    return cur_inode;
}


// allocates a free inode slot in inode_directory
// returns the index allocated, or -1 if none free
static int allocate_inode_slot() {
    for (int i = 0; i < MAX_INODES; i++) {
        if (!inode_directory[i].used)
            return i;
    }
    return -1;
}

int split_path(const char *path, char *par_path, char *basename) {
    if (!path || !par_path || !basename) return -1;
    if (strcmp(path, "/") == 0) return -1;  // Root has no parent

    char temp[256];
    strncpy(temp, path, 255);
    temp[255] = '\0';

    char *last_slash = strrchr(temp, '/');
    if (!last_slash || last_slash == temp) {
        // If there's no valid parent, treat it as direct root child
        strcpy(par_path, "/");
        strcpy(basename, last_slash ? last_slash + 1 : temp);
    } else {
        *last_slash = '\0';
        strcpy(par_path, temp);
        strcpy(basename, last_slash + 1);
    }
    return 0;
}

/* tfs_mkfs: format a new tinyFS file system 
initialize all data to 0x00, set magic numbers,
initialize and write superblock and inodes. */
int tfs_mkfs(char *filename, int nBytes) {
    // open or create disk
    if (openDisk(filename, nBytes) < 0) return TFS_ERR_NO_DISK;

    // create superblock and write to block 0
    superblock_t sb;
    memset(&sb, 0, sizeof(superblock_t));
    sb.type = 1;
    sb.magic = 0x44;
    sb.root_inode = 1;
    sb.free_list = MAX_INODES + 1; 
    if (writeBlock(0, 0, &sb) < 0) return TFS_ERR_WRITE_FAIL;

    // initialize all inode blocks (blocks 1 to MAX_INODES) to 0
    inode_t blank;
    memset(&blank, 0, sizeof(inode_t));
    for (int i = 0; i < MAX_INODES; i++) {
        if (writeBlock(0, i + 1, &blank) < 0)
            return TFS_ERR_WRITE_FAIL;
        inode_directory[i].used = 0;
    }

    // initialize root dir block 1
    inode_t root;
    memset(&root, 0, sizeof(inode_t));
    root.type = 2;
    root.is_dir = 1;
    strcpy(root.filename, "/");
    root.size = 0;
    root.first_block = -1;
    root.par_inode = -1;
    root.next_sibling = -1;
    inode_directory[0].used = 1;
    inode_directory[0].inode = root;
    inode_directory[0].block_number = 1;

    if (writeBlock(0, 1, &root) < 0) return TFS_ERR_WRITE_FAIL;

    // reset file descriptor table
    memset(fd_table, 0, sizeof(fd_table));

    // set next free block ptr
    next_free_data_block = MAX_INODES + 1;

    disk_mounted = 1;
    strncpy(mounted_disk, filename, sizeof(mounted_disk));
    return TFS_SUCCESS;
}

/* tfs_mount: mount an existing TinyFS file system
opens disk using disk emulator, reads superblock to 
verify type and magic number, initializes fd table */
int tfs_mount(char *diskname) {
    // open existing disk
    if (openDisk(diskname, 0) < 0) return TFS_ERR_NO_DISK;
    
    // read and verify the superblock.
    superblock_t sb;
    if (readBlock(0, 0, &sb) < 0) return TFS_ERR_READ_FAIL;
    if (sb.type != 1 || sb.magic != 0x44) return TFS_ERR_INVALID_BLOCK; 
    
    // set disk as mounted
    disk_mounted = 1;
    strncpy(mounted_disk, diskname, sizeof(mounted_disk));

    // load persistent inode directory from disk
    load_inode_directory();

    // calculate next free block alloc
    int max_allocation = MAX_INODES;
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_directory[i].used && inode_directory[i].inode.first_block > max_allocation) {
            max_allocation = inode_directory[i].inode.first_block;
        }
    }
    next_free_data_block = max_allocation+ 1;

    // clear and initialize the file descriptor table
    memset(fd_table, 0, sizeof(fd_table));
    
    return TFS_SUCCESS;
}

/* tfs_unmount: unmount the TinyFS file system
close disk and reset in-memory data */
int tfs_unmount(void) {
    if (!disk_mounted) return TFS_ERR_NO_DISK;
    // flush any in-memory metadata changes to disk here
    closeDisk(0);
    // reset system states
    disk_mounted = 0;
    memset(mounted_disk, 0, sizeof(mounted_disk));
    memset(inode_directory, 0, sizeof(inode_directory));
    memset(fd_table, 0, sizeof(fd_table));
    return TFS_SUCCESS;
}
fileDescriptor tfs_openFile(char *name) {
    if (!disk_mounted) return TFS_ERR_NO_DISK;

    // Parse directory path and extract parent path and file name
    char par_path[256], dir_basename[MAX_FILENAME_LEN + 1];
    if (split_path(name, par_path, dir_basename) < 0) return TFS_ERR_INVALID_PATH;
    printf("[DEBUG] tfs_openFile: Parsed Path -> Parent: '%s', Basename: '%s'\n", par_path, dir_basename);

    // Ensure parent directory exists
    int par_inode_idx = find_inode_by_name(par_path);
    if (par_inode_idx < 0 || !inode_directory[par_inode_idx].inode.is_dir) {
        printf("[DEBUG] tfs_openFile: Parent directory '%s' does not exist.\n", par_path);
        return TFS_ERR_INVALID_PATH;
    }

    inode_t *par_inode = &inode_directory[par_inode_idx].inode;
    printf("[DEBUG] tfs_openFile: Parent directory '%s' found at inode %d\n", par_path, par_inode_idx);

    // **🔍 Look for the file inside the parent directory (check sibling list)**
    int inode_index = -1;
    int cur_idx = par_inode->first_block;  // Start searching from the first child

    while (cur_idx != -1) {
        printf("[DEBUG] tfs_openFile: Checking inode %d -> '%s'\n",
               cur_idx, inode_directory[cur_idx].inode.filename);

        if (strcmp(inode_directory[cur_idx].inode.filename, dir_basename) == 0) {
            inode_index = cur_idx;
            printf("[DEBUG] tfs_openFile: Found existing file '%s' at Inode %d\n", dir_basename, inode_index);
            break;
        }
        cur_idx = inode_directory[cur_idx].inode.next_sibling;
    }

    if (inode_index < 0) {
        // 🔹 **File does not exist, allocate a new inode**
        printf("[DEBUG] tfs_openFile: File '%s' does not exist. Allocating new inode...\n", dir_basename);
        inode_index = allocate_inode_slot();
        if (inode_index < 0) return TFS_ERR_NO_FREE_BLOCKS;

        inode_t new_inode;
        memset(&new_inode, 0, sizeof(inode_t));
        new_inode.type = 2;
        new_inode.is_dir = 0;
        strncpy(new_inode.filename, dir_basename, MAX_FILENAME_LEN);
        new_inode.filename[MAX_FILENAME_LEN] = '\0';
        new_inode.size = 0;
        new_inode.par_inode = par_inode_idx;
        new_inode.first_block = -1;
        new_inode.next_sibling = -1;

        // ✅ **Insert into the correct directory's file list**
        if (par_inode->first_block == -1) {
            par_inode->first_block = inode_index; // First file in directory
        } else {
            // Traverse siblings to find the end of the list
            int last_idx = par_inode->first_block;
            while (inode_directory[last_idx].inode.next_sibling != -1) {
                last_idx = inode_directory[last_idx].inode.next_sibling;
            }
            inode_directory[last_idx].inode.next_sibling = inode_index;
        }

        // ✅ **Save inode in memory and on disk**
        inode_directory[inode_index].used = 1;
        inode_directory[inode_index].inode = new_inode;
        inode_directory[inode_index].block_number = inode_index + 1;
        if (write_inode_to_disk(inode_index) < 0 || write_inode_to_disk(par_inode_idx) < 0) return TFS_ERR_WRITE_FAIL;

        printf("[DEBUG] tfs_openFile: Successfully created file '%s' (Inode %d) under '%s'\n", dir_basename, inode_index, par_path);
    }

    // ✅ **Assign a file descriptor**
    int fd = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!fd_table[i].used) {
            fd = i;
            break;
        }
    }
    if (fd < 0) return TFS_ERR_INVALID_FD;

    fd_table[fd].used = 1;
    fd_table[fd].inode_index = inode_index;
    fd_table[fd].file_pointer = 0;

    printf("[DEBUG] tfs_openFile: Opened file '%s' (Inode %d) with FD %d\n", dir_basename, inode_index, fd);
    return fd;
}


/* tfs_closeFile: close an open file
close file by marking file descriptor entry as unused */
int tfs_closeFile(fileDescriptor fd) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used) return TFS_ERR_INVALID_FD;
    // flush any changes (like inode updates) to disk if necessary.
    fd_table[fd].used = 0;
    return TFS_SUCCESS;
}

/* tfs_writeFile: write file content (supports multi-block files) */
int tfs_writeFile(fileDescriptor fd, char *buffer, int size) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used) return TFS_ERR_INVALID_FD;
    
    // calculate how many blocks are needed (each block holds EFFECTIVE_DATA bytes)
    int blocks_needed = (size + EFFECTIVE_DATA - 1) / EFFECTIVE_DATA;
    if (blocks_needed < 1) blocks_needed = 1;

    // allocate an array to hold data block numbers
    // @todo: free old blocks here since using malloc
    int *data_blocks = malloc(sizeof(int) * blocks_needed);
    if (!data_blocks) return TFS_ERR_NO_FREE_BLOCKS;

    for (int i = 0; i < blocks_needed; i++) {
        int blk = allocate_data_block();
        if (blk < 0) {
            free(data_blocks);
            return TFS_ERR_NO_FREE_BLOCKS;
        }
        data_blocks[i] = blk;
    }
    // write each file extent block.
    for (int i = 0; i < blocks_needed; i++) {
        unsigned char block_buf[BLOCKSIZE];
        memset(block_buf, 0, BLOCKSIZE);
        block_buf[0] = 3;
        block_buf[1] = 0x44;

        // next pointer: if not last block, next data block number; else -1.
        int next_ptr = (i < blocks_needed - 1) ? data_blocks[i + 1] : -1;
        memcpy(block_buf + 2, &next_ptr, sizeof(int));

        // copy file data into block
        int offset = i * EFFECTIVE_DATA;
        int bytes_to_copy = EFFECTIVE_DATA;
        if (offset + bytes_to_copy > size)
            bytes_to_copy = size - offset;
        memcpy(block_buf + DATA_HEADER_SIZE, buffer + offset, bytes_to_copy);

        // write to disk
        if (writeBlock(0, data_blocks[i], block_buf) < 0) {
            free(data_blocks);
            return TFS_ERR_WRITE_FAIL;
        }
    }
    // update inode with file size and first data block
    int inode_index = fd_table[fd].inode_index;
    inode_t *ino = &inode_directory[inode_index].inode;
    ino->size = size;
    ino->first_block = data_blocks[0];
    fd_table[fd].file_pointer = 0;
    // write inode to disk
    if (write_inode_to_disk(inode_index) < 0) {
        free(data_blocks);
        return TFS_ERR_WRITE_FAIL;
    }
    free(data_blocks);
    return TFS_SUCCESS;
}

// tfs_deleteFile: Delete a file and free its blocks
int tfs_deleteFile(fileDescriptor fd) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used) return TFS_ERR_INVALID_FD;
    int inode_index = fd_table[fd].inode_index;
    // mark the inode as free in persistent directory
    inode_directory[inode_index].used = 0;
    // overwrite the inode block with zeros
    inode_t blank;
    memset(&blank, 0, sizeof(inode_t));
    if (writeBlock(0, inode_index + 1, &blank) < 0) return TFS_ERR_WRITE_FAIL;
    fd_table[fd].used = 0;
    printf("File deletion successful for inode slot %d\n", inode_index);
    return TFS_SUCCESS;
}

// tfs_readByte: read one byte from the file (supports multi-block files)
int tfs_readByte(fileDescriptor fd, char *buffer) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used) return TFS_ERR_INVALID_FD;

    // retrieve inode and validate file ptr
    int inode_index = fd_table[fd].inode_index;
    inode_t ino = inode_directory[inode_index].inode;
    if (fd_table[fd].file_pointer >= ino.size) return TFS_ERR_INVALID_FD;

    int pos = fd_table[fd].file_pointer; // absolute offset in file
    int block_index = pos / EFFECTIVE_DATA;
    int offset_in_block = pos % EFFECTIVE_DATA;

    // traverse the chain of data blocks.
    int current_block = ino.first_block;
    unsigned char block_buf[BLOCKSIZE];
    for (int i = 0; i < block_index; i++) {
        if (readBlock(0, current_block, block_buf) < 0) return TFS_ERR_READ_FAIL;
        int next_ptr;
        memcpy(&next_ptr, block_buf + 2, sizeof(int));
        if (next_ptr == -1) return TFS_ERR_READ_FAIL;
        current_block = next_ptr;
    }
    // read byte
    if (readBlock(0, current_block, block_buf) < 0) return TFS_ERR_READ_FAIL;
    *buffer = block_buf[DATA_HEADER_SIZE + offset_in_block];
    // inc ptr
    fd_table[fd].file_pointer++;
    return TFS_SUCCESS;
}


// tfs_seek: set the file pointer to a specified offset
int tfs_seek(fileDescriptor fd, int offset) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used) return TFS_ERR_INVALID_FD;
    int inode_index = fd_table[fd].inode_index;
    if (offset < 0 || offset > inode_directory[inode_index].inode.size) return TFS_ERR_READ_FAIL;
    fd_table[fd].file_pointer = offset;
    return TFS_SUCCESS;
}

/* tfs_rename: renames an open file */
int tfs_rename(fileDescriptor fd, char *name) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used) return TFS_ERR_INVALID_FD;

    if (strlen(name) > MAX_FILENAME_LEN) return TFS_ERR_INVALID_FILE;

    int inode_index = fd_table[fd].inode_index;
    if (!inode_directory[inode_index].used) return TFS_ERR_INVALID_FILE;

    strncpy(inode_directory[inode_index].inode.filename, name, MAX_FILENAME_LEN+1);
    inode_directory[inode_index].inode.filename[MAX_FILENAME_LEN] = '\0';  

    if (write_inode_to_disk(inode_index) < 0) return TFS_ERR_WRITE_FAIL;

    return TFS_SUCCESS;
}

/* tfs_readdir: prints filenames */
void tfs_readdir() {
    printf("Printing TinyFS contents:\n");

    int count = 0;
    for (int i=0; i< MAX_INODES; i++) {
        if (inode_directory[i].used) {
            printf(" - %s\n", inode_directory[i].inode.filename);
            count++;
        }
    }
    if (count == 0) printf("No files found\n");
}


int tfs_createDir(char *dirName) {
    if (!disk_mounted) return TFS_ERR_NO_DISK;

    // Extract parent directory and basename
    char par_path[256], dir_basename[MAX_FILENAME_LEN];
    if (split_path(dirName, par_path, dir_basename) < 0) return TFS_ERR_INVALID_PATH;

    int par_inode_idx = find_inode_by_name(par_path);
    if (par_inode_idx < 0 || !inode_directory[par_inode_idx].inode.is_dir) return TFS_ERR_INVALID_PATH;
    
    // Check if directory already exists
    if (find_inode_by_name(dirName) >= 0) return TFS_ERR_INVALID_FILE;

    // Allocate new inode for directory
    int new_inode_idx = allocate_inode_slot();
    if (new_inode_idx < 0) return TFS_ERR_NO_FREE_BLOCKS;

    inode_t new_inode;
    memset(&new_inode, 0, sizeof(inode_t));
    new_inode.type = 2;
    new_inode.is_dir = 1;
    strncpy(new_inode.filename, dir_basename, MAX_FILENAME_LEN);
    new_inode.filename[MAX_FILENAME_LEN] = '\0';
    new_inode.size = 0;
    new_inode.first_block = -1;
    new_inode.par_inode = par_inode_idx;
    new_inode.next_sibling = -1;

    // Link new directory to parent
    if (inode_directory[par_inode_idx].inode.first_block == -1) {
        inode_directory[par_inode_idx].inode.first_block = new_inode_idx;
    } else {
        int cur = inode_directory[par_inode_idx].inode.first_block;
        while (inode_directory[cur].inode.next_sibling != -1) {
            cur = inode_directory[cur].inode.next_sibling;
        }
        inode_directory[cur].inode.next_sibling = new_inode_idx;
    }

    // Store in memory and on disk
    inode_directory[new_inode_idx].used = 1;
    inode_directory[new_inode_idx].inode = new_inode;
    inode_directory[new_inode_idx].block_number = new_inode_idx + 1;
    if (write_inode_to_disk(new_inode_idx) < 0 || write_inode_to_disk(par_inode_idx) < 0) return TFS_ERR_WRITE_FAIL;

    //printf("[DEBUG] Successfully created directory '%s' under '%s' (Inode %d)\n",
     //      dir_basename, par_path, new_inode_idx);

    return TFS_SUCCESS;
}

/* tfs_removeDir: deletes empty dir */
int tfs_removeDir(char *dirName) {
    if (!disk_mounted) return TFS_ERR_NO_DISK;

    // find dir inode
    int dir_inode_idx = find_inode_by_name(dirName);
    if (dir_inode_idx < 0) return TFS_ERR_INVALID_PATH;

    inode_t *dir_inode = &inode_directory[dir_inode_idx].inode;
    if (!dir_inode->is_dir) return TFS_ERR_INVALID_FILE;

    // check if empty
    int not_empty = 0;
    for (int i=0; i<MAX_INODES; i++) {
        if (inode_directory[i].used && inode_directory[i].inode.par_inode == dir_inode_idx) {
            not_empty = 1;
            break;
        }
    }
    if (not_empty) return TFS_ERR_DIR;

    // empty, unlink dir from par
    int par_idx = dir_inode->par_inode;
    if (par_idx >= 0) {
        inode_t *par = &inode_directory[par_idx].inode;

        // update linked list
        if (par->first_block == dir_inode_idx) {
            par->first_block = dir_inode->next_sibling;
        } else {
            int prev = par->first_block;
            while (prev >= 0) {
                inode_t *sibling = &inode_directory[prev].inode;
                if (sibling->next_sibling == dir_inode_idx) {
                    sibling->next_sibling = dir_inode->next_sibling;
                    break;
                }
                if (sibling->next_sibling < 0) break; 
                prev = sibling->next_sibling;
            }

        }
        // write to disk
        if (write_inode_to_disk(par_idx) < 0) return TFS_ERR_WRITE_FAIL;
    }

    // free dir inode
    inode_directory[dir_inode_idx].used = 0;
    memset(&inode_directory[dir_inode_idx].inode, 0, sizeof(inode_t));

    if (write_inode_to_disk(dir_inode_idx) < 0) return TFS_ERR_WRITE_FAIL;

    printf("Successfully removed empty directory: %s\n", dirName);
    return TFS_SUCCESS;
}

int tfs_removeAll(char *dirName) {
    if (!disk_mounted) return TFS_ERR_NO_DISK;

    // find dir inode
    int dir_inode_idx = find_inode_by_name(dirName);
    if (dir_inode_idx < 0) return TFS_ERR_INVALID_PATH;

    inode_t *dir_inode = &inode_directory[dir_inode_idx].inode;

    if (!dir_inode->is_dir) return TFS_ERR_DIR;

    // recursively delete all child inodes
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_directory[i].used && inode_directory[i].inode.par_inode == dir_inode_idx) {
            if (inode_directory[i].inode.is_dir) {
                if (tfs_removeAll(inode_directory[i].inode.filename) < 0) return TFS_ERR_REMOVE_FAIL;
            } else {
                // delete
                int file_fd = tfs_openFile(inode_directory[i].inode.filename);
                if (file_fd >= 0) tfs_deleteFile(file_fd);
            }
        }
    }

    return tfs_removeDir(dirName);
}
