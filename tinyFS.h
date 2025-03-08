#ifndef TINYFS_H
#define TINYFS_H

#include "disk.h"
// #include "tinyFS.c"

#define BLOCKSIZE 256
#define DEFAULT_DISK_SIZE 10240
#define DEFAULT_DISK_NAME "tinyFSDisk"
#define MAX_INODES 5
#define MAX_FILES 40
#define DATA_HEADER_SIZE 6
#define EFFECTIVE_DATA (BLOCKSIZE - DATA_HEADER_SIZE)

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

typedef struct {
    int used; // 1 if in use, 0 if free
    inode_t inode; // inode for the file
    int block_number;  // block number where the inode is stored
} persistent_inode_entry_t;

// In-memory file descriptor entry
typedef struct {
    int used;
    int inode_index;       // Index into the inode_directory
    int file_pointer;      // Current read/write offset
} file_entry_t;


static file_entry_t fd_table[MAX_FILES];
static int disk_mounted = 0;
static char mounted_disk[256] = {0};
// static file_entry_t fd_table[MAX_FILES];
static persistent_inode_entry_t inode_directory[MAX_INODES];
static int next_free_data_block = MAX_INODES + 1;




// Global variable for simple free block allocation.
// We assume blocks 0-9 are reserved (e.g. block 0 for the superblock and possibly other blocks for future inode tables, etc
// static int next_free_block = 10; 
// Define fileDescriptor type
typedef int fileDescriptor;

// TinyFS API functions
int tfs_mkfs(char *filename, int nBytes);
int tfs_mount(char *diskname);
int tfs_unmount(void);
fileDescriptor tfs_openFile(char *name);
int tfs_closeFile(fileDescriptor fd);
int tfs_writeFile(fileDescriptor fd, char *buffer, int size);
int tfs_deleteFile(fileDescriptor fd);
int tfs_readByte(fileDescriptor fd, char *buffer);
int tfs_seek(fileDescriptor fd, int offset);
static int write_inode_to_disk(int index);
static void load_inode_directory();
static int find_inode_by_name(const char *name);


#endif
