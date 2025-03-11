#include "disk.h"
#include "tinyFS.h"
#include "tinyFS_errno.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// global variables
static mounted_disk md = {0};
static inode_entry_t inode_table[_TFS_MAX_INODES];
static file_entry_t file_table[_TFS_MAX_INODES];

int tfs_mkfs(char *filename, int nBytes)
{
    // validate filename
    if (!filename)
    {
        filename = DEFAULT_DISK_NAME;
    }

    // validate disk extension
    char *fileExtension = strrchr(filename, '.');
    char newFilename[strlen(filename) + 5];
    // add .dsk extension if no extension provided
    if (!fileExtension)
    {
        sprintf(newFilename, "%s%s", filename, ".dsk");
        filename = newFilename;
    }
    else
    {
        if (strcmp(fileExtension, ".dsk") != 0)
        {
            tfs_errno = TFS_ERR_INVALID_ARGUMENT;
            return TFS_FAILURE;
        }
    }

    // validate disk size
    if (!nBytes)
    {
        nBytes = DEFAULT_DISK_SIZE;
    }

    // create disk
    if ((md.disk_descriptor = openDisk(filename, nBytes)) == -1)
    {
        tfs_errno = TFS_ERR_OPEN;
        return TFS_FAILURE;
    }

    // create superblock
    superblock_t superblock;
    memset(&superblock, 0x00, BLOCKSIZE);
    superblock.type = 1;
    superblock.magic_number = _TFS_MAGIC_NUMBER;
    superblock.root_inode = 1;
    superblock.free_block = _TFS_MAX_INODES + 1;

    // write superblock to disk
    if (writeBlock(md.disk_descriptor, 0, &superblock) == TFS_FAILURE)
    {
        tfs_errno = TFS_ERR_WRITE;
        return TFS_FAILURE;
    }

    // create inode block
    inodeblock_t inodeblock;
    memset(&inodeblock, 0x00, BLOCKSIZE);
    inodeblock.type = 2;
    inodeblock.magic_number = superblock.magic_number;

    // write inode blocks to disk
    for (int i = 1; i < _TFS_MAX_INODES + 1; i++)
    {
        if (writeBlock(md.disk_descriptor, i, &inodeblock) == TFS_FAILURE)
        {
            tfs_errno = TFS_ERR_WRITE;
            return TFS_FAILURE;
        }
    }

    // create free block
    freeblock_t freeblock;
    memset(&freeblock, 0x00, BLOCKSIZE);
    freeblock.type = 4;
    freeblock.magic_number = superblock.magic_number;

    // write free blocks to disk
    for (int i = _TFS_MAX_INODES + 1; i < nBytes / BLOCKSIZE; i++)
    {
        // set linked list of free blocks
        freeblock.next_block = (i + 1) % (nBytes / BLOCKSIZE);
        if (writeBlock(md.disk_descriptor, i, &freeblock) == TFS_FAILURE)
        {
            tfs_errno = TFS_ERR_WRITE;
            return TFS_FAILURE;
        }
    }

    // close disk
    if (closeDisk(md.disk_descriptor) == TFS_FAILURE)
    {
        tfs_errno = TFS_ERR_CLOSE;
        return TFS_FAILURE;
    }

    tfs_errno = TFS_SUCCESS;
    return TFS_SUCCESS;
}

int tfs_mount(char *diskname)
{
    // check for mounted disk
    if (md.mounted)
    {
        tfs_errno = TFS_ERR_MOUNTED_DISK;
        return TFS_FAILURE;
    }

    // validate disk extension
    char *diskExtension = strrchr(diskname, '.');
    char newDiskname[strlen(diskname) + 5];
    // add .dsk extension if no extension provided
    if (!diskExtension)
    {
        sprintf(newDiskname, "%s%s", diskname, ".dsk");
        diskname = newDiskname;
    }
    else
    {
        if (strcmp(diskExtension, ".dsk") != 0)
        {
            tfs_errno = TFS_ERR_INVALID_ARGUMENT;
            return TFS_FAILURE;
        }
    }

    // open disk
    if ((md.disk_descriptor = openDisk(diskname, 0)) == -1)
    {
        tfs_errno = TFS_ERR_OPEN;
        return TFS_FAILURE;
    }

    // get disk size
    if ((md.size = lseek(md.disk_descriptor, 0, SEEK_END)) == -1)
    {
        tfs_errno = TFS_ERR_SEEK;
        return TFS_FAILURE;
    }

    // get superblock
    if (readBlock(md.disk_descriptor, 0, &md.superblock) == -1)
    {
        tfs_errno = TFS_ERR_READ;
        return TFS_FAILURE;
    }

    // initialize inode table
    inodeblock_t inodeblock;
    for (int i = 1; i < _TFS_MAX_INODES + 1; i++)
    {
        // get inode block
        if (readBlock(md.disk_descriptor, i, &inodeblock) == -1)
        {
            tfs_errno = TFS_ERR_READ;
            return TFS_FAILURE;
        }

        // validate inode block
        if (inodeblock.magic_number != md.superblock.magic_number || inodeblock.type != 2)
        {
            tfs_errno = TFS_ERR_CORRUPTED_DISK;
            return TFS_FAILURE;
        }

        // check if inode is active
        if (inodeblock.first_block != 0)
        {
            // update inode table
            inode_table[i - 1].active = 1;
            memcpy(&inode_table[i - 1].inode, &inodeblock, sizeof(inodeblock_t));
        }
    }

    // initialize file table
    for (int i = 0; i < _TFS_MAX_INODES; i++)
    {
        file_table[i].active = 0;
    }

    // validate blocks
    block_t block;
    for (int i = _TFS_MAX_INODES + 1; i < md.size / BLOCKSIZE; i++)
    {
        // get block
        if (readBlock(md.disk_descriptor, i, &block) == -1)
        {
            tfs_errno = TFS_ERR_READ;
            return TFS_FAILURE;
        }

        // validate block
        if (block.magic_number != md.superblock.magic_number)
        {
            tfs_errno = TFS_ERR_CORRUPTED_DISK;
            return TFS_FAILURE;
        }
    }

    // mount disk
    md.mounted = 1;

    tfs_errno = TFS_SUCCESS;
    return TFS_SUCCESS;
}

int tfs_unmount(void)
{
    // check for mounted disk
    if (!md.mounted)
    {
        tfs_errno = TFS_ERR_NO_DISK;
        return TFS_FAILURE;
    }

    // close disk
    if (closeDisk(md.disk_descriptor) == -1)
    {
        tfs_errno = TFS_ERR_CLOSE;
        return TFS_FAILURE;
    }

    // unmount disk
    md.mounted = 0;

    tfs_errno = TFS_SUCCESS;
    return TFS_SUCCESS;
}

fileDescriptor tfs_openFile(char *name)
{
    // check for mounted disk
    if (!md.mounted)
    {
        tfs_errno = TFS_ERR_NO_DISK;
        return TFS_FAILURE;
    }

    // validate filename
    if (!name || strlen(name) > _TFS_MAX_FILENAME_LENGTH)
    {
        tfs_errno = TFS_ERR_INVALID_ARGUMENT;
        return TFS_FAILURE;
    }

    // check for file in file table
    time_t curtime;
    time_t old_time;
    inodeblock_t *inode_table_block;
    for (int i = 0; i < _TFS_MAX_INODES; i++)
    {
        if (file_table[i].active && strcmp(file_table[i].filename, name) == 0)
        {
            // update inode table
            inode_table_block = &inode_table[file_table[i].inode_table_entry].inode;
            old_time = inode_table_block->accessed;
            time(&curtime);
            inode_table_block->accessed = curtime;

            // write inode block to disk
            if (writeBlock(md.disk_descriptor, file_table[i].inode_table_entry + 1, inode_table_block) == -1)
            {
                // revert time accessed
                inode_table_block->accessed = old_time;
                tfs_errno = TFS_ERR_WRITE;
                return TFS_FAILURE;
            }

            tfs_errno = TFS_SUCCESS;
            return i;
        }
    }

    // not in file table

    // check for file in inode table
    int file_table_entry = -1;
    for (int i = 0; i < _TFS_MAX_INODES; i++)
    {
        inode_table_block = &inode_table[i].inode;
        if (inode_table[i].active && strcmp(inode_table_block->filename, name) == 0)
        {
            // find free file entry
            for (int j = 0; j < _TFS_MAX_INODES; j++)
            {
                if (!file_table[j].active)
                {
                    file_table_entry = j;
                    break;
                }
            }
            if (file_table_entry == -1)
            {
                tfs_errno = TFS_ERR_NO_SPACE;
                return TFS_FAILURE;
            }

            // update inode table
            old_time = inode_table_block->accessed;
            time(&curtime);
            inode_table_block->accessed = curtime;

            // write inode block to disk
            if (writeBlock(md.disk_descriptor, i + 1, inode_table_block) == -1)
            {
                // revert time accessed
                inode_table[i].inode.accessed = old_time;
                tfs_errno = TFS_ERR_WRITE;
                return TFS_FAILURE;
            }

            // update file table
            file_table[file_table_entry].active = 1;
            file_table[file_table_entry].file_descriptor = 0;
            file_table[file_table_entry].inode_table_entry = i;
            strncpy(file_table[file_table_entry].filename, name, _TFS_MAX_FILENAME_LENGTH + 1);

            tfs_errno = TFS_SUCCESS;
            return file_table_entry;
        }
    }

    // not in inode table

    // find free inode block
    int inode_table_entry = -1;
    for (int i = 0; i < _TFS_MAX_INODES; i++)
    {
        if (!inode_table[i].active)
        {
            inode_table_entry = i;
            break;
        }
    }

    // find free file entry
    file_table_entry = -1;
    for (int i = 0; i < _TFS_MAX_INODES; i++)
    {
        if (!file_table[i].active)
        {
            file_table_entry = i;
            break;
        }
    }

    // check for free inode block and file entry
    if (inode_table_entry == -1 || file_table_entry == -1)
    {
        tfs_errno = TFS_ERR_NO_SPACE;
        return TFS_FAILURE;
    }

    // create inode block
    inodeblock_t inodeblock;
    memset(&inodeblock, 0x00, BLOCKSIZE);
    inodeblock.type = 2;
    inodeblock.magic_number = md.superblock.magic_number;
    inodeblock.size = 0;
    time(&curtime);
    inodeblock.created = curtime;
    inodeblock.modified = curtime;
    inodeblock.accessed = curtime;
    strncpy(inodeblock.filename, name, _TFS_MAX_FILENAME_LENGTH + 1);

    // write inode block to disk
    if (writeBlock(md.disk_descriptor, inode_table_entry + 1, &inodeblock) == -1)
    {
        tfs_errno = TFS_ERR_WRITE;
        return TFS_FAILURE;
    }

    // update inode table and file table
    inode_table[inode_table_entry].active = 1;
    memcpy(&inode_table[inode_table_entry].inode, &inodeblock, sizeof(inodeblock_t));

    file_table[file_table_entry].active = 1;
    file_table[file_table_entry].file_descriptor = 0;
    file_table[file_table_entry].inode_table_entry = inode_table_entry;
    strncpy(file_table[file_table_entry].filename, name, _TFS_MAX_FILENAME_LENGTH + 1);

    tfs_errno = TFS_SUCCESS;
    return file_table_entry;
}

int tfs_closeFile(fileDescriptor FD)
{
    // check if disk is mounted
    if (!md.mounted)
    {
        tfs_errno = TFS_ERR_NO_DISK;
        return TFS_FAILURE;
    }

    // validate file descriptor
    if (!file_table[FD].active)
    {
        tfs_errno = TFS_ERR_FILE_DESCRIPTOR;
        return TFS_FAILURE;
    }

    // close file
    file_table[FD].active = 0;

    tfs_errno = TFS_SUCCESS;
    return TFS_SUCCESS;
}

int tfs_writeFile(fileDescriptor FD, char *buffer, int size)
{
    // check if disk is mounted
    if (!md.mounted)
    {
        tfs_errno = TFS_ERR_NO_DISK;
        return TFS_FAILURE;
    }

    // validate file descriptor
    if (!file_table[FD].active)
    {
        tfs_errno = TFS_ERR_FILE_DESCRIPTOR;
        return TFS_FAILURE;
    }

    // check for read only file
    inodeblock_t *inode_table_block = &inode_table[file_table[FD].inode_table_entry].inode;
    if (inode_table_block->read_only)
    {
        tfs_errno = TFS_ERR_READ_ONLY;
        return TFS_FAILURE;
    }

    // calculate blocks needed
    int blocks_needed = size / _TFS_EFFECTIVE_DATA_SIZE;
    if (size % _TFS_EFFECTIVE_DATA_SIZE != 0)
    {
        blocks_needed++;
    }

    // write file to disk
    block_t block;
    datablock_t datablock;
    memset(&datablock, 0x00, BLOCKSIZE);
    datablock.type = 3;
    datablock.magic_number = md.superblock.magic_number;
    int blocks_written = 0;
    int offset = 0;
    int bytes_to_write = _TFS_EFFECTIVE_DATA_SIZE;
    int block_number = md.superblock.free_block;
    while (blocks_written < blocks_needed)
    {
        // check for free blocks
        if (block_number == 0)
        {
            tfs_errno = TFS_ERR_NO_SPACE;
            return TFS_FAILURE;
        }

        // get free block from disk for next block
        if (readBlock(md.disk_descriptor, block_number, &block) == -1)
        {
            tfs_errno = TFS_ERR_READ;
            return TFS_FAILURE;
        }

        // copy data to data block
        offset = blocks_written * _TFS_EFFECTIVE_DATA_SIZE;
        if (offset + bytes_to_write > size)
        {
            bytes_to_write = size - offset;
        }
        memcpy(datablock.data, buffer + blocks_written * _TFS_EFFECTIVE_DATA_SIZE, bytes_to_write);

        // set free block to next block if needed otherwise set to 0
        blocks_written++;
        if (blocks_written == blocks_needed)
        {
            datablock.next_block = 0;
        }
        else
        {
            datablock.next_block = block.next_block;
        }

        // write data block to disk
        if (writeBlock(md.disk_descriptor, block_number, &datablock) == -1)
        {
            tfs_errno = TFS_ERR_WRITE;
            return TFS_FAILURE;
        }

        // set next block
        block_number = block.next_block;
    }

    // update inode table
    inode_table_block->size = size;
    inode_table_block->first_block = md.superblock.free_block;
    time_t curtime;
    time_t old_modified = inode_table_block->modified;
    time_t old_access = inode_table_block->accessed;
    time(&curtime);
    inode_table_block->modified = curtime;
    inode_table_block->accessed = curtime;

    // write inode block to disk
    if (writeBlock(md.disk_descriptor, file_table[FD].inode_table_entry + 1, inode_table_block) == -1)
    {
        // revert time accessed and modified
        inode_table_block->modified = old_modified;
        inode_table_block->accessed = old_access;
        tfs_errno = TFS_ERR_WRITE;
        return TFS_FAILURE;
    }

    // write superblock to disk
    md.superblock.free_block = block_number;
    if (writeBlock(md.disk_descriptor, 0, &md.superblock) == -1)
    {
        tfs_errno = TFS_ERR_WRITE;
        return TFS_FAILURE;
    }

    // Set file descriptor to start of file
    file_table[FD].file_descriptor = 0;

    tfs_errno = TFS_SUCCESS;
    return TFS_SUCCESS;
}

int tfs_deleteFile(fileDescriptor FD)
{
    // check if disk is mounted
    if (!md.mounted)
    {
        tfs_errno = TFS_ERR_NO_DISK;
        return TFS_FAILURE;
    }

    // check if file descriptor is valid
    if (!file_table[FD].active)
    {
        tfs_errno = TFS_ERR_FILE_DESCRIPTOR;
        return TFS_FAILURE;
    }

    // check for read only file
    inodeblock_t *inode_table_block = &inode_table[file_table[FD].inode_table_entry].inode;
    if (inode_table_block->read_only)
    {
        tfs_errno = TFS_ERR_READ_ONLY;
        return TFS_FAILURE;
    }

    // reset inode block
    inodeblock_t inodeblock;
    memset(&inodeblock, 0x00, BLOCKSIZE);
    inodeblock.type = 2;
    inodeblock.magic_number = md.superblock.magic_number;

    // write inode block to disk
    if (writeBlock(md.disk_descriptor, file_table[FD].inode_table_entry + 1, &inodeblock) == -1)
    {
        tfs_errno = TFS_ERR_WRITE;
        return TFS_FAILURE;
    }

    // reset data blocks
    datablock_t datablock;
    freeblock_t freeblock;
    memset(&freeblock, 0x00, BLOCKSIZE);
    freeblock.type = 4;
    freeblock.magic_number = md.superblock.magic_number;
    int block_number = inode_table_block->first_block;
    int total_blocks = inode_table_block->size / _TFS_EFFECTIVE_DATA_SIZE;
    if (inode_table_block->size % _TFS_EFFECTIVE_DATA_SIZE != 0)
    {
        total_blocks++;
    }
    for (int i = 0; i < total_blocks; i++)
    {
        // get data block
        if (readBlock(md.disk_descriptor, block_number, &datablock) == -1)
        {
            tfs_errno = TFS_ERR_READ;
            return TFS_FAILURE;
        }

        // check for end of file
        if (datablock.next_block == 0)
        {
            // set free block to superblock free block
            freeblock.next_block = md.superblock.free_block;
            // set superblock free block to first block
            md.superblock.free_block = inode_table_block->first_block;

            // write free block to disk
            if (writeBlock(md.disk_descriptor, block_number, &freeblock) == -1)
            {
                tfs_errno = TFS_ERR_WRITE;
                return TFS_FAILURE;
            }

            // write superblock to disk
            if (writeBlock(md.disk_descriptor, 0, &md.superblock) == -1)
            {
                tfs_errno = TFS_ERR_WRITE;
                return TFS_FAILURE;
            }

            break;
        }

        // write free block to disk
        freeblock.next_block = datablock.next_block;
        if (writeBlock(md.disk_descriptor, block_number, &freeblock) == -1)
        {
            tfs_errno = TFS_ERR_WRITE;
            return TFS_FAILURE;
        }

        // set next block
        block_number = datablock.next_block;
    }

    // update inode table and file table
    inode_table[file_table[FD].inode_table_entry].active = 0;
    file_table[FD].active = 0;

    tfs_errno = TFS_SUCCESS;
    return TFS_SUCCESS;
}

int tfs_readByte(fileDescriptor FD, char *buffer)
{
    // check if disk is mounted
    if (!md.mounted)
    {
        tfs_errno = TFS_ERR_NO_DISK;
        return TFS_FAILURE;
    }

    // validate file descriptor
    if (!file_table[FD].active)
    {
        tfs_errno = TFS_ERR_FILE_DESCRIPTOR;
        return TFS_FAILURE;
    }

    // check for end of file
    inodeblock_t *inodeblock = &inode_table[file_table[FD].inode_table_entry].inode;
    if (file_table[FD].file_descriptor == inodeblock->size)
    {
        tfs_errno = TFS_ERR_END_OF_FILE;
        return TFS_FAILURE;
    }

    // find data block
    datablock_t datablock;
    int block_number = inodeblock->first_block;
    int block_offset = file_table[FD].file_descriptor / _TFS_EFFECTIVE_DATA_SIZE;
    int data_offset = file_table[FD].file_descriptor % _TFS_EFFECTIVE_DATA_SIZE;
    for (int i = 0; i < block_offset + 1; i++)
    {
        // get data block
        if (readBlock(md.disk_descriptor, block_number, &datablock) == -1)
        {
            tfs_errno = TFS_ERR_READ;
            return TFS_FAILURE;
        }
        block_number = datablock.next_block;
    }

    // update inode table
    time_t curtime;
    time_t old_time = inodeblock->accessed;
    time(&curtime);
    inodeblock->accessed = curtime;

    // write inode block to disk
    if (writeBlock(md.disk_descriptor, file_table[FD].inode_table_entry + 1, inodeblock) == -1)
    {
        // revert time accessed
        inodeblock->accessed = old_time;
        tfs_errno = TFS_ERR_WRITE;
        return TFS_FAILURE;
    }

    // read byte from data block and update file descriptor
    *buffer = datablock.data[data_offset];
    file_table[FD].file_descriptor++;

    tfs_errno = TFS_SUCCESS;
    return TFS_SUCCESS;
}

int tfs_seek(fileDescriptor FD, int offset)
{
    // validate file descriptor
    if (!file_table[FD].active)
    {
        tfs_errno = TFS_ERR_FILE_DESCRIPTOR;
        return TFS_FAILURE;
    }

    // validate offset
    inodeblock_t *inodeblock = &inode_table[file_table[FD].inode_table_entry].inode;
    if (offset < 0 || offset > inodeblock->size)
    {
        tfs_errno = TFS_ERR_INVALID_ARGUMENT;
        return TFS_FAILURE;
    }

    // set file pointer to offset
    file_table[FD].file_descriptor = offset;

    tfs_errno = TFS_SUCCESS;
    return TFS_SUCCESS;
}

int tfs_makeRO(char *name)
{
    // check for mounted disk
    if (!md.mounted)
    {
        tfs_errno = TFS_ERR_NO_DISK;
        return TFS_FAILURE;
    }

    // validate filename
    if (!name || strlen(name) > _TFS_MAX_FILENAME_LENGTH)
    {
        tfs_errno = TFS_ERR_INVALID_ARGUMENT;
        return TFS_FAILURE;
    }

    // check for file in file table
    int current_mode;
    inodeblock_t *inode_table_block;
    for (int i = 0; i < _TFS_MAX_INODES; i++)
    {
        inode_table_block = &inode_table[file_table[i].inode_table_entry].inode;
        if (file_table[i].active && strcmp(file_table[i].filename, name) == 0)
        {
            // set read only flag
            current_mode = inode_table_block->read_only;
            inode_table_block->read_only = 1;

            // write inode block to disk
            if (writeBlock(md.disk_descriptor, file_table[i].inode_table_entry + 1, inode_table_block) == -1)
            {
                // revert inode table
                inode_table_block->read_only = current_mode;
                tfs_errno = TFS_ERR_WRITE;
                return TFS_FAILURE;
            }

            tfs_errno = TFS_SUCCESS;
            return TFS_SUCCESS;
        }
    }

    // not in file table

    // check for file in inode table
    for (int i = 0; i < _TFS_MAX_INODES; i++)
    {
        inode_table_block = &inode_table[i].inode;
        if (inode_table[i].active && strcmp(inode_table_block->filename, name) == 0)
        {
            // set read only flag
            current_mode = inode_table_block->read_only;
            inode_table_block->read_only = 1;

            // write inode block to disk
            if (writeBlock(md.disk_descriptor, i + 1, inode_table_block) == -1)
            {
                inode_table_block->read_only = current_mode;
                tfs_errno = TFS_ERR_WRITE;
                return TFS_FAILURE;
            }

            tfs_errno = TFS_SUCCESS;
            return TFS_SUCCESS;
        }
    }

    tfs_errno = TFS_ERR_INVALID_ARGUMENT;
    return TFS_FAILURE;
}

int tfs_makeRW(char *name)
{
    // check for mounted disk
    if (!md.mounted)
    {
        tfs_errno = TFS_ERR_NO_DISK;
        return TFS_FAILURE;
    }

    // validate filename
    if (!name || strlen(name) > _TFS_MAX_FILENAME_LENGTH)
    {
        tfs_errno = TFS_ERR_INVALID_ARGUMENT;
        return TFS_FAILURE;
    }

    // check for file in file table
    int current_mode;
    inodeblock_t *inode_table_block;
    for (int i = 0; i < _TFS_MAX_INODES; i++)
    {
        inode_table_block = &inode_table[file_table[i].inode_table_entry].inode;
        if (file_table[i].active && strcmp(file_table[i].filename, name) == 0)
        {
            // unset read only flag
            current_mode = inode_table_block->read_only;
            inode_table_block->read_only = 0;

            // write inode block to disk
            if (writeBlock(md.disk_descriptor, file_table[i].inode_table_entry + 1, inode_table_block) == -1)
            {
                // revert inode table
                inode_table_block->read_only = current_mode;
                tfs_errno = TFS_ERR_WRITE;
                return TFS_FAILURE;
            }

            tfs_errno = TFS_SUCCESS;
            return TFS_SUCCESS;
        }
    }

    // check for file in inode table
    for (int i = 0; i < _TFS_MAX_INODES; i++)
    {
        inode_table_block = &inode_table[i].inode;
        if (inode_table[i].active && strcmp(inode_table_block->filename, name) == 0)
        {
            // unset read only flag
            current_mode = inode_table_block->read_only;
            inode_table_block->read_only = 0;

            // write inode block to disk
            if (writeBlock(md.disk_descriptor, i + 1, inode_table_block) == -1)
            {
                // revert inode table
                inode_table_block->read_only = current_mode;
                tfs_errno = TFS_ERR_WRITE;
                return TFS_FAILURE;
            }

            tfs_errno = TFS_SUCCESS;
            return TFS_SUCCESS;
        }
    }

    tfs_errno = TFS_ERR_INVALID_ARGUMENT;
    return TFS_FAILURE;
}

int tfs_writeByte(fileDescriptor FD, unsigned int data)
{
    // check if disk is mounted
    if (!md.mounted)
    {
        tfs_errno = TFS_ERR_NO_DISK;
        return TFS_FAILURE;
    }

    // validate file descriptor
    if (!file_table[FD].active)
    {
        tfs_errno = TFS_ERR_FILE_DESCRIPTOR;
        return TFS_FAILURE;
    }

    // check for end of file
    inodeblock_t *inodeblock = &inode_table[file_table[FD].inode_table_entry].inode;
    if (file_table[FD].file_descriptor == inodeblock->size)
    {
        tfs_errno = TFS_ERR_END_OF_FILE;
        return TFS_FAILURE;
    }

    // find data block
    datablock_t datablock;
    int block_number = inodeblock->first_block;
    int block_offset = file_table[FD].file_descriptor / _TFS_EFFECTIVE_DATA_SIZE;
    int data_offset = file_table[FD].file_descriptor % _TFS_EFFECTIVE_DATA_SIZE;
    for (int i = 0; i < block_offset + 1; i++)
    {
        // get data block
        if (readBlock(md.disk_descriptor, block_number, &datablock) == -1)
        {
            tfs_errno = TFS_ERR_READ;
            return TFS_FAILURE;
        }

        // exit if block offset reached
        if (i == block_offset)
        {
            break;
        }

        block_number = datablock.next_block;
    }

    // write byte to data block and update file descriptor
    datablock.data[data_offset] = data;
    file_table[FD].file_descriptor++;

    // write data block to disk
    if (writeBlock(md.disk_descriptor, block_number, &datablock) == -1)
    {
        tfs_errno = TFS_ERR_WRITE;
        return TFS_FAILURE;
    }

    // update inode table
    time_t curtime;
    time_t old_modified = inodeblock->modified;
    time_t old_access = inodeblock->accessed;
    time(&curtime);
    inodeblock->modified = curtime;
    inodeblock->accessed = curtime;

    // write inode block to disk
    if (writeBlock(md.disk_descriptor, file_table[FD].inode_table_entry + 1, inodeblock) == -1)
    {
        // revert time accessed and modified
        inodeblock->modified = old_modified;
        inodeblock->accessed = old_access;
        tfs_errno = TFS_ERR_WRITE;
        return TFS_FAILURE;
    }

    tfs_errno = TFS_SUCCESS;
    return TFS_SUCCESS;
}

int tfs_readFileInfo(fileDescriptor FD, file_info *info)
{
    // check if disk is mounted
    if (!md.mounted)
    {
        tfs_errno = TFS_ERR_NO_DISK;
        return TFS_FAILURE;
    }

    // validate file descriptor
    if (!file_table[FD].active)
    {
        tfs_errno = TFS_ERR_FILE_DESCRIPTOR;
        return TFS_FAILURE;
    }

    // get inode block
    inodeblock_t *inodeblock = &inode_table[file_table[FD].inode_table_entry].inode;

    // Format the time into a string
    struct tm *time_info;
    time_info = localtime(&inodeblock->created);
    strftime(info->created, sizeof(char) * 22, "[%m-%d-%Y %H:%M:%S]", time_info);
    time_info = localtime(&inodeblock->modified);
    strftime(info->modified, sizeof(char) * 22, "[%m-%d-%Y %H:%M:%S]", time_info);
    time_info = localtime(&inodeblock->accessed);
    strftime(info->accessed, sizeof(char) * 22, "[%m-%d-%Y %H:%M:%S]", time_info);

    tfs_errno = TFS_SUCCESS;
    return TFS_SUCCESS;
}


int tfs_rename(fileDescriptor FD, char *newName) {
    // validate file descriptor
    if (!file_table[FD].active)
    {
        tfs_errno = TFS_ERR_FILE_DESCRIPTOR;
        return TFS_FAILURE;
    }

    // validate length and name
    if (strlen(newName) > _TFS_MAX_FILENAME_LENGTH) {
        tfs_errno = TFS_ERR_INVALID_ARGUMENT;
        return TFS_FAILURE;
    }

    // Check if a file with the new name already exists
    for (int i = 0; i < _TFS_MAX_INODES; i++) {
        if (inode_table[i].active && strcmp(inode_table[i].inode.filename, newName) == 0) {
            tfs_errno = TFS_ERR_INVALID_ARGUMENT;
            return TFS_FAILURE;
        }
    }

    // find inode idx
    int inode_idx = file_table[FD].inode_table_entry;
    if (!inode_table[inode_idx].active) {
        tfs_errno = TFS_ERR_INVALID_ARGUMENT;
        return TFS_FAILURE;
    }

    // rename
    strncpy(inode_table[inode_idx].inode.filename, newName, _TFS_MAX_FILENAME_LENGTH);
    inode_table[inode_idx].inode.filename[_TFS_MAX_FILENAME_LENGTH] = '\0';
    // update file table
    strncpy(file_table[FD].filename, newName, _TFS_MAX_FILENAME_LENGTH);
    file_table[FD].filename[_TFS_MAX_FILENAME_LENGTH] = '\0';

    return TFS_SUCCESS;
}

void tfs_readdir() {
    printf("----------------- \n");
    printf("TinyFS Directory: \n");

    int count = 0;
    for (int i = 0; i < _TFS_MAX_INODES; i++) {
        if (inode_table[i].active) {
            printf("*  %s\n", inode_table[i].inode.filename);
            count++;
        }
    }

    if (count == 0) {
        printf("No files found.\n");
    }
    printf("----------------- \n");
}
