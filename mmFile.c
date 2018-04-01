#include "mmFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct file
{
	char* fileName;
	int fileDescriptor;
	int numberOfBlocks;
	int bytesUsed;
	int timesAskedForBlocks;
	void** blocksArray;
};


/*Basic functionality*/

int createFile(pointerToFile* theFile, char* fileName)
{
	(*theFile)=malloc(sizeof(struct file));/*Allocate space for a file*/
	if((*theFile)==NULL)
	{/*Could not allocate memory for file*/
		printf("Cannot allocate memory for file creation.\n");
		return FALSE;
	}
	/*Initialise the fileds*/
	(*theFile)->fileName=malloc((strlen(fileName)+1)*sizeof(char));
	strcpy((*theFile)->fileName,fileName);
	(*theFile)->fileDescriptor=0;
	(*theFile)->numberOfBlocks=0;
	(*theFile)->bytesUsed=0;
	(*theFile)->timesAskedForBlocks=0;
	(*theFile)->blocksArray=NULL;
	return TRUE;
}

int allocateFileBlocks(pointerToFile* theFile, int blocksNumber)
{
	(*theFile)->blocksArray=malloc(blocksNumber*sizeof(void*));/*Allocate space for the array of pointers*/
	if((*theFile)->blocksArray==NULL)
	{
		printf("Cannot allocate file blocks.\n");
		return FALSE;
	}
	int i=0;
	for(i=0;i<blocksNumber;++i)
	{
		(*theFile)->blocksArray[i]=malloc(BLOCK_SIZE);/*Allocate space for a block*/
		if((*theFile)->blocksArray[i]==NULL)
		{
			printf("Cannot allocate file blocks\n");
			return FALSE;
		}
	}
	(*theFile)->numberOfBlocks=blocksNumber;
	return TRUE;
}

void setBlockNumber(pointerToFile* theFile, int blocksNumber)
{
	(*theFile)->numberOfBlocks=blocksNumber;
}

int getBlockNumber(pointerToFile* theFile)
{
	return (*theFile)->numberOfBlocks;
}

char* getMMFileName(pointerToFile* theFile)
{
	return (*theFile)->fileName;
}

void destroyFile(pointerToFile *theFile)
{
	free((*theFile)->fileName);
	if((*theFile)->blocksArray!=NULL)
	{
		int i;
		for(i=0;i<(*theFile)->numberOfBlocks;++i)
		{
			free((*theFile)->blocksArray[i]);
		}
		free((*theFile)->blocksArray);
	}
	free((*theFile));
	return;
}

/*More functionality for a file*/

void* getFileBlock(pointerToFile* theFile)
{
	int block=(*theFile)->timesAskedForBlocks;
	(*theFile)->bytesUsed=(*theFile)->bytesUsed+BLOCK_SIZE;/*One more block has been used*/
	(*theFile)->timesAskedForBlocks=(*theFile)->timesAskedForBlocks+1;
	return (*theFile)->blocksArray[block];
}

void setBytesWrittenOnLastBlock(pointerToFile* theFile, int lastBytes)
{
	(*theFile)->bytesUsed=(*theFile)->bytesUsed+lastBytes;
}

void setFileSize(pointerToFile* theFile, int numOfBytes)
{
	(*theFile)->bytesUsed=numOfBytes;
}

void copyFileContent(pointerToFile* theFile, char* difileName, int offset)
{
	int fdC;/*File descriptor for the copy file*/
	if(offset==ARG_NOT_USED)
	{
		fdC=open(difileName,O_WRONLY|O_APPEND);
		if(fdC<0)
		{
			printf("Could not create copy file\n");
			return;
		}
	}
	else
	{
		fdC=open(difileName,O_WRONLY);
		if(fdC<0)
		{
			printf("Could not create copy file\n");
			return;
		}
		lseek(fdC,offset,SEEK_SET);
	}
	int i=0;
	for(i=0;i<(*theFile)->numberOfBlocks;++i)
	{
		write(fdC,(*theFile)->blocksArray[i],BLOCK_SIZE);
	}
	close(fdC);
}


/*Retrieve file contents from .di file*/
int retrieveFileContents(char* difileName, int offset, int blocksOccupied, int bytesUsed,char* fileName)
{
	/*First open the .di file to read*/
	int fdD=open(difileName,O_RDONLY);
	if(fdD<0)
	{
		printf("Cannot open %s to retrieve data.\n",difileName );
		return FALSE;
	}
	/*Move the fdD to the rigth position*/
	lseek(fdD,offset,SEEK_SET);
	/*Now open the file to be retrieved*/
	int fdR=open(fileName,O_WRONLY);
	if(fdR<0)
	{
		printf("Cannot open %s to write data\n",fileName );
		close(fdD);
		return FALSE;
	}
	/*Both files opened OK, now let's copy byte by byte*/
	int i;
	void *buffer=malloc(1);
	for(i=0;i<bytesUsed;++i)
	{
		read(fdD,buffer,1);
		write(fdR,buffer,1);
	}
	/*Now close all files*/
	close(fdR);
	close(fdD);
	/*Free the memory and leave*/
	free(buffer);
	return TRUE;
}




























