#include "tinyFS.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Example on-disk superblock structure (fits in 256 bytes)
typedef struct {
    unsigned char type;      // Should be 1 for superblock
    unsigned char magic;     // Should be 0x44
    int root_inode;          // Block number for the root inode (or first inode)
    int free_list;           // Pointer to the free block list (or use a bit vector)
    char padding[BLOCKSIZE - 10];  // Padding to fill 256 bytes
} superblock_t;

// Example on-disk inode structure (also 256 bytes)
typedef struct {
    unsigned char type;      // 2 for inode
    char filename[9];        // Up to 8 characters + null terminator
    int size;                // File size in bytes
    int first_block;         // Block number of first file extent
    char padding[BLOCKSIZE - (1 + 9 + 4 + 4)];  // Padding to fill 256 bytes
} inode_t;

// In-memory file descriptor entry
#define MAX_FILES 40
typedef struct {
    int used;
    inode_t inode;
    int file_pointer;       // Current read/write offset
    int inode_block;        // Block number where the inode is stored (if applicable)
} file_entry_t;

static file_entry_t fd_table[MAX_FILES];
static int disk_mounted = 0;
static char mounted_disk[256] = {0};
// Global variable for simple free block allocation.
// We assume blocks 0-9 are reserved (e.g. block 0 for the superblock and possibly other blocks for future inode tables, etc
static int next_free_block = 10; 

// Helper function to allocate a new block
static int allocate_block() {
    int total_blocks = DEFAULT_DISK_SIZE / BLOCKSIZE;
    if (next_free_block >= total_blocks) {
        return -1;
    }
    return next_free_block++;
}



// tfs_mkfs: Format a new TinyFS file system
// makes new disk using disk emulator (phase 1) and initializes superblock by setting type, magic, root_inode, and free_list
int tfs_mkfs(char *filename, int nBytes) {
    // Create a new disk using the disk emulator.
    if (openDisk(filename, nBytes) < 0) return -1;
    
    // Initialize the superblock.
    superblock_t sb;
    memset(&sb, 0, sizeof(superblock_t));
    sb.type = 1;         // Superblock type
    sb.magic = 0x44;     // Magic number
    sb.root_inode = -1;  // No root inode yet (or set this to a designated inode block)
    sb.free_list = 1;    // Assume free blocks start at block 1
    
    // Write the superblock to block 0.
    if (writeBlock(0, 0, &sb) < 0) return -1;
    
    next_free_block = 10;

    // The remaining blocks are already initialized to 0 by openDisk()
    return 0;
}

// tfs_mount: Mount an existing TinyFS file system
// opens disk using disk emulator, reads superblock to verify type and magic number, initializes file descriptor table
int tfs_mount(char *diskname) {
    if (openDisk(diskname, 0) < 0) return -1;
    
    // Read and verify the superblock.
    superblock_t sb;
    if (readBlock(0, 0, &sb) < 0) return -1;
    if (sb.type != 1 || sb.magic != 0x44) {
        return -1;  // Not a valid TinyFS file system
    }
    
    disk_mounted = 1;
    strncpy(mounted_disk, diskname, sizeof(mounted_disk));
    
    // Initialize the file descriptor table
    memset(fd_table, 0, sizeof(fd_table));
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

// tfs_openFile: Open or create a file
// close file by marking file descriptor entry as unused
fileDescriptor tfs_openFile(char *name) {
    // Check if file already exists in our in-memory table
    for (int i = 0; i < MAX_FILES; i++) {
        if (fd_table[i].used && strcmp(fd_table[i].inode.filename, name) == 0) {
            return i;
        }
    }
    
    // Create a new inode for the file
    inode_t new_inode;
    memset(&new_inode, 0, sizeof(inode_t));
    new_inode.type = 2;  // inode type
    strncpy(new_inode.filename, name, 8);
    new_inode.filename[8] = '\0';
    new_inode.size = 0;
    new_inode.first_block = -1;  // No data blocks allocated yet
    
    // Find a free file descriptor slot
    int fd = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!fd_table[i].used) {
            fd = i;
            break;
        }
    }
    if (fd == -1) return -1;  // No free file descriptor available
    
    fd_table[fd].used = 1;
    fd_table[fd].inode = new_inode;
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

// tfs_writeFile: Write a file's content (overwriting previous data)
// write file from buffer into datablock, update inode with size and block number, update file pointer
int tfs_writeFile(fileDescriptor fd, char *buffer, int size) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used)
        return -1;
    
    // Calculate how many data bytes can be stored per extent
    int header_size = 6;  // 1 byte type, 1 byte magic, 4 bytes next pointer
    int effective_data = BLOCKSIZE - header_size;  // e.g., 250 bytes
    int blocks_needed = (size + effective_data - 1) / effective_data;
    if (blocks_needed < 1) blocks_needed = 1;
    
    // Allocate an array to hold the allocated block numbers
    int *blocks = malloc(sizeof(int) * blocks_needed);
    if (!blocks) return -1;
    for (int i = 0; i < blocks_needed; i++) {
        blocks[i] = allocate_block();
        if (blocks[i] < 0) {
            free(blocks);
            return -1;
        }
    }
    
    // Write each extent block.
    for (int i = 0; i < blocks_needed; i++) {
        unsigned char extent[BLOCKSIZE];
        memset(extent, 0, BLOCKSIZE);
        extent[0] = 3;    // block type for file extent
        extent[1] = 0x44; // magic number
        
        // Set next pointer: if not the last block, point to the next allocated block; else, -1
        int next_ptr = (i < blocks_needed - 1) ? blocks[i+1] : -1;
        memcpy(extent + 2, &next_ptr, sizeof(int));
        
        // Copy the portion of file data for this block
        int offset = i * effective_data;
        int bytes_to_copy = effective_data;
        if (offset + bytes_to_copy > size)
            bytes_to_copy = size - offset;
        memcpy(extent + header_size, buffer + offset, bytes_to_copy);
        
        if (writeBlock(0, blocks[i], extent) < 0) {
            free(blocks);
            return -1;
        }
    }
    
    int first_block = blocks[0];
    free(blocks);
    
    // Update the inode with the total file size and the pointer to the first extent block
    fd_table[fd].inode.size = size;
    fd_table[fd].inode.first_block = first_block;
    fd_table[fd].file_pointer = 0;
    
    return 0;
}


// tfs_deleteFile: Delete a file and free its blocks
// delete file by marking file descriptor entry as unused
int tfs_deleteFile(fileDescriptor fd) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used) return -1;
    // Mark the file's blocks as free in your free block management.
    fd_table[fd].used = 0;
    return 0;
}

// tfs_readByte: Read one byte from the file at the current file pointer
// read single byte from file based on current file pointer, increment file pointer by 1, return error if file pointer exceeds file size
int tfs_readByte(fileDescriptor fd, char *buffer) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used)
        return -1;
    if (fd_table[fd].file_pointer >= fd_table[fd].inode.size)
        return -1;
    
    int header_size = 6;
    int effective_data = BLOCKSIZE - header_size;
    
    int file_offset = fd_table[fd].file_pointer;
    int block_index = file_offset / effective_data;
    int offset_in_block = file_offset % effective_data;
    
    // Traverse the chain of extents to find the correct block
    int current_block = fd_table[fd].inode.first_block;
    for (int i = 0; i < block_index; i++) {
        unsigned char extent[BLOCKSIZE];
        if (readBlock(0, current_block, extent) < 0)
            return -1;
        int next_ptr;
        memcpy(&next_ptr, extent + 2, sizeof(int));
        if (next_ptr == -1)
            return -1;  // Unexpected end of chain
        current_block = next_ptr;
    }
    
    // Read the extent block that contains our byte
    unsigned char extent[BLOCKSIZE];
    if (readBlock(0, current_block, extent) < 0)
        return -1;
    
    // Data is stored starting at offset header_size
    *buffer = extent[header_size + offset_in_block];
    fd_table[fd].file_pointer++;
    return 0;
}


// tfs_seek: Set the file pointer to a specified offset
// changes file pointer to an absolute offset within file 
int tfs_seek(fileDescriptor fd, int offset) {
    if (fd < 0 || fd >= MAX_FILES || !fd_table[fd].used) return -1;
    if (offset < 0 || offset > fd_table[fd].inode.size) return -1;
    
    fd_table[fd].file_pointer = offset;
    return 0;
}
