FEATURES: ABDEH

Names:
    Summer Cai
    Shawheen Ghezavat
    Lukas Shipley
    Casey Tran

Implementation:
    TinyFS provides a simple file system with basic file operations, including
    file creation, deletion, read/write, renaming, and listing directory contents.
    However, tradeoffs were made to keep the system simple and efficient the flat
    directory structure sacrifices organization but avoids complexity. TinyFS
    successfully handles file operations, maintaining filesystem integrity, and
    supporting efficient disk usage through defragmentation.

    The library is implemented with a superblock and 8 inodes allocated upon
    creation. The inodes are either active or inactive representing if the a file
    is stored in the inode. It uses a linked list implementation for connecting
    data blocks and free blocks. This allows for easy storage of files as data
    blocks can be linked as needed. However, the tradeoff is that blocks are not
    stored contiguously which makes reading and writing to the file slower. The
    library stores all of the data in a single directory. Each inode has a
    read-only bit that when set prevents writes to and deletions of the file.
    Initially, when a file is created, no data blocks are allocated. Data blocks
    are allocated when the file is written to and the file size is updated. when
    the disk is created all free blocks are linked in a contiguous chain with the
    superblock pointing to the first free block.

    - tfs_mkfs() - Create a filesystem 
    - tfs_mount() - Mount the filesystem 
    - tfs_unmount() - Unmount the filesystem 
    - tfs_openFile() - Open a file 
    - tfs_closeFile() - Close a file 
    - tfs_writeFile() - Write full content to a file 
    - tfs_deleteFile() - Delete a file 
    - tfs_readByte() - Read a byte 
    - tfs_seek() - Move the file pointer 

Additional Functionality:
    A: Fragmentation Info & Defragmentation: we implemented tfs_displayFragments(), 
    which provides a visual map of the disk, distinguishing between used and free
    blocks, and tfs_defrag(), which moves data blocks to ensure that free space is
    consolidated at the end of the disk. This was tested by creating and deleting 
    files to observe fragmentation patterns before and after defragmentation.

    - tfs_displayFragments() - Shows a map of all blocks with the non-free blocks clearly designated.
    - tfs_defrag() - Moves blocks such that all free blocks are contiguous at the end of the disk.

    B: Directory Listing & File Renaming: we implemented tfs_readdir(), which lists
    all files in the root directory, and tfs_rename(), which allows renaming of open
    files while ensuring filename length constraints. This was tested by creating 
    multiple files, renaming them, and checking if they appear correctly in directory
    listings.

    - tfs_rename(fileDescriptor FD, char* newName) - Renames an open file.
    - tfs_readdir() - Lists all files on the filesystem.

    D: Read-Only & Byte-Level Writing: we implemented tfs_makeRO() and tfs_makeRW(), 
    which toggle file permission between read-only and read-write. We also implemented
    tfs_writeByte, which allows modification of a specific byte inside a file at the
    current fileDescriptor. The file descriptor was incremented by one. This was tested
    by setting a file to read-only and attempting to write/delete it. Then, converted
    the file back to read/write and verified successful writes. A byte was also written
    to a file and read to confirm that the byte was written.

    - tfs_makeRO(char *name) - Makes a file read-only.
    - tfs_makeRW(char *name) - Reverts a file to read-write mode.
    - tfs_writeByte(fileDescriptor FD, unsigned int data) - Writes a single byte to the file at the current pointer.

    E: Timestamps for File Operations: we implemented tfs_readFileInfo(fileDescriptor FD),
    which retrieves creation, modification, and access timestamps. This was tested by
    creating files and checking their timestamps, modifying files and verifying timestamp
    updates.

    - tfs_readFileInfo(fileDescriptor FD) - Prints creation, modification, and access times for a file.

    H: Filesystem Consistency Checks: we implemented this by verifying that on mounting,
    blocks are either allocated to an inode or on the free list. No orphaned blocks exist.
    No blocks are assigned to both free list and inode. We tested this by creating and deleting
    files, then unmounted and mounted to verify consistency, and verified correct
    handling of invalid disk states. 

    - Inconsistencies verified upon mount
        - Disk size (through openDisk)
        - Block count (through blocks verified)
        - Superblock
            - Magic number
            - Empty byte
            - Type
            - First free block
        - Inode blocks
            - Magic number
            - Empty byte
            - Type
            - First data block
            - Timestamps
        - Data blocks
            - Magic number
            - Empty byte
            - Type
            - Next data block
        - Free blocks
            - Magic number
            - Empty byte
            - Type
            - Next free block

Limitations:
    The number of inode blocks is set to 8 inodes. This means that the disk size will be at least
    2304 bytes to account for the superblock and the 8 inodes. Disk sizes larger than this are
    supported by the library.