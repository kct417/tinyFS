/* TinyFS demo file
 *  * Foaad Khosmood, Cal Poly / modified Winter 2014
 *   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libTinyFS.h"

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

    printf("\n[TEST] Creating directories...\n");
    /* Create directories */
    if (tfs_createDir("/home") == 0)
        printf("[SUCCESS] Created '/home'\n");
    else
        printf("[ERROR] Failed to create '/home'\n");

    if (tfs_createDir("/home/user") == 0)
        printf("[SUCCESS] Created '/home/user'\n");
    else
        printf("[ERROR] Failed to create '/home/user'\n");

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
    //printf("(a) File content: %s\n(b) File content: %s\nReady to store in TinyFS\n",
    //       afileContent, bfileContent);

    /* read or write files to TinyFS */

    /* Open and process '/home/user/afile' */
    printf("\n[TEST] Opening '/home/user/afile'...\n");
    aFD = tfs_openFile("/home/user/afile");

    if (aFD < 0)
        perror("[ERROR] tfs_openFile failed on afile");
    else
    {
        if (tfs_readByte(aFD, &readBuffer) < 0)
        {
            /* If readByte fails, file is empty - write content */
            printf("[INFO] '/home/user/afile' is empty. Writing new content...\n");
            if (tfs_writeFile(aFD, afileContent, afileSize) == 0)
                printf("[SUCCESS] Written to '/home/user/afile'\n");
            else
                printf("[ERROR] Failed to write '/home/user/afile'\n");
        }
        else
        {
            /* If file exists, print contents */
            printf("\n[TEST] Reading from '/home/user/afile':\n");
            printf("%c", readBuffer); // First byte already read
            while (tfs_readByte(aFD, &readBuffer) >= 0)
                printf("%c", readBuffer);
            printf("\n");
        }
    }

    /* Open and process '/home/user/bfile' */
    printf("\n[TEST] Opening '/home/user/bfile'...\n");
    bFD = tfs_openFile("/home/user/bfile");

    if (bFD < 0)
        perror("[ERROR] tfs_openFile failed on bfile");
    else
    {
        if (tfs_readByte(bFD, &readBuffer) < 0)
        {
            /* If readByte fails, file is empty - write content */
            printf("[INFO] '/home/user/bfile' is empty. Writing new content...\n");
            if (tfs_writeFile(bFD, bfileContent, bfileSize) == 0)
                printf("[SUCCESS] Written to '/home/user/bfile'\n");
            else
                printf("[ERROR] Failed to write '/home/user/bfile'\n");
        }
        else
        {
            /* If file exists, print contents */
            printf("\n[TEST] Reading from '/home/user/bfile':\n");
            printf("%c", readBuffer); // First byte already read
            while (tfs_readByte(bFD, &readBuffer) >= 0)
                printf("%c", readBuffer);
            printf("\n");
        }
    }
    debug_inode_table();
    
    /* Free both content buffers */
    free(bfileContent);
    free(afileContent);
    if (tfs_unmount() < 0)
        perror("tfs_unmount failed");

    printf("\nend of demo\n\n");
    return 0;
}