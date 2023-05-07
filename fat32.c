/*********************************/
/* FAT32 Directory Prompt        */
/*                               */
/* Written by - Trenton Schwartz */
/*                               */
/*********************************/
/**********************************************************/
/* This project reads in an image file from terminal      */
/* and reads in commands from the terminal which allows   */
/* the user to interface directly with the disk image.    */
/**********************************************************/

#pragma pack(push,1)
#define _GNU_SOURCE

#include "helper.h"

//ABSTRACT
//Read in the Master Boot Record
//Parse the MBR
//Read user command
//Execute user command

int main(int argc, char* argv[], char* env[])
{
    //Invalid arguments
    if(argc <= 1)
    {
        printf("No directory provided. Please try again.\n");
        abort();
    }

    image = malloc(argc*sizeof(unsigned char));
    memmove(image, argv[1], strlen(argv[1])+1);

    //Open file
    filePtr = fopen(image, "r");

    //Read the sector into a char array, return the number of characters read
    unsigned char* mbr = malloc(512);

    fseek(filePtr, 0 * sizeof(unsigned char), SEEK_SET);
    uint numberOfIntegersRead = fread(mbr, sizeof(unsigned char), BPB_BytsPerSec, filePtr);

    //Load MBR with data
    PackMBR(&MBR, mbr);

    //Read in partition1
    //We've already read one sector - read to lbaBegin minus the read sector
    unsigned char* partitionData = malloc(512);
    fseek(filePtr, ((MBR.partition1.lbaBegin) * 512), SEEK_SET);

    numberOfIntegersRead = fread(partitionData, sizeof(unsigned char), 512, filePtr);


    //Load the BPB with relevant (or not) data
    PackBPB(&BPB, partitionData);

    u_int32_t numberOfSectors = BPB.BPB_TotSec32; 
    u_int16_t sizeOfCluster  = BPB.BPB_BytsPerSec * BPB.BPB_SecPerClus;
    u_int32_t totalNumberOfClusters = (u_int32_t)(numberOfSectors*BPB.BPB_BytsPerSec / (double)sizeOfCluster);


    unsigned char cluster[sizeOfCluster+1]; //Initialize our cluster

    cluster[sizeOfCluster] = '\n';

    //We're done with this fileptr...
    fclose(filePtr);

    //Read user input
    bool keepLooping = true;

    uint currentDirectory = BPB.BPB_RootClus;
    while(keepLooping)
    {
        char* command = malloc(40 * sizeof(char));
        printf("/>");
        //Input command
        scanf("%39s", command);

        int fileSize = 0;
        //If command is EXTRACT, look for a file input
        //This needs to factor in 8.3 AND 83 files (without the period)
        file.fileName = "\0";
        file.fileName = malloc(120*sizeof(char));

        //If command is EXTRACT
        if(strncasecmp(command, "EXTRACT", 8) == 0) 
        {
            //Read in the file name from the user after the user enters a space
            //This input formatter will take a string of characters until a newline is reached
            scanf(" %119[^\n]s", file.fileName);

            //Ensure this is null terminated
            strcat(file.fileName, "\0");
            file.fileSize = strlen(file.fileName)+1;
            file.isSFN = fileNameSFNValidator();
            
            Extract(currentDirectory);
        }
        //If command is DIR
        else if(strncasecmp(command, "DIR", 3) == 0)
        {
            //Read the directory
            Readdir(currentDirectory);
        }
        //If command is CD
        else if(strncasecmp(command, "CD", 2) == 0)
        {
            //Read in the file name from the user after the user enters a space
            //This input formatter will take a string of characters until a newline is reached
            scanf(" %119[^\n]s", file.fileName);

            //Ensure this is null terminated
            strcat(file.fileName, "\0");
            file.fileSize = strlen(file.fileName)+1;
            file.isSFN = fileNameSFNValidator();
            uint nextDirectory = ChangeDirectory(currentDirectory);
            if(nextDirectory != ((uint)-1)) currentDirectory = nextDirectory;
            if(currentDirectory == 0) currentDirectory = 2;
        }
        //Exit program
        else if(strncasecmp(command, "QUIT", 4) == 0)
        {
            keepLooping = false;
            printf("Shutting down...\n");
        }

        printf("\n");
        free(command);
    }
}