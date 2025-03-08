#include "tinyFS.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



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

// finds the inode with matching filename in the persistent directory
// returns the index in inode_directory if found, or -1 if not found
static int find_inode_by_name(const char *name) {
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_directory[i].used && strcmp(inode_directory[i].inode.filename, name) == 0)
            return i;
    }
    return -1;
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



// tfs_mkfs: format a new tinyFS file system
int tfs_mkfs(char *filename, int nBytes) {
    if (openDisk(filename, nBytes) < 0)
        return -1;

    // create superblock and write to block 0
    superblock_t sb;
    memset(&sb, 0, sizeof(superblock_t));
    sb.type = 1;
    sb.magic = 0x44;
    sb.root_inode = -1;
    sb.free_list = 1;  
    if (writeBlock(0, 0, &sb) < 0)
        return -1;

    // initialize inode blocks (blocks 1 to MAX_INODES) to 0
    inode_t blank;
    memset(&blank, 0, sizeof(inode_t));
    for (int i = 0; i < MAX_INODES; i++) {
        if (writeBlock(0, i + 1, &blank) < 0)
            return -1;
        inode_directory[i].used = 0;
    }

    // reset file descriptor table
    memset(fd_table, 0, sizeof(fd_table));

    next_free_data_block = MAX_INODES + 1;

    disk_mounted = 1;
    strncpy(mounted_disk, filename, sizeof(mounted_disk));
    return 0;
}

// tfs_mount: Mount an existing TinyFS file system
// opens disk using disk emulator, reads superblock to verify type and magic number, initializes file descriptor table
int tfs_mount(char *diskname) {
    if (openDisk(diskname, 0) < 0) return -1;
    
    // read and verify the superblock.
    superblock_t sb;
    if (readBlock(0, 0, &sb) < 0) return -1;
    // printf("DEBUG: Superblock: type=%d, magic=0x%x\n", sb.type, sb.magic);
    if (sb.type != 1 || sb.magic != 0x44) return -1;  // not a valid TinyFS file system
    
    
    disk_mounted = 1;
    strncpy(mounted_disk, diskname, sizeof(mounted_disk));

    // load persistent inode directory from disk
    load_inode_directory();
    // debug print, loaded inodes
    // for (int i = 0; i < MAX_INODES; i++) {
    //     if (inode_directory[i].used) {
    //         // printf("DEBUG: Inode %d loaded: filename='%s', size=%d, first_block=%d\n",
    //                 i, inode_directory[i].inode.filename,
    //                 inode_directory[i].inode.size,
    //                 inode_directory[i].inode.first_block);
    //     }
    // }

    int max_allocation = MAX_INODES;
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_directory[i].used && inode_directory[i].inode.first_block > max_allocation) {

            max_allocation = inode_directory[i].inode.first_block;
        }
    }

    // printf("DEBUG: next free data block: %d\n", max_allocation + 1);

    // clear/initialize the file descriptor table
    memset(fd_table, 0, sizeof(fd_table));
    next_free_data_block = max_allocation+ 1;
    return 0;
}

// tfs_unmount: Unmount the TinyFS file system
// close disk and reset in-memory data
int tfs_unmount(void) {
    if (!disk_mounted) return -1;
    
    // Flush any in-memory metadata changes to disk here
    if (closeDisk(0) < 0) return -1;
    
    disk_mounted = 0;
    
    memset(mounted_disk, 0, sizeof(mounted_disk));

    return 0;
}

// tfs_openFile: Open or create a file.
fileDescriptor tfs_openFile(char *name) {
    if (!disk_mounted) return -1;

    int inode_index = find_inode_by_name(name);
    if (inode_index < 0) {
        // file doesn't exist Allocate a new inode
        inode_index = allocate_inode_slot();
        if (inode_index < 0)
            return -1; // no free inode slot

        inode_t new_inode;
        memset(&new_inode, 0, sizeof(inode_t));
        new_inode.type = 2;
        strncpy(new_inode.filename, name, 8);
        new_inode.filename[8] = '\0';
        new_inode.size = 0;
        new_inode.first_block = -1; // no data yet

        inode_directory[inode_index].used = 1;
        inode_directory[inode_index].inode = new_inode;
        inode_directory[inode_index].block_number = inode_index + 1;
        if (write_inode_to_disk(inode_index) < 0)return -1;
    }

    // now create a new file descriptor entry
    int fd = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!fd_table[i].used) {
            fd = i;
            break;
        }
    }
    if (fd < 0)
        return -1;

    fd_table[fd].used = 1;
    fd_table[fd].inode_index = inode_index;
    fd_table[fd].file_pointer = 0;
    return fd;
}


// tfs_closeFile: Close an open file
// close file by marking file descriptor entry as unused
int tfs_closeFile(fileDescriptor fd) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used) return -1;
    // Flush any changes (like inode updates) to disk if necessary.
    fd_table[fd].used = 0;
    return 0;
}

// tfs_writeFile: Write file content (supports multi-block files)
int tfs_writeFile(fileDescriptor fd, char *buffer, int size) {

    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used)
        return -1;
    // Calculate how many blocks are needed (each block holds EFFECTIVE_DATA bytes)
    int blocks_needed = (size + EFFECTIVE_DATA - 1) / EFFECTIVE_DATA;
    if (blocks_needed < 1)
        blocks_needed = 1;
    // Allocate an array to hold data block numbers
    int *data_blocks = malloc(sizeof(int) * blocks_needed);
    if (!data_blocks)
        return -1;
    for (int i = 0; i < blocks_needed; i++) {
        int blk = allocate_data_block();
        if (blk < 0) {
            free(data_blocks);
            return -1;
        }
        data_blocks[i] = blk;
    }
    // Write each file extent block.
    for (int i = 0; i < blocks_needed; i++) {
        unsigned char block_buf[BLOCKSIZE];
        memset(block_buf, 0, BLOCKSIZE);
        // Header: type = 3 (data block), magic = 0x44.
        block_buf[0] = 3;
        block_buf[1] = 0x44;
        // Next pointer: if not last block, next data block number; else -1.
        int next_ptr = (i < blocks_needed - 1) ? data_blocks[i + 1] : -1;
        memcpy(block_buf + 2, &next_ptr, sizeof(int));
        // printf("DEBUG: Writing block %d: header: type=%d, magic=0x%x, next=%d\n",
            // data_blocks[i], block_buf[0], block_buf[1], next_ptr);
        // Copy file data for this block.
        int offset = i * EFFECTIVE_DATA;
        int bytes_to_copy = EFFECTIVE_DATA;
        if (offset + bytes_to_copy > size)
            bytes_to_copy = size - offset;
        memcpy(block_buf + DATA_HEADER_SIZE, buffer + offset, bytes_to_copy);
        // printf("DEBUG: Block %d: copying %d bytes from offset %d\n",
            // data_blocks[i], bytes_to_copy, offset);
        if (writeBlock(0, data_blocks[i], block_buf) < 0) {
            free(data_blocks);
            return -1;
        }
    }
    // Update inode with file size and first data block
    int inode_index = fd_table[fd].inode_index;
    inode_t *ino = &inode_directory[inode_index].inode;
    ino->size = size;
    ino->first_block = data_blocks[0];
    fd_table[fd].file_pointer = 0;
    if (write_inode_to_disk(inode_index) < 0) {
        free(data_blocks);
        return -1;
    }
    free(data_blocks);
    return 0;
}



// tfs_deleteFile: Delete a file and free its blocks
int tfs_deleteFile(fileDescriptor fd) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used)
        return -1;
    int inode_index = fd_table[fd].inode_index;
    // Debug: Print file info before deletion
    printf("DEBUG: Deleting file '%s' from inode slot %d (block %d)\n",
           inode_directory[inode_index].inode.filename,
           inode_index,
           inode_directory[inode_index].block_number);
    // mark the inode as free in persistent directory
    inode_directory[inode_index].used = 0;
    // overwrite the inode block with zeros
    inode_t blank;
    memset(&blank, 0, sizeof(inode_t));
    if (writeBlock(0, inode_index + 1, &blank) < 0)
        return -1;
    fd_table[fd].used = 0;
    printf("DEBUG: File deletion successful for inode slot %d\n", inode_index);

    return 0;
}

// tfs_readByte: Read one byte from the file (supports multi-block files)
int tfs_readByte(fileDescriptor fd, char *buffer) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used)
        return -1;
    int inode_index = fd_table[fd].inode_index;
    inode_t ino = inode_directory[inode_index].inode;
    if (fd_table[fd].file_pointer >= ino.size)
        return -1;
    int pos = fd_table[fd].file_pointer; // absolute offset in file
    int block_index = pos / EFFECTIVE_DATA;
    int offset_in_block = pos % EFFECTIVE_DATA;
    // traverse the chain of data blocks.
    int current_block = ino.first_block;
    unsigned char block_buf[BLOCKSIZE];
    // printf("DEBUG: tfs_readByte: file pointer=%d, block_index=%d, offset_in_block=%d\n",
        // pos, block_index, offset_in_block);
    for (int i = 0; i < block_index; i++) {
        if (readBlock(0, current_block, block_buf) < 0)
            return -1;
        int next_ptr;
        memcpy(&next_ptr, block_buf + 2, sizeof(int));
        // printf("DEBUG: tfs_readByte: at block %d, next_ptr=%d\n", current_block, next_ptr);
        if (next_ptr == -1)
            return -1;
        current_block = next_ptr;
    }
    if (readBlock(0, current_block, block_buf) < 0)
        return -1;
    *buffer = block_buf[DATA_HEADER_SIZE + offset_in_block];
    // printf("DEBUG: tfs_readByte: read '%c' from block %d\n", *buffer, current_block);
    fd_table[fd].file_pointer++;
    return 0;
}


// tfs_seek: Set the file pointer to a specified offset
int tfs_seek(fileDescriptor fd, int offset) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used)
        return -1;
    int inode_index = fd_table[fd].inode_index;
    if (offset < 0 || offset > inode_directory[inode_index].inode.size)
        return -1;
    fd_table[fd].file_pointer = offset;
    return 0;
}