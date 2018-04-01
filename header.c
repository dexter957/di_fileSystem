#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


struct header
{
	int sizeInBlocks;	/*How many blocks .di file occupies*/
	int sizeInBytes;	/*The actual size of the .di file in bytes*/
	int dataStart;		/*The first block in the data section*/
	int directoriesStart;/*The last block in the data section*/
	int metadataStart;	/*The first block of metadata*/
	int dinodesNumber;	/*How many dinodes there are*/
	int numberOfFiles;	/*How many files are stored*/
};


int createHeader(pointerToHeader* theHeader)
{
	(*theHeader)=malloc(sizeof(struct header));
	if((*theHeader)==NULL)
	{
		printf("Cannot allocate memory for header\n");
		return FALSE;
	}
	/*Initialise the fields*/
	(*theHeader)->sizeInBlocks=0;
	(*theHeader)->sizeInBytes=0;
	(*theHeader)->dataStart=0;
	(*theHeader)->directoriesStart=0;
	(*theHeader)->metadataStart=0;
	(*theHeader)->dinodesNumber=0;
	(*theHeader)->numberOfFiles=0;
	return TRUE;
}

void setSizeInBlocks(pointerToHeader* theHeader,int sizeInBlocks)
{
	(*theHeader)->sizeInBlocks=sizeInBlocks;
	return;
}

void setSizeInBytes(pointerToHeader* theHeader,int sizeInBytes)
{
	(*theHeader)->sizeInBytes=sizeInBytes;
	return;
}

void setDataStart(pointerToHeader* theHeader,int dataStart)
{
	(*theHeader)->dataStart=dataStart;
	return;	
}

void setDirectoriesStart(pointerToHeader* theHeader,int directoriesStart)
{
	(*theHeader)->directoriesStart=directoriesStart;
	return;	
}

void setMetadataStart(pointerToHeader* theHeader,int metadataStart)
{
	(*theHeader)->metadataStart=metadataStart;
	return;	
}

void setDinodesNumber(pointerToHeader* theHeader,int dinodesNumber)
{
	(*theHeader)->dinodesNumber=dinodesNumber;
	return;	
}

void setNumberOfFiles(pointerToHeader* theHeader,int numberOfFiles)
{
	(*theHeader)->numberOfFiles=numberOfFiles;
	return;	
}

int getNumberOfFiles(pointerToHeader* theHeader)
{
	return (*theHeader)->numberOfFiles;
}

int getDinodesNumber(pointerToHeader* theHeader)
{
	return (*theHeader)->dinodesNumber;
}

int getMetadataStart(pointerToHeader* theHeader)
{
	return (*theHeader)->metadataStart;
}

int getDirectoriesStart(pointerToHeader* theHeader)
{
	return (*theHeader)->directoriesStart;
}

int getSizeInBlocks(pointerToHeader* theHeader)
{
	return (*theHeader)->sizeInBlocks;
}

int getSizeInBytes(pointerToHeader* theHeader)
{
	return (*theHeader)->sizeInBytes;
}

int getDataStart(pointerToHeader* theHeader)
{
	return (*theHeader)->dataStart;
}

void destroyHeader(pointerToHeader* theHeader)
{
	free((*theHeader));
	return;
}

long int getSizeOfStructHeader()
{
	return sizeof(struct header);
}


void printHeader(pointerToHeader theHeader)
{
	printf("sizeInBlocks %d\n", theHeader->sizeInBlocks);
	printf("sizeInBytes %d\n", theHeader->sizeInBytes);
	printf("dataStart %d\n", theHeader->dataStart );
	printf("directoriesStart %d\n", theHeader->directoriesStart );
	printf("metadataStart %d\n", theHeader->metadataStart );
	printf("dinodesNumber %d \n", theHeader->dinodesNumber);
	printf("numberOfFiles %d \n", theHeader->numberOfFiles);
}

/*Write the content of a header file to the .di file*/

int copyHeaderContents(pointerToHeader *theHeader, char* difileName)
{
	/*Open the di file*/
	int fd=open(difileName,O_WRONLY);
	if(fd<0)
	{/*Fail to open di file*/
		printf("Could not open file %s to write to it\n",difileName );
		return FALSE;
	}
	void* headerBuffer=malloc(BLOCK_SIZE);
	memmove(headerBuffer,(*theHeader),getSizeOfStructHeader());
	write(fd,headerBuffer,BLOCK_SIZE);
	close(fd);
	free(headerBuffer);
	return TRUE;
}




/*Retrieve header from .di file*/


int retrieveHeader(pointerToHeader *theHeader, char* difileName)
{
	/*We suppose that we have gotten as input argument a newly created header*/
	/*First open the .di file to read it*/
	int fdO=open(difileName,O_RDONLY);
	if(fdO<0)
	{
		printf("Could not open %s file to read\n",difileName );
		return FALSE;
	}
	read(fdO,(*theHeader),getSizeOfStructHeader());/*Read the header of the file*/
	/*Close the file*/
	close(fdO);
	return TRUE;
}