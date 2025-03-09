#ifndef DISK_H
#define DISK_H

#define BLOCKSIZE 256 // each block has size of 256 bytes

// open disk file and set disk space to n_bytes
int openDisk(char *filename, int nBytes);

// close disk
int closeDisk(int disk);

// read block w block_num from disk to buffer
int readBlock(int disk, int bnum, void *block);

// write BLOCKSIZE bytes from buffer to block w block_num
int writeBlock(int disk, int bNum, void *block);

#endif