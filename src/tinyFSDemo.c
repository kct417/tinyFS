#include <tinyFS.h>
#include <tinyFS_errno.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SLEEP_TIME 1

int main()
{
    int test = 1;
    char *filename;
    char *content;
    char buffer;
    file_info info;

    // create disk with default size
    if (tfs_mkfs(DEFAULT_DISK_NAME, DEFAULT_DISK_SIZE) == TFS_FAILURE)
    {
        fprintf(stderr, "Disk creation error\n");
        return 1;
    }
    printf("[TEST %d] : Disk created with default name and size\n", test++);

    // mount disk
    if (tfs_mount(DEFAULT_DISK_NAME) == TFS_FAILURE)
    {
        fprintf(stderr, "Disk mount error\n");
        return 1;
    }
    printf("[TEST %d] : Disk mounted\n", test++);

    // attempt to mount disk again
    if (tfs_mount("demo") != TFS_FAILURE)
    {
        fprintf(stderr, "Disk mount succeeded, but disk is already mounted\n");
        return 1;
    }
    printf("[TEST %d] : Disk mount failed as expected\n", test++);

    // list directory (should be empty)
    printf("[TEST %d] : Listing directory contents (should be empty)\n", test++);
    tfs_readdir();

    // open file1
    filename = "file1";
    fileDescriptor fd1 = tfs_openFile(filename);
    if (fd1 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to open %s\n", filename);
        return 1;
    }
    printf("[TEST %d] : Opened %s\n", test++, filename);

    // get file1 times
    if (tfs_readFileInfo(fd1, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times of %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times of file1\n", test++);

    printf("Sleeping for %d second(s)...\n", SLEEP_TIME);
    for (int i = 0; i < SLEEP_TIME; i++)
    {
        printf("%d...\n", SLEEP_TIME - i);
        sleep(1);
    }

    // open file2
    filename = "file2";
    fileDescriptor fd2 = tfs_openFile(filename);
    if (fd2 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to open %s\n", filename);
        return 1;
    }
    printf("[TEST %d] : Opened %s\n", test++, filename);

    // list directory (should list files 1 and 2)
    printf("[TEST %d] : Listing directory contents (should contain file1 and file2)\n", test++);
    tfs_readdir();

    // rename file2 -> file3
    filename = "file3";
    if (tfs_rename(fd2, filename) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to rename file2 to %s\n", filename);
        return 1;
    }
    printf("[TEST %d] : Renamed file2 to %s\n", test++, filename);

    // attempt to renamed to exceeded length filename
    filename = "renamed_file";
    if (tfs_rename(fd2, filename) != TFS_FAILURE)
    {
        fprintf(stderr, "File renamed, but should've failed\n");
        return 1;
    }
    printf("[TEST %d] : Failed to rename file3 to %s, as expected\n", test++, filename);

    // list directory contents (should contain file1 and file3)
    printf("[TEST %d] : Listing directory contents (should contain file1 and file3)\n", test++);
    tfs_readdir();

    // get file3 times
    if (tfs_readFileInfo(fd2, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times for file3\n", test++);

    // write to file1
    content = "Hello, world!\n";
    if (tfs_writeFile(fd1, content, strlen(content)) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to write to file1\n");
        return 1;
    }
    printf("[TEST %d] : Wrote to file1\n", test++);

    // get file1 times
    if (tfs_readFileInfo(fd1, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times of file1\n", test++);

    printf("Sleeping for %d second(s)...\n", SLEEP_TIME);
    for (int i = 0; i < SLEEP_TIME; i++)
    {
        printf("%d...\n", SLEEP_TIME - i);
        sleep(1);
    }

    // write to file3
    content = "Goodbye, world!\n";
    if (tfs_writeFile(fd2, content, strlen(content)) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to write to file3\n");
        return 1;
    }
    printf("[TEST %d] : Wrote to file3\n", test++);

    // get file3 times
    if (tfs_readFileInfo(fd2, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times for file3\n", test++);

    printf("Sleeping for %d second(s)...\n", SLEEP_TIME);
    for (int i = 0; i < SLEEP_TIME; i++)
    {
        printf("%d...\n", SLEEP_TIME - i);
        sleep(1);
    }

    // read from file1
    while (tfs_readByte(fd1, &buffer) != TFS_FAILURE)
    {
        printf("%c", buffer);
    }
    printf("[TEST %d] : Read from file1\n", test++);

    // get file1 times
    if (tfs_readFileInfo(fd1, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times of file1\n", test++);

    // read from file3
    while (tfs_readByte(fd2, &buffer) != TFS_FAILURE)
    {
        printf("%c", buffer);
    }
    printf("[TEST %d] : Read from file3\n", test++);

    // get file3 times
    if (tfs_readFileInfo(fd2, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times for file3\n", test++);

    printf("Sleeping for %d second(s)...\n", SLEEP_TIME);
    for (int i = 0; i < SLEEP_TIME; i++)
    {
        printf("%d...\n", SLEEP_TIME - i);
        sleep(1);
    }

    // attempt to read from file1 again
    if (tfs_readByte(fd1, &buffer) != TFS_FAILURE)
    {
        fprintf(stderr, "File descriptor is at end of file, but read succeeded\n");
        return 1;
    }
    printf("[TEST %d] : Read from file1 failed as expected\n", test++);

    // get file1 times
    if (tfs_readFileInfo(fd1, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times of file1\n", test++);

    // read from file1 again
    if (tfs_seek(fd1, 0) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to seek to beginning of file1\n");
        return 1;
    }
    while (tfs_readByte(fd1, &buffer) != TFS_FAILURE)
    {
        printf("%c", buffer);
    }
    printf("[TEST %d] : Read from file1 again\n", test++);

    // get file1 times
    if (tfs_readFileInfo(fd1, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times of file1\n", test++);

    // close file1
    if (tfs_closeFile(fd1) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to close file1\n");
        return 1;
    }
    printf("[TEST %d] : Closed file1\n", test++);

    // attempt to delete file1
    if (tfs_deleteFile(fd1) != TFS_FAILURE)
    {
        fprintf(stderr, "File1 is closed, but delete succeeded\n");
        return 1;
    }
    printf("[TEST %d] : Delete file1 failed as expected\n", test++);

    printf("Sleeping for %d second(s)...\n", SLEEP_TIME);
    for (int i = 0; i < SLEEP_TIME; i++)
    {
        printf("%d...\n", SLEEP_TIME - i);
        sleep(1);
    }

    // open file1 again
    fd1 = tfs_openFile("file1");
    if (fd1 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to open file1\n");
        return 1;
    }
    printf("[TEST %d] : Opened file1 again\n", test++);

    // get file1 times
    if (tfs_readFileInfo(fd1, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times of file1\n", test++);

    // delete file1
    if (tfs_deleteFile(fd1) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to delete file1\n");
        return 1;
    }
    printf("[TEST %d] : Deleted file1\n", test++);

    // rename file3 -> file2
    filename = "file2";
    if (tfs_rename(fd2, filename) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to rename file3 to %s\n", filename);
        return 1;
    }
    printf("[TEST %d] : Renamed file3 to %s\n", test++, filename);

    // list directory contents (should contain only file 2)
    printf("[TEST %d] : Listing directory contents (should only contain file2)\n", test++);
    tfs_readdir();

    // set file2 to read only
    filename = "file2";
    if (tfs_makeRO(filename) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to make %s read only\n", filename);
        return 1;
    }
    printf("[TEST %d] : Made %s read only\n", test++, filename);

    printf("Sleeping for %d second(s)...\n", SLEEP_TIME);
    for (int i = 0; i < SLEEP_TIME; i++)
    {
        printf("%d...\n", SLEEP_TIME - i);
        sleep(1);
    }

    // attempt to write to file2
    content = "This should fail\n";
    if (tfs_writeFile(fd2, content, strlen(content)) != TFS_FAILURE)
    {
        fprintf(stderr, "File2 is read only, but write succeeded\n");
        return 1;
    }
    printf("[TEST %d] : Write to file2 failed as expected\n", test++);

    // get file2 times
    if (tfs_readFileInfo(fd2, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times for file2\n", test++);

    // attempt to delete file2
    if (tfs_deleteFile(fd2) != TFS_FAILURE)
    {
        fprintf(stderr, "File2 is read only, but delete succeeded\n");
        return 1;
    }
    printf("[TEST %d] : Delete file2 failed as expected\n", test++);

    // get file2 times
    if (tfs_readFileInfo(fd2, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times for file2\n", test++);

    // set file2 to read write
    if (tfs_makeRW(filename) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to make %s read write\n", filename);
        return 1;
    }
    printf("[TEST %d] : Made %s read write\n", test++, filename);

    printf("Sleeping for %d second(s)...\n", SLEEP_TIME);
    for (int i = 0; i < SLEEP_TIME; i++)
    {
        printf("%d...\n", SLEEP_TIME - i);
        sleep(1);
    }

    // write to file2
    content = "This should succeed\n";
    if (tfs_writeFile(fd2, content, strlen(content)) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to write to %s after making it read write\n", filename);
        return 1;
    }
    printf("[TEST %d] : Wrote to %s\n", test++, filename);

    // get file2 times
    if (tfs_readFileInfo(fd2, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times for file2\n", test++);

    printf("Sleeping for %d second(s)...\n", SLEEP_TIME);
    for (int i = 0; i < SLEEP_TIME; i++)
    {
        printf("%d...\n", SLEEP_TIME - i);
        sleep(1);
    }

    // read from file2
    while (tfs_readByte(fd2, &buffer) != TFS_FAILURE)
    {
        printf("%c", buffer);
    }
    printf("[TEST %d] : Read from file2\n", test++);

    // get file2 times
    if (tfs_readFileInfo(fd2, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times for file2\n", test++);

    printf("Sleeping for %d second(s)...\n", SLEEP_TIME);
    for (int i = 0; i < SLEEP_TIME; i++)
    {
        printf("%d...\n", SLEEP_TIME - i);
        sleep(1);
    }

    // write byte to file2
    int offset = 4;
    if (tfs_seek(fd2, offset) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to seek to offset %d of file2\n", offset);
        return 1;
    }
    if (tfs_writeByte(fd2, 'X') == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to write byte to file2\n");
        return 1;
    }
    printf("[TEST %d] : Wrote byte to file2\n", test++);

    // get file2 times
    if (tfs_readFileInfo(fd2, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times for file2\n", test++);

    printf("Sleeping for %d second(s)...\n", SLEEP_TIME);
    for (int i = 0; i < SLEEP_TIME; i++)
    {
        printf("%d...\n", SLEEP_TIME - i);
        sleep(1);
    }

    // read from file2 again
    if (tfs_seek(fd2, 0) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to seek to beginning of file2\n");
        return 1;
    }
    while (tfs_readByte(fd2, &buffer) != TFS_FAILURE)
    {
        printf("%c", buffer);
    }
    printf("[TEST %d] : Read from file2 again\n", test++);

    // get file2 times
    if (tfs_readFileInfo(fd2, &info) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to get times for %s\n", filename);
        return 1;
    }
    printf("Time Created: %s\n", info.created);
    printf("Time Modified: %s\n", info.modified);
    printf("Time Accessed: %s\n", info.accessed);
    printf("[TEST %d] : Got times for file2\n", test++);

    // delete file2
    if (tfs_deleteFile(fd2) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to delete %s\n", filename);
        return 1;
    }
    printf("[TEST %d] : Deleted %s\n", test++, filename);

    // unmount disk
    if (tfs_unmount() == TFS_FAILURE)
    {
        fprintf(stderr, "Disk unmount error\n");
        return 1;
    }
    printf("[TEST %d] : Disk unmounted\n", test++);

    // attempt to unmount disk again
    if (tfs_unmount() != TFS_FAILURE)
    {
        fprintf(stderr, "Disk unmount succeeded, but disk is not mounted\n");
        return 1;
    }
    printf("[TEST %d] : Disk unmount failed as expected\n", test++);

    printf("\nAll tests passed\n\nEnd of Demo\n\n");

    return 0;
}
