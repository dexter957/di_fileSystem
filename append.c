#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include "list.h"
#include "stattools.h"
#include "diNode.h"
#include "directoryFile.h"
#include "mmFile.h"
#include "header.h"

#define FILEPATH "./"
#define COMPR_SUFF ".gz"

int retrieveDinodes(listPointer* dinodes, listPointer* directories, char* difile, long int offset, int numOfDinodes);
int dinodesForFirstLevelFile(listPointer *dinodes, char* fileName);
int createHierachy(listPointer* list, char* directoryName, char* initialDir);
int retrieveDirectories(listPointer* directories, listPointer diNodeList, pointerToDinode dinode, char* diFile);
int updateRoot(listPointer *directories, listPointer* dinodes, char* difile, char** firstLevelContents, int arrayLength);
int createDirectoryEntries(listPointer* directories, listPointer dinodes, char* directoryName);
int getNumOfEntriesInDirectory(char* directoryName);
void setParentAndCurrent(listPointer *dinodes, listPointer* directories,char* path, int currentDir, int parentInode);
int copyFiles(listPointer* dinodes, char* difile, int *blocksInFile, int* bytesInFile, int offset, int *extraBlocks);
int createMMFile(pointerToFile* theFile, char* fileName, long int fileSize, int* blocksUsed);
int setDifileHeader(pointerToHeader* theHeader,char* difile, int directoriesStart, int metaDataStart, int bytesInFile, int blocksInFile, int dinodesNumber);
int copyDirectories(listPointer directories, listPointer* dinodes,int* blocksInFile, char* difile, int* bytesInFile);
int copyDinodes(listPointer dinodes, int* blocksInFile, char* difile, int* bytesInFile, int extraBlocks);


int main(int argc, char* argv[]){
	
	int i;
	char* difile=argv[0];
	int compressionFlag=1;
	int filesNum=argc-2;
	int argsStart=2;
	int otherInput=2;
	char** flatFiles=malloc(filesNum*sizeof(char*));

	if(strcmp(argv[compressionFlag],"1")==0)
	{
		/*Now, for the flat files, add the suffix .gz*/
		for(i=argsStart;i<argc;++i)
		{
			char* realName=malloc((strlen(argv[i])+strlen(FILEPATH)+1)*sizeof(char));
			strcpy(realName,FILEPATH);
			strcat(realName,argv[i]);
			if(isDir(realName)==FALSE)
			{
				flatFiles[i-otherInput]=malloc((strlen(argv[i])+strlen(COMPR_SUFF)+1)*sizeof(char));
				strcpy(flatFiles[i-otherInput],argv[i]);
				strcat(flatFiles[i-otherInput],COMPR_SUFF);
			}
			else
			{
				flatFiles[i-otherInput]=malloc((strlen(argv[i])+1)*sizeof(char));
				strcpy(flatFiles[i-otherInput],argv[i]);
			}
			free(realName);
		}

		int pid=fork();
		if(pid==0)
		{
			execv("./compress",&argv[argsStart]);
		}
		else
		{
			int status;
			wait(&status);
		}
	}
	else
	{
		for(i=argsStart;i<argc;++i)
		{
			flatFiles[i-otherInput]=malloc((strlen(argv[i])+1)*sizeof(char));
			strcpy(flatFiles[i-otherInput],argv[i]);
		}
	}
	/*First we need to retrieve the dinodes already written in the .di file*/
	listPointer dinodesList=initList(); 			/*Pointer to a dinodes list*/
	listPointer directoriesList=initList();			/*Pointer to directories list*/
	pointerToHeader theHeader;
	if(createHeader(&theHeader)==FALSE)
	{
		printf("Could not create header.\nExiting . . .\n");
		exit(1);
	}
	/*Header created ok*/
	if(retrieveHeader(&theHeader, difile)==FALSE)
	{
		printf("Could not retrieve header\n");
		exit(1);
	}
	long int metaDataOffset=getMetadataStart(&theHeader)*BLOCK_SIZE;
	long int directoriesOffset=getDirectoriesStart(&theHeader)*BLOCK_SIZE;
	int numberOfDinodes=getDinodesNumber(&theHeader);
	/*These are some variables we will need later*/
	int blocksInFile=1; /*Don't forget the header*/
	long int bytesInFile=0;
	int newDirectoriesStart=0;
	int newMetadataStart=0;
	int extraBlocks=0; /*We will use this number to shift starting blocks*/
	/*Now go to that region and start retrieving*/
	retrieveDinodes(&dinodesList, &directoriesList, difile , metaDataOffset,numberOfDinodes);
	/*Now traverse the dinodes list, and get the blocks written by the files and the size they occupy*/
	for(i=0;i<numberOfDinodes;++i)
	{
		pointerToDinode aDinode=getNthValue(dinodesList,i);
		if(isDirectory(&aDinode)==TRUE)
		{
			continue;
		}
		blocksInFile+=getBlocksOccupying(&aDinode);
		bytesInFile+=getFileSizeInBytes(&aDinode);
		/*We want to get the bytes and blocks of data, when it is about files*/
	}

	/*Now print the retrieved list to make sure everything is ok*/
	/*Now we need to process the input we were given*/
	for(i=0;i<filesNum;++i)
	{/*Now break down the hierarchy of every directory in you arguments*/
		char* currentFile=malloc((strlen(flatFiles[i])+strlen(FILEPATH)+1)*sizeof(char));
		strcpy(currentFile,FILEPATH);
		strcat(currentFile,flatFiles[i]);
		if(isDir(currentFile)==TRUE)
		{
			createHierachy(&dinodesList,flatFiles[i],flatFiles[i]);
		}
		else
		{
			dinodesForFirstLevelFile(&dinodesList, flatFiles[i]);
		}
		free(currentFile);
	}
	for(i=0;i<filesNum;++i)
	{/*Now break down the hierarchy of every directory in you arguments*/
		char* currentFile=malloc((strlen(flatFiles[i])+strlen(FILEPATH)+1)*sizeof(char));
		strcpy(currentFile,FILEPATH);
		strcat(currentFile,flatFiles[i]);
		if(isDir(currentFile)==TRUE)
		{
			createDirectoryEntries(&directoriesList, dinodesList, flatFiles[i]);
		}
		free(currentFile);
	}
	/*Created dinodes for the new entries, and added them to the dinodes list*/
	/*Now we need to update the root directory*/
	updateRoot(&directoriesList, &dinodesList, difile, flatFiles,filesNum);
	setParentAndCurrent(&dinodesList, &directoriesList,"", 0, 0);
	/*Now we need to copy the files*/
	/*We will write the files where the directories used to start*/
	long int appendStart=directoriesOffset;
	copyFiles(&dinodesList, difile, &blocksInFile, &bytesInFile, appendStart, &extraBlocks);
	newDirectoriesStart=blocksInFile;
	copyDirectories(directoriesList, &dinodesList ,&blocksInFile, difile, &bytesInFile);
	newMetadataStart=blocksInFile;
	copyDinodes(dinodesList, &blocksInFile, difile, &bytesInFile, extraBlocks);
	setDifileHeader(&theHeader, difile, newDirectoriesStart, newMetadataStart,bytesInFile, blocksInFile, listLength(dinodesList));
	/*Release your resources and exit*/
	freeFunction freer = &destroyDinode;
	deleteAllAndFree(&dinodesList, freer);
	freer=&destroyDFile;
	//printDirectoriesList(directoriesList);
	deleteAllAndFree(&directoriesList,freer);
	destroyHeader(&theHeader);
	for (i=0;i<filesNum;++i)
	{
		free(flatFiles[i]);
	}
	free(flatFiles);
}


int retrieveDinodes(listPointer* dinodes, listPointer* directories, char* difile, long int offset, int numOfDinodes)
{ 
	long int bytesRead=0;
	int i;
	pointerToDinode aDinode;
	for(i=0;i<numOfDinodes;++i)
	{
		if(createDinode(&aDinode)==FALSE)
		{
			printf("Cannot initialise the dinode1\nExiting\n");
			return FALSE;
		}
		retrieveDinode( &aDinode, offset ,difile);
		//printDinode(aDinode);
		/*Retrieved the dinode, now add it to the list*/
		if(i==0)
		{
			insert(dinodes, (void*)aDinode, "root", "root");/*Insert the root dinode*/
		}
		else
		{
			insert(dinodes, (void*)aDinode, NULL, NULL);/*Insert the dinode in the list*/
		}
		offset+=getSizeOfStructDinode();/*Offset for the next dinode*/
		bytesRead+=getSizeOfStructDinode();
		if((BLOCK_SIZE-bytesRead)<getSizeOfStructDinode())
		{/*There is no room for another dinode in the current block we are reading*/
			offset+=(BLOCK_SIZE-bytesRead);/*Move on to the next block*/
			bytesRead=0;
		}
		if(isDirectory(&aDinode)==TRUE)
		{/*If the dinode represents a directory, retrieve it as well*/
			retrieveDirectories(directories, (*dinodes), aDinode, difile);
		}
	}
	return TRUE;
}

int dinodesForFirstLevelFile(listPointer *dinodes, char* fileName)
{
	char* realFileName=malloc((strlen(fileName)+strlen(FILEPATH)+1)*sizeof(char));
	strcpy(realFileName,FILEPATH);
	strcat(realFileName,fileName);
	/*Now stat the file*/
	pointerToDinode aDinode;
	createDinode(&aDinode);
	setDinodeNum(&aDinode, listLength(*dinodes));
	setOwner(&aDinode, getUserId(realFileName));
	setDirectory(&aDinode,isDir(realFileName) );
	setOwnerAccessRights(&aDinode, getUserRights(realFileName));
	setGroupAccessRights(&aDinode, getGroupRights(realFileName));
	setUniverseAccessRights(&aDinode, getOtherRights(realFileName));
	setFileSizeInBytes(&aDinode, getSize(realFileName));
	setLastAccess(&aDinode, (time_t)getAccessTime(realFileName));
	setLastModification(&aDinode, (time_t)getModificationTime(realFileName));
	setLastStatusChange(&aDinode, (time_t)getStatusChangeTime(realFileName));
	insert(dinodes, (void*)aDinode, fileName, realFileName);
	free(realFileName);
	return TRUE;
}

int createHierachy(listPointer* list, char* directoryName, char* initialDir)
{
	/*
		This function breaks down the hierarchy of a given directory, and creates the dinodes for said directory, as well as all its contents,
		in a recursive manner.
		Apply this function for every directory in the argument list.
	*/
	char* file;
	pointerToDinode aDinode;
	DIR *dp;
	struct dirent *ep;
	dp=opendir(directoryName);
	if(dp!=NULL)
	{
		while((ep=readdir(dp))!=NULL)
		{
			if((strcmp(ep->d_name,".")==0)||(strcmp(ep->d_name,"..")==0))
			{
				continue;
			}
			/*Now create a dinode for this file/directory*/
			file=malloc((strlen(directoryName)+strlen("/")+strlen(ep->d_name)+1)*sizeof(char));
			strcpy(file,directoryName);
			strcat(file,"/");
			strcat(file,ep->d_name);
			createDinode(&aDinode);
			setDinodeNum(&aDinode, listLength(*list));
			setOwner(&aDinode, getUserId(file));
	 		setDirectory(&aDinode,isDir(file) );
	 		setOwnerAccessRights(&aDinode, getUserRights(file));
	 		setGroupAccessRights(&aDinode, getGroupRights(file));
	 		setUniverseAccessRights(&aDinode, getOtherRights(file));
	 		setFileSizeInBytes(&aDinode, getSize(file));
	 		setLastAccess(&aDinode, (time_t)getAccessTime(file));
	 		setLastModification(&aDinode, (time_t)getModificationTime(file));
	 		setLastStatusChange(&aDinode, (time_t)getStatusChangeTime(file));
	 		insert(list, (void*)aDinode, ep->d_name, file);
	 		if(isDir(file)==TRUE)
	 		{
	 			createHierachy(list,file,initialDir);
	 		}
	 		//++dinodes;
	 		free(file);
		}
		closedir(dp);
	}
	else
	{
		;
	}
	/*First get the full path to stat it*/
	if(strcmp(directoryName,initialDir)==0)
	{
		file=malloc((strlen(FILEPATH)+strlen(directoryName)+1)*sizeof(char));
		strcpy(file,FILEPATH);
		strcat(file,directoryName);
		createDinode(&aDinode);
		setDinodeNum(&aDinode, listLength(*list));
		setOwner(&aDinode, getUserId(file));
		setDirectory(&aDinode,isDir(file) );
		setOwnerAccessRights(&aDinode, getUserRights(file));
		setGroupAccessRights(&aDinode, getGroupRights(file));
		setUniverseAccessRights(&aDinode, getOtherRights(file));
		setFileSizeInBytes(&aDinode, getSize(file));
		setLastAccess(&aDinode, (time_t)getAccessTime(file));
		setLastModification(&aDinode, (time_t)getModificationTime(file));
		setLastStatusChange(&aDinode, (time_t)getStatusChangeTime(file));
		insert(list, (void*)aDinode, directoryName, file);
		free(file);
	}
	return TRUE;
}


int retrieveDirectories(listPointer* directories, listPointer diNodeList, pointerToDinode dinode, char* diFile)
{
	long int dirContentStartBlock, dirContentByteSize;
	pointerToDFile directory;
	pointerToDFileEntry entry;
	comparisonFunction searchByDiNodeNum = &compareDinodeNum;
	listPointer found = initList();
	dirContentStartBlock = getStartingBlock(&dinode);
	dirContentByteSize = getFileSizeInBytes(&dinode);

	if(!createDFile(&directory))
	{
		printf("Directory File Entry Creation Error\n");
		return FALSE;
	}
	if(!retrieveDFile(&directory, dirContentStartBlock*BLOCK_SIZE, dirContentByteSize, diFile))
	{
		printf("Could not retrieve Directory File from diFile\n");
		return FALSE;
	}
	//printDFile(directory);
	int directoryEntries = getNumberOfDFileEntries(&directory);
	int j;
	for(j = 0; j < directoryEntries; j++)
	{
		if((entry = getDFileEntry_nth(&directory, j)) == NULL)
		{
			printf("Could not retrieve Directory File Entry from diFile\n");
			return FALSE;
		}
	}
	/*Now add the directory file into the list*/
	insert(directories, (void*)directory, NULL, NULL);
	return TRUE;
}

int updateRoot(listPointer *directories, listPointer* dinodes, char* difile, char** firstLevelContents, int arrayLength)
{
	/*We need to add the new files to the root directory, as entries*/
	/*First get the existent number of entries*/
	pointerToDinode rootNode=(pointerToDinode)getNthValue((*dinodes),0);
	pointerToDFile rootDir=(pointerToDFile)getNthValue((*directories), 0);
	int oldNumOfEntries=getNumberOfDFileEntries(&rootDir);
	int newNumOfEntries=oldNumOfEntries+arrayLength;
	pointerToDFile newRoot; /*Our new root directory*/
	if(createDFile(&newRoot)==FALSE)
	{
		printf("Cannot create new root directory, returning\n");
		return FALSE;
	}
	if(allocateDFileEntries(&newRoot, newNumOfEntries-2)==FALSE)
	{
		printf("Cannot allocate space for new root entries\n");
		return FALSE;
	}
	/*Now copy the old root entries*/
	int i;
	pointerToDFileEntry anEntry;
	for(i=0;i<oldNumOfEntries;++i)
	{
		anEntry=getDFileEntry_nth(&rootDir, i);
		addDFileEntry(&newRoot, i,getINodeNumber(&anEntry), getFileName(&anEntry));
	}
	//printDFile(newRoot);
	/*Now we need to add the new entries*/
	char* realName;
	long int entriesSize=0;
	int j;
	for(j=0;j<arrayLength;++j)
	{/*Get the dinode for each of first level entries*/
		realName=malloc((strlen(FILEPATH)+strlen(firstLevelContents[j])+1)*sizeof(char));
		strcpy(realName,FILEPATH);
		strcat(realName,firstLevelContents[j]);
		pointerToDinode aDinode=searchByRealName((*dinodes), realName);/*Get the dinode for the first level file i*/
		if(aDinode!=NULL)
		{/*Now create the directory entry*/
			int inode=getDinodeNum(&aDinode);
			addDFileEntry(&newRoot, i, inode, firstLevelContents[j]);
			entriesSize+=2*sizeof(int)+strlen(firstLevelContents[j]);
			++i;
		}
		free(realName);
	}
	//printDFile(newRoot);
	/*Insert the entriesSize*/
	entriesSize+=getFileSizeInBytes(&rootNode);
	setFileSizeInBytes(&rootNode, entriesSize);
	/*Now you need to insert the directory node to the top of the list of directories*/
	freeFunction freer=&destroyDFile;
	deleteAndFreeTheFirst(directories, freer);
	//printDirectoriesList(*directories);
	insertAtStart(directories, (void*)newRoot, "root", "root");
	//printDirectoriesList(*directories);
	return TRUE;
}


int createDirectoryEntries(listPointer* directories, listPointer dinodes, char* directoryName)
{
	DIR *dp;
	char* file;
	struct dirent *ep;
	dp=opendir(directoryName);
	pointerToDFile aDfile;
	int myNumOfEntries=0;
	int entries=0;
	long int entriesSize=0;
	if(dp!=NULL)
	{
		/*Create a new directory entry*/
		createDFile(&aDfile);
		myNumOfEntries=getNumOfEntriesInDirectory(directoryName);
		allocateDFileEntries(&aDfile, myNumOfEntries);/*Allocate space for the directory entries*/
		while((ep=readdir(dp))!=NULL)
		{
			if((strcmp(ep->d_name,".")==0)||(strcmp(ep->d_name,"..")==0))
			{
				continue;
			}
			/*Now create a dinode for this file/directory*/
			file=malloc((strlen(directoryName)+strlen("/")+strlen(ep->d_name)+1)*sizeof(char));
			strcpy(file,directoryName);
			strcat(file,"/");
			strcat(file,ep->d_name);
			/*Now search for its inode number*/
			pointerToDinode aDinode=searchByRealName(dinodes, file);
			if(aDinode!=NULL)
			{/*Now create the directory entry*/
				int inode=getDinodeNum(&aDinode);
				addDFileEntry(&aDfile, entries, inode, ep->d_name);
				++entries;
				entriesSize+=2*sizeof(int)+strlen(ep->d_name)+1;
			}
			if(isDir(file)==TRUE)
	 		{
	 			createDirectoryEntries(directories, dinodes, file);
	 		}
	 		free(file);
		}
		insert(directories, (void*)aDfile, directoryName, directoryName);
		(void)closedir(dp);
	}
	else
	{
		;
	}
}


int getNumOfEntriesInDirectory(char* directoryName)
{/*Returns the number of entries (files, directories) in a directory*/
	int entries=0;
	DIR *dp;
	struct dirent *ep;
	dp=opendir(directoryName);
	if(dp!=NULL)
	{
		while((ep=readdir(dp))!=NULL)
		{
			if((strcmp(ep->d_name,".")==0)||(strcmp(ep->d_name,"..")==0))
			{
				continue;
			}
			++entries;
		}
		(void)closedir(dp);
	}
	else
	{
		printf("Could not open directory %s\n",directoryName );
	}
	return entries;
}

void setParentAndCurrent(listPointer *dinodes, listPointer* directories,char* path, int currentDir, int parentInode)
{/*Starting from the root, sets the parent and current entry in every directory*/
	/*The path when called for root is ""*/
	int dinodesNum=listLength((*dinodes));
	int directoriesNum=listLength((*directories));
	pointerToDFile parentDir;
	pointerToDFile childDir;
	pointerToDinode aDinode;
	pointerToDFileEntry anEntry;
	int entriesSize=0;
	char* currentDirRealName;
	if(currentDir==0)
	{
		currentDirRealName=malloc((strlen("root")+1)*sizeof(char));
		strcpy(currentDirRealName,"root");
	}
	else
	{
		/*The current path has a name like ""/""/ . We do not need the last slash, so we do not copy it*/
		currentDirRealName=malloc(strlen(path)*sizeof(char));
		snprintf(currentDirRealName,(strlen(path)),"%s",path);
	}
	/*First get the root directory*/
	parentDir=getNthValue((*directories),currentDir);
	/*Now get its entries*/
	int i;
	int entries=getNumberOfDFileEntries(&parentDir);
	int entryInode;
	char* entryName;
	for(i=0;i<entries;++i)
	{
		anEntry=getDFileEntry_nth(&parentDir, i);
		entryName=getFileName(&anEntry);
		entriesSize+=2*sizeof(int)+strlen(entryName);
		if((strcmp(entryName,".")!=0)&&(strcmp(entryName,"..")!=0))
		{
			entryInode=getINodeNumber(&anEntry);
			/*Get the inode of the number*/
			aDinode=(pointerToDinode)getNthValue((*dinodes),entryInode);/*Get the dinode of the entry*/
			if(isDirectory(&aDinode)==TRUE)
			{/*If the entry is about a directory*/
				char* realName;
				realName=malloc((strlen(path)+strlen(entryName)+1)*sizeof(char));
				strcpy(realName,path);
				strcat(realName,entryName);
				/*Now that you have the real name, search for it in the directories list*/
				childDir=searchByRealName((*directories), realName);
				if(childDir==NULL)
				{
					continue;/*This entry was already here, so it is set*/
				}
				int childEntries=getNumberOfDFileEntries(&childDir);
				addDFileEntry(&childDir,childEntries-2,parentInode,"..");/*Insert parent*/
				addDFileEntry(&childDir,childEntries-1,entryInode,".");/*Insert current entry*/
				/*Now do this for the child's directories*/
				char* newPath=malloc((strlen(path)+strlen(entryName)+strlen("/")+1)*sizeof(char));
				strcpy(newPath,path);
				strcat(newPath,entryName);
				strcat(newPath,"/");
				setParentAndCurrent(dinodes, directories, newPath,getPositionByRealName((*directories),realName),entryInode);
				free(realName);
				free(newPath);
			}
		}
	}
	if(parentInode!=0)
	{/*root is set*/
		aDinode=getNthValue((*dinodes),parentInode);
		setFileSizeInBytes(&aDinode,entriesSize);
		/*Set blocks occupying*/
		int blocks=entriesSize/BLOCK_SIZE;
		if((entriesSize%BLOCK_SIZE)!=0)
		{
			++blocks;
		}
		setBlocksOccupying(&aDinode,blocks);
	}
	free(currentDirRealName);
}

int createMMFile(pointerToFile* theFile, char* fileName, long int fileSize, int* blocksUsed)
{
	if(createFile(theFile,fileName)==FALSE)
	{
		printf("Could not create file %s\n",fileName );
		return FALSE;
	}
	/*Now find the number of blocks you need for this file*/
	int blocksToAllocate;
	blocksToAllocate=((int)fileSize/BLOCK_SIZE);
	if(((int)fileSize%BLOCK_SIZE)!=0)
	{
		++blocksToAllocate;
	}
	if(allocateFileBlocks(theFile, blocksToAllocate)!=TRUE)
	{
		printf("Could not allocate file blocks\nExiting . . .\n");
		exit(1);
	}
	int bytesRead=0;
	int fdO=open(fileName,O_RDONLY);/*Open the file to copy it*/
	if(fdO<0)
	{
		printf("Could not create original file\n");
		return FALSE;
	}
	while(bytesRead<(int)fileSize)
	
		void* buffer=getFileBlock(theFile);
		bytesRead=bytesRead+read(fdO,buffer,BLOCK_SIZE)
	
	close(fdO);
	(*blocksUsed)=blocksToAllocate;
	return TRUE;
}


int copyFiles(listPointer* dinodes, char* difile, int *blocksInFile, int* bytesInFile, int offset, int* extraBlocks)
{
	pointerToFile aFile;
	pointerToDinode aDinode;
	int i;
	int numberOfDinodes=listLength((*dinodes));
	for(i=0;i<numberOfDinodes;++i)
	{
		aDinode=(pointerToDinode)getNthValue((*dinodes),i);
		//printDinode(aDinode);
		if(isDirectory(&aDinode)==TRUE)
		{/*It is a directory;skip it*/
			continue;
		}
		else
		{/*The file is not a directory, so we procceed to copying it*/
			/*If the file has been retrieved, name sections will be null*/
			if(getRealNameNth((*dinodes),i)==NULL)
			{
				continue;/*Move on to the next, since this file is already written*/
			}

			if(createFile(&aFile, getRealNameNth((*dinodes), i))==FALSE)
			{
				printf("Could not create file %s\nExiting . . .\n",getRealNameNth((*dinodes),i) );
				return FALSE;
			}
			else
			{
			}
			setStartingBlock(&aDinode, (*blocksInFile));
			long int fileSize=getFileSizeInBytes(&aDinode);
			int blocksToAllocate;
			blocksToAllocate=((int)fileSize/BLOCK_SIZE);
			if(((int)fileSize%BLOCK_SIZE)!=0)
			{
				++blocksToAllocate;
			}
			(*extraBlocks)=(*extraBlocks)+blocksToAllocate;
			if(allocateFileBlocks(&aFile, blocksToAllocate)!=TRUE)
			{
				printf("Could not allocate file blocks\nExiting . . .\n");
				return FALSE;
			}
			setBlocksOccupying(&aDinode, blocksToAllocate);
			int bytesRead=0;
			int fdO;/*File descriptor for the original*/
			fdO=open(getRealNameNth((*dinodes),i),O_RDONLY);/*Open the file to copy it*/
			if(fdO<0)
			{
				printf("Could not create original file\n");
				return;
			}
			while(bytesRead<(int)fileSize)
			{
				void* buffer=getFileBlock(&aFile);
				bytesRead=bytesRead+read(fdO,buffer,BLOCK_SIZE);
			}
			close(fdO);
			(*bytesInFile)=(*bytesInFile)+fileSize;
			(*blocksInFile)=(*blocksInFile)+blocksToAllocate;
			copyFileContent(&aFile, difile, offset);
			offset+=blocksToAllocate*BLOCK_SIZE;
			destroyFile(&aFile);
		}
	}
	return TRUE;
}

int copyDirectories(listPointer directories, listPointer* dinodes,int* blocksInFile, char* difile, int* bytesInFile)
{
	int numOfDirectories=listLength(directories);
	int i;
	int bytesWritten;
	pointerToDFile aDirectory;
	pointerToDinode aDinode;
	for(i=0;i<numOfDirectories;++i)
	{
		bytesWritten=0;
		aDirectory=(pointerToDFile)getNthValue(directories,i);
		//printDFile(aDirectory);
		copyDFileContent(&aDirectory, difile,&bytesWritten);
		/*Now make the dinode show the starting block of a directory*/
		char* realName;
		char* dirName=getRealNameNth(directories, i);
		if(dirName!=NULL)
		{
			/*
			NULL, it is a retrieved directory, and we will shift its position later
			plus, we have already measured the space it occupies.
			We want only to set new values for the new directories and the root directory, that has been updated
			*/	
			char* ret;
			ret=strchr(dirName,'/');
			if((ret==NULL)&&(strcmp(dirName,"root")!=0))
			{/*first level directory*/
				realName=malloc((strlen(FILEPATH)+strlen(dirName)+1)*sizeof(char));
				strcpy(realName,FILEPATH);
				strcat(realName,dirName);
				aDinode=(pointerToDinode)searchByRealName((*dinodes), realName);
				setStartingBlock(&aDinode,(*blocksInFile));
				setFileSizeInBytes(&aDinode,bytesWritten);
				free(realName);
			}
			else
			{
				aDinode=(pointerToDinode)searchByRealName((*dinodes), dirName);
				setStartingBlock(&aDinode,(*blocksInFile));
				setFileSizeInBytes(&aDinode,bytesWritten);
			}
		}
		(*bytesInFile)=(*bytesInFile)+bytesWritten;
		(*blocksInFile)=(*blocksInFile)+(bytesWritten/BLOCK_SIZE);
		if((bytesWritten%BLOCK_SIZE)!=0)
		{
			(*blocksInFile)=(*blocksInFile)+1;
		}
	}
	return TRUE;
}

int setDifileHeader(pointerToHeader* theHeader,char* difile, int directoriesStart, int metaDataStart, int bytesInFile, int blocksInFile, int dinodesNumber)
{
	setSizeInBlocks(theHeader, blocksInFile);	
	setSizeInBytes(theHeader, bytesInFile);
	setDataStart(theHeader, 1);
	setDirectoriesStart(theHeader, directoriesStart);
	setMetadataStart(theHeader, metaDataStart);
	setDinodesNumber(theHeader, dinodesNumber);
	if(copyHeaderContents(theHeader,difile)==FALSE)
	{
		printf("Could not copy header content to file\nExiting . . .\n");
		return FALSE;
	}
	return TRUE;
}

int copyDinodes(listPointer dinodes, int* blocksInFile, char* difile, int* bytesInFile, int extraBlocks)
{
	int i;
	//printDinodesList(dinodes);
	pointerToDinodeFile aDinodeFile;
	pointerToDinode aDinode;
	int numOfDinodes=listLength(dinodes);
	if(createDinodeFile(&aDinodeFile)==FALSE)
	{
		printf("Cannot create a dinode file to store the dinodes\nExiting . . .\n");
		return FALSE;
	}
	if(allocateBlocksForDinodeFile(&aDinodeFile, numOfDinodes)==FALSE)
	{
		printf("Cannot allocate space for the dinode file\nExiting . . .\n");
		return FALSE;
	}
	for(i=0;i<numOfDinodes;++i)
	{
		aDinode=(pointerToDinode)getNthValue(dinodes, i);
		//printDinode(aDinode);
		if(
			(isDirectory(&aDinode)==TRUE)&&(getRealNameNth(dinodes, i)==NULL))
		{/*A retrieved directory will be shifted*/
			int startingBlock=getStartingBlock(&aDinode)+extraBlocks;
			setStartingBlock(&aDinode,startingBlock);
		}
		copyDinodeToDinodeFile(&aDinodeFile, &aDinode);
	}
	if(copyDinodesContent(&aDinodeFile, difile, ARG_NOT_USED)==FALSE)
	{
		printf("Cannot copy the file\nExiting . . .\n");
		return FALSE;
	}
	(*bytesInFile)=(*bytesInFile)+getSizeOfStructDinode()*numOfDinodes;
	(*blocksInFile)=(*blocksInFile)+((getSizeOfStructDinode()*numOfDinodes)/BLOCK_SIZE);
	if((int)((getSizeOfStructDinode()*numOfDinodes)%BLOCK_SIZE)!=0)
	{
		(*blocksInFile)=(*blocksInFile)+1;
	}
	return TRUE;
}