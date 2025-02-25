#include "libDisk.h"

int openDisk(char *filename, int nBytes) {
    /* opens a file and alloc the first nBytes of it
    as space for the disk.
    if nBytes % BLOCKSIZE != 0 then round to closest multiple.
    if nBytes < BLOCKSIZE return -1.
    if nBytes > BLOCKSIZE and there is already a file with
    that name, overwrite.
    if nBytes = 0, an existing disk is opened.
    return -1 on fail, disk number on success.
    */
    int fd;
    // check if file exists
    if (nBytes == 0) {
        // open existing disk
        fd = open(filename, O_RDWR);
        if (fd < 0) return -1;
    } else {
        // disk file doesn't exist, create
        // round to nearest multiple
        nBytes = (nBytes / BLOCKSIZE) * BLOCKSIZE;
        if (nBytes < BLOCKSIZE) {
            return -1;
        }
        // create or overwrite file. rwx permission to everyone 
        fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0777);
        if (fd < 0) return -1;

        // alloc disk space for open file nBytes
        if (ftruncate(fd, nBytes) < 0) {
            close(fd);
            return -1;
        }
    }

    return fd;
}

int closeDisk(int disk) {
    if (disk < 0) return -1;
    int closed = close(disk);
    return closed;
}

int readBlock(int disk, int bNum, void *block) {
    /* read entire block of BLOCKSIZE bytes from open disk.
    copies result into block buf.
    bNum = byte offset
    offset = bNum*BLOCKSIZE
    return 0 on success, -1 on fail
    */
    if (disk < 0 || bNum < 0 || block == NULL) return -1;
    off_t offset = bNum * BLOCKSIZE;
    // move file ptr to offset
    if (lseek(disk, offset, SEEK_SET) < 0) return -1;
    // read bytes into block buf
    if (read(disk, block, BLOCKSIZE) != BLOCKSIZE) return -1;

    return 0;
}

int writeBlock(int disk, int bNum, void *block) {
    /* write entire block of "block" into block.
    bNum = byte offset
    offset = bNum*BLOCKSIZE
    return 0 on success, -1 on fail
    */
    if (disk < 0 || bNum < 0 || block == NULL) return -1;
    off_t offset = bNum * BLOCKSIZE;
    // move file ptr to offset
    if (lseek(disk, offset, SEEK_SET) < 0) return -1;
    // write bytes into block buf
    if (write(disk, block, BLOCKSIZE) != BLOCKSIZE) return -1;

    return 0;
}

