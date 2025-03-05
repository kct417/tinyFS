#include "libDisk.h"
#include "libTinyFS.h"

int mount = 0;  // flag to know if diskname has been mounted or not yet

// initialize a TinyFS file system
int tfs_mkfs(char *filename, int nBytes)
{
    int disk = openDisk(filename, nBytes);
    if (disk < 0) return -1;

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
    if (disk < 0) {return -1;}

    // put superblock into sb
    Superblock sb;
    readBlock(disk, 0, &sb);

    if (sb.magicNumber != MAGIC_NUMBER)
    {
        close(disk);
        return -1;
    }
    
    mount = 1;
    return disk;
}

int tfs_unmount(void)
{
    // unmounting things

    mount = 0;
    
    return 0;
}
