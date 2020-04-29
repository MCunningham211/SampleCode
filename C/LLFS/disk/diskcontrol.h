#ifndef diskcontrol_H
#define diskcontrol_H

#include <stdio.h>
#include <stdlib.h>

void writeBlock(FILE* disk, int blockNum, char* data);
char* readBlock(FILE* disk, int blockNUM, char* buffer);

#endif