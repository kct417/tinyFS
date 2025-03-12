FEATURES: ABDEH

Names:
    Summer Cai
    Shawheen Ghezavat
    Lukas Shipley
    Casey Tran

Implementation: TinyFS provides a simple file system with basic file operations, 
including file creation, deletion, read/write, renaming, and listing directory 
contents. However, tradeoffs were made to keep the system simple and efficient-
the flat directory structure sacrifices organization but avoids complexity. 
TinyFS successfully handles file operations, maintaining filesystem integrity, 
and supporting efficient disk usage through defragmentation.

Additional Functionality:
A: Fragmentation Info & Defragmentation: we implemented tfs_displayFragments(), 
which provides a visual map of the disk, distinguishing between used and free
blocks, and tfs_defrag(), which moves data blocks to ensure that free space is
consolidated at the end of the disk. This was tested by creating and deleting 
files to observe fragmentation patterns before and after defragmentation.

B: Directory Listing & File Renaming: we implemented tfs_readdir(), which lists
all files in the root directory, and tfs_rename(), which allows renaming of open
files while ensuring filename length constraints. This was tested by creating 
multiple files, renaming them, and checking if they appear correctly in directory
listings

D: Read-Only & Byte-Level Writing: we implemented tfs_makeRO() and tfs_makeRW(), 
which toggle file permission between read-only and read-write. We also implemented
tfs_writeByte, which allows modification of a specific byte inside a file at an 
offset. This was tested by setting a file to read-only and attempting to write/delete it.
Then, converted the file back to read/write and verified successful writes.

E: Timestamps for File Operations: we implemented tfs_readFileInfo(fileDescriptor FD),
which retrieves creation, modification, and access timestamps. This was tested by
creating files and checking their timestamps, modifying files and verifying timestamp
updates. 

H: Filesystem Consistency Checks: we implemented this by verifying that on mounting,
blocks are either allocated to an inode or on the free list. No orphaned blocks exist.
No blocks are assigned to both free list and inode. We tested this by creating and deleting
files, then unmounted and mounted to verify consistency, and verified correct
handling of invalid disk states. 

Limitations: No known limitations or bugs.
