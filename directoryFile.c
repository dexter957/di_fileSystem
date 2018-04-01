#include "directoryFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/*
A directory is a special file that contains entries about its contents. Each entry consists of a specific set of information:
the name of a file inside the directory (that file may be a another directory or a simple file), the length of the name, and the inode
for that file.
*/


struct dFileEntry
{
	int inodeNumber;
	int fileNameLength;
	char* fileName;
};

struct dFile
{
	int numOfEntries;
	pointerToDFileEntry *entries;
	int numOfBlocks;
	void** blocksArray;
};


/*Functions for the directory entries in a directory file*/

int createDFileEntry(pointerToDFileEntry *theDFileEntry)
{
	(*theDFileEntry)=malloc(sizeof(struct dFileEntry));
	if((*theDFileEntry)==NULL)
	{
		printf("Cannot allocate space for the directory file\n");
		return FALSE;
	}
	(*theDFileEntry)->inodeNumber=0;
	(*theDFileEntry)->fileNameLength=0;
	(*theDFileEntry)->fileName=NULL;
	return TRUE;
}


void destroyDFileEntry(pointerToDFileEntry *theDFileEntry)
{
	if((*theDFileEntry)->fileName!=NULL)
	{
		free((*theDFileEntry)->fileName);
	}
	free((*theDFileEntry));
}

void setINodeNumber(pointerToDFileEntry *theDFileEntry, int inodeNumber)
{
	(*theDFileEntry)->inodeNumber=inodeNumber;
}

int setFileName(pointerToDFileEntry *theDFileEntry, char* fileName)
{
	(*theDFileEntry)->fileName=malloc((strlen(fileName)+1)*sizeof(char));
	if((*theDFileEntry)->fileName==NULL)
	{
		printf("Cannot allocate memory for filename\n");
		return FALSE;
	}
	strcpy((*theDFileEntry)->fileName,fileName);
	(*theDFileEntry)->fileNameLength=strlen(fileName)+1;/*Set the length of the name*/	return TRUE;
}

int getINodeNumber(pointerToDFileEntry *theDFileEntry)
{
	return (*theDFileEntry)->inodeNumber;
}

char* getFileName(pointerToDFileEntry *theDFileEntry)
{
	return (*theDFileEntry)->fileName;
}

int getFileNameLength(pointerToDFileEntry *theDFileEntry)
{
	return (*theDFileEntry)->fileNameLength;
}

long int getSizeOfStructDFileEntry()
{
	return sizeof(struct dFileEntry);
}

void printDFileEntry(pointerToDFileEntry theEntry)
{
	printf("inodeNumber:%d\n",theEntry->inodeNumber);
	printf("fileNameLength:%d\n",theEntry->fileNameLength );
	printf("fileName:%s\n",theEntry->fileName );
}


/*Functions for the directory files*/


int createDFile(pointerToDFile *theDFile)
{
	//printf("Creating file for pointer %p\n",theDFile);
	(*theDFile)=malloc(sizeof(struct dFile));
	if((*theDFile)==NULL)
	{
		printf("Cannot allocate space for the directory file\n");
		return FALSE;
	}
	(*theDFile)->numOfEntries=0;
	(*theDFile)->entries=NULL;
	(*theDFile)->numOfBlocks=0;
	(*theDFile)->blocksArray=NULL;
	return TRUE;
}

int allocateDFileEntries(pointerToDFile *theDFile, int numOfEntries)
{
	(*theDFile)->numOfEntries=numOfEntries+2;/*We add the entries for the current folder, and its parent*/
	(*theDFile)->entries=malloc(sizeof(pointerToDFileEntry)*(numOfEntries+2));/*Allocate space for the array of entries*/
	if((*theDFile)->entries==NULL)
	{
		printf("Cannot allocate space for directory entries\n");
		return FALSE;
	}
	int i;
	for(i=0;i<(numOfEntries+2);++i)
	{
		if(createDFileEntry(&((*theDFile)->entries[i]))==FALSE)
		{
			printf("Cannot allocate space for directory entries\n");
			return FALSE;
		}
	}
	return TRUE;
}

int addDFileEntry(pointerToDFile *theDFile, int n,int inodeNum, char* fileName)
{
//	printf("Came to add the entry\n");
	if(n>(*theDFile)->numOfEntries)
	{
		printf("Entries exceeded\n");
		return FALSE;
	}
	setINodeNumber((&((*theDFile)->entries[n])), inodeNum);
	setFileName((&((*theDFile)->entries[n])),fileName);
	return TRUE;
}


int getNumberOfDFileEntries(pointerToDFile *theDFile)
{
	return (*theDFile)->numOfEntries;
}

pointerToDFileEntry getDFileEntry_nth(pointerToDFile *theDFile, int n)
{/*Returns the n_th file entry*/
	if(n>(*theDFile)->numOfEntries)
	{
		printf("Exceeded entries\n");
		return NULL;
	}
	return (*theDFile)->entries[n];
}

long int getSizeOfStructDFile()
{
	return sizeof(struct dFile);
}

void printDFile(pointerToDFile theDFile)
{
	int i;
	for(i=0;i<theDFile->numOfEntries;++i)
	{
		printDFileEntry(theDFile->entries[i]);
	}
}


void destroyDFile(pointerToDFile *theDFile)
{
	int i;
	for(i=0;i<(*theDFile)->numOfEntries;++i)
	{
		free((*theDFile)->entries[i]);
	}
	free((*theDFile)->entries);
	if((*theDFile)->blocksArray!=NULL)
	{
		for(i=0;i<(*theDFile)->numOfBlocks;++i)
		{
			free((*theDFile)->blocksArray[i]);
		}
		free((*theDFile)->blocksArray);
	}
	free((*theDFile));
	return;
}

/*Functions for writing a directory file to a .di file*/

void copyDFileContent(pointerToDFile *theDFile, char* difileName, int* bytesWritten)
{
	int fdC;/*File descriptor for the copy file*/
	fdC=open(difileName,O_WRONLY|O_APPEND);
	if(fdC<0)
	{
		printf("Could not create copy file\n");
		return;
	}
	/*First allocate the blocks you need*/
	int i;
	long int bytes=0;
	int inodeNum;
	int lengthOfFileName;
	char* theFileName;
	for(i=0;i<(*theDFile)->numOfEntries;++i)
	{
		lengthOfFileName=getFileNameLength(&((*theDFile)->entries[i]));
		bytes+=2*sizeof(int)+lengthOfFileName;
	}
	(*bytesWritten)=bytes;
	(*theDFile)->numOfBlocks=bytes/BLOCK_SIZE;
	if(bytes%BLOCK_SIZE!=0)
	{
		(*theDFile)->numOfBlocks=(*theDFile)->numOfBlocks+1;
	}
	/*Now allocate the space for these blocks*/
	(*theDFile)->blocksArray=malloc(sizeof(void*)*((*theDFile)->numOfBlocks));
	for(i=0;i<((*theDFile)->numOfBlocks);++i)
	{
		(*theDFile)->blocksArray[i]=malloc(BLOCK_SIZE);
	}
	int j;
	int stopped=0;
	i=0;
	int spaceLeft=BLOCK_SIZE;
	void* whereToWrite;
	while(i<((*theDFile)->numOfBlocks))
	{
		whereToWrite=(*theDFile)->blocksArray[i];
		for(j=stopped;j<((*theDFile)->numOfEntries);++j)
		{
			inodeNum=getINodeNumber(&((*theDFile)->entries[j]));
			lengthOfFileName=getFileNameLength(&((*theDFile)->entries[j]));
			theFileName=getFileName(&((*theDFile)->entries[j]));
			if(spaceLeft>=(lengthOfFileName+2*((int)sizeof(int))))
			{/*There is enough space to write in the current block*/
				memmove(whereToWrite, &inodeNum,sizeof(int) );
				whereToWrite=whereToWrite+sizeof(int);
				memmove(whereToWrite,&lengthOfFileName,sizeof(int));
				whereToWrite=whereToWrite+sizeof(int);
				memmove(whereToWrite,theFileName,lengthOfFileName);
				whereToWrite=whereToWrite+lengthOfFileName;
				spaceLeft=spaceLeft-lengthOfFileName+(2*((int)sizeof(int)));
			}
			else
			{
				stopped=j;/*The entry you could not write here*/
				spaceLeft=BLOCK_SIZE;/*Now you are writing to a new block*/
				break;
			}
		}
		++i;
	}
	/*Everything has been written on the blocks, now let's copy the info to the .di file*/
	for(i=0;i<(*theDFile)->numOfBlocks;++i)
	{
		write(fdC,(*theDFile)->blocksArray[i],BLOCK_SIZE);
	}
	close(fdC);
	/*Free the blocks you have allocated*/
	for (i = 0; i < (*theDFile)->numOfBlocks; ++i)
	{
		free((*theDFile)->blocksArray[i]);
	}
	free((*theDFile)->blocksArray);
	(*theDFile)->blocksArray=NULL;
}


/*Function to retrieve a directory file*/


int retrieveDFile(pointerToDFile *theDFile, int offset, int sizeInBytes,char* difileName)
{
	int numOfEntries=0;
	/*First we need to find out how many entries we have*/
	int fd=open(difileName,O_RDONLY);
	if(fd<0)
	{
		printf("Cannot open %s file to read\n",difileName );
		return FALSE;
	}
	lseek(fd,offset,SEEK_SET);
	struct dFileEntry anEntry;
	int bytesRead=0;
	while(bytesRead<sizeInBytes)
	{
		read(fd,&anEntry.inodeNumber,sizeof(int));
		read(fd,&anEntry.fileNameLength,sizeof(int));
		anEntry.fileName=malloc(anEntry.fileNameLength*sizeof(char));
		read(fd,anEntry.fileName,anEntry.fileNameLength);
		free(anEntry.fileName);
		bytesRead+=2*sizeof(int)+anEntry.fileNameLength;
		++numOfEntries;
	}
	close(fd);
	/*Now that you know haw many entries you have, allocate space for them*/
	printDFile((*theDFile));
	if(allocateDFileEntries(theDFile, numOfEntries-2)==FALSE)
	{
		printf("Cannot allocate space for directory file\n");
		return FALSE;
	}
	fd=open(difileName,O_RDONLY);
	if(fd<0)
	{
		printf("Cannot open %s file to read\n",difileName );
		return FALSE;
	}
	lseek(fd,offset,SEEK_SET);
	/*Now get all entries*/
	int i;
	for(i=0;i<numOfEntries;++i)
	{
		read(fd,&anEntry.inodeNumber,sizeof(int));
		read(fd,&anEntry.fileNameLength,sizeof(int));
		anEntry.fileName=malloc(anEntry.fileNameLength*sizeof(char));
		read(fd,anEntry.fileName,anEntry.fileNameLength);
		setINodeNumber((&((*theDFile)->entries[i])), anEntry.inodeNumber);
		setFileName((&((*theDFile)->entries[i])), anEntry.fileName);
		free(anEntry.fileName);
	}
	/*Got all entries, now closing the file once and for all*/
	close(fd);
	return TRUE;
}


















