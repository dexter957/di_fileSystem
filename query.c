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


int hierarchicalFileSearch(listPointer, listPointer, listPointer*, char*, char*, char*, char*);


int main(int argc, char* argv[]){
	char *diFile = argv[0], **inputs = (argv+1);
	int i, j, diNodeSum, inputCount = argc - 1;
	//used mainly in diNode retrieval
	long metadataOffset, dirOffset, dataPerBlockRead = 0, diNodeSize =  getSizeOfStructDinode();
	freeFunction freeDiNode = &destroyDinode;
	listPointer diNodeList = initList();
	pointerToDinode dinode;

	//used for result retrieval


	//query preparation
	//header retrieval
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

	//diNodes retrieval
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

	//directory data retrieval
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


	/***********************************************************************************************/
	/***********************************************************************************************/
	/***********************************************************************************************/
	//actual query function
	//char *rootStr =  malloc(sizeof(char)*(strlen(diFile)+4));
	//strcpy(rootStr, diFile);
	//strcat(rootStr, "->/");
	for(i = 0; i < inputCount; i++){
		char *zipped;
		listPointer root = diNodeList;
		listPointer resultsPerInput = initList();

		zipped = malloc(sizeof(char)*(strlen(inputs[i]) + 4));
		if(zipped == NULL){
			printf("Allocation Error\n");
			exit(1);
		}
		strcpy(zipped, inputs[i]);
		strcat(zipped, ".gz");

		if(hierarchicalFileSearch(root, diNodeList, &resultsPerInput, "", inputs[i], zipped, diFile)){ //not sure if "" or "/"
			//found result(s)
			printf("\nResult(s) with the search term %s found:\n", inputs[i]);
			printList(resultsPerInput);
			printf("End Of resutls\n\n");

			deleteAllAndFree(&resultsPerInput, NULL);
		}
		else{
			printf("\nNo results with the search term %s found\n\n", inputs[i]);
		}

		free(zipped);

	}

	deleteAllAndFree(&diNodeList, freeDiNode);

	destroyHeader(&header);
	
	exit(0);

}



int hierarchicalFileSearch(listPointer current, listPointer list, listPointer *results, char* path, char* queryTerm, char* zipped, char* diFile){
	int j, returnFlag = FALSE;
	char *newPath = "";
	pointerToDinode dinode;

	if(getFilename(current) != NULL){
		newPath = malloc(sizeof(char)*(strlen(path) + strlen(getFilename(current)) + 2));
		if(newPath == NULL){
			printf("Allocation Error\n");
			exit(1);
		}
		strcpy(newPath, path);
		strcat(newPath, "/");
		strcat(newPath, getFilename(current));
	}

	if(getFilename(current) != NULL && (strcmp(getFilename(current), queryTerm) == 0 || strcmp(getFilename(current), zipped) == 0 || strcmp(newPath, queryTerm) == 0 || strcmp(newPath, zipped) == 0)){ 
		char *temp = malloc(sizeof(char)*(strlen("root") + strlen(newPath) + 1));
		if(temp == NULL){
			printf("Allocation Error\n");
			exit(1);
		}
		strcpy(temp, "root");
		strcat(temp, newPath);
		if(!insert(results, NULL, temp, NULL)){
			printf("List Insertion Error\n");
			exit(1);
		}		

		free(temp);
		
		returnFlag = TRUE;
	}

	dinode = getValue(&current);
	if(isDirectory(&dinode)){

		long int dirContentStartBlock, dirContentByteSize;
		pointerToDFile directory;
		pointerToDFileEntry entry;
		comparisonFunction searchByDiNodeNum = &compareDinodeNum;
		listPointer found = initList();

		//fix path

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

				if(hierarchicalFileSearch(found, list, results, newPath, queryTerm, zipped, diFile)){
					returnFlag = TRUE;
				}
			}
		}

		destroyDFile(&directory);
		//read entries
		//fix path for every one of them //check the "." and ".." conditions
		//run the hierarchicalsearch on all of them
		//if(hierachicalFileSearch(...)){
			//returnFlag == TRUE;
		//}
		//return true if one of them were true
	}
	//if file do nothing

	if(getFilename(current) != NULL){
		free(newPath);
	}

	return returnFlag;
}