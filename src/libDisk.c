#include "libDisk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *disk_file = NULL;
static int disk_size = 0;

int openDisk(char *filename, int nBytes) {

    // if nBytes is zero, open existing disk; don't overwrite
    if (nBytes == 0) {
        disk_file = fopen(filename,"rb+");
        if (disk_file == NULL) {
            return -1;
        }
        // assume file size is mutliple of BLOCKSIZE
        // move file pointer to end of file
        fseek(disk_file, 0, SEEK_END);
        // return file pointer position (represents disk size in bytes)
        disk_size = ftell(disk_file);
        // reset file pointer to start of file in order to start from beginning
        rewind(disk_file);
        return 0;
    }

    // make sure nBytes is at least one block size
    if (nBytes < BLOCKSIZE) {
        return -1;
    }

    // nBytes to multiple of BLOCKSIZE
    int actual_size = (nBytes / BLOCKSIZE) * BLOCKSIZE;
    // open file for reading and writing
    disk_file = fopen(filename,"wb+");
    // if file open fails, return -1
    if (disk_file == NULL) {
        return -1;
    }

    // allocate space, write zeros to file
    char zero = 0;
    for (int i = 0; i < actual_size; i++) {
        fwrite(&zero, 1, 1, disk_file);
    }

    // flush file to disk, forces buffered data to be written to disk
    fflush(disk_file);
    // size stored in disk_size; keeps track of allocated disk space
    disk_size = actual_size;
    // reset file pointer to start of file
    rewind(disk_file);
    return 0;
}

int closeDisk(int disk) {
    // if disk is not open, return -1
    if (disk_file == NULL) {
        return -1;
    }
    // close file
    int res = fclose(disk_file);
    // set file pointer to NULL
    disk_file = NULL;
    // set disk size to zero for next open
    disk_size = 0;
    return res;
}

int readBlock(int disk, int bNum, void *block) {
    // if disk is not open, return -1
    if (disk_file == NULL) {
        return -1;
    }
    // if bNum is out of bounds, return -1
    if (bNum < 0 || bNum >= disk_size / BLOCKSIZE) {
        return -1;
    }
    // move file pointer to bNum * BLOCKSIZE
    if(fseek(disk_file, bNum * BLOCKSIZE, SEEK_SET) !=0 ) {
        return -1;
    }
    // read BLOCKSIZE bytes from file to buffer
    int result = fread(block, 1, BLOCKSIZE, disk_file);
    if (result != BLOCKSIZE) {
        return -1;
    }
    return 0;
}

int writeBlock(int disk, int bNum, void *block) {
    if (!disk_file) {
        printf("[ERROR] Disk is not open!\n");
        return -1;
    }
    long offset = (long) bNum * BLOCKSIZE;
    if (bNum < 0 || offset >= disk_size) {
        printf("[ERROR] Invalid block number: %d (Out of bounds)\n", bNum);
        return -1;
    }
    if (fseek(disk_file, offset, SEEK_SET) != 0) {
        printf("[ERROR] fseek() failed for block %d\n", bNum);
        return -1;
    }
    size_t bytesWritten = fwrite(block, 1, BLOCKSIZE, disk_file);
    fflush(disk_file); // Ensure data is written to disk

    if (bytesWritten != BLOCKSIZE) {
        printf("[ERROR] fwrite() failed: expected %d bytes, got %ld\n", BLOCKSIZE, bytesWritten);
        return -1;
    }

    return 0;
}