#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libDisk.h"

#define NUM_TEST_DISKS 4 /* number of disks to test with */
#define BLOCKSIZE 256


#define NUM_BLOCKS 50 /* total number of blocks on each disk */
#define NUM_TEST_BLOCKS 10
#define TEST_BLOCKS {25,39,8,9,15,21,25,33,35,42}

int main() 
{
    int index=0; 
    int index2=0;
    int index3=0;
    int retValue=0;
    int disks[NUM_TEST_DISKS]; /* holds disk numbers being tested here */
    char diskName[] = "diskX.dsk"; /* Unix file name for the disks */
    char *buffer; /* holds one block of information */
    int testBlocks[NUM_TEST_BLOCKS] = TEST_BLOCKS; /* to contain a number of blocks to test */

    for (index =0; index < NUM_TEST_DISKS; index++) 
    {
        /* create a new buffer and fill it with $ */
        buffer = malloc(BLOCKSIZE * sizeof(char));

        diskName[4] = '0'+index;
        disks[index] = openDisk(diskName,0);
        
        if (disks[index] < 0)
        { 
            printf("] Open failed with (%i). Disk probably does not exist.\n",disks[index]);
            
	    disks[index] = openDisk(diskName,BLOCKSIZE * NUM_BLOCKS); /* create the disk */
	    if (disks[index] < 0)
            {
                printf("] openDisk() failed to create a disk. This should never happen. Exiting. \n");
		exit(0); 
	    }
          
            memset(buffer,'$',BLOCKSIZE);
            for (index2 = 0; index2 < NUM_TEST_BLOCKS; index2++)
            {
                retValue = writeBlock(disks[index],testBlocks[index2],buffer);
                if (retValue < 0)
		{
		    printf("] Failed to write to block %i of disk %s. Exiting (%i).\n",testBlocks[index2],diskName,retValue);
		    exit(0);
		}
                printf("] Successfully wrote to block %i of disk %s.\n",testBlocks[index2],diskName);
            }
        }
        else
        {
	    printf("] Existing disk %s opened.\n",diskName);
	    /* determine if the testBlocks contain the dollar Signal. 
 * Check every single byte */
            for (index2 = 0; index2 < NUM_TEST_BLOCKS; index2++)
            {
		if (readBlock(disks[index],testBlocks[index2],buffer) < 0)
                {
                    printf("] Failed to read block %i of disk %s. Exiting.\n",testBlocks[index2],diskName);
                    exit(0);
                }

		for (index3 =0; index3 < BLOCKSIZE; index3++)
                {
                    if (buffer[index3] != '$')
                    {
                        printf("] Failed. Byte #%i of block %i of disk %s was supposed to be a \"$\". Exiting\n.",
                               index3,testBlocks[index2],diskName);
                        exit(0);
                    }
                }
            }
            printf("] Previous writes were varified. Now, delete the .dsk files if you want to run this test again.\n");
       } 
    }
    return 0;
}
