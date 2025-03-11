#include <tinyFS.h>
#include <tinyFS_errno.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SLEEP_TIME 1

void printFileInfo(fileDescriptor fd, char *filename, int testNum)
{
    if (tfs_readFileInfo(fd) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return;
    }
    printf("[TEST %d] : File info for %s\n", testNum, filename);
}

void sleepForSeconds(int seconds)
{
    printf("-----------------\n");
    for (int i = 0; i < seconds; i++)
    {
        printf("Sleeping...\n");
        sleep(1);
    }
    printf("-----------------\n");
}

void listDirectory(int testNum)
{
    printf("[TEST %d] : Listing directory contents\n", testNum);
    // printf("-----------------\n");
    // printf("TinyFS Directory:\n");
    tfs_readdir();
    // printf("-----------------\n");
}

int main()
{
    int test = 1;
    fileDescriptor fd1, fd2;
    char *content;
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

    if (tfs_mount("demo") != TFS_FAILURE)
    {
        fprintf(stderr, "Disk mount succeeded, but disk is already mounted\n");
        return 1;
    }
    printf("[TEST %d] : Disk mount failed as expected\n", test++);

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
    printFileInfo(fd2, "file2", test++);

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

    if (tfs_rename(fd2, "renamed_file") != TFS_FAILURE)
    {
        fprintf(stderr, "Renamed file3 to an invalid name, should have failed\n");
        return 1;
    }
    printf("[TEST %d] : Renaming file3 to an invalid name failed as expected\n", test++);

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

    // Print directory contents after remount
    listDirectory(test++); // Should contain file1 and file3

    sleepForSeconds(SLEEP_TIME);

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

    // ------------------------
    // PHASE 4: Writing & Reading
    // ------------------------

    content = "Hello, world!";
    if (tfs_writeFile(fd1, content, strlen(content)) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to write to file1\n");
        return 1;
    }
    printf("[TEST %d] : Wrote to file1\n", test++);
    printFileInfo(fd1, "file1", test++);

    content = "Goodbye, world!";
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

    printFileInfo(fd2, "file3", test++);

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
    printf("[TEST %d] : Delete file3 failed as expected\n", test++);

    printFileInfo(fd2, "file3", test++);
    listDirectory(test++); // Should contain file1 and file3

    // ------------------------
    // PHASE 6: Cleanup
    // ------------------------

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

    if (tfs_deleteFile(fd2) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to delete file3\n");
        return 1;
    }
    printf("[TEST %d] : Deleted file3\n", test++);

    if (tfs_unmount() == TFS_FAILURE)
    {
        fprintf(stderr, "Disk unmount error\n");
        return 1;
    }
    printf("[TEST %d] : Disk unmounted\n", test++);

    printf("\nAll tests passed!\n");
    return 0;
}
