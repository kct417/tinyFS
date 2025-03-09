#include "libDisk.h"
#include "Disk_errno.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int openDisk(char *filename, int nBytes)
{
    // check for filename
    if (!filename)
    {
        dsk_errno = DSK_EINVAL;
        return DSK_FAILURE;
    }

    // check file suffix
    char *fileExtension = strrchr(filename, '.');
    char newFilename[strlen(filename) + 5];
    // add .dsk extension if not provided
    if (!fileExtension)
    {
        sprintf(newFilename, "%s%s", filename, ".dsk");
        filename = newFilename;
    }
    // check for valid extension
    else
    {
        if (strcmp(fileExtension, ".dsk") != 0)
        {
            dsk_errno = DSK_EINVAL;
            return DSK_FAILURE;
        }
    }

    // check if file exists
    int fd;
    if (nBytes == 0)
    {
        // open existing disk
        if ((fd = open(filename, O_RDWR)) == -1)
        {
            dsk_errno = DSK_EOPEN;
            return DSK_FAILURE;
        }
        return fd;
    }

    // return failure if nBytes < BLOCKSIZE
    if (nBytes < BLOCKSIZE)
    {
        dsk_errno = DSK_EINVAL;
        return DSK_FAILURE;
    }

    // create or overwrite file
    if ((fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1)
    {
        dsk_errno = DSK_EOPEN;
        return DSK_FAILURE;
    }
    // alloc disk space for open file nBytes rounded to nearest multiple of BLOCKSIZE
    nBytes -= nBytes % BLOCKSIZE;
    char buffer[nBytes];
    memset(buffer, 0x00, nBytes);
    if (write(fd, buffer, nBytes) != nBytes)
    {
        dsk_errno = DSK_EWRITE;
        return DSK_FAILURE;
    }

    dsk_errno = DSK_SUCCESS;
    return fd;
}

int closeDisk(int disk)
{
    // check for file descriptor
    if (disk == -1)
    {
        dsk_errno = DSK_EBADF;
        return DSK_FAILURE;
    }

    // close file descriptor
    if (close(disk) == -1)
    {
        dsk_errno = DSK_ECLOSE;
        return DSK_FAILURE;
    }

    dsk_errno = DSK_SUCCESS;
    return DSK_SUCCESS;
}

int readBlock(int disk, int bNum, void *block)
{
    // check for invalid input
    if (disk == -1 || bNum < 0 || block == NULL)
    {
        dsk_errno = DSK_EINVAL;
        return DSK_FAILURE;
    }

    // move file descriptor to offset
    off_t offset = bNum * BLOCKSIZE;
    if (lseek(disk, offset, SEEK_SET) == -1)
    {
        dsk_errno = DSK_ESEEK;
        return DSK_FAILURE;
    }

    // read bytes into block buffer
    ssize_t bytesRead = 0;
    if (bytesRead = read(disk, block, BLOCKSIZE) != BLOCKSIZE)
    {
        if (bytesRead == 0)
        {
            dsk_errno = DSK_EOF;
            return DSK_SUCCESS;
        }

        dsk_errno = DSK_EREAD;
        return DSK_FAILURE;
    }

    dsk_errno = DSK_SUCCESS;
    return DSK_SUCCESS;
}

int writeBlock(int disk, int bNum, void *block)
{
    // check for bad input
    if (disk == -1 || bNum < 0 || block == NULL)
    {
        dsk_errno = DSK_EINVAL;
        return DSK_FAILURE;
    }

    // move file descriptor to offset
    off_t offset = bNum * BLOCKSIZE;
    if (lseek(disk, offset, SEEK_SET) == -1)
    {
        dsk_errno = DSK_ESEEK;
        return DSK_FAILURE;
    }

    // write bytes into block buffer
    if (write(disk, block, BLOCKSIZE) != BLOCKSIZE)
    {
        dsk_errno = DSK_EWRITE;
        return DSK_FAILURE;
    }

    dsk_errno = DSK_SUCCESS;
    return DSK_SUCCESS;
}
