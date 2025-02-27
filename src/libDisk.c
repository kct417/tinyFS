#include "libDisk.h"


int openDisk(char *filename, int nBytes) {

    if (nBytes < BLOCKSIZE) return -1;  // minimum size of check for disk

    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);   // using open remember to close
    if (fd < 0) return -1;

    if (nBytes > 0) {
        if (ftruncate(fd, (nBytes / BLOCKSIZE) * BLOCKSIZE) < 0)    // ensure correct size - int division so no remainder
        {
            close(fd);
            return -1;
        }
    }

    return fd;
}

// read BLOCKSIZE bytes from disk into block
int readBlock(int disk, int blockNum, void *block) {
    if (disk < 0 || blockNum < 0 || block == NULL) return -1;

    if (lseek(disk, blockNum * BLOCKSIZE, SEEK_SET) < 0) return -1; // move to the correct block
    if (read(disk, block, BLOCKSIZE) < 0) return -1;           // read the block

    return 0;
}

// write BLOCKSIZE bytes to blockNum in disk
int writeBlock(int disk, int blockNum, void *block) {
    if (disk < 0 || blockNum < 0 || block == NULL) return -1;

    if (lseek(disk, blockNum * BLOCKSIZE, SEEK_SET) < 0) return -1; // move to the correct block
    if (write(disk, block, BLOCKSIZE) < 0) return -1;          // write the block   

    return 0;
}
