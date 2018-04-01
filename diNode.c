#include "diNode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>



/*
We also need the methods that write these dinodes to the file and retrieve them
*/

struct dinode
{
	int dinodeNum;
	int owner;
	int group;
	int directory;
	int ownerAccessRights;
	int groupAccessRights;
	int universeAccessRights;
	int sizeInBytes;
	int blocksOccupying;
	time_t lastAccess;
	time_t lastModification;
	time_t lastStatusChange;
	int startingBlock;	/*The block where the file this dinode represents starts*/
};

struct dinodeFile
{
	int numOfDiNodes;
	int numOfBlocks;
	long int bytesUsed;
	void** blocksArray;
	/*These fields below are to facilitate the process of writing the dinodes to a file one by one*/
	int writingBlock;
	int spaceLeft;
	int dinodesWritten;
	void* whereToWrite;
};
/* ^^ This structure represents the file, purely as an mm entity, where we will write the dinodes*/


int createDinode(pointerToDinode* theDinode)
{
	(*theDinode)=malloc(sizeof(struct dinode));
	if((*theDinode)==NULL)
	{
		printf("Could not allocate memory to make the dinode\n");
		return FALSE;
	}
	(*theDinode)->dinodeNum=-1;
	(*theDinode)->owner=-1;
	(*theDinode)->group=-1;
	(*theDinode)->directory=FALSE;
	(*theDinode)->ownerAccessRights=0755;
	(*theDinode)->groupAccessRights=0755;
	(*theDinode)->universeAccessRights=0755;
	(*theDinode)->sizeInBytes=0;
	(*theDinode)->blocksOccupying=0;
	(*theDinode)->lastAccess=time(NULL);
	(*theDinode)->lastModification=time(NULL);
	(*theDinode)->lastStatusChange=time(NULL);
	return TRUE;
}

void setDinodeNum(pointerToDinode* theDinode, int num)
{
	(*theDinode)->dinodeNum=num;
}

void setOwner(pointerToDinode* theDinode, int owner)
{
	(*theDinode)->owner=owner;
	return;
}

void setGroup(pointerToDinode *theDinode, int group)
{
	(*theDinode)->group=group;
	return;
}

void setDirectory(pointerToDinode* theDinode, int yesORno)
{
	(*theDinode)->directory=yesORno;
	return;
}

void setOwnerAccessRights(pointerToDinode* theDinode, int accessRights)
{
	(*theDinode)->ownerAccessRights=accessRights;
	return;
}

void setGroupAccessRights(pointerToDinode* theDinode, int accessRights)
{
	(*theDinode)->groupAccessRights=accessRights;
	return;
}

void setUniverseAccessRights(pointerToDinode* theDinode, int accessRights)
{
	(*theDinode)->universeAccessRights=accessRights;
	return;
}

void setFileSizeInBytes(pointerToDinode* theDinode, long int sizeInBytes)
{
	(*theDinode)->sizeInBytes=sizeInBytes;
	return;
}

void setBlocksOccupying(pointerToDinode* theDinode, int blocksOccupying)
{
	(*theDinode)->blocksOccupying=blocksOccupying;
	return;
}

void setLastAccess(pointerToDinode* theDinode, time_t lastAccess)
{
	(*theDinode)->lastAccess=lastAccess;
	return;
}

void setLastModification(pointerToDinode* theDinode, time_t lastModification)
{
	(*theDinode)->lastModification=lastModification;
	return;
}

void setLastStatusChange(pointerToDinode* theDinode, time_t lastStatusChange)
{
	(*theDinode)->lastStatusChange=lastStatusChange;
	return;
}
void setStartingBlock(pointerToDinode* theDinode, int startingBlock)
{
	(*theDinode)->startingBlock=startingBlock;
	return;
}

int getDinodeNum(pointerToDinode *theDinode)
{
	return (*theDinode)->dinodeNum;
}

int getOwner(pointerToDinode* theDinode)
{
	return (*theDinode)->owner;
}

int getGroup(pointerToDinode *theDinode)
{
	return (*theDinode)->group;
}

int isDirectory(pointerToDinode* theDinode)
{
	return (*theDinode)->directory;
}

int getOwnerAccessRights(pointerToDinode* theDinode)
{
	return (*theDinode)->ownerAccessRights;
}

int getGroupAccessRights(pointerToDinode* theDinode)
{
	return (*theDinode)->groupAccessRights;
}

int getUniverseAccessRights(pointerToDinode* theDinode)
{
	return (*theDinode)->universeAccessRights;
}

long int getFileSizeInBytes(pointerToDinode *theDinode)
{
	return (*theDinode)->sizeInBytes;
}

int getBlocksOccupying(pointerToDinode *theDinode)
{
	return (*theDinode)->blocksOccupying;
}

int getStartingBlock(pointerToDinode *theDinode)
{
	return (*theDinode)->startingBlock;
}

time_t getLastAccess(pointerToDinode *theDinode)
{
	return (*theDinode)->lastAccess;
}

time_t getLastModification(pointerToDinode *theDinode)
{
	return (*theDinode)->lastModification;
}

time_t getLastStatusChange(pointerToDinode *theDinode)
{
	return (*theDinode)->lastStatusChange;
}

void destroyDinode(pointerToDinode* theDinode)
{
	free((*theDinode));
}

long int getSizeOfStructDinode()
{
	return sizeof(struct dinode);
}

void printDinode(pointerToDinode theDinode)
{
	printf("dinode:%d\n",theDinode->dinodeNum );
	printf("owner=%d\n",theDinode->owner );
	printf("group=%d\n",theDinode->group );
	printf("directory=%d\n",theDinode->directory );
	printf("ownerAccessRights=%o\n",theDinode->ownerAccessRights );
	printf("groupAccessRights=%o\n",theDinode->groupAccessRights );
	printf("universeAccessRights=%o\n",theDinode->universeAccessRights );
	printf("sizeInBytes=%d\n",theDinode->sizeInBytes );
	printf("blocksOccupying=%d\n",theDinode->blocksOccupying );
	printf("startingBlock=%d\n",theDinode->startingBlock );
	printf("lastAccess=%s\n",ctime(&(theDinode->lastAccess)) );
	printf("lastModification=%s\n",ctime(&(theDinode->lastModification)));
	printf("lastStatusChange=%s\n", ctime(&(theDinode->lastStatusChange)));
}

int compareDinodeNum(pointerToDinode theDinode, int num)
{
	if(theDinode->dinodeNum == num) return TRUE;
	else return FALSE;
}

/*We also need functions to store and retrieve inodes from the file*/

int createDinodeFile(pointerToDinodeFile *theDinodeFile)
{
	(*theDinodeFile)=malloc(sizeof(struct dinodeFile));
	if((*theDinodeFile)==NULL)
	{
		printf("Cannot allocate space to create an mm dinode file\n");
		return FALSE;
	}
	(*theDinodeFile)->numOfBlocks=0;
	(*theDinodeFile)->bytesUsed=0;
	(*theDinodeFile)->numOfDiNodes=0;
	(*theDinodeFile)->blocksArray=NULL;
	(*theDinodeFile)->writingBlock=0;
	(*theDinodeFile)->dinodesWritten=0;
	(*theDinodeFile)->spaceLeft=BLOCK_SIZE;
	(*theDinodeFile)->whereToWrite=NULL;
	return TRUE;
}


int allocateBlocksForDinodeFile(pointerToDinodeFile *theDinodeFile, int numOfDiNodes)
{
	/*All dinodes have the same size, so all we need is their number in total*/
	(*theDinodeFile)->numOfDiNodes=numOfDiNodes;
	(*theDinodeFile)->bytesUsed=numOfDiNodes*(getSizeOfStructDinode());
	(*theDinodeFile)->numOfBlocks=(((*theDinodeFile)->bytesUsed)/BLOCK_SIZE);
	if((((*theDinodeFile)->bytesUsed)%BLOCK_SIZE)!=0)
	{
		(*theDinodeFile)->numOfBlocks=(*theDinodeFile)->numOfBlocks+1;
	}
	(*theDinodeFile)->blocksArray=malloc(((*theDinodeFile)->numOfBlocks)*sizeof(void*));
	if((*theDinodeFile)->blocksArray==NULL)
	{
		printf("Cannot allocate blocks for dinodes\n");
		return FALSE;
	}
	int i;
	for(i=0;i<(*theDinodeFile)->numOfBlocks;++i)
	{
		(*theDinodeFile)->blocksArray[i]=malloc(BLOCK_SIZE);
		if((*theDinodeFile)->blocksArray[i]==NULL)
		{
			printf("Cannot allocate blocks for dinodes\n");
			return FALSE;
		}
	}
	/*Allocated space for the blocks array, return*/
	(*theDinodeFile)->whereToWrite=(*theDinodeFile)->blocksArray[0];/*Prepare for when you are going to write*/
	return TRUE;
}

void copyDinodeToDinodeFile(pointerToDinodeFile *theDinodeFile, pointerToDinode *theDinode)
{
	/*First check if you have enough space in the block you are writing*/
	long int sizeofDinode=getSizeOfStructDinode();
	if((*theDinodeFile)->spaceLeft<sizeofDinode)
	{/*There is not enough space left in the block we are currently writing*/
		(*theDinodeFile)->writingBlock=(*theDinodeFile)->writingBlock+1;
		(*theDinodeFile)->whereToWrite=(*theDinodeFile)->blocksArray[((*theDinodeFile)->writingBlock)];
		(*theDinodeFile)->spaceLeft=BLOCK_SIZE;
	}
	memmove((*theDinodeFile)->whereToWrite,(*theDinode),sizeofDinode);
	(*theDinodeFile)->whereToWrite=(*theDinodeFile)->whereToWrite+sizeofDinode;
	(*theDinodeFile)->spaceLeft=(*theDinodeFile)->spaceLeft-sizeofDinode;
	(*theDinodeFile)->dinodesWritten=(*theDinodeFile)->dinodesWritten+1;
	return;
}

int copyDinodesContent(pointerToDinodeFile *theDinodeFile, char* difileName, int offset)
{
	/*This function will copy the contents of the dinodes blocks to the .di file*/
	int fdO;
	if(offset==ARG_NOT_USED)
	{/*We will append the file*/
		fdO=open(difileName,O_WRONLY|O_APPEND);
		if(fdO<0)
		{
			printf("Cannot open %s file to write\n",difileName );
			return FALSE;
		}
	}
	else
	{
		fdO=open(difileName,O_WRONLY);
		if(fdO<0)
		{
			printf("Cannot open %s file to write\n",difileName );
			return FALSE;
		}
		lseek(fdO,offset,SEEK_SET);
	}
	int i;
	for(i=0;i<(*theDinodeFile)->numOfBlocks;++i)
	{
		write(fdO,(*theDinodeFile)->blocksArray[i],BLOCK_SIZE);
	}
	close(fdO);
	return TRUE;
}


void destroyDinodeFile(pointerToDinodeFile *theDinodeFile)
{
	int i;
	for(i=0;i<(*theDinodeFile)->numOfBlocks;++i)
	{
		free((*theDinodeFile)->blocksArray[i]);
	}
	free((*theDinodeFile)->blocksArray);
	free((*theDinodeFile));
	return;
}

long int getSizeOfStructDinodeFile()
{
	return sizeof(struct dinodeFile);
}


int retrieveDinode(pointerToDinode *theDinode, int offset, char* difileName)
{
	int fd=open(difileName,O_RDONLY);
	if(fd<0)
	{
		printf("Cannot open %s to read it\n",difileName );
		return FALSE;
	}
	lseek(fd,offset,SEEK_SET);
	read(fd,(*theDinode),getSizeOfStructDinode());
	close(fd);
	return TRUE;
}