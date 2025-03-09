#ifndef _LIBDISK_H
#define _LIBDISK_H

/* The default size of the disk and file system block */
#define BLOCKSIZE 256

/* This functions opens a regular UNIX file and designates the first
nBytes of it as space for the emulated disk. If nBytes is not exactly a
multiple of BLOCKSIZE then the disk size will be the closest multiple
of BLOCKSIZE that is lower than nByte (but greater than 0) If nBytes is
less than BLOCKSIZE failure should be returned. If nBytes > BLOCKSIZE
and there is already a file by the given filename, that file’s content
may be overwritten. If nBytes is 0, an existing disk is opened, and the
content must not be overwritten in this function. There is no requirement
to maintain integrity of any file content beyond nBytes. The return value
is negative on failure or a disk number on success. */
int openDisk(char *filename, int nBytes);

/* self explanatory */
int closeDisk(int disk);

/* readBlock() reads an entire block of BLOCKSIZE bytes from the open
disk (identified by ‘disk’) and copies the result into a local buffer
(must be at least of BLOCKSIZE bytes). The bNum is a logical block
number, which must be translated into a byte offset within the disk. The
translation from logical to physical block is straightforward: bNum=0
is the very first byte of the file. bNum=1 is BLOCKSIZE bytes into the
disk, bNum=n is n*BLOCKSIZE bytes into the disk. On success, it returns
0. -1 or smaller is returned if disk is not available (hasn’t been
opened) or any other failures. You must define your own error code
system. */
int readBlock(int disk, int bNum, void *block);

/* writeBlock() takes disk number ‘disk’ and logical block number ‘bNum’
and writes the content of the buffer ‘block’ to that location. ‘block’
must be integral with BLOCKSIZE. Just as in readBlock(), writeBlock()
must translate the logical block bNum to the correct byte position in
the file. On success, it returns 0. -1 or smaller is returned if disk
is not available (i.e. hasn’t been opened) or any other failures. You
must define your own error code system. */
int writeBlock(int disk, int bNum, void *block);

#endif
