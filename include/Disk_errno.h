#ifndef DSK_ERRNO_H
#define DSK_ERRNO_H

#define DSK_SUCCESS 0  // Success
#define DSK_FAILURE -1 // Failure

#define DSK_EBADF -2  // Bad file descriptor
#define DSK_ECLOSE -3 // Close error
#define DSK_EINVAL -4 // Invalid argument

#define DSK_EOF -5    // End of file
#define DSK_EOPEN -6  // Open error
#define DSK_EREAD -7  // Read error
#define DSK_ESEEK -8  // Seek error
#define DSK_EWRITE -9 // Write error

static int dsk_errno = DSK_SUCCESS;

#endif
