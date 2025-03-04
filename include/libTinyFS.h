#ifndef LIBTINYFS_H
#define LIBTINYFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "libDisk.h"

#define MAGIC_NUMBER 0x44
#define BLOCKSIZE 256  // Block size in bytes
#define DEFAULT_DISK_SIZE 10240 // Default disk size in bytes
#define DEFAULT_DISK_NAME "tinyFSDisk"
typedef int fileDescriptor;

typedef struct
{
    char blockType;
    char magicNumber;
    int freeBlockPointer;
    int rootInodeBlock;
    int totalBlocks;
    int freeBlocksCount;
} Superblock;

typedef struct
{
    char blockType;
    char fileName[8];    // supports 8 characters no more
    int nextInode;
    int fileSize;
    int dataBlockPointers[30];
} Inode;

typedef struct
{
    char blockType;
    char magic;
    char data[BLOCKSIZE - 4];
} FileExtent;

typedef struct
{
    char blockType;
    char magic;
    int next;
} FreeBlock;


int tfs_mkfs(char *filename, int nBytes);
int tfs_mount(char *diskname);
int tfs_umount(void);
fileDescriptor tfs_openFile(char *name);
int tfs_closeFile(fileDescriptor FD);
int tfs_writeFile(fileDescriptor FD, char *buffer, int size);
int tfs_deleteFile(char *FD);
int tfs_readByte(fileDescriptor FD, char *buffer);
int tfs_seek(fileDescriptor FD, int offset);

#endif // LIBTINYFS_H