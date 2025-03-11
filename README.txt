FEATURES:

Names:
    Summer Cai
    Shawheen Ghezavat
    Lukas Shipley
    Casey Tran

Implementation:

Additional Functionality:

Limitations:

Testing:

- TinyFSDemo.c is our test file to test our tinyFS functionality.  
  It validates the core functionality (Phase 1 + 2 - 70%) and additional features

- Base Implementation Phase 1+2 (70%)

    tfs_mkfs() - Create a filesystem 
    tfs_mount() - Mount the filesystem 
    tfs_unmount() - Unmount the filesystem 
    tfs_openFile() - Open a file 
    tfs_closeFile() - Close a file 
    tfs_writeFile() - Write full content to a file 
    tfs_deleteFile() - Delete a file 
    tfs_readByte() - Read a byte 
    tfs_seek() - Move the file pointer 

- Additional Features
    - Directory Listing & File Renaming (10%)
        - tfs_rename(fileDescriptor FD, char* newName) - Renames an open file.
        - tfs_readdir() - Lists all files on the filesystem.
    - Read-Only & Write-Byte Support (10%)
        - tfs_makeRO(char *name) - Makes a file read-only.
        - tfs_makeRW(char *name) - Reverts a file to read-write mode.
        - tfs_writeByte(fileDescriptor FD, unsigned int data) - Writes a single byte to the file at the current pointer.
    - File Timestamps (10%)
        - tfs_readFileInfo(fileDescriptor FD, file_info *info) - Retrieves creation, modification, and access times for a file.
