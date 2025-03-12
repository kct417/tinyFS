#include <tinyFS.h>
#include <tinyFS_errno.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SLEEP_TIME 1

void sleepForSeconds(int seconds)
{
    printf("Sleeping for %d second(s)...\n", seconds);
    for (int i = 0; i < seconds; i++)
    {
        printf("%d...\n", seconds - i);
        sleep(1);
    }
}

int showBlockMap()
{
    // test displaying block map
    if (tfs_displayFragments() != TFS_SUCCESS)
    {
        fprintf(stderr, "Display map failed in this way: %d\n", tfs_errno);
        return 1;
    }
    return 0;
}

void listDirectory(int testNum)
{
    printf("[TEST %d] : Listing directory contents\n", testNum);
    tfs_readdir();
}

void printFileInfo(fileDescriptor fd, char *filename, int testNum)
{
    if (tfs_readFileInfo(fd) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return;
    }
    printf("[TEST %d] : File Info for %s\n", testNum, filename);
}

int main()
{
    int test = 1;
    fileDescriptor fd1, fd2;
    char content[1000];
    char buffer;

    // ------------------------
    // PHASE 1: Filesystem Initialization
    // ------------------------

    if (tfs_mkfs(DEFAULT_DISK_NAME, DEFAULT_DISK_SIZE) == TFS_FAILURE)
    {
        fprintf(stderr, "Disk creation error\n");
        return 1;
    }
    printf("[TEST %d] : Disk created with default name and size\n", test++);

    if (tfs_mount(DEFAULT_DISK_NAME) == TFS_FAILURE)
    {
        fprintf(stderr, "Disk mount error\n");
        return 1;
    }
    printf("[TEST %d] : Disk mounted\n", test++);

    listDirectory(test++); // Should be empty

    // ------------------------
    // PHASE 2: File Operations (Creation, Metadata)
    // ------------------------

    fd1 = tfs_openFile("file1");
    if (fd1 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to open file1\n");
        return 1;
    }
    printf("[TEST %d] : Opened file1\n", test++);
    printFileInfo(fd1, "file1", test++);

    sleepForSeconds(SLEEP_TIME);

    fd2 = tfs_openFile("file2");
    if (fd2 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to open file2\n");
        return 1;
    }
    printf("[TEST %d] : Opened file2\n", test++);

    listDirectory(test++); // Should contain file1 and file2

    // ------------------------
    // PHASE 3: Rename, Unmount, Remount
    // ------------------------

    if (tfs_rename(fd2, "file3") == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to rename file2 to file3\n");
        return 1;
    }
    printf("[TEST %d] : Renamed file2 to file3\n", test++);

    listDirectory(test++); // Should contain file1 and file3

    if (tfs_unmount() == TFS_FAILURE)
    {
        fprintf(stderr, "Disk unmount error\n");
        return 1;
    }
    printf("[TEST %d] : Disk unmounted\n", test++);

    if (tfs_mount(DEFAULT_DISK_NAME) == TFS_FAILURE)
    {
        fprintf(stderr, "Disk mount error\n");
        return 1;
    }
    printf("[TEST %d] : Disk mounted\n", test++);

    // Re-open file1 after remount
    fd1 = tfs_openFile("file1");
    if (fd1 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to reopen file1 after remount\n");
        return 1;
    }

    // Re-open file3 after remount
    fd2 = tfs_openFile("file3");
    if (fd2 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to reopen file3 after remount\n");
        return 1;
    }

    printFileInfo(fd2, "file3", test++);

    // ------------------------
    // PHASE 4: Writing & Reading
    // ------------------------

    for (int i = 0; i < 500; i++)
    {
        content[i] = 'a';
    }
    content[498] = '\n';
    content[499] = '\0';
    if (tfs_writeFile(fd1, content, strlen(content)) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to write to file1\n");
        return 1;
    }
    printf("[TEST %d] : Wrote to file1\n", test++);
    printFileInfo(fd1, "file1", test++);

    for (int i = 0; i < 1000; i++)
    {
        content[i] = 'b';
    }
    content[998] = '\n';
    content[999] = '\0';
    if (tfs_writeFile(fd2, content, strlen(content)) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to write to file3\n");
        return 1;
    }
    printf("[TEST %d] : Wrote to file3\n", test++);
    printFileInfo(fd2, "file3", test++);

    sleepForSeconds(SLEEP_TIME);

    printf("[TEST %d] : Reading file1: ", test++);
    while (tfs_readByte(fd1, &buffer) != TFS_FAILURE)
    {
        printf("%c", buffer);
    }
    printf("\n");

    printf("[TEST %d] : Reading file3: ", test++);
    while (tfs_readByte(fd2, &buffer) != TFS_FAILURE)
    {
        printf("%c", buffer);
    }
    printf("\n");

    // ------------------------
    // PHASE 5: File Protection (Read-Only Mode)
    // ------------------------

    if (tfs_makeRO("file3") == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to make file3 read-only\n");
        return 1;
    }
    printf("[TEST %d] : Made file3 read-only\n", test++);

    if (tfs_writeFile(fd2, "New data", 8) != TFS_FAILURE)
    {
        fprintf(stderr, "Write should have failed for read-only file3\n");
        return 1;
    }
    printf("[TEST %d] : Write to file3 failed as expected\n", test++);

    if (tfs_deleteFile(fd2) != TFS_FAILURE)
    {
        fprintf(stderr, "Delete should have failed for read-only file3\n");
        return 1;
    }
    printf("[TEST %d] : Delete of file3 failed as expected\n", test++);

    if (tfs_makeRW("file3") == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to make file3 read-write\n");
        return 1;
    }
    printf("[TEST %d] : Made file3 read-write\n", test++);

    if (tfs_deleteFile(fd1) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to delete file1\n");
        return 1;
    }
    printf("[TEST %d] : Deleted file1\n", test++);

    // ------------------------
    // PHASE 6: Defragmentation
    // ------------------------

    // write 500 characters to three large files test
    char *largeContent = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                         "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                         "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
                         "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
                         "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

    char *filenames[] = {"lf1", "lf2", "lf3", "lf4", "lf5", "lf6", "lf7"};
    for (int i = 0; i < 7; i++)
    {
        fileDescriptor fd = tfs_openFile(filenames[i]);
        if (fd == TFS_FAILURE)
        {
            fprintf(stderr, "Failed to open %s\n", filenames[i]);
            return 1;
        }
        printf("[TEST %d] : Opened %s\n", test++, filenames[i]);

        if (tfs_writeFile(fd, largeContent, 500) == TFS_FAILURE)
        {
            fprintf(stderr, "Failed to write 500 characters to %s\n", filenames[i]);
            return 1;
        }
        printf("[TEST %d] : Wrote 500 characters to %s\n", test++, filenames[i]);
    }

    // delete specific large files
    int filesToDelete[] = {1, 3, 5}; // indices of lf2, lf4, lf6
    for (int i = 0; i < 3; i++)
    {
        fileDescriptor fd = tfs_openFile(filenames[filesToDelete[i]]);
        if (fd == TFS_FAILURE)
        {
            fprintf(stderr, "Failed to open %s\n", filenames[filesToDelete[i]]);
            return 1;
        }
        if (tfs_deleteFile(fd) == TFS_FAILURE)
        {
            fprintf(stderr, "Failed to delete %s\n", filenames[filesToDelete[i]]);
            return 1;
        }
        printf("[TEST %d] : Deleted %s\n", test++, filenames[filesToDelete[i]]);
    }

    // display block map after deletion
    if (showBlockMap() != 0)
    {
        return 1;
    }
    printf("[TEST %d] : Block map displayed after deletion\n", test++);

    // defrag test
    if (tfs_defrag() == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to defrag\n");
        return 1;
    }
    printf("[TEST %d] : Defrag succeeded\n", test++);

    // display block map after deletion
    if (showBlockMap() != 0)
    {
        return 1;
    }
    printf("[TEST %d] : Block map displayed after deletion\n", test++);

    // ------------------------
    // PHASE 7: Cleanup
    // ------------------------

    if (tfs_unmount() == TFS_FAILURE)
    {
        fprintf(stderr, "Disk unmount error\n");
        return 1;
    }
    printf("[TEST %d] : Disk unmounted\n", test++);

    printf("\nAll tests passed!\n");
    return 0;
}
