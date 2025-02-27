#ifndef LIBTINYFS_H
#define LIBTINYFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libDisk.h"

#define MAGIC_NUMBER 0x44

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


#endif // LIBTINYFS_H