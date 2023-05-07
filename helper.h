
/******************/
/*Helper.h        */
/*Written by      */
/*Trenton Schwartz*/
/******************/

/*
This header file holds all of the helper functions in thet
codebase. The intention is that very little functionality lies in main, and
most of what is there is calling functions that exist here.
There's a lot of functions that deal with sectors, clusters and offsets.
*/

#ifndef HELPER_H
#define HELPER_H

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <sys/wait.h>
#include <math.h>
#include <errno.h>

int BPB_BytsPerSec = 512;

u_int8_t ATTR_READ_ONLY = 0x01;
u_int8_t ATTR_HIDDEN = 0x02;
u_int8_t ATTR_SYSTEM = 0x04;
u_int8_t ATTR_VOLUME_ID = 0x08;
u_int8_t ATTR_DIRECTORY = 0x10;
u_int8_t ATTR_ARCHIVE = 0X20;
u_int8_t ATTR_LONG_NAME = 0x0F;

u_int8_t LAST_LONG_ENTRY = 0x40;
FILE* filePtr;
char* image;

struct File
{
    unsigned char* fileName;
    int fileSize;
    bool isSFN;
}file;

struct cluster
{
    unsigned char* cluster; //The actual cluster bytes
    uint clusterOffset; //The cluster offset in the data region
    bool clusterFound; //Whether this cluster is valid
}cluster;

//GetUIntFromFat
union uint32Char4
{
    unsigned char bytes[4];
    u_int32_t number;
}converter;


struct Partition
{
    unsigned char bootFlag;
    unsigned char chsBegin[3];
    unsigned char typeCode;
    unsigned char chsEnd[3];
    u_int32_t lbaBegin;
    u_int32_t numberOfSectors;
};

struct MasterBootRecord
{
    unsigned char bootCode[446]; //Ignore
    struct Partition partition1;
    struct Partition partition2;
    struct Partition partition3;
    struct Partition partition4;
    u_int16_t mbrPattern;
}MBR;

//Refer to document page 23
/// @brief This represents the byte layout of a standard Directory Entry in FAT32.
struct DirectoryEntry
{
    /* data */
    unsigned char DIR_Name8[9]; //8 filename.
    unsigned char DIR_Name3[4]; //3 filename.

    u_int8_t DIR_Attr; //What is this directory.

    u_int8_t DIR_NTRes; //Unused for the purposes of this project.

    u_int8_t DIR_CrtTimeTenth; //Millisecond stamp at file creation time.

    u_int16_t DIR_CrtTime; //Time file was created.

    u_int16_t DIR_CrtDate; //Data file was created.

    u_int16_t DIR_LstAccDate; //Last access date.

    u_int16_t DIR_FstClusHI; //High word of the entry's first cluster number.

    u_int16_t DIR_WrtTime; //Time of last write.

    u_int16_t DIR_WrtDate; //Date of last write.

    u_int16_t DIR_FstClusLO; //Low word of this entry's first cluster number.

    u_int32_t DIR_FileSize; //32-Bit DWORD holding this file's size in bytes.
};

/// @brief  This represents the byte layout of a Long Directory Entry in FAT32.
struct LongDirectoryEntry
{
    u_int8_t LDIR_Ord; //Order of this entry in the sequence of long dir entries associated with the short dir entry at the end of the long dir set.

    unsigned char LDIR_Name1[6]; //Characters 1-5 of the long-name sub-component in this dir entry.

    u_int8_t LDIR_Attr; //Attributes - must be ATTR_LONG_NAME.

    u_int8_t LDIR_Type; //If zero, indicates a directory entry that is a sub-component of a long name.

    u_int8_t LDIR_Chksum; //Checksum of name in the short dir entry at the end of the long dir set.

    unsigned char LDIR_Name2[7]; //Characters 6-11 of the long-name sub-component in this dir entry.

    u_int16_t LDIR_FstClusLO; //Must be ZERO. Meaningless in context of a long dir entry.

    unsigned char LDIR_Name3[3]; //Characters 12-13 of the long-name sub-component in this dir entry.
};

struct FATDirectory
{
    struct DirectoryEntry dir;
    char* filename;
    unsigned char** clusters;
    uint numClusters;
    bool fileFound;
}fatDir;

/// @brief This struct contains the bit formations which represent the date in FAT32 format.
struct DateFormat
{
    u_int8_t dayOfMonth : 5; //Max value = 31 //Bits 0-4
    u_int8_t monthOfYear : 4; //Max value = 12 //Bits 5-8
    u_int8_t yearsSince1980 : 7; //Max value = 127 //Bits 9-15
};

/// @brief This struct contains the bit formations which represent the time in FAT32 format.
struct TimeFormat
{
    u_int8_t secondCount : 5; //Max value = 29 (58 seconds) //Bits 0-4
    u_int8_t minuteCount : 6; //Max value = 59 (minutes) //Bits 5-10
    u_int8_t hoursCount : 5; //Max value = 23 (hours) //Bits 11-15
};


struct BPBStruct
{
    unsigned char BS_jmpBoot[4]; // Jump instruction to boot code
    unsigned char BS_OEMNane[9]; // 8-Character string (not null terminated)
    unsigned short BPB_BytsPerSec; // Had BETTER be 512!
    unsigned char BPB_SecPerClus; // How many sectors make up a cluster?
    unsigned short BPB_RsvdSecCnt; // # of reserved sectors at the beginning (including the BPB)?
    unsigned char BPB_NumFATs; // How many copies of the FAT are there? (had better be 2)
    unsigned short BPB_RootEntCnt; // ZERO for FAT32
    unsigned short BPB_TotSec16; // ZERO for FAT32
    unsigned char BPB_Media; // SHOULD be 0xF8 for "fixed", but isn't critical for us
    unsigned short BPB_FATSz16; // ZERO for FAT32
    unsigned short BPB_SecPerTrk; // Don't care; we're using LBA; no CHS
    unsigned short BPB_NumHeads; // Don't care; we're using LBA; no CHS
    unsigned int BPB_HiddSec; // Don't care ?
    unsigned int BPB_TotSec32; // Total Number of Sectors on the volume
    unsigned int BPB_FATSz32; // How many sectors long is ONE Copy of the FAT?
    unsigned short BPB_Flags; // Flags (see document)
    unsigned short BPB_FSVer; // Version of the File System
    unsigned int BPB_RootClus; // Cluster number where the root directory starts (should be 2)
    unsigned short BPB_FSInfo; // What sector is the FSINFO struct located in? Usually 1
    unsigned short BPB_BkBootSec; // REALLY should be 6 – (sector # of the boot record backup)
    unsigned char BPB_Reserved[13]; // Should be all zeroes -- reserved for future use
    unsigned char BS_DrvNum; // Drive number for int 13 access (ignore)
    unsigned char BS_Reserved1; // Reserved (should be 0)
    unsigned char BS_BootSig; // Boot Signature (must be 0x29)
    unsigned char BS_VolID[5]; // Volume ID
    unsigned char BS_VolLab[12]; // Volume Label
    unsigned char BS_FilSysType[9]; // Must be "FAT32 "
    unsigned char unused[421]; // Not used
    unsigned char signature[3]; // MUST BE 0x55 AA
} BPB;

/// @brief Gets the offset for the Data Sector of the volume
/// @return The index of first data sector.
uint GetFirstDataSector()
{
    //LBA begin
    //Plus
    //Num reserved sectors
    //Plus
    //Number of Fat * sectors per fat
    //Plus
    //Number of root entries * 32 / bytes per sector
    //Plus
    //Root offset
    uint partitionStartLBA = MBR.partition1.lbaBegin; //Offset lba for partition -FAT32 starts here
    uint RootDirSectors = ((BPB.BPB_RootEntCnt * 32) + (BPB.BPB_BytsPerSec - 1)) / BPB.BPB_BytsPerSec; // This SHOULD Be zero
    uint FirstDataSector = BPB.BPB_RsvdSecCnt + (BPB.BPB_NumFATs * BPB.BPB_FATSz32) + RootDirSectors;

    //Calculate total offset
    uint rootDirLBA = partitionStartLBA + //Index of BPB
    FirstDataSector; //Index of root directory region

    return rootDirLBA;
}

/// @brief Returns the index of the first fat sector in the volume.
/// @return The index of the first fat sector.
uint GetFirstFatSector()
{

    uint partitionStartLBA = MBR.partition1.lbaBegin; //Offset lba for partition -FAT32 starts here
    uint RootDirSectors = ((BPB.BPB_RootEntCnt * 32) + (BPB.BPB_BytsPerSec - 1)) / BPB.BPB_BytsPerSec; // This SHOULD Be zero
    uint FirstFatSector = BPB.BPB_RsvdSecCnt + RootDirSectors; //We do not want to bypass the fat volumes here

    //Calculate total offset
    uint FatDirLBA = partitionStartLBA + //Index of BPB
    FirstFatSector; //Index of fat region

    return FatDirLBA;
}

/// @brief Returns the equivalent sector given a cluster offset in the data region.
/// @param clusterNum The cluster offset in the data region.
/// @return The sector number from the beginning of the disk image.
uint GetSectorOfDataCluster(uint clusterNum)
{
    uint actualSector = ((clusterNum - 2) * BPB.BPB_SecPerClus) + GetFirstDataSector();
    return actualSector;
}

/// @brief Returns the byte offset of the fat contents for a given cluster offset in the data region.
/// @param clusterNum The cluster offset from the data region.
/// @return The byte offset which can be used to locate the fat data relating to a cluster in the data region.
uint GetUIntOfFatCluster(uint clusterNum)
{
    uint FATOffset = clusterNum*4;
    //Contains the sector number of the fat contents
    uint ThisFATSecNum = BPB.BPB_RsvdSecCnt + (FATOffset / BPB.BPB_BytsPerSec);
    //Contains the offset of the data within our sector
    uint ThisFATEntOffset = FATOffset % BPB.BPB_BytsPerSec;

    return ThisFATEntOffset + (ThisFATSecNum+MBR.partition1.lbaBegin)*BPB.BPB_BytsPerSec;
}


//NOTE: This function is kind of ugly, but I needed it for many functions in readdir.
//It is not intended for memory which overlaps. Keep in mind, it is similar in design to memcpy, NOT memmove. 
//If dest does not have enough space, a buffer overflow WILL occur. BE CAREFUL.
/// @brief This function is intended to function like memcpy, but perform custom offsets and iterations. 
/// @param dest Destination array being copied to. The size must be at least one greater than srcSize / iterator. It will be null terminated.
/// @param src Source array being copied from.
/// @param srcSize The number of items being copied from src. The size available in dest must be AT LEAST srcSize / iterator.
/// @param forOffset The offset used when beginning to copy. Src will begin being copied at index forOffset.
/// @param destOffset Destination will begin being copied to from index destOffset.
/// @param iterator The custom iterator for this function. Iterator - 1 is the number of bytes skipped between each copy operation in src.
/// @return The number of bytes copied minus one. The index returned would be the final index copied to in dest.
int OffsetCopier(unsigned char* dest, unsigned char* src, int srcSize, int forOffset, int destOffset, int iterator)
{
    //Initial dest index (not counting the offset)
    int counter = 0;
    //Loop from forOffset to forOffset + srcSize, where each iteration increments i by iterator.
    for(int i = forOffset; i < forOffset+srcSize; i+=iterator)
    {
        //This byte is not used.
        if(src[i] != 0xFF )
        {
            //Set dest to src[i]
            dest[counter + destOffset] = src[i];
        }
        //Copy over a null terminator (\0) instead of 0xFF.
        else dest[counter+destOffset] = '\0';
        
        //Increment counter because dest[counter+destOffset] was written to.
        counter++;
    }

    //Set the final byte at index counter (which is now one greater than the previous index written to)
    //To the null terminator. This terminates the string, and allows it to work appropriately with functions which expect it to be there
    //or are not dependent on size. If size did not factor for a null terminator, memory errors shall ensue.
    dest[counter] = '\0';
    return counter;
}


void PackPartition(struct Partition* part, unsigned char bytes[16])
{
    part->bootFlag = bytes[0];
    OffsetCopier(part->chsBegin, bytes, 3, 1, 0, 1);
    part->typeCode = bytes[4];
    OffsetCopier(part->chsEnd, bytes, 3, 5, 0, 1);
    part->lbaBegin = bytes[8] | ((u_int16_t) bytes[9] << 8) | ((u_int32_t) bytes[10] << 16) |  ((u_int32_t) bytes[11] << 24); 
    part->numberOfSectors = bytes[12] | ((u_int16_t) bytes[13] << 8) | ((u_int32_t) bytes[14] << 16) |  ((u_int32_t) bytes[15] << 24); 
}

void PackMBR(struct MasterBootRecord* mbr, unsigned char sector[512])
{
    OffsetCopier(mbr->bootCode, sector, 446, 0, 0, 1);
    unsigned char partition[16];
    OffsetCopier(partition, sector, 16, 446, 0, 1);
    PackPartition(&mbr->partition1, partition);
    OffsetCopier(partition, sector, 16, 462, 0, 1);
    PackPartition(&mbr->partition2, partition);
    OffsetCopier(partition, sector, 16, 478, 0, 1);
    PackPartition(&mbr->partition3, partition);
    OffsetCopier(partition, sector, 16, 494, 0, 1);
    PackPartition(&mbr->partition4, partition);
    mbr->mbrPattern = sector[510] | ((u_int16_t)sector[511] << 8);
}


/// @brief Gets the next cluster given the current cluster offset of the data region. Packs the data into the Cluster struct.
/// @param currentCluster The offset of the current cluster in the data region.
/// @return Whether or not the next cluster is valid.
bool GetNextClusterFromCurrent(uint currentCluster)
{

    filePtr = fopen(image, "r");
    uint clusterByteSize = BPB.BPB_BytsPerSec*BPB.BPB_SecPerClus;

    cluster.clusterFound = true;
    cluster.cluster = malloc(clusterByteSize);

    //Now, with our array size in mind, retrace our steps and initialize our clusters array
    if((currentCluster & 0x0FFFFFFF) != 0x0FFFFFFF)
    {
        //Initialize this cluster
        cluster.cluster = malloc(clusterByteSize);

        fseek(filePtr, GetSectorOfDataCluster(currentCluster)*BPB.BPB_BytsPerSec, SEEK_SET);
        fread(cluster.cluster, sizeof(unsigned char), clusterByteSize, filePtr);



        //Set file pointer to address of next offset
        //If our prior fat gave us 0x09, then our next address is 9 uints into the fat.
        fseek(filePtr, GetUIntOfFatCluster(currentCluster), SEEK_SET);
        fread(&cluster.clusterOffset, sizeof(uint), 1, filePtr);
        cluster.clusterFound = (cluster.clusterOffset & 0x0FFFFFFF) != 0x0FFFFFFF;
    }
    else
    {
        cluster.clusterFound = false;
    }

    fclose(filePtr);
    return cluster.clusterFound;

}

/// @brief This function packs a LongDirectoryEntry struct with a directory in a sector. This directory is found using an offset.
/// @param directory The DirectoryEntry struct being packed.
/// @param sector The sector containing the correct directory.
/// @param offset The offset through which the directory can be found in the sector. Offset % 32 MUST equal zero, and it must be < (512-32) but > 0 bytes.
void PackLongDirectoryEntry(struct LongDirectoryEntry* directory, unsigned char* cluster, int offset)
{
    directory->LDIR_Ord = cluster[offset+0];
    OffsetCopier(directory->LDIR_Name1, cluster, 10, offset+1, 0, 2);
    directory->LDIR_Attr = cluster[offset+11];
    directory->LDIR_Type = cluster[offset+12];
    directory->LDIR_Chksum = cluster[offset+13];
    OffsetCopier(directory->LDIR_Name2, cluster, 12, offset+14, 0, 2);
    directory->LDIR_FstClusLO = cluster[offset+26] | (cluster[offset+27] << 8);
    OffsetCopier(directory->LDIR_Name3, cluster, 4, offset+28, 0, 2);
}

/// @brief This function packs a DirectoryEntry struct with a directory in a sector. This directory is found using an offset.
/// @param directory The DirectoryEntry struct being packed.
/// @param sector The sector containing the correct directory.
/// @param offset The offset through which the directory can be found in the sector. Offset % 32 MUST equal zero, and it must be < (512-32) but > 0 bytes.
void PackDirectoryEntry(struct DirectoryEntry* directory, unsigned char* cluster, int offset)
{
    if(offset % 32 != 0 || offset < 0) return;
    OffsetCopier(directory->DIR_Name8, cluster,8, offset+0,0,1);
    OffsetCopier(directory->DIR_Name3, cluster,3,offset+8,0,1);
    directory->DIR_Attr = cluster[offset+11];
    directory->DIR_NTRes = cluster[offset+12];
    directory->DIR_CrtTimeTenth = cluster[offset+13];
    //These are in little endian format (least-significant byte on the right)
    directory->DIR_CrtTime = cluster[offset+14] | ((u_int16_t)(cluster[offset+15]) << 8);
    directory->DIR_CrtDate = cluster[offset+16] | ((u_int16_t)(cluster[offset+17]) << 8);
    directory->DIR_LstAccDate = cluster[offset+18] | ((u_int16_t)cluster[offset+19] << 8);
    directory->DIR_FstClusHI = cluster[offset+20] | ((u_int16_t)cluster[offset+21] << 8);
    directory->DIR_WrtTime = cluster[offset+22] | (cluster[offset+23] << 8);
    directory->DIR_WrtDate = cluster[offset+24] | (cluster[offset+25] << 8);
    directory->DIR_FstClusLO = cluster[offset+26] | (u_int16_t)(cluster[offset+27] << 8);
    directory->DIR_FileSize = cluster[offset+28] | (cluster[offset+29] << 8) | (cluster[offset+30] << 16) | (cluster[offset+31] << 24);
}

/// @brief Packs 2 bytes into the TimeFormat struct
/// @param time The TimeFormat struct to be packed
/// @param bitPackage The package to be loaded 
void PackTime(struct TimeFormat* time, u_int16_t bitPackage)
{
    
    //This is initially measured in two second intervals -
    //Multiply by two to get an accurate count
    time->secondCount = (bitPackage & 0x001F)*2;
    time->minuteCount = (bitPackage & 0x07E0) >> 5;
    time->hoursCount = (bitPackage & 0xF800) >> 11;
}

/// @brief Packs 2 bytes into the DateFormat struct
/// @param date The DateFormat struct to be packed
/// @param bitPackage The package to be loaded
void PackDate(struct DateFormat* date, u_int16_t bitPackage)
{
    date->dayOfMonth = (bitPackage & 0x001F);
    date->monthOfYear = (bitPackage & 0x01E0) >> 5;
    date->yearsSince1980 = (bitPackage & 0xFE00) >> 9;
}

/// @brief Returns whether or not the given directory is a LFD
/// @param directoryEntry The directory in question
/// @return Whether or not the directory is a LFD
bool isLongFileDirectory(unsigned char directoryEntry[32])
{
    return directoryEntry[11] == (unsigned char)ATTR_LONG_NAME;
}


void PackBPB(struct BPBStruct* bpb, unsigned char bytes[512])
{
    memcpy(bpb->BS_jmpBoot, bytes, 3);
    OffsetCopier(bpb->BS_OEMNane, bytes, 8, 3, 0, 1);
    bpb->BPB_BytsPerSec = bytes[11] | ((u_int16_t)bytes[12] << 8);
    bpb->BPB_SecPerClus = bytes[13];
    bpb->BPB_RsvdSecCnt = bytes[14] | ((u_int16_t)bytes[15] << 8);
    bpb->BPB_NumFATs = bytes[16];
    bpb->BPB_RootEntCnt = bytes[17] | ((u_int16_t)bytes[18] << 8);;
    bpb->BPB_TotSec16 = bytes[19] | ((u_int16_t)bytes[20] << 8);;
    bpb->BPB_Media = bytes[21];
    bpb->BPB_FATSz16 = bytes[22] | ((u_int16_t)bytes[23] << 8);;
    bpb->BPB_SecPerTrk = bytes[24] | ((u_int16_t)bytes[25] << 8);;
    bpb->BPB_NumHeads = bytes[26] | ((u_int16_t)bytes[27] << 8);;
    bpb->BPB_HiddSec = bytes[28] | ((u_int16_t)bytes[29] << 8) | ((u_int32_t)bytes[30] << 16) | ((u_int32_t)bytes[31] << 24);
    bpb->BPB_TotSec32 = bytes[32] | ((u_int16_t)bytes[33] << 8) | ((u_int32_t)bytes[34] << 16) | ((u_int32_t)bytes[35] << 24);
    bpb->BPB_FATSz32 = bytes[36] | ((u_int16_t)bytes[37] << 8) | ((u_int32_t)bytes[38] << 16) | ((u_int32_t)bytes[39] << 24);
    bpb->BPB_Flags = bytes[40] | ((u_int16_t)bytes[41] << 8);
    bpb->BPB_FSVer = bytes[42] | ((u_int16_t)bytes[43] << 8);
    bpb->BPB_RootClus = bytes[44] | ((u_int16_t)bytes[45] << 8) | ((u_int32_t)bytes[46] << 16) | ((u_int32_t)bytes[47] << 24);
    bpb->BPB_FSInfo = bytes[48] | ((u_int16_t)bytes[49] << 8);
    bpb->BPB_BkBootSec = bytes[50] | ((u_int16_t)bytes[51] << 8);
    OffsetCopier(bpb->BPB_Reserved, bytes, 12, 52, 0, 1);
    bpb->BS_DrvNum = bytes[64];
    bpb->BS_Reserved1 = bytes[65];
    bpb->BS_BootSig = bytes[66];
    OffsetCopier(bpb->BS_VolID, bytes, 4, 67, 0, 1);
    OffsetCopier(bpb->BS_VolLab, bytes, 11, 71, 0, 1);
    OffsetCopier(bpb->BS_FilSysType, bytes, 8, 82, 0, 1);
    OffsetCopier(bpb->unused, bytes, 420, 90, 0, 1);
    OffsetCopier(bpb->signature, bytes, 2, 510, 0, 1);
}

/// @brief Checks to see if a filename is a SFN.
/// @return Whether it is or not.
bool fileNameSFNValidator()
{
    for(int i = 0; i < file.fileSize-1; i++)
    {
        //If the string matchess any of these cases, return false
        if( file.fileName[0] == 0x20)return false;
        if((file.fileName[i]  < 0x20) 
        || (file.fileName[i] == 0x22)
        || (file.fileName[i] >= 0x2A && file.fileName[i] <= 0x2F)
        || (file.fileName[i] >= 0x3A && file.fileName[i] <= 0x3F)
        || (file.fileName[i] >= 0x5B && file.fileName[i] <= 0x5D)
        || (file.fileName[i] == 0x7C))
        {
            return false;
        }
    }

    //None of the illegal characters were found - its a SFN
    return true;
}

void displaySector(unsigned char* sector)
{
    // Display the contents of sector[] as 16 rows of 32 bytes each. Each row is shown as 16 bytes,
    // a "-", and then 16 more bytes. The left part of the display is in hex; the right part is in
    // ASCII (if the character is printable; otherwise we display ".".
    //
    for (int i = 0; i < 16; i++) // for each of 16 rows
    { //
    
        for (int j = 0; j < 32; j++) // for each of 32 values per row
        { //
            printf("%02X ", sector[i * 32 + j]); // Display the byte in hex
            if (j % 32 == 15) printf("- "); // At the half-way point? Show divider
        }

        printf(" "); // Separator space between hex & ASCII
        for (int j = 0; j < 32; j++) // For each of those same 32 bytes
        { //
            if (j == 16) printf(" "); // Halfway? Add a space for clarity
            int c = (unsigned int)sector[i * 32 + j]; // What IS this char's ASCII value?
            if (c >= 32 && c <= 127) printf("%1c", c); // IF printable, display it
            else printf("."); // Otherwise, display a "."
        } //

        printf("\n"); // That’s the end of this row
    }
}

void displayCluster(unsigned char* cluster)
{
    // Display the contents of sector[] as 16 rows of 32 bytes each. Each row is shown as 16 bytes,
    // a "-", and then 16 more bytes. The left part of the display is in hex; the right part is in
    // ASCII (if the character is printable; otherwise we display ".".
    //
    int clusterN  = 0;
    while(clusterN < 8)
    {
        for (int i = 0; i < 16; i++) // for each of 16 rows
        { //
        
            for (int j = 0; j < 32; j++) // for each of 32 values per row
            { //
                printf("%02X ", cluster[clusterN *512 + i * 32 + j]); // Display the byte in hex
                if (j % 32 == 15) printf("- "); // At the half-way point? Show divider
            }

            printf(" "); // Separator space between hex & ASCII
            for (int j = 0; j < 32; j++) // For each of those same 32 bytes
            { //
                if (j == 16) printf(" "); // Halfway? Add a space for clarity
                int c = (unsigned int)cluster[clusterN*512 + i * 32 + j]; // What IS this char's ASCII value?
                if (c >= 32 && c <= 127) printf("%1c", c); // IF printable, display it
                else printf("."); // Otherwise, display a "."
            } //

            printf("\n"); // That’s the end of this row
        }
        printf("\n");
        clusterN++;
    }
}




//We need a function to return a dynamically allocated array of clusters.
//We do not know the size immediately. This may require two walkthroughs.

/// @brief This function is designed to get an array of the linked clusters starting from the first directory
/// @param clusters The array to have the linked list of clusters stored in
/// @param fatBlockZero The sector number of the fat block offset
/// @param fatTableClusterLo The cluster number of the first cluster of the directory
/// @param arg The file name containing our image
/// @return The number of clusters in the linked list, also the size of the array of clusters.
int GetDirectoryFromClusterLO(uint fatTableClusterLo)
{
    uint clusterByteSize = BPB.BPB_BytsPerSec*BPB.BPB_SecPerClus;
    //Open file
    filePtr = fopen(image, "r");

    //Used to count how many clusters are in this directory
    uint numClusters = 0;
    
    //Next cluster is the first one.
    uint nextCluster = fatTableClusterLo;

    //This loop will give us the size of our cluster
    //While cluster is nonnegative
    while((nextCluster & 0x0FFFFFFF) != 0x0FFFFFFF)
    {
        //Iterate our number of clusters
        numClusters++; 

        //Set file pointer to address of next offset
        //If our prior fat gave us 0x09, then our next address is 9 uints into the fat.
        fseek(filePtr, GetUIntOfFatCluster(nextCluster), SEEK_SET);
        
        
        fread(converter.bytes, sizeof(unsigned char), 4, filePtr);
        nextCluster = converter.number & 0x0FFFFFFF;
    }  

    //DESIGN QUESTION: Why am I running the same loop twice?
    //This problem could easily be avoided with a dynamic array, as I would just add the clusters
    //to the dynamic array in the first loop. If I wanted to do this statically, and define the size of the loop
    //when creating the array, this would be the methodology.
    //The dynamic route would be more efficient, but in the meantime, the static route is easier
    //and does not require the creation of a dynamic array data type in the C language.
    //A better solution would be ideal.

    int clusterIterator = 0; //The cluster iterator

    //Our next cluster is the first one.
    nextCluster = fatTableClusterLo;
    fatDir.clusters = malloc(numClusters*clusterByteSize);

    //Now, with our array size in mind, retrace our steps and initialize our clusters array
    while((nextCluster & 0x0FFFFFFF) != 0x0FFFFFFF && clusterIterator < numClusters)
    {
        //Initialize this cluster
        fatDir.clusters[clusterIterator] = malloc(clusterByteSize);

        //Here we have our current cluster number.
        //Use this number and the data region offset to get our cluster and add it to
        //the array.
        //Then, find the next cluster!
        fseek(filePtr, GetSectorOfDataCluster(nextCluster)*BPB.BPB_BytsPerSec, SEEK_SET);
        fread(fatDir.clusters[clusterIterator], sizeof(unsigned char), clusterByteSize, filePtr);

        //Iterate our cluster iterator
        clusterIterator++;

        //Set file pointer to address of next offset
        //If our prior fat gave us 0x09, then our next address is 9 uints into the fat.
        fseek(filePtr, GetUIntOfFatCluster(nextCluster), SEEK_SET);
        
        fread(converter.bytes, sizeof(unsigned char), 4, filePtr);
        nextCluster = converter.number & 0x0FFFFFFF;
    }

    fclose(filePtr);
    fatDir.numClusters = numClusters;
    return numClusters;
}

/// @brief This function takes the first cluster of a directory and displays all relevant file information related to it.
/// @param cluster The first cluster of the directory.
void Readdir(uint loCluster)
{
    uint numClusters = GetDirectoryFromClusterLO(loCluster);
    int bytesPerCluster = BPB.BPB_SecPerClus * BPB.BPB_BytsPerSec; //Number of bytes in the cluster

    int dirCounter = 0;
    struct LongDirectoryEntry *longDirs;
    bool longDirectoryActive = false;
    u_int8_t numberOfLdirs = 0;
    u_int32_t totalBytes = 0;
    u_int16_t totalFiles = 0;

    for(int clusterNum = 0; clusterNum < numClusters; clusterNum++)
    {
        //Each iteration, directory contains one sector
        for(uint sectorNum = 0; sectorNum < BPB.BPB_SecPerClus; sectorNum++)
        {
            //Each iteration is a single directory entry
            for(uint i = 0; i < BPB.BPB_BytsPerSec; i+=32)
            {
                //At sector 0: 0-511
                //Sector 1: 512-1023
                uint currentIndex = i + BPB.BPB_BytsPerSec * sectorNum;

                //This directory is free
                if(fatDir.clusters[clusterNum][currentIndex] == 0xE5) continue;

                //This directory is free as are all directory entries after it in this sector.
                if(fatDir.clusters[clusterNum][currentIndex] == 0x0) break;

                //Is LongFileDirectory
                if(isLongFileDirectory(&fatDir.clusters[clusterNum][currentIndex]))
                {
                    longDirectoryActive = true;

                    u_int8_t ldirCurrentOrder = (fatDir.clusters[clusterNum][currentIndex+0] & 0x0F);
                    bool isLastEntry = ((fatDir.clusters[clusterNum][currentIndex+0] & 0xF0) == LAST_LONG_ENTRY);

                    //This is the final entry of a long directory (but the first one we will find)
                    //Load valuable information about this LDIR
                    if(isLastEntry)
                    {
                        //Get size of longDirs
                        numberOfLdirs = (fatDir.clusters[clusterNum][currentIndex+0] & 0x0F);

                        //Initialize longDirs array of structs
                        //This will hold all of our long directories, which we can use to
                        //print the long names.
                        longDirs = malloc(32*ldirCurrentOrder);

                        //We just read in the last entry, and on our next iteration, it will either be another long entry (not the last one) or a short entry. 
                        isLastEntry = false;
                    }

                    //The index of our current long directory (number of directories minus long directory order byte)
                    u_int8_t longDirsIndex = numberOfLdirs - ldirCurrentOrder;
                    
                    //Pack longDirs at current index with the directory information
                    PackLongDirectoryEntry(&longDirs[longDirsIndex], fatDir.clusters[clusterNum], currentIndex);
                }
                //Is short FileDirectory
                else
                {
                    struct DirectoryEntry directoryEntry;
                    //Pack directoryEntry with our current directory info

                    PackDirectoryEntry(&directoryEntry, fatDir.clusters[clusterNum], currentIndex);
                    struct TimeFormat tf;
                    struct DateFormat df;

                    //Pack structs with packages
                    PackTime(&tf, directoryEntry.DIR_CrtTime);
                    PackDate(&df, directoryEntry.DIR_CrtDate);

                    //If attribute is volume ID
                    if((directoryEntry.DIR_Attr & ATTR_VOLUME_ID) == ATTR_VOLUME_ID)
                    {
                        //Display volume information
                        printf("Volume in drive %s is %s%s\n\n",BPB.BS_VolID ,directoryEntry.DIR_Name8, directoryEntry.DIR_Name3);
                        printf("Directory of %s:/\n\n", BPB.BS_VolLab);
                    }  
                    //If attribute is not SYSTEM or HIDDEN print their information.
                    else if((directoryEntry.DIR_Attr & ATTR_HIDDEN) != ATTR_HIDDEN && (directoryEntry.DIR_Attr &&  ATTR_SYSTEM) != ATTR_SYSTEM)
                    {
                        //Begin printing
                        //Print the date info
                        printf("%02u/%02u/%02u ",df.dayOfMonth,df.monthOfYear,df.yearsSince1980+1980);

                        //Print the time info
                        if(tf.hoursCount>12) printf("%02u:%02u PM ", tf.hoursCount - 12, tf.minuteCount);
                        else printf("%02u:%02u AM ", tf.hoursCount, tf.minuteCount);

                        if(directoryEntry.DIR_Attr != ATTR_DIRECTORY)
                        {
                            //Print the file size in bytes
                            //This sets it so that the bytes will be separated out in the thousands place by comma
                            setlocale(LC_NUMERIC, "");
                            printf("      %'14u ",directoryEntry.DIR_FileSize);
                            
                            //Keep track of total bytes used by files in this directory
                            totalBytes += directoryEntry.DIR_FileSize;

                            //Increment number of files
                            totalFiles += 1;

                            //Print the files 8.3 name
                            printf("%s.%s ", directoryEntry.DIR_Name8, directoryEntry.DIR_Name3);
                        }
                        else
                        {
                            //This is a directory - print this flag.
                            printf("<DIR> ");

                            //Print the files 8.3 name
                            printf("%23s%s  ", directoryEntry.DIR_Name8, directoryEntry.DIR_Name3);
                            dirCounter++;
                        }


                        //Print long directory name
                        if(longDirectoryActive)
                        {
                            for(int i = numberOfLdirs-1; i >= 0; i--)
                            {
                                printf("%s",longDirs[i].LDIR_Name1);
                                printf("%s",longDirs[i].LDIR_Name2);
                                printf("%s",longDirs[i].LDIR_Name3);
                            }

                            //Clear out (reset) this ldir information
                            numberOfLdirs = 0;
                            //We have printed out the important information gathered in longDirs.
                            //Since we are moving on to either a new long directory or another short one,
                            //we can free this memory now.
                            free(longDirs);

                            //We are no longer actively reading in a long directory.
                            longDirectoryActive = false;
                        }

                        //Done printing
                        printf("\n");
                    }
                    //Pass over this directory (it is either a system, hidden, or volume ID directory)
                    else
                    {
                        //This case seems to imply a long directory got corrupted and was never finished.
                        //This shouldn't happen, but if it does, we will not print it out, and instead will reset it.
                        if(longDirectoryActive)
                        {
                            //Reset ldir info
                            numberOfLdirs = 0;
                            free(longDirs);
                            longDirectoryActive = false;
                        }
                    }

                }

            }
        }
    }

    //We're done with this cluster array for now
    free(fatDir.clusters);

    //Print out summary data
    printf("\n%u File(s) %'10u bytes\n", totalFiles, totalBytes);
    printf("%u Dir(s)\n", dirCounter++);
}

/// @brief Using the filename and a fat table low cluster offset, fills the directory information into the FATDirectory struct.
/// @param fatTableClusterLo The offset of the low cluster of a particular directory.
void GetDirectoryFromFilename(uint fatTableClusterLo)
{
    GetDirectoryFromClusterLO(fatTableClusterLo);
    //We should already have the clusters initialized at this point,
    //as well as the number of clusters.

    //Now, we need to parse through the directory and find a file with the name we're looking for.

    int bytesPerCluster = BPB.BPB_SecPerClus * BPB.BPB_BytsPerSec; //Number of bytes in the cluster

    struct LongDirectoryEntry *longDirs;
    bool longDirectoryActive = false;
    u_int8_t numberOfLdirs = 0;
    u_int32_t totalBytes = 0;
    u_int16_t totalFiles = 0;
    fatDir.filename = "";
    fatDir.fileFound = false;

    //Read through the clusters
    for(int clusterNum = 0; clusterNum < fatDir.numClusters; clusterNum++)
    {
        //Each iteration, directory contains one sector
        for(uint sectorNum = 0; sectorNum < BPB.BPB_SecPerClus; sectorNum++)
        {
            //Each iteration is a single directory entry
            for(uint i = 0; i < BPB.BPB_BytsPerSec; i+=32)
            {
                //At sector 0: 0-511
                //Sector 1: 512-1023
                uint currentIndex = i + BPB.BPB_BytsPerSec * sectorNum;

                //This directory is free
                if(fatDir.clusters[clusterNum][currentIndex] == 0xE5) continue;

                //This directory is free as are all directory entries after it in this sector.
                if(fatDir.clusters[clusterNum][currentIndex] == 0x0) break;

                //Is LongFileDirectory
                if(isLongFileDirectory(&fatDir.clusters[clusterNum][currentIndex]))
                {
                    longDirectoryActive = true;

                    u_int8_t ldirCurrentOrder = (fatDir.clusters[clusterNum][currentIndex+0] & 0x0F);
                    bool isLastEntry = ((fatDir.clusters[clusterNum][currentIndex+0] & 0xF0) == LAST_LONG_ENTRY);

                    //This is the final entry of a long directory (but the first one we will find)
                    //Load valuable information about this LDIR
                    if(isLastEntry)
                    {
                        //Get size of longDirs
                        numberOfLdirs = (fatDir.clusters[clusterNum][currentIndex+0] & 0x0F);

                        //Initialize longDirs array of structs
                        //This will hold all of our long directories, which we can use to
                        //print the long names.
                        longDirs = malloc(32*ldirCurrentOrder);

                        //We just read in the last entry, and on our next iteration, it will either be another long entry (not the last one) or a short entry. 
                        isLastEntry = false;
                    }

                    //The index of our current long directory (number of directories minus long directory order byte)
                    u_int8_t longDirsIndex = numberOfLdirs - ldirCurrentOrder;
                    
                    //Pack longDirs at current index with the directory information
                    PackLongDirectoryEntry(&longDirs[longDirsIndex], fatDir.clusters[clusterNum], currentIndex);
                }
                //Is short FileDirectory
                else
                {
                    struct DirectoryEntry directoryEntry;
                    //Pack directoryEntry with our current directory info

                    PackDirectoryEntry(&directoryEntry, fatDir.clusters[clusterNum], currentIndex);
                    struct TimeFormat tf;
                    struct DateFormat df;

                    //Pack structs with packages
                    PackTime(&tf, directoryEntry.DIR_CrtTime);
                    PackDate(&df, directoryEntry.DIR_CrtDate);

                    //If attribute is volume ID
                    if((directoryEntry.DIR_Attr & ATTR_VOLUME_ID) == ATTR_VOLUME_ID)
                    {
                        //Volume
                    }  
                    //If attribute is not SYSTEM or HIDDEN print their information.
                    else if((directoryEntry.DIR_Attr & ATTR_HIDDEN) != ATTR_HIDDEN && (directoryEntry.DIR_Attr &&  ATTR_SYSTEM) != ATTR_SYSTEM)
                    {
                        //Not a directory
                        if(directoryEntry.DIR_Attr != ATTR_DIRECTORY)
                        {
                            //File shortname
                        }
                        else
                        {
                            //This is a directory
                        }

                        //Tells us if this directory is the one we were
                        //looking for.
                        bool sameString = false;

                        unsigned char* fileName;
                        
                        //This file might have an 8.3 directory name
                        if(longDirectoryActive)
                        {
                            //This variable holds the actual stored file name
                            fileName = malloc(120*sizeof(unsigned char));
                            strcpy(fileName, "");

                            //Compare files using long name                 
                            for(int i = numberOfLdirs-1; i >= 0; i--)
                            {
                                strncat(fileName, longDirs[i].LDIR_Name1, 5);
                                strncat(fileName, longDirs[i].LDIR_Name2, 6);
                                strncat(fileName, longDirs[i].LDIR_Name3, 2);
                            }
                            //Make sure its null terminated
                            strncat(fileName, "\0", file.fileSize);
                            sameString = (strncasecmp(fileName, file.fileName, file.fileSize) == 0);
                            
                            //Clear out (reset) this ldir information
                            numberOfLdirs = 0;

                            //We have printed out the important information gathered in longDirs.
                            //Since we are moving on to either a new long directory or another short one,
                            //we can free this memory now.
                            free(longDirs);

                            longDirectoryActive = false;
                        }

                        //This file may be an 8.3 file
                        if(file.fileSize <= 14 && file.isSFN && !sameString)
                        {

                            //Compare to short name
                            fileName = malloc(120*sizeof(unsigned char));

                            //This index is the final index with content
                            uint nameIndex = 0;

                            //We need to trim these spaces
                            for(int i = 0; i < 8; i++)
                            {
                                //This index contains garbage - the rest of the string is bad
                                if(directoryEntry.DIR_Name8[i] == 0x20) break;

                                //Increment the valid index
                                nameIndex++;
                            }

                            //Copy DIR_NAME8 only up to the index with contents
                            strncpy(fileName, directoryEntry.DIR_Name8, nameIndex);
                            fileName[nameIndex] =  '\0';

                            //How many characters in our extension
                            nameIndex = 0;

                            //We need to trim these spaces
                            for(int i = 0; i < 3; i++)
                            {
                                //This index contains garbage - the rest of the string is bad
                                if(directoryEntry.DIR_Name3[i] == 0x20) break;

                                //Increment the valid index
                                nameIndex++;
                            }

                            strncat(fileName, directoryEntry.DIR_Name3, nameIndex);

                            //Make sure its null terminated
                            //strncat(fileName, "\0", file.fileSize);
                    
                            sameString = (strncasecmp(fileName, file.fileName, file.fileSize) == 0);
                        }
                        
                        //We found the directory.
                        if(sameString)
                        {       
                            fatDir.filename = "\0";
                            fatDir.filename = malloc(120*sizeof(unsigned char));

                            //Store the file name into fatDir
                            strncpy(fatDir.filename, fileName, file.fileSize);
                            strncat(fatDir.filename, "\0", file.fileSize);
                            
                            //Set the directory data
                            fatDir.dir = directoryEntry;
                            fatDir.fileFound = true;
                            sameString = false;

                            //We're done with this.
                            free(fileName);
                            return;
                        }
                    }
                    //Pass over this directory (it is either a system, hidden, or volume ID directory)
                    else
                    {
                        //This case seems to imply a long directory got corrupted and was never finished.
                        //This shouldn't happen, but if it does, we will not print it out, and instead will reset it.
                        if(longDirectoryActive)
                        {
                            //Reset ldir info
                            numberOfLdirs = 0;
                            free(longDirs);
                            longDirectoryActive = false;
                        }
                    }
                }
            }
        }
    }
}

/// @brief Attempts to extract a given directory based on its low cluster index in the data region. 
/// Extracting the directory will copy it into a file in the same directory.
/// @param fatTableClusterLo The index of the low cluster of a directory in the data region.
void Extract(uint fatTableClusterLo)
{
    uint clusterByteSize = BPB.BPB_BytsPerSec*BPB.BPB_SecPerClus;

    //With the disk image and file name
    //We need to find the file in the disk image with the same name.
    GetDirectoryFromFilename(fatTableClusterLo);
    if(!fatDir.fileFound) 
    {
        printf("File Not Found\n");
        return;
    }

    //The directory is loaded into fatDir
    //printf("HI Clus: 0x%X\n",fatDir.dir.DIR_FstClusHI);
    //printf("LO Clus: 0x%X\n",fatDir.dir.DIR_FstClusLO);
    u_int32_t fileClusterOffset = fatDir.dir.DIR_FstClusLO | ((u_int32_t) fatDir.dir.DIR_FstClusHI << 16);
    //printf("Cluster number: 0x%X\n", fileClusterOffset);

    // Create a file
    FILE* newfile = fopen(fatDir.filename, "w");
    cluster.clusterOffset = fileClusterOffset;
    uint clusterCount = fatDir.dir.DIR_FileSize / clusterByteSize;
    uint bytesInLastCluster = (fatDir.dir.DIR_FileSize % clusterByteSize); 

    uint clusterIterator = 0;
    while(clusterIterator <= clusterCount)
    {
        GetNextClusterFromCurrent(cluster.clusterOffset);
        uint bytesToRead = (clusterIterator == clusterCount) ? bytesInLastCluster : clusterByteSize;
        
        for(uint i = 0; i < bytesToRead; i++)
        {
            fputc(cluster.cluster[i], newfile);
        }
        clusterIterator++;
    }
    fclose(newfile);
    
    free(cluster.cluster);
    free(file.fileName);
    free(fatDir.clusters);
    free(fatDir.filename);

    //Clear the input buffer
    fseek(stdin, 0, SEEK_END);
}

uint ChangeDirectory(uint fatTableClusterLo)
{
    uint clusterByteSize = BPB.BPB_BytsPerSec*BPB.BPB_SecPerClus;

    if(strncmp(file.fileName, ".", file.fileSize) == 0 ||
    strncmp(file.fileName, "..", file.fileSize) == 0) file.isSFN = true;

    //With the disk image and file name
    //We need to find the file in the disk image with the same name.
    GetDirectoryFromFilename(fatTableClusterLo);

    if(!fatDir.fileFound || fatDir.dir.DIR_Attr != ATTR_DIRECTORY) 
    {
        printf("Directory Not Found\n");
        return -1;
    }

    free(cluster.cluster);
    free(file.fileName);
    free(fatDir.clusters);
    free(fatDir.filename);

    //Clear the input buffer
    fseek(stdin, 0, SEEK_END);
    return (fatDir.dir.DIR_FstClusLO | (fatDir.dir.DIR_FstClusHI << 16));
}


#endif