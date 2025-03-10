#ifndef _TINYFS_H
#define _TINYFS_H

/* The default size of the disk and file system block */
#define BLOCKSIZE 256

/* Your program should use a 10240 Byte disk size giving you 40 blocks
total. This is a default size. You must be able to support different
possible values */
#define DEFAULT_DISK_SIZE 10240

/* use this name for a default emulated disk file name */
#define DEFAULT_DISK_NAME "tinyFSDisk"

/* use as a special type to keep track of files */
typedef int fileDescriptor;

/* Makes a blank TinyFS file system of size nBytes on the unix file
specified by ‘filename’. This function should use the emulated disk
library to open the specified unix file, and upon success, format the
file to be a mountable disk. This includes initializing all data to 0x00,
setting magic numbers, initializing and writing the superblock and
inodes, etc. Must return a specified success/error code. */
int tfs_mkfs(char *filename, int nBytes);

/* tfs_mount(char *diskname) “mounts” a TinyFS file system located within
‘diskname’. tfs_unmount(void) “unmounts” the currently mounted file
system. As part of the mount operation, tfs_mount should verify the file
system is the correct type. In tinyFS, only one file system may be
mounted at a time. Use tfs_unmount to cleanly unmount the currently
mounted file system. Must return a specified success/error code. */
int tfs_mount(char *diskname);
int tfs_unmount(void);

/* Creates or Opens a file for reading and writing on the currently
mounted file system. Creates a dynamic resource table entry for the file,
and returns a file descriptor (integer) that can be used to reference
this entry while the filesystem is mounted. */
fileDescriptor tfs_openFile(char *name);

/* Closes the file, de-allocates all system resources, and removes table
entry */
int tfs_closeFile(fileDescriptor FD);

/* Writes buffer ‘buffer’ of size ‘size’, which represents an entire
file’s content, to the file system. Previous content (if any) will be
completely lost. Sets the file pointer to 0 (the start of file) when
done. Returns success/error codes. */
int tfs_writeFile(fileDescriptor FD, char *buffer, int size);

/* deletes a file and marks its blocks as free on disk. */
int tfs_deleteFile(fileDescriptor FD);

/* reads one byte from the file and copies it to buffer, using the
current file pointer location and incrementing it by one upon success.
If the file pointer is already past the end of the file then
tfs_readByte() should return an error and not increment the file pointer.
*/
int tfs_readByte(fileDescriptor FD, char *buffer);

/* change the file pointer location to offset (absolute). Returns
success/error codes.*/
int tfs_seek(fileDescriptor FD, int offset);

// internal definitions
#define MAGIC_NUMBER 0x44

#define MAX_FILENAME_LENGTH 8
#define MAX_INODES 8

#define BLOCK_PADDING (BLOCKSIZE - sizeof(unsigned char) * 4 - sizeof(int))
#define SUPERBLOCK_PADDING (BLOCKSIZE - sizeof(unsigned char) * 4 - sizeof(int) * 2)
#define INODEBLOCK_PADDING (BLOCKSIZE - sizeof(unsigned char) * 4 - sizeof(int) * 2 - MAX_FILENAME_LENGTH - 1)
#define FREEBLOCK_PADDING (BLOCKSIZE - sizeof(unsigned char) * 4 - sizeof(int))

#define EFFECTIVE_DATA_SIZE (BLOCKSIZE - sizeof(unsigned char) * 4 - sizeof(int))

// type definitions
typedef struct block_t
{
    unsigned char type;
    unsigned char magic_number;
    unsigned char block_address;
    unsigned char empty;
    int next_block;
    unsigned char data[BLOCK_PADDING];
} block_t;

typedef struct superblock_t
{
    unsigned char type;
    unsigned char magic_number;
    unsigned char block_address;
    unsigned char empty;
    int root_inode;
    int free_block;
    unsigned char data[SUPERBLOCK_PADDING];
} superblock_t;

typedef struct inodeblock_t
{
    unsigned char type;
    unsigned char magic_number;
    unsigned char block_address;
    unsigned char empty;
    int first_block;
    int size;
    char filename[MAX_FILENAME_LENGTH + 1];
    unsigned char data[INODEBLOCK_PADDING];
} inodeblock_t;

typedef struct datablock_t
{
    unsigned char type;
    unsigned char magic_number;
    unsigned char block_address;
    unsigned char empty;
    int next_block;
    char data[EFFECTIVE_DATA_SIZE];
} datablock_t;

typedef struct freeblock_t
{
    unsigned char type;
    unsigned char magic_number;
    unsigned char block_address;
    unsigned char empty;
    int next_block;
    unsigned char data[FREEBLOCK_PADDING];
} freeblock_t;

typedef struct inode_entry_t
{
    int active;
    inodeblock_t inode;
} inode_entry_t;

typedef struct file_entry_t
{
    int active;
    int file_descriptor;
    int inode_table_entry;
    char filename[MAX_FILENAME_LENGTH + 1];
} file_entry_t;

typedef struct mounted_disk
{
    int mounted;
    int size;
    int disk_descriptor;
    superblock_t superblock;
} mounted_disk;

#endif
