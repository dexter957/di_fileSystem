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

/*
We need the input arguments to be in the program's folder
*/


int createHierachy(listPointer* dinodes, char* directoryName,char* initialDir);
int getNumOfEntriesInDirectory(char* directoryName);
int createDirectoryEntries(listPointer* directories, listPointer dinodes, char* directoryName);
int createMMFile(pointerToFile* theFile, char* fileName, long int fileSize, int* blocksUsed);
int createFatherOfAll(listPointer* dinodes, listPointer* directories, char** firstLevelContents, int arrayLength);
int insertDataToRootDirectory(listPointer dinodes, listPointer* directories, char** firstLevelContents, int arrayLength);
int copyDinodes(listPointer dinodes, int* blocksInFile, char* difile, int* bytesInFile);
int copyDirectories(listPointer directories, listPointer* dinodes, int* blocksInFile, char* difile, int* bytesInFile);
int retreiveDirectories(char* difile, int numOfDirectories);
int copyFiles(listPointer* dinodes, char* difile, int *blocksInFile, int* bytesInFile);
int setDifileHeader(pointerToHeader* theHeader,char* difile, int directoriesStart, int metaDataStart, int bytesInFile, int blocksInFile, int dinodesNumber);
int retrieveFirstEntry(char* difile, int directoriesStart);
void setParentAndCurrent(listPointer *dinodes, listPointer* directories,char* path, int currentDir,int parentInode);
int dinodesForFirstLevelFile(listPointer *dinodes, char* fileName);

int main(int argc, char* argv[]){

	char* difile=argv[0];
	int compressionFlag=1;
	int filesNum=argc-2;
	int argsStart=2;
	int otherInput=2;
	char** flatFiles=malloc(filesNum*sizeof(char*));

	int numOfDinodes=0;
	int i;
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
	/*First we need to create the dinodes list*/
	listPointer dinodesList=initList(); 			/*Pointer to a dinodes list*/
	listPointer directoriesList=initList();			/*Pointer to a list of directories*/
	/*We need to do several jobs for each argument*/
	createFatherOfAll(&dinodesList,&directoriesList,flatFiles,filesNum);
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
	/*We are going to insert the data/entries to the root directory*/
	insertDataToRootDirectory(dinodesList,&directoriesList,flatFiles,filesNum);
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
	/*Go set parent and current for every directory*/
	setParentAndCurrent(&dinodesList, &directoriesList,"", 0,0);

	int blocksInFile=0;
	int bytesInFile=0;
	int dataStart=1;
	int metaDataStart=0;
	int directoriesStart=0;

	pointerToHeader theHeader;
	if(createHeader(&theHeader)==FALSE)
	{
		printf("Could not create the header\nExiting . . .\n");
		exit(1);
	}

	++blocksInFile;/*The header occupies one block; the first*/
	/*Header created, now let's write it to the file*/
	if(copyHeaderContents(&theHeader,difile)==FALSE)
	{
		printf("Could not copy header content to file\nExiting . . .\n");
		exit(1);
	}
	/*Wrote header contents, now going to write to the difile*/
	copyFiles(&dinodesList, difile, &blocksInFile, &bytesInFile);
	
	directoriesStart=blocksInFile;

	/*Copy the directories*/
	copyDirectories(directoriesList, &dinodesList ,&blocksInFile, difile, &bytesInFile);
	metaDataStart=blocksInFile;
	
	/*Copy the dinodes*/
	copyDinodes(dinodesList, &blocksInFile, difile, &bytesInFile);

	/*Update the header*/
	setDifileHeader(&theHeader, difile, directoriesStart, metaDataStart,bytesInFile, blocksInFile, listLength(dinodesList));

	/*Cleanup and exit*/
	freeFunction freer = &destroyDinode;
	deleteAllAndFree(&dinodesList, freer);
	freer=&destroyDFile;
	deleteAllAndFree(&directoriesList,freer);
	destroyHeader(&theHeader);
	for (i=0;i<filesNum;++i)
	{
		free(flatFiles[i]);
	}
	free(flatFiles);
}

int dinodesForFirstLevelFile(listPointer *dinodes, char* fileName)
{/*Create the dinodes for a first level file*/
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
	 		free(file);
		}
		closedir(dp);
	}
	else
	{
		;
	}
	/*If dir5 was your input the very first time, you need to create a dinode for it too*/
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
	//	printf("Could not open directory %s\n",directoryName);
	}
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
		return FALSE;
	}

	int bytesRead=0;
	int fdO=open(fileName,O_RDONLY);/*Open the file to copy it*/
	if(fdO<0)
	{
		printf("Could not create original file\n");
		return FALSE;
	}
	while(bytesRead<(int)fileSize)
	{
		void* buffer=getFileBlock(theFile);
		bytesRead=bytesRead+read(fdO,buffer,BLOCK_SIZE);
	}
	close(fdO);
	(*blocksUsed)=blocksToAllocate;
	return TRUE;
}


int createFatherOfAll(listPointer* dinodes, listPointer* directories, char** firstLevelContents, int arrayLength)
{/*Creates the root directory and its root dinode. By the end of the function, these two entities are existant, but EMPTY*/

	/*We need to create the directory "root"*/
	pointerToDFile rootDir;
	if(createDFile(&rootDir)==FALSE)
	{
		printf("Cannot create the mm structure for the directory file\nExit . . .\n");
		exit(1);
	}
	/*Directory created ok*/
	if(allocateDFileEntries(&rootDir, arrayLength)==FALSE)
	{
		printf("Cannot allocate space for the file entries\nExiting . . .\n");
		exit(1);
	}
		/*Space for entries allocated ok*/
	/*Now add the EMPTY OF ENTRIES directory into the list of directories*/
	insert(directories, (void*)rootDir, "root", "root");
	//printf("Added empty directory in the directories list\n");
	/*Now create the first dinode, the one that represents this root directory*/
	pointerToDinode rootNode;
	if(createDinode(&rootNode)==FALSE)
	{
		printf("Cannot initialise the dinode1\nExiting\n");
		exit(1);
	}
	setDinodeNum(&rootNode, 0);
	setDirectory(&rootNode, TRUE);
	//printf("Created dinode for root\n");
	/*Now insert THE EMPTY dinode first into the list*/
	insert(dinodes, (void*)rootNode, "root", "root");
	//printf("Inserted the dinode\n");
	return TRUE;
}


int insertDataToRootDirectory(listPointer dinodes, listPointer* directories, char** firstLevelContents, int arrayLength)
{/*Inserts the entries to the root directory*/

	int i;
	char* realName;
	pointerToDinode aDinode;
	pointerToDFile aDirectory=(pointerToDFile)getNthValue((*directories), 0);/*Get the first directory, the root*/
	long int entriesSize=0;
	/*Add to it the entries for the first level files/directories*/
	for(i=0;i<arrayLength;++i)
	{/*Get the dinode for each of first level entries*/
		realName=malloc((strlen(FILEPATH)+strlen(firstLevelContents[i])+1)*sizeof(char));
		strcpy(realName,FILEPATH);
		strcat(realName,firstLevelContents[i]);
		pointerToDinode aDinode=searchByRealName(dinodes, realName);/*Get the dinode for the first level file i*/
		if(aDinode!=NULL)
		{/*Now create the directory entry*/
			int inode=getDinodeNum(&aDinode);
			addDFileEntry(&aDirectory, i, inode, firstLevelContents[i]);
			entriesSize+=2*sizeof(int)+strlen(firstLevelContents[i]);
		}
		free(realName);
	}
	/*Now add the current folder and the parent folder. In this case, they are the same*/
	realName=malloc((strlen("root")+1)*sizeof(char));
	strcpy(realName,"root");
	aDinode=(pointerToDinode)searchByRealName(dinodes,realName);
	addDFileEntry(&aDirectory,arrayLength,getDinodeNum(&aDinode),".");/*Insert current*/
	addDFileEntry(&aDirectory,arrayLength+1,getDinodeNum(&aDinode),"..");/*Insert parent*/
	free(realName);
	entriesSize+=4*sizeof(int)+strlen(".")+2+strlen("..");
	setFileSizeInBytes(&aDinode, entriesSize);/*Finally set the entries size*/
	aDirectory=(pointerToDFile)getNthValue((*directories), 0);
	return TRUE;
}


int copyDinodes(listPointer dinodes, int* blocksInFile, char* difile, int* bytesInFile)
{
	int i;
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
	destroyDinodeFile(&aDinodeFile);
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

		copyDFileContent(&aDirectory, difile,&bytesWritten);
		/*Now make the dinode show the starting block of a directory*/
		char* realName;
		char* dirName=getRealNameNth(directories, i);
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
		(*bytesInFile)=(*bytesInFile)+bytesWritten;
		(*blocksInFile)=(*blocksInFile)+(bytesWritten/BLOCK_SIZE);
		if((bytesWritten%BLOCK_SIZE)!=0)
		{
			(*blocksInFile)=(*blocksInFile)+1;
		}
	}
	return TRUE;
}

int copyFiles(listPointer* dinodes, char* difile, int *blocksInFile, int* bytesInFile)
{/*Copies every file in the dinodes list to the .di file*/

	pointerToFile aFile;
	pointerToDinode aDinode;
	int i;
	int numberOfDinodes=listLength((*dinodes));
	for(i=0;i<numberOfDinodes;++i)
	{
		aDinode=(pointerToDinode)getNthValue((*dinodes),i);
		if(isDirectory(&aDinode)==TRUE)
		{/*It is a directory;skip it*/
			continue;
		}
		else
		{/*The file is not a directory, so we procceed to copying it*/
			if(createFile(&aFile, getRealNameNth((*dinodes), i))==FALSE)
			{
				printf("Could not create file %s\nExiting . . .\n",getRealNameNth((*dinodes),i) );
				return FALSE;
			}
			else
			{
				;
			}
			setStartingBlock(&aDinode, (*blocksInFile));
			long int fileSize=getFileSizeInBytes(&aDinode);
			int blocksToAllocate;
			blocksToAllocate=((int)fileSize/BLOCK_SIZE);
			if(((int)fileSize%BLOCK_SIZE)!=0)
			{
				++blocksToAllocate;
			}
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
			copyFileContent(&aFile, difile, ARG_NOT_USED);
			destroyFile(&aFile);
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


int retrieveFirstEntry(char* difile, int directoriesStart)
{
	long int offset=directoriesStart*BLOCK_SIZE;
	long int bytesRead=0;
	int i;
	pointerToDFile anDirectory;
	if(createDFile(&anDirectory)==FALSE)
	{
		printf("Cannot create the mm structure for the directory file\nExit . . .\n");
		exit(1);
	}
	printDFile(anDirectory);
	if(retrieveDFile(&anDirectory, offset, (int)(2*sizeof(int)+strlen("dir21")+1),difile)==FALSE)
	{
		printf("Cannot retrieve info from file %s\nExiting . . .",difile);
		return FALSE;
	}
	return TRUE;
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
	//int parentInode;
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