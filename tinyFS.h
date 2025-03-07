#ifndef TINYFS_H
#define TINYFS_H

#include "disk.h"

#define BLOCKSIZE 256
#define DEFAULT_DISK_SIZE 10240
#define DEFAULT_DISK_NAME "tinyFSDisk"

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

#endif
