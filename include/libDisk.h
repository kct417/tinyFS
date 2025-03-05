#ifndef LIBDISK_H
#define LIBDISK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BLOCKSIZE 256  // Block size in bytes


int openDisk(char *filename, int nBytes);
int readBlock(int disk, int blockNum, void *block);
int writeBlock(int disk, int blockNum, void *block);

#endif // LIBDISK_H