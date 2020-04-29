#include <stdio.h>
#include <stdlib.h>
//Copied from the tutorial 9 code
const int BLOCK_SIZE = 512;

void writeBlock(FILE* disk, int blockNum, char* data){
	fseek(disk, BLOCK_SIZE*blockNum, SEEK_SET);
	fwrite(data, BLOCK_SIZE, 1, disk);
}
char* readBlock(FILE* disk, int blockNUM, char* buffer){
	fseek(disk, blockNUM * BLOCK_SIZE, SEEK_SET);
	fread(buffer, BLOCK_SIZE, 1, disk);
	return buffer;
}