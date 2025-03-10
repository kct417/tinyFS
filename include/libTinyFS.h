#ifndef TINYFS_H
#define TINYFS_H

#include "libDisk.h"
#include <stdint.h>


#define BLOCKSIZE 256
#define DEFAULT_DISK_SIZE 10240
#define DEFAULT_DISK_NAME "tinyFSDisk"
#define MAX_INODES 10
#define MAX_FILES 40
#define MAX_FILENAME_LEN 8
#define DATA_HEADER_SIZE 6
#define EFFECTIVE_DATA (BLOCKSIZE - DATA_HEADER_SIZE)

typedef struct {
    unsigned char type; // 1 for superblock
    unsigned char magic; 
    int root_inode; // block number of root dir inode
    int free_list; // pointer to free block
    char padding[BLOCKSIZE - 6]; 
} superblock_t;

typedef struct {
    unsigned char type; // 2 for inode
    uint8_t is_dir; // 0 = file, 1 = dir
    char filename[9]; 
    int size; 
    int first_block; // block number of first file ext
    int par_inode; // parent dir
    int next_sibling; // next inode in the same dir - linked list
    char padding[BLOCKSIZE - 21];  // Padding to fill 256 bytes
} inode_t;

typedef struct {
    int used; // 1 if in use, 0 if free
    inode_t inode; // inode for the file
    int block_number;  // block number where the inode is stored
} persistent_inode_entry_t;

// in-memory file descriptor entry
typedef struct {
    int used;
    int inode_index;       // index into the inode_directory
    int file_pointer;      // Current read/write offset
} file_entry_t;


static file_entry_t fd_table[MAX_FILES];
static int disk_mounted = 0;
static char mounted_disk[256] = {0};
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
int tfs_createDir(char *dirName);
int tfs_removeDir(char *dirName);
int tfs_removeAll(char *dirName);


#endif