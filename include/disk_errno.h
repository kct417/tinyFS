#ifndef DSK_ERRNO_H
#define DSK_ERRNO_H

#define DSK_SUCCESS 0  // Success
#define DSK_FAILURE -1 // Failure

#define DSK_ERR_INVALID_ARGUMENT -2 // Invalid argument
#define DSK_ERR_FILE_DESCRIPTOR -3  // Bad file descriptor
#define DSK_ERR_OPEN -4             // Open error
#define DSK_ERR_CLOSE -5            // Close error
#define DSK_ERR_READ -6             // Read error
#define DSK_ERR_WRITE -7            // Write error
#define DSK_ERR_SEEK -8             // Seek error
#define DSK_ERR_END_OF_FILE -9      // End of file

static int dsk_errno = DSK_SUCCESS;

#endif
