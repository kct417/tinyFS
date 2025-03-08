#include "libDisk.h"
#include "libTinyFS.h"
#include "tinyFS_errno.h"

typedef enum {
    UNMOUNTED,
    MOUNTED
} MountStatus;

MountStatus mount = UNMOUNTED;  // flag to know if diskname has been mounted or not yet

int disk;  // file descriptor for disk
FD fdTable[MAX_FILES]; // file descriptor table

// initialize a TinyFS file system
int tfs_mkfs(char *filename, int nBytes)
{
    if (nBytes < BLOCKSIZE) {return TFS_INVALID_ARG;}

    int disk = openDisk(filename, nBytes);
    if (disk < 0) return TFS_DISK_ERROR;

    // set up superblock as first block at idx 0
    Superblock sb;
    writeBlock(disk, 0, &sb);

    // fill rest of disk with blocks
    for (int i = 1; i < nBytes / BLOCKSIZE; i++)
    {
        FreeBlock fb = {4, MAGIC_NUMBER, i + 1};
        writeBlock(disk, i, &fb);   // place block at idx of block number
    }

    return 0;
}

// mount a TinyFS file system located within diskname
int tfs_mount(char* diskname)
{
    if (mount) {tfs_unmount();}

    int disk = openDisk(diskname, 0);
    if (disk < 0) return TFS_DISK_ERROR;

    // put superblock into sb
    Superblock sb;
    readBlock(disk, 0, &sb);

    if (sb.magic != MAGIC_NUMBER)
    {
        close(disk);
        return TFS_CORRUPTED_FS;
    }
    
    mount = MOUNTED;
    return disk;
}

int tfs_unmount(void)
{
    if (mount == UNMOUNTED) {return TFS_NOT_MOUNTED;}

    mount = UNMOUNTED;
    
    return 0;
}

fileDescriptor tfs_openFile(char *name)
{
    if (strlen(name) > 8) {return TFS_INVALID_ARG;}     // file name too long
    
    if (mount == UNMOUNTED) {return TFS_NOT_MOUNTED;}   // disk not mounted

    // find file in the inode table
    for (int i = 0; i < MAX_FILES; i++)
    {
        Inode inode;
        readBlock(disk, i + 1, &inode);   // read inode at idx i + 1, skip superblock

        if (inode.blockType == INODE && strcmp(inode.fileName, name) == 0)
        {
            // check magic number
            if (inode.magic != MAGIC_NUMBER) {return TFS_CORRUPTED_FS;}

            // find free file descriptor slot
            for (int j = 0; j < MAX_FILES; j++)
            {
                if (fdTable[j].isOpen == 0)
                {
                    fdTable[j].inodeBlock = i + 1;   // set inode block
                    fdTable[j].offset = 0;           // set offset to 0
                    fdTable[j].mode = READ_ONLY;     // set mode to read only
                    fdTable[j].isOpen = 1;           // mark as open

                    return j;   // return fd idx
                }
            }

            return TFS_NO_FREE_FDBLOCKS;   // return error if all file descriptors are in use
        }

    }

    return TFS_FILE_NOT_FOUND; // return error if file is not found
}
