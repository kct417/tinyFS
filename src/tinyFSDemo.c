#include <tinyFS.h>
#include <tinyFS_errno.h>

#include <stdio.h>
#include <string.h>

int main()
{
    int test = 1;
    int *bitmap;

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

    // open file1
    char *filename = "file1";
    fileDescriptor fd1 = tfs_openFile(filename);
    if (fd1 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to open %s\n", filename);
        return 1;
    }
    printf("[TEST %d] : Opened %s\n", test++, filename);

    // open file2
    filename = "file2";
    fileDescriptor fd2 = tfs_openFile(filename);
    if (fd2 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to open %s\n", filename);
        return 1;
    }
    printf("[TEST %d] : Opened %s\n", test++, filename);







    // open file3
    filename = "file3";
    fileDescriptor fd3 = tfs_openFile(filename);
    if (fd3 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to open %s\n", filename);
        return 1;
    }
    printf("[TEST %d] : Opened %s\n", test++, filename);

    // open file4
    filename = "file4";
    fileDescriptor fd4 = tfs_openFile(filename);
    if (fd4 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to open %s\n", filename);
        return 1;
    }
    printf("[TEST %d] : Opened %s\n", test++, filename);

    // open file5
    filename = "file5";
    fileDescriptor fd5 = tfs_openFile(filename);
    if (fd5 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to open %s\n", filename);
        return 1;
    }
    printf("[TEST %d] : Opened %s\n", test++, filename);

    // open file6
    filename = "file6";
    fileDescriptor fd6 = tfs_openFile(filename);
    if (fd6 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to open %s\n", filename);
        return 1;
    }
    printf("[TEST %d] : Opened %s\n", test++, filename);

    // open file7
    filename = "file7";
    fileDescriptor fd7 = tfs_openFile(filename);
    if (fd7 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to open %s\n", filename);
        return 1;
    }
    printf("[TEST %d] : Opened %s\n", test++, filename);

    // // open 7 new files in a loop
    // for (int i = 1; i <= 7; i++)
    // {
        //     char filename[8];
        //     snprintf(filename, sizeof(filename), "file%d", i); // file 1 to 7
        //     fileDescriptor fd = tfs_openFile(filename);
        //     if (fd == TFS_FAILURE)
        //     {
            //         fprintf(stderr, "Failed to open %s\n", filename);
            //         return 1;
            //     }
            //     printf("[TEST %d] : Opened %s\n", test++, filename);
            // }
            
            
            
            
            
            
            
            
            // test displaying block map
            bitmap = malloc(sizeof(int) * NUM_BLOCKS);
            if (bitmap == NULL)
            {
                fprintf(stderr, "Failed to allocate memory for bitmap\n");
                return 1;
            }
            if (tfs_displayMap(bitmap) != TFS_SUCCESS)
            {
                fprintf(stderr, "Display map failed in this way: %d\n", tfs_errno);
                return 1;
            }
            printf("[TEST %d] : Displayed block map\n", test++);
            free(bitmap);
            
            
            
            
            
            
    // delete file4
    if (tfs_deleteFile(fd4) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to delete file4\n");
        return 1;
    }
    printf("[TEST %d] : Deleted file4\n", test++);

    // delete file6
    if (tfs_deleteFile(fd6) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to delete file6\n");
        return 1;
    }
    printf("[TEST %d] : Deleted file6\n", test++);



    // write to file1
    char *content = "Hello, world!\n";
    if (tfs_writeFile(fd1, content, strlen(content)) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to write to file1\n");
        return 1;
    }
    printf("[TEST %d] : Wrote to file1\n", test++);
    
    // write to file2
    content = "Goodbye, world!\n";
    if (tfs_writeFile(fd2, content, strlen(content)) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to write to file2\n");
        return 1;
    }
    printf("[TEST %d] : Wrote to file2\n", test++);
    
    // write to files 3, 5, and 7
    for (int i = 3; i <= 7; i += 2) {
        char filename[8];
        snprintf(filename, sizeof(filename), "file%d", i); // file 3, 5, 7
        char content[20];
        snprintf(content, sizeof(content), "Content of %s\n", filename);
        fileDescriptor fd = tfs_openFile(filename);
        if (fd == TFS_FAILURE) {
            fprintf(stderr, "Failed to open %s\n", filename);
            return 1;
        }
        if (tfs_writeFile(fd, content, strlen(content)) == TFS_FAILURE) {
            fprintf(stderr, "Failed to write to %s\n", filename);
            return 1;
        }
        printf("[TEST %d] : Wrote to %s\n", test++, filename);
        if (tfs_closeFile(fd) == TFS_FAILURE) {
            fprintf(stderr, "Failed to close %s\n", filename);
            return 1;
        }
        printf("[TEST %d] : Closed %s\n", test++, filename);
    }
    





    // test displaying block map
    bitmap = malloc(sizeof(int) * NUM_BLOCKS);
    if (bitmap == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for bitmap\n");
        return 1;
    }
    if (tfs_displayMap(bitmap) != TFS_SUCCESS)
    {
        fprintf(stderr, "Display map failed in this way: %d\n", tfs_errno);
        return 1;
    }
    printf("[TEST %d] : Displayed block map\n", test++);
    free(bitmap);











    // read from file1
    char buffer;
    while (tfs_readByte(fd1, &buffer) != TFS_FAILURE)
    {
        printf("%c", buffer);
    }
    printf("[TEST %d] : Read from file1\n", test++);
    
    // read from file2
    while (tfs_readByte(fd2, &buffer) != TFS_FAILURE)
    {
        printf("%c", buffer);
    }
    printf("[TEST %d] : Read from file2\n", test++);
    
    // attempt to read from file1 again
    if (tfs_readByte(fd1, &buffer) != TFS_FAILURE)
    {
        fprintf(stderr, "File descriptor is at end of file, but read succeeded\n");
        return 1;
    }
    printf("[TEST %d] : Read from file1 failed as expected\n", test++);

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

    // open file1 again
    fd1 = tfs_openFile("file1");
    if (fd1 == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to open file1\n");
        return 1;
    }
    printf("[TEST %d] : Opened file1 again\n", test++);

    // delete file1
    if (tfs_deleteFile(fd1) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to delete file1\n");
        return 1;
    }
    printf("[TEST %d] : Deleted file1\n", test++);

    // set file2 to read only
    filename = "file2";
    if (tfs_makeRO(filename) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to make %s read only\n", filename);
        return 1;
    }
    printf("[TEST %d] : Made %s read only\n", test++, filename);

    // attempt to write to file2
    content = "This should fail\n";
    if (tfs_writeFile(fd2, content, strlen(content)) != TFS_FAILURE)
    {
        fprintf(stderr, "File2 is read only, but write succeeded\n");
        return 1;
    }
    printf("[TEST %d] : Write to file2 failed as expected\n", test++);

    // attempt to delete file2
    if (tfs_deleteFile(fd2) != TFS_FAILURE)
    {
        fprintf(stderr, "File2 is read only, but delete succeeded\n");
        return 1;
    }
    printf("[TEST %d] : Delete file2 failed as expected\n", test++);

    // set file2 to read write
    if (tfs_makeRW(filename) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to make %s read write\n", filename);
        return 1;
    }
    printf("[TEST %d] : Made %s read write\n", test++, filename);

    // attempt to write to file2
    content = "This should succeed\n";
    if (tfs_writeFile(fd2, content, strlen(content)) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to write to %s after making it read write\n", filename);
        return 1;
    }
    printf("[TEST %d] : Wrote to %s\n", test++, filename);

    // read from file2
    while (tfs_readByte(fd2, &buffer) != TFS_FAILURE)
    {
        printf("%c", buffer);
    }
    printf("[TEST %d] : Read from file2\n", test++);

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

    // delete file2
    if (tfs_deleteFile(fd2) == TFS_FAILURE)
    {
        fprintf(stderr, "Failed to delete %s\n", filename);
        return 1;
    }
    printf("[TEST %d] : Deleted %s\n", test++, filename);





    // test tfs_defrag
    if (tfs_defrag() == TFS_FAILURE)
    {
        fprintf(stderr, "Defrag failed\n");
        return 1;
    }
    printf("[TEST %d] : Defrag succeeded\n", test++);

    // test displaying block map
    bitmap = malloc(sizeof(int) * NUM_BLOCKS);
    if (bitmap == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for bitmap\n");
        return 1;
    }
    if (tfs_displayMap(bitmap) != TFS_SUCCESS)
    {
        fprintf(stderr, "Display map failed in this way: %d\n", tfs_errno);
        return 1;
    }
    printf("[TEST %d] : Displayed block map\n", test++);
    free(bitmap);





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
