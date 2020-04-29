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

bool blockAvailable(int blocknumber)
{
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  readBlock(disk, 1, buffer);
  char character = buffer[blocknumber / 8];
  int bit = blocknumber % 8;
  free(buffer);
  fclose(disk);
  return !((character >> bit) & 1);
}
int findFreeBlock()
{
  for (int i = 0; i < NUMBLOCKS; i++)
    if (blockAvailable(i))
      return i;
  return -1;
}
void markblockUsed(int blocknumber)
{
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  readBlock(disk, 1, buffer);
  char character = buffer[blocknumber / 8];
  character |= 1 << (blocknumber % 8);
  buffer[blocknumber / 8] = (char)character;
  writeBlock(disk, 1, buffer);
  fclose(disk);
  free(buffer);
}
void markblockFree(int blocknumber)
{
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  readBlock(disk, 1, buffer);
  char character = buffer[blocknumber / 8];
  character &= ~(1 << (blocknumber % 8));
  buffer[blocknumber / 8] = (char)character;
  writeBlock(disk, 1, buffer);
  fclose(disk);
  free(buffer);
}
struct INode
{
  int fileSize;
  int fileType;
  uint16_t blocksUsed[12];
};
bool INodeAvailable(int nodeNumber)
{
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  readBlock(disk, 0, buffer);
  char character = buffer[(nodeNumber / 8) + (sizeof(int) * 3)];
  int bit = nodeNumber % 8;
  free(buffer);
  fclose(disk);
  return !((character >> bit) & 1);
}
int getFreeINode()
{
  for (int i = 0; i < NUMINODES; i++)
    if (INodeAvailable(i))
      return i;
  return -1;
}
void markINodeUsed(int nodeNumber)
{
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  readBlock(disk, 0, buffer);
  char character = buffer[(nodeNumber / 8) + (sizeof(int) * 3)];
  character |= 1 << (nodeNumber % 8);

  buffer[(nodeNumber / 8) + (sizeof(int) * 3)] = (char)character;
  writeBlock(disk, 0, buffer);
  fclose(disk);
  free(buffer);
}
void writeINode(int number, struct INode node)
{
  markINodeUsed(number);
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  int address = 2 + number / 16;
  int index = number % 16;
  readBlock(disk, address, buffer);
  int location = (sizeof(int) * 2 + sizeof(uint16_t) * 12) * (index);
  memcpy(buffer + location, &(node.fileSize), sizeof(int));
  memcpy(buffer + location + sizeof(int), &(node.fileType), sizeof(int));
  for (int i = 0; i < 12; i++)
    memcpy(buffer + location + sizeof(int) * 2 + i * sizeof(int16_t), &(node.blocksUsed[i]), sizeof(int16_t));
  writeBlock(disk, address, buffer);
  fclose(disk);
  free(buffer);
}
struct INode *readINode(int number)
{
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  struct INode *node = (struct INode *)malloc(sizeof(struct INode));
  int address = 2 + number / 16;
  int index = number % 16;
  readBlock(disk, address, buffer);
  int location = (sizeof(int) * 2 + sizeof(uint16_t) * 12) * (index);
  memcpy(&(node->fileSize), buffer + location, sizeof(int));
  memcpy(&(node->fileType), buffer + location + sizeof(int), sizeof(int));
  for (int i = 0; i < 12; i++)
    memcpy(&(node->blocksUsed[i]), buffer + location + sizeof(int) * 2 + i * sizeof(int16_t), sizeof(int16_t));
  fclose(disk);
  free(buffer);
  return node;
}
uint8_t createINode(int fileSize, int fileType, int BlocksUsed[])
{
  int nodeNumber = getFreeINode(); //returns -1 if it can't
  if (nodeNumber == -1)
    return nodeNumber;
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  struct INode *node = (struct INode *)malloc(sizeof(struct INode));
  node->fileSize = fileSize;
  node->fileType = fileType;

  int fancyBlockNumber = 0;
  if (fileSize > 11)
  {
    fancyBlockNumber = findFreeBlock();
    if (fancyBlockNumber == -1)
    {
      printf("Can't Create INode Indirection, File System is out of memory \n");
      free(buffer);
      free(node);
      return -1;
    }
    markblockUsed(fancyBlockNumber);
  }
  for (uint16_t i = 0; i < fileSize; i++)
  {
    if (i < 11)
    {
      node->blocksUsed[i] = BlocksUsed[i];
      markblockUsed(BlocksUsed[i]);
    }
    else if (i == 11)
    {
      node->blocksUsed[i] = fancyBlockNumber;
      memcpy(buffer + (i - 11) * sizeof(uint16_t), &BlocksUsed[i], sizeof(uint16_t));
    }
    else
    {
      memcpy(buffer + (i - 11) * sizeof(uint16_t), &BlocksUsed[i], sizeof(uint16_t));
    }
  }
  writeBlock(disk, node->blocksUsed[11], buffer);
  fclose(disk);
  writeINode(nodeNumber, *node);
  free(buffer);
  free(node);
  return nodeNumber;
}
void addToDirectory(uint8_t nodeBlock, char *fileName, int fileType)
{
  int Blocks[9];
  for (int i = 0; i < 9; i++)
  {
    Blocks[i] = i + 10;
  }
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  readBlock(disk, Blocks[8], buffer);
  uint8_t x = 0;
  for (x = 0; x < 128; x++)
  {
    if (buffer[x] == 0)
    {
      buffer[x] = 1;
      break;
    }
  }
  if (x == 128)
  {
    printf("Cannot add file, no more room");
    return;
  }
  writeBlock(disk, Blocks[8], buffer);
  int blockNumber = x / 8;
  readBlock(disk, Blocks[blockNumber], buffer);
  memcpy(buffer + 32 * (x % 16) * sizeof(char), &nodeBlock, 1);
  memcpy(buffer + 32 * (x % 16) * sizeof(char) + sizeof(char), fileName, strlen(fileName));
  writeBlock(disk, Blocks[blockNumber], buffer);
  fclose(disk);
  free(buffer);
}

int getINodeFromDirectory(char *fileName);
void deleteFile(char *fileName);

void createFolder(char *name, char *content, int type, int length)
{
  int *value = (int *)malloc(sizeof(int));
  value[0] = findFreeBlock();
  if (value[0] == -1)
  {
    printf("Can't Create Folder, File System is out of memory \n");
    return;
  }
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)calloc(BLOCKSIZE, 1);
  writeBlock(disk, value[0], buffer);
  fclose(disk);
  free(buffer);
  markblockUsed(value[0]);
  uint16_t nodeNumber = createINode(1, type, value);
  addToDirectory(nodeNumber, name, 0);
}
void addFileToFolder(char *fileName, char *folderName)
{
  int number = getINodeFromDirectory(folderName);
  if (number == -1)
  {
    printf("Folder %s Was not found \n", folderName);
    return;
  }
  struct INode *node = readINode(number);

  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  readBlock(disk, node->blocksUsed[0], buffer);

  uint16_t storage = 0;
  printf("Adding to folder %s \n\n", folderName);

  for (uint16_t i = 1; i < 16; i++)
  {
    memcpy(&storage, buffer + i * sizeof(uint16_t), sizeof(uint16_t));
    if (storage == 0)
    {
      uint16_t x = 1;
      memcpy(buffer + i * sizeof(uint16_t), &x, sizeof(uint16_t));
      memcpy(buffer + i * 32 * sizeof(char), fileName, strlen(fileName));
      writeBlock(disk, node->blocksUsed[0], buffer);
      fclose(disk);
      free(buffer);
      return;
    }
  }
  printf("File Not added, no more room in %s", folderName);
  fclose(disk);
  free(buffer);
}
void deleteFileFromFolder(char *fileName, char *folderName)
{
  int number = getINodeFromDirectory(folderName);
  if (number == -1)
  {
    printf("Folder %s Was not found \n", folderName);
    return;
  }
  struct INode *node = readINode(number);
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  readBlock(disk, node->blocksUsed[0], buffer);
  uint16_t storage = 0;
  uint16_t x = 0;
  for (uint16_t i = 1; i < 16; i++)
  {
    memcpy(&storage, buffer + i * sizeof(uint16_t), sizeof(uint16_t));
    if (storage == 1)
    {
      if (strcmp(fileName, buffer + i * 32 * sizeof(char)) == 0)
      {
        memcpy(buffer + i * sizeof(uint16_t), &x, sizeof(uint16_t));
        writeBlock(disk, node->blocksUsed[0], buffer);
        printf("File %s deleted from directory %s \n", fileName, folderName);
        fclose(disk);
        free(buffer);
        return;
      }
    }
  }
  printf("File Not Found In DIrectory, %s not removed\n", folderName);
  fclose(disk);
  free(buffer);
}
void printFolderContents(char *fileName)
{
  int number = getINodeFromDirectory(fileName);
  if (number == -1)
  {
    printf("Folder %s Was not found \n", fileName);
    return;
  }
  struct INode *node = readINode(number);
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  readBlock(disk, node->blocksUsed[0], buffer);

  uint16_t blockNUM = 0;
  printf("Printing Directory for File %s \n", fileName);
  for (int i = 1; i < 16; i++)
  {
    memcpy(&blockNUM, buffer + i * sizeof(uint16_t), sizeof(uint16_t));
    if (blockNUM)
      printf("%s \n", (buffer + i * 32 * sizeof(char)));
  }
  fclose(disk);
  free(buffer);
}
int folderContains(char *fileName, char *folderName)
{
  int number = getINodeFromDirectory(folderName);
  if (number == -1)
  {
    printf("Folder %s Was not found \n", folderName);
    return 0;
  }
  struct INode *node = readINode(number);
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  readBlock(disk, node->blocksUsed[0], buffer);
  uint16_t storage = 0;
  for (uint16_t i = 1; i < 16; i++)
  {
    memcpy(&storage, buffer + i * sizeof(uint16_t), sizeof(uint16_t));
    if (storage == 1)
    {
      if (strcmp(fileName, buffer + i * 32 * sizeof(char)) == 0)
      {
        fclose(disk);
        free(buffer);
        printf("File %s was found in folder %s", fileName, folderName);
        return 1;
      }
    }
  }
  printf("File %s Not Found In Folder: %s \n", fileName, folderName);
  fclose(disk);
  free(buffer);
  return 0;
}
void deleteFolder(char *fileName)
{
  int number = getINodeFromDirectory(fileName);
  if (number == -1)
  {
    printf("Folder %s Was not found \n", fileName);
    return;
  }
  struct INode *node = readINode(number);
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  readBlock(disk, node->blocksUsed[0], buffer);
  uint16_t blockNUM = 0;
  printf("Printing Directory for File %s \n\n", fileName);
  for (int i = 1; i < 16; i++)
  {
    memcpy(&blockNUM, buffer + i * sizeof(uint16_t), sizeof(uint16_t));
    if (blockNUM)
    {
      fclose(disk);
      free(buffer);
      printf("Folder not Empty, File %s not deleted \n", fileName);
      return;
    }
  }
  fclose(disk);
  free(buffer);
  deleteFile(fileName);
}
int pathExists(char *input)
{
  char str[120];
  strcpy(str, input);
  const char s[2] = "/";
  char *token1;
  char *token2;
  token1 = strtok(str, s);
  token2 = strtok(NULL, s);

  char val1str[40];
  char val2str[40];

  while (token2 != NULL)
  {
    printf("%s/%s\n", token1, token2);
    strcpy(val1str, token1);
    strcpy(val2str, token2);
    if (!folderContains(val2str, val1str))
    {
      return 0;
    }
    token1 = token2;
    token2 = strtok(NULL, s);
  }
  return 1;
}
void createFile(char *name, char *content, int type, int length)
{
  int fileSize = strlen(content);
  int nameLength = strlen(name);
  if (!type)
  {
    createFolder(name, content, type, length);
    return;
  }
  if (fileSize > 266 * 512)
  {
    printf("Failed to create File: File is too large: %d\n length = %d bust must be less than %d\n",length,nameLength,266*512);
    return;
  }
  if (nameLength > 31)
  {
    printf("Failed to create File: File name is too long:\n length = %d bust must be less than 31\n", nameLength);
    return;
  }
  int size = 1 + fileSize / 512;
  int x[size];
  FILE *disk;
  char *buffer;

  for (int i = 0; i < size; i++)
  {
    x[i] = findFreeBlock();
    if (x[i] == -1)
    {
      printf("Can't Create File, File System is out of memory \n");
      free(buffer);
      return;
    }
    markblockUsed(x[i]);
    disk = fopen(FILEPATH, "rb+");
    buffer = (char *)calloc(BLOCKSIZE, sizeof(char));
    if (i == size - 1)
    {
      memcpy(buffer, (content + i * BLOCKSIZE), BLOCKSIZE - (BLOCKSIZE - length % BLOCKSIZE));
    }
    else
    {
      memcpy(buffer, (content + i * BLOCKSIZE), BLOCKSIZE);
    }
    writeBlock(disk, x[i], buffer);
    fclose(disk);
    free(buffer);
  }
  int INode = createINode(size, type, x);
  addToDirectory(INode, name, 1);
  printf("File %s added!\n", name);
  printf("File Length: %d\n", fileSize);
  printf("Blocks Used: %d\n", size);
  printf("INodeUsed %d\n\n", INode);
}
void deleteINode(int Number)
{
  struct INode *node = readINode(Number);
  int fileSize = node->fileSize;
  uint16_t refLocation = 0;
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  if (fileSize > 11)
  {
    char *refBuffer = (char *)malloc(BLOCKSIZE);
    for (int i = 0; i < ((node->fileSize) - 11); i++)
    {
      if (i == 0)
        readBlock(disk, node->blocksUsed[11], refBuffer);
      memcpy(&refLocation, refBuffer + (i) * sizeof(uint16_t), sizeof(uint16_t));
      fclose(disk);
      markblockFree(refLocation);
      disk = fopen(FILEPATH, "rb+");
    }
    free(refBuffer);
  }
  readBlock(disk, 0, buffer);
  char character = buffer[(Number / 8) + (sizeof(int) * 3)];
  character &= ~(1 << (Number % 8));
  buffer[(Number / 8) + (sizeof(int) * 3)] = (char)character;
  writeBlock(disk, 0, buffer);
  fclose(disk);
  free(buffer);
  for (int i = 0; (i < 11) && (i < fileSize); i++)
    markblockFree(node->blocksUsed[i]);
  free(node);
}
int getINodeFromDirectory(char *fileName)
{
  int Blocks[9];
  for (int i = 0; i < 9; i++)
    Blocks[i] = i + 10;
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  uint8_t x = 0;
  char *string = (char *)calloc(32 * sizeof(char), 1);
  for (x = 0; x < 128; x++)
  {
    readBlock(disk, Blocks[x / 16], buffer);
    //printf("%d:::%d      %s, %d\n",Blocks[x/16],x,fileName,(int)strlen(fileName));
    memcpy(string, buffer + (x % 16) * 32 * sizeof(char) + sizeof(char), strlen(fileName));
    if (strncmp(fileName, string, strlen(fileName)) == 0)
    {
      printf("File %s Found in Directry\n", string);
      fclose(disk);
      free(buffer);
      free(string);
      return x;
    };
  }
  printf("File %s Not Found in Directry\n", fileName);
  fclose(disk);
  free(buffer);
  free(string);
  return -1;
}
void deleteFile(char *fileName)
{
  int number = getINodeFromDirectory(fileName);
  if (number == -1)
    return;
  deleteINode(number);
  int Blocks[9];
  for (int i = 0; i < 9; i++)
    Blocks[i] = i + 10;
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  uint8_t x = 0;
  char *string = (char *)calloc(32 * sizeof(char), 1);
  for (x = 0; x < 128; x++)
  {
    readBlock(disk, Blocks[x / 16], buffer);
    memcpy(string, buffer + (x % 16) * 32 * sizeof(char) + sizeof(char), strlen(fileName));
    if (strncmp(fileName, string, strlen(fileName)) == 0)
    {
      char *replace = (char *)calloc(32 * sizeof(char), 1);
      memcpy(buffer + (x % 16) * 32 * sizeof(char), replace, 32 * sizeof(char));
      writeBlock(disk, Blocks[x / 16], buffer);
      free(replace);
      readBlock(disk, Blocks[8], buffer);
      buffer[x] = 0;
      writeBlock(disk, Blocks[8], buffer);
      free(buffer);
      free(string);
      fclose(disk);
      printf("Deleting File: %s\n", fileName);
      return;
    };
  }
  fclose(disk);
  free(buffer);
  free(string);
}
void printDebugDirectory()
{
  int Blocks[9];
  for (int i = 0; i < 9; i++)
    Blocks[i] = i + 10;
  FILE *disk = fopen(FILEPATH, "rb+");
  char *LastBlock = (char *)malloc(BLOCKSIZE);
  char *buffer = (char *)malloc(BLOCKSIZE);
  readBlock(disk, Blocks[8], LastBlock);
  printf("\nDirectory: \n");
  for (int x = 0; x < 9; x++)
  { //should be  < 128
    readBlock(disk, Blocks[x / 8], buffer);
    printf("Entry: %d    Used: %d    NodeNumber: %d   ContentName: %s \n", x, LastBlock[x], (buffer + 32 * (x % 16) * sizeof(char))[0], buffer + 32 * (x % 16) * sizeof(char) + sizeof(char));
  }
  printf("\n\n");
  fclose(disk);
  free(buffer);
  free(LastBlock);
}
void printDirectory()
{
  int Blocks[9];
  for (int i = 0; i < 9; i++)
    Blocks[i] = i + 10;
  FILE *disk = fopen(FILEPATH, "rb+");
  char *LastBlock = (char *)malloc(BLOCKSIZE);
  char *buffer = (char *)malloc(BLOCKSIZE);
  readBlock(disk, Blocks[8], LastBlock);
  printf("\nDirectory: \n");
  for (int x = 0; x < 128; x++)
  { //should be  < 128
    readBlock(disk, Blocks[x / 8], buffer);
    if (LastBlock[x])
      printf("%s \n", buffer + 32 * (x % 16) * sizeof(char) + sizeof(char));
  }
  printf("\n\n");
  fclose(disk);
  free(buffer);
  free(LastBlock);
}
void readFile(char *fileName)
{ 
  int x = getINodeFromDirectory(fileName);
  if (x == -1)
  {
    printf("File %s Not found/n", fileName);
    return;
  }
  struct INode *FileNode = readINode(x);
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  char *refBuffer = (char *)malloc(BLOCKSIZE);
  uint16_t refLocation = 0;
  printf("Printing File: \n %s %d \n", fileName, FileNode->fileSize);
  for (int i = 0; i < FileNode->fileSize; i++)
  {
    if (i < 11)
    {
      printf("%s", readBlock(disk, FileNode->blocksUsed[i], buffer));
    }
    else
    {
      if (i == 11)
        readBlock(disk, FileNode->blocksUsed[11], refBuffer);
      memcpy(&refLocation, refBuffer + (i - 11) * sizeof(uint16_t), sizeof(uint16_t));
      printf("%s", readBlock(disk, refLocation, buffer));
    }
  }
  printf("\n");
  fclose(disk);
  free(buffer);
  free(FileNode);
  free(refBuffer);
}
void printAtPath(char *input)
{
  if (!pathExists(input))
  {
    printf("File Not Found\n");
    return;
  }
  const char s[2] = "/";
  char *token1;
  char *token2;
  token1 = strtok(input, s);
  token2 = strtok(NULL, s);

  char val1str[40];
  char val2str[40];

  while (token2 != NULL)
  {
    printf("%s/%s\n", token1, token2);
    strcpy(val1str, token1);
    strcpy(val2str, token2);
    token1 = token2;
    token2 = strtok(NULL, s);
  }
  readFile(token2);
}
void createDirectory()
{
  int Blocks[9];
  for (int i = 0; i < 9; i++)
  {
    Blocks[i] = i + 10;
    markblockUsed(i + 10);
  }
  uint8_t INode = createINode(9, 0, Blocks);
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  readBlock(disk, Blocks[8], buffer);
  buffer[0] = 1;
  writeBlock(disk, Blocks[8], buffer);
  readBlock(disk, 10, buffer);
  memcpy(buffer, &INode, 1);
  memcpy(buffer + 1, "directory", strlen("directory"));
  writeBlock(disk, 10, buffer);
  fclose(disk);
  free(buffer);
}
void initBlock0()
{
  FILE *disk = fopen(FILEPATH, "rb+");
  char *buffer = (char *)malloc(BLOCKSIZE);
  int magicNum = 31415;
  int block = NUMBLOCKS;
  int inode = NUMINODES;
  memcpy(buffer, &magicNum, sizeof(int));
  memcpy(buffer + sizeof(int), &block, sizeof(int));
  memcpy(buffer + sizeof(int) * 2, &inode, sizeof(int));
  writeBlock(disk, 0, buffer);
  fclose(disk);
  free(buffer);
}
void formatDisk()
{
  FILE *disk = fopen(FILEPATH, "w");
  char *init = calloc((BLOCKSIZE * NUMBLOCKS), 1);
  fwrite(init, BLOCKSIZE * NUMBLOCKS, 1, disk);
  free(init);
  fclose(disk);
  for (int i = 0; i < 10; i++)
    markblockUsed(i);
  initBlock0();
  createDirectory();
  createFile("root", "", 0, 0);
}
void eatFile(char *fileName)
{
  FILE *disk = fopen(fileName, "rb+");
  fseek(disk, 0, SEEK_END);
  int fsize = ftell(disk);
  fseek(disk, 0, SEEK_SET); /* same as rewind(f); */
  char *string = malloc(fsize + 1);
  fread(string, 1, fsize, disk);
  fclose(disk);
  string[fsize] = '\0';
  createFile(fileName, string, 1, fsize + 1);
  free(string);
}