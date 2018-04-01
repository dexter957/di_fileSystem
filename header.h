#include "diHeader.h"

typedef struct header *pointerToHeader;

int createHeader(pointerToHeader* theHeader);
void setSizeInBlocks(pointerToHeader* theHeader,int sizeInBlocks);
void setSizeInBytes(pointerToHeader* theHeader,int sizeInBytes);
void setDataStart(pointerToHeader* theHeader,int dataStart);
void setDirectoriesStart(pointerToHeader* theHeader,int dataEnd);
void setMetadataStart(pointerToHeader* theHeader,int metadataStart);
void setDinodesNumber(pointerToHeader* theHeader,int dinodesNumber);
void setNumberOfFiles(pointerToHeader* theHeader,int numberOfFiles);
int getNumberOfFiles(pointerToHeader* theHeader);
int getDinodesNumber(pointerToHeader* theHeader);
int getMetadataStart(pointerToHeader* theHeader);
int getDirectoriesStar(pointerToHeader* theHeader);
int getSizeInBlocks(pointerToHeader* theHeader);
int getSizeInBytes(pointerToHeader* theHeader);
int getDataStart(pointerToHeader* theHeader);
void destroyHeader(pointerToHeader* theHeader);
long int getSizeOfStructHeader();
void printHeader(pointerToHeader theHeader);

/*Write the content of a header file to the .di file*/
int copyHeaderContents(pointerToHeader *theHeader, char* difileName);

/*Retrieve header from .di file*/
int retrieveHeader(pointerToHeader *theHeader, char* difileName);