#ifndef TINYFS_ERRNO_H
#define TINYFS_ERRNO_H

// man 3 errno

#define TFS_SUCCESS 0         // Success
#define TFS_ERROR -1          // Generic error
#define TFS_DISK_ERROR -2     // Disk error
#define TFS_INVALID_ARG -3    // Invalid argument
#define TFS_FILE_NOT_FOUND -4 // File not found
#define TFS_NO_FREE_BLOCKS -5 // No free blocks
#define TFS_FILE_EXISTS -6    // File exists
#define TFS_READ_ONLY -7      // Read only
#define TFS_NOT_MOUNTED -8    // Not mounted
#define TFS_OUT_OF_BOUNDS -9  // Out of bounds
#define TFS_NO_SPACE -10      // No space left on file system
#define TFS_CORRUPTED_FS -11  // Corrupted file system

#endif // TINYFS_ERRNO_H