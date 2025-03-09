#ifndef LIBTINYFS_H
#define LIBTINYFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "libDisk.h"

#define MAGIC_NUMBER 0x44       // 0x44 == 'D'
#define BLOCKSIZE 256           // Block size in bytes
#define NUM_BLOCKS 40           // Number of blocks in disk
#define MAX_FILES 32            // Maximum number of files not including superblock
#define DEFAULT_DISK_SIZE 10240 // Default disk size in bytes
#define DEFAULT_DISK_NAME "tinyFSDisk"

#define INODE_TABLE_START_BLK 3
#define INODE_TABLE_BLKS 5

// block types
#define SUPERBLOCK 1
#define INODE 2
#define FILE_EXTENT 3
#define FREE 4

typedef int fileDescriptor;

typedef struct
{
    char blockType;
    char magic;
    int freeBitmap;
    int inodeBitmap;
    int totalBlocks;
    int freeBlocks;
} Superblock;

typedef struct
{
    char blockType;
    char magic;
    char fileName[8]; // supports 8 characters no more
    int nextInode;
    int fileSize;
    int dataBlockPointers[30];
} Inode;

typedef struct
{
    char blockType;
    char magic;
    int next;                 // update from char to int
    char data[BLOCKSIZE - 8]; // adjust size based on new struct layout
} FileExtent;

typedef struct
{
    char blockType;
    char magic;
    int next;
} FreeBlock;

typedef struct
{
    int inodeBlock;
    int offset;
    int isOpen;
} FD;

int tfs_mkfs(char *filename, int nBytes);
int tfs_mount(char *diskname);
int tfs_unmount(void);
fileDescriptor tfs_openFile(char *name);
int tfs_closeFile(fileDescriptor FD);
int tfs_writeFile(fileDescriptor FD, char *buffer, int size);
int tfs_deleteFile(char *FD);
int tfs_readByte(fileDescriptor FD, char *buffer);
int tfs_seek(fileDescriptor FD, int offset);

#endif // LIBTINYFS_H