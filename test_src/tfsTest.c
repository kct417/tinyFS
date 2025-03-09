/* TinyFS demo file
 *  * Foaad Khosmood, Cal Poly / modified Winter 2014
 *   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinyFS.h"
#include "libTinyFS.h"
#include "TinyFS_errno.h"

/* simple helper function to fill Buffer with as many inPhrase strings as possible before reaching size */
int fillBufferWithPhrase(char *inPhrase, char *Buffer, int size)
{
    int index = 0, i;
    if (!inPhrase || !Buffer || size <= 0 || size < strlen(inPhrase))
        return -1;

    while (index < size)
    {
        for (i = 0; inPhrase[i] != '\0' && (i + index < size); i++)
            Buffer[i + index] = inPhrase[i];
        index += i;
    }
    Buffer[size - 1] = '\0'; /* explicit null termination */
    return 0;
}

/* This program will create 2 files (of sizes 200 and 1000) to be read from or stored in the TinyFS file system. */
int main()
{
    char readBuffer;
    char *afileContent, *bfileContent; /* buffers to store file content */
    int afileSize = 200;               /* sizes in bytes */
    int bfileSize = 1000;

    char phrase1[] = "hello world from (a) file ";
    char phrase2[] = "(b) file content ";

    fileDescriptor aFD, bFD;
    int i, returnValue;

    /* try to mount the disk */
    if (tfs_mount(DEFAULT_DISK_NAME) < 0) /* if mount fails */
    {
        tfs_mkfs(DEFAULT_DISK_NAME, DEFAULT_DISK_SIZE); /* then make a new disk */
        if (tfs_mount(DEFAULT_DISK_NAME) < 0)           /* if we still can't open it... */
        {
            perror("failed to open disk"); /* then just exit */
            return;
        }
    }

    afileContent = (char *)malloc(afileSize * sizeof(char));
    if (fillBufferWithPhrase(phrase1, afileContent, afileSize) < 0)
    {
        perror("failed");
        return;
    }

    bfileContent = (char *)malloc(bfileSize * sizeof(char));
    if (fillBufferWithPhrase(phrase2, bfileContent, bfileSize) < 0)
    {
        perror("failed");
        return;
    }

    /* print content of files for debugging */
    printf("(a) File content: %s\n(b) File content: %s\nReady to store in TinyFS\n",
           afileContent, bfileContent);

    /* read or write files to TinyFS */

    aFD = tfs_openFile("afile");

    if (aFD < 0)
    {
        perror("tfs_openFile failed on afile");
    }

    /* now, was there already a file named "afile" that had some content? If we can read from it, yes!
     *  * If we can't read from it, it presumably means the file was empty.
     *   * If the size is 0 (all new files are sized 0) then any "readByte()" should fail, so
     *    * it's a new file and empty */
    if (tfs_readByte(aFD, &readBuffer) < 0)
    {
        /* if readByte() fails, there was no afile, so we write to it */
        if (tfs_writeFile(aFD, afileContent, afileSize) < 0)
        {
            perror("tfs_writeFile failed");
        }
        else
            printf("Successfully written to afile\n");
    }
    else
    {
        /* if yes, then just read and print the rest of afile that was already there */
        printf("\n*** reading afile from TinyFS: \n%c", readBuffer); /* print the first byte already read */
        /* now print the rest of it, byte by byte */
        while (tfs_readByte(aFD, &readBuffer) >= 0) /* go until readByte fails */
            printf("%c", readBuffer);

        /* close file */
        if (tfs_closeFile(aFD) < 0)
            perror("tfs_closeFile failed");

        /* now try to delete the file. It should fail because aFD is no longer valid */
        if (tfs_deleteFile(aFD) < 0)
        {
            aFD = tfs_openFile("afile"); /* so we open it again */
            if (tfs_deleteFile(aFD) < 0)
                perror("tfs_deleteFile failed");
        }
        else
            perror("tfs_deleteFile should have failed");
    }

    /* now bfile tests */
    bFD = tfs_openFile("bfile");

    if (bFD < 0)
    {
        perror("tfs_openFile failed on bfile");
    }

    if (tfs_readByte(bFD, &readBuffer) < 0)
    {
        if (tfs_writeFile(bFD, bfileContent, bfileSize) < 0)
        {
            perror("tfs_writeFile failed");
        }
        else
            printf("Successfully written to bfile\n");
    }
    else
    {
        printf("\n*** reading bfile from TinyFS: \n%c", readBuffer);
        while (tfs_readByte(bFD, &readBuffer) >= 0)
            printf("%c", readBuffer);

        tfs_deleteFile(bFD);
    }

    /* Free both content buffers */
    free(bfileContent);
    free(afileContent);
    if (tfs_unmount() < 0)
        perror("tfs_unmount failed");

    printf("\nend of demo\n\n");
    return 0;
}
