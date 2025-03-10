#ifndef _TINYFS_ERRNO_H
#define _TINYFS_ERRNO_H

#define TFS_SUCCESS 0  // Success
#define TFS_FAILURE -1 // Failure

#define TFS_ERR_MOUNTED_DISK -2      // Mount error
#define TFS_ERR_NO_DISK -3           // No such device
#define TFS_ERR_CORRUPTED_DISK -4    // Invalid argument
#define TFS_ERR_OPEN -5              // Open error
#define TFS_ERR_CLOSE -6             // Close error
#define TFS_ERR_READ -7              // Read error
#define TFS_ERR_WRITE -8             // Write error
#define TFS_ERR_DELETE -9            // Delete error
#define TFS_ERR_SEEK -10             // Seek error
#define TFS_ERR_INVALID_ARGUMENT -11 // Invalid argument
#define TFS_ERR_FILE_DESCRIPTOR -12  // Bad file descriptor
#define TFS_ERR_END_OF_FILE -13      // End of file
#define TFS_ERR_FILE_LIMIT -14       // Too many open files in system
#define TFS_ERR_NO_SPACE -15         // No space left on device

static int tfs_errno = TFS_SUCCESS;

#endif
