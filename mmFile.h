#include "diHeader.h"


typedef struct file *pointerToFile;

int createFile(pointerToFile* theFile, char* fileName);
int allocateFileBlocks(pointerToFile* theFile, int blocksNumber);
void setBlockNumber(pointerToFile* theFile, int blocksNumber);
int getBlockNumber(pointerToFile* theFile);
char* getMMFileName(pointerToFile* theFile);
void destroyFile(pointerToFile *theFile);
void* getFileBlock(pointerToFile* theFile);
void setBytesWrittenOnLastBlock(pointerToFile* theFile, int lastBytes);
void setFileSize(pointerToFile* theFile, int numOfBytes);
void copyFileContent(pointerToFile* theFile, char* difileName, int offset);
/*Retrieve file contents from .di file*/
int retrieveFileContents(char* difileName, int offset, int blocksOccupied, int bytesUsed,char* fileName);