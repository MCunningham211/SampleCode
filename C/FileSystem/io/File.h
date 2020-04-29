#ifndef FILE_H
#define FILE_H

#define FILEPATH "../disk/vdisk"
#define BLOCKSIZE 512
#define NUMBLOCKS 4096
#define NUMINODES 128

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <float.h>
#include "../disk/diskcontrol.h"

struct INode
{
  int fileSize;
  int fileType;
  uint16_t blocksUsed[12];
};

/** 
 * Adds a file to a folder such as root
 */
void addFileToFolder(char* fileName, char* folderName);
/** 
 * Adds a file to the directory structure
 */
void addToDirectory(uint8_t nodeBlock, char* fileName, int fileType);
/** 
 * Checks the bit vector to see if a given block is free
 */
bool blockAvailable(int blocknumber);
/** 
 * Creates the initial quick Directory file
 */
void createDirectory();
/** 
 * Creates a folder. called when create file is called with file type 0
 */
void createFile(char* name, char* content, int type, int length);
/** 
 * Creates a file and writes it's content
 */
void createFolder(char* name, char* content, int type, int length);
/** 
 * Creates an I node for a file and returns the Inode number
 */
uint8_t createINode(int fileSize, int fileType, int BlocksUsed[]);
/** 
 * Deletes  file and frees it's resources
 */
void deleteFile(char *fileName);
/** 
 * Deletes a file from a folder
 */
void deleteFileFromFolder(char* fileName, char* folderName);
/** 
 * Deletes a folder. Only works if the folder is not empty
 */
void deleteFolder(char* fileName);
/** 
 * Frees an Inode and the blocks the Inode is pointing to including those pointed to in the indirect block
 */
void deleteINode(int Number);
/** 
 * Reads the contents of a file in the actual computers file system and stores it here
 */
void eatFile(char* fileName);
/** 
 * Checks the bit vector and returns the number of the next free block
 */
int findFreeBlock();
/** 
 * Checks if a folder contains a file
 */
int folderContains(char* fileName, char* folderName);
/** 
 * Initial Disk Write
 */
void formatDisk();
/** 
 * Checks the Inodes and returns a free one
 */
int getFreeINode();
/**
 * Looks at the directory and returns the inode number from the file
 */
int getINodeFromDirectory(char *fileName);
/** 
 * Initiats Block0 which contains metadata about the filesystem
 */
void initBlock0();
/** 
 * Checks if a given Inode is currently used
 */
bool INodeAvailable(int nodeNumber);
/** 
 * Marks a block in the bit vector as free
 */
void markblockFree(int blocknumber);
/** 
 * Marks a block in the bit vector as used
 */
void markblockUsed(int blocknumber);
/** 
 * Marks an inode as in use
 */
void markINodeUsed(int nodeNumber);
/** 
 * checks if a file with some path exists Ie: root/filename
 */
int pathExists(char* input);
/** 
 * Print a file given a file at the path
 */
void printAtPath(char *input);
/** 
 * For dubugging
 */
void printDebugDirectory();
/** 
 * Prints a list containig every file in the system ordered by the inode number they use
 */
void printDirectory();
/** 
 * Prints the contents of a folder
 */
void printFolderContents(char* fileName);
/** 
 * Prints the contents of a file
 */
void readFile(char* fileName);
/** 
 * returns an inode given an inode number
 */
struct INode* readINode(int number);
/** 
 * Writes Inode Values to the file system
 */
void writeINode(int number, struct INode node);


#endif