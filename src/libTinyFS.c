#include "libDisk.h"
#include "libTinyFS.h"
#include "tinyFS_errno.h"

typedef enum {
    UNMOUNTED,
    MOUNTED
} MountStatus;

MountStatus mount = UNMOUNTED;  // flag to know if diskname has been mounted or not yet

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

    if (sb.magicNumber != MAGIC_NUMBER)
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
    if (strlen(name) > 8) {return TFS_INVALID_ARG;} // file name too long
    
    if (mount == UNMOUNTED) {return TFS_NOT_MOUNTED;}

    for (int i = 0; i < MAX_FILES; i++)
    {

    }

    // return fd;
}
