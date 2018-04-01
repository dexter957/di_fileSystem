#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "list.h"
#include "diHeader.h"
#include "header.h"
#include "mmFile.h"
#include "diNode.h"
#include "directoryFile.h"
#include "stattools.h"


void hierachicalPathCompletion(listPointer*, listPointer, char*, char*);


int main(int argc, char* argv[]){
	char *diFile = argv[0];
	int i, j, diNodeSum;
	long metadataOffset, dirOffset, dataPerBlockRead = 0, diNodeSize =  getSizeOfStructDinode();
	listPointer diNodeList = initList();
	pointerToDinode dinode;

	freeFunction freeDiNode = &destroyDinode;

	pointerToHeader header;
	if(createHeader(&header)==FALSE)
	{
		printf("Header Creation Error\n");
		exit(1);
	}
	if(retrieveHeader(&header, diFile)==FALSE)
	{
		printf("Could not retrieve header\n");
		exit(1);
	}

	metadataOffset = getMetadataStart(&header)*BLOCK_SIZE;
	dirOffset = getDirectoriesStart(&header)*BLOCK_SIZE;
	diNodeSum = getDinodesNumber(&header);

	for(i = 0; i < diNodeSum; i++){
		if(!createDinode(&dinode)){
			printf("DiNode Creation Error\n");
			exit(1);
		}
		if(!retrieveDinode(&dinode, metadataOffset, diFile)){
			printf("Could not retrieve diNode from diFile\n");
			exit(1);
		}

		if(!insert(&diNodeList, dinode, NULL, NULL)){
			printf("List Insertion Error\n");
			exit(1);
		}

		metadataOffset += diNodeSize;
		dataPerBlockRead += diNodeSize;

		if((BLOCK_SIZE - dataPerBlockRead) < diNodeSize){
			metadataOffset += (BLOCK_SIZE - dataPerBlockRead);
			dataPerBlockRead = 0;
		}

	}

	for(i = 0; i < diNodeSum; i++){

		dinode = getNthValue(diNodeList, i);

		if(isDirectory(&dinode)){
			long int dirContentStartBlock, dirContentByteSize;
			pointerToDFile directory;
			pointerToDFileEntry entry;
			comparisonFunction searchByDiNodeNum = &compareDinodeNum;
			listPointer found = initList();

			dirContentStartBlock = getStartingBlock(&dinode);
			dirContentByteSize = getFileSizeInBytes(&dinode);

			if(!createDFile(&directory)){
				printf("Directory File Entry Creation Error\n");
				exit(1);
			}
	
			if(!retrieveDFile(&directory, dirContentStartBlock*BLOCK_SIZE, dirContentByteSize, diFile)){
				printf("Could not retrieve Directory File from diFile\n");
				exit(1);
			}


			int directoryEntries = getNumberOfDFileEntries(&directory);

			for(j = 0; j < directoryEntries; j++){

				if((entry = getDFileEntry_nth(&directory, j)) == NULL){
					printf("Could not retrieve Directory File Entry from diFile\n");
					exit(1);
				}
				
				if(getFileName(&entry) != NULL && strcmp(getFileName(&entry), ".") != 0 && strcmp(getFileName(&entry), "..") != 0){

					found = search(diNodeList, (int)getINodeNumber(&entry), searchByDiNodeNum);

					if(found == NULL){
						printf("Could not find DiNode from diFile based on DiNodeNum\n");
						exit(1);
					}
				
					setFilename(&found, getFileName(&entry));
				}
			}

			destroyDFile(&directory);
		}

	}

	//hierarchy
	hierarchicalPathCompletion(&diNodeList, diNodeList, "root", diFile);

	printDinodesList(diNodeList);

	deleteAllAndFree(&diNodeList, freeDiNode);

	destroyHeader(&header);
	
	exit(0);
}



void hierarchicalPathCompletion(listPointer *current, listPointer list, char* path, char* diFile){
	int j;
	char *newPath;
	pointerToDinode dinode;


	if(getFilename((*current)) != NULL){
		newPath = malloc(sizeof(char)*(strlen(path) + strlen(getFilename((*current))) + 2));
		if(newPath == NULL){
			printf("Allocation Error\n");
			exit(1);
		}
		strcpy(newPath, path);
		strcat(newPath, "/");
		strcat(newPath, getFilename((*current)));

		setRealname(current, newPath);
	}
	else{
		setFilename(current, path);
		setRealname(current, path);
		newPath = path;
	}

	dinode = getValue(current);
	if(isDirectory(&dinode)){

		long int dirContentStartBlock, dirContentByteSize;
		pointerToDFile directory;
		pointerToDFileEntry entry;
		comparisonFunction searchByDiNodeNum = &compareDinodeNum;
		listPointer found = initList();


		//find all the directories entries
		dirContentStartBlock = getStartingBlock(&dinode);
		dirContentByteSize = getFileSizeInBytes(&dinode);

		if(!createDFile(&directory)){
			printf("Directory File Entry Creation Error\n");
			exit(1);
		}	
	
		if(!retrieveDFile(&directory, dirContentStartBlock*BLOCK_SIZE, dirContentByteSize, diFile)){
			printf("Could not retrieve Directory File from diFile\n");
			exit(1);
		}	

		int directoryEntries = getNumberOfDFileEntries(&directory);

		for(j = 0; j < directoryEntries; j++){

			if((entry = getDFileEntry_nth(&directory, j)) == NULL){
				printf("Could not retrieve Directory File Entry from diFile\n");
				exit(1);
			}
				
			if(getFileName(&entry) != NULL && strcmp(getFileName(&entry), ".") != 0 && strcmp(getFileName(&entry), "..") != 0){

				found = search(list, (int)getINodeNumber(&entry), searchByDiNodeNum);

				if(found == NULL){
					printf("Could not find DiNode from diFile based on DiNodeNum\n");
					exit(1);
				}

				hierarchicalPathCompletion(&found, list, newPath, diFile);
			}
		}

		destroyDFile(&directory);
	}

	if(strcmp(getFilename((*current)), "root") != 0){
		free(newPath);
	}

	return;
}