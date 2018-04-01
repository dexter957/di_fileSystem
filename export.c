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


void fullExport(listPointer, listPointer, char*, char*);
void singleExport(listPointer, listPointer, char*, char*, char*, char*, int);



int main(int argc, char* argv[]){
	char *diFile = argv[0], **inputs = (argv+1);
	int i, j, diNodeSum;
	//used mainly in diNode retrieval
	long metadataOffset, dirOffset, dataPerBlockRead = 0, diNodeSize =  getSizeOfStructDinode();
	freeFunction freeDiNode = &destroyDinode;
	listPointer diNodeList = initList();
	pointerToDinode dinode;
	char *cwd, *temp;

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


	//currently working direcotry
	temp = strrchr(diFile, '/');
	if(temp == NULL){
		cwd = malloc(sizeof(char)*1024);
		getcwd(cwd, 1024);
	}
	else{
		char *temp2;
		cwd = malloc(sizeof(char)*(strlen(diFile) + 1));
		strcpy(cwd, diFile);
		cwd[strlen(cwd) - strlen(temp)] = '\0';

		temp = diFile;
		temp2 = malloc(sizeof(char)*(strlen(diFile) + 1));
		strcpy(temp2, diFile);
		while((temp = strchr(temp, '/')) != NULL){
			temp2[strlen(temp2) - strlen(temp)] = '\0';
			mkdir(temp2, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			strcpy(temp2, diFile);
			temp = temp + 1;
		}
		free(temp2);

	}

	if(argc == 1){ //full export

		//full export
		fullExport(diNodeList, diNodeList, diFile, cwd);

	}
	else{
		int inputCount = argc - 1;

		for(i = 0; i < inputCount; i++){
			char *zipped;

			zipped = malloc(sizeof(char)*(strlen(inputs[i]) + 4));
			if(zipped == NULL){
				printf("Allocation Error\n");
				exit(1);
			}
			strcpy(zipped, inputs[i]);
			strcat(zipped, ".gz");

			singleExport(diNodeList, diNodeList, cwd, inputs[i], zipped, diFile, FALSE);

			free(zipped);
		}

	}

	free(cwd);

	deleteAllAndFree(&diNodeList, freeDiNode);

	destroyHeader(&header);
	
	exit(0);
}





void fullExport(listPointer current, listPointer list, char* diFile, char* path){
	int j;
	char *newPath;
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

		setRealname(&current, newPath);
	}
	else{
		newPath = path;
	}

	dinode = getValue(&current);
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

		mkdir(newPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

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

				fullExport(found, list, diFile, newPath);
			}
		}

		destroyDFile(&directory);
	}
	else{
		int blocksOccupied = getBlocksOccupying(&current), startingBlock = getStartingBlock(&current), fd;
		long int bytesUsed = getFileSizeInBytes(&current);
		char *temp = strstr(newPath, ".gz");
		int pid, status;

		if((pid = fork()) == -1){
			printf("Fork Error\n");
			exit(1);
		}
		else if(pid == 0){
			if(execlp("touch", "rouch", newPath, NULL) == -1){
				printf("Execlp Exception\n");
				exit(1);
			}
		}

		wait(&status);

		if(!retrieveFileContents(diFile, startingBlock*BLOCK_SIZE, blocksOccupied, bytesUsed, newPath)){
			printf("File Extraction Error\n");
		}

		if(temp != NULL){

			if((pid = fork()) == -1){
				printf("Fork Error\n");
				exit(1);
			}
			else if(pid == 0){
				if(execlp("gzip", "gzip", "-d", newPath, NULL) == -1){
					printf("Execlp Exception\n");
					exit(1);
				}
			}

			wait(&status);
		}

	}

	if(getFilename(current) != NULL){
		free(newPath);
	}

	return;

}






void singleExport(listPointer current, listPointer list, char* path, char* queryTerm, char* zipped, char* diFile, int flag){
	int j;
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
		flag = TRUE;
	}

	dinode = getValue(&current);
	if(isDirectory(&dinode)){

		long int dirContentStartBlock, dirContentByteSize;
		pointerToDFile directory;
		pointerToDFileEntry entry;
		comparisonFunction searchByDiNodeNum = &compareDinodeNum;
		listPointer found = initList();

		if(flag == TRUE){

			if(mkdir(newPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0){
				printf("Directory Creation Error\n");
				exit(1);
			}

			path = newPath;
		}

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
				
				singleExport(found, list, path, queryTerm, zipped, diFile, flag);
			}
		}

		destroyDFile(&directory);
	}
	else{
		if(flag == TRUE){
			int blocksOccupied = getBlocksOccupying(&current), startingBlock = getStartingBlock(&current), fd;
			long int bytesUsed = getFileSizeInBytes(&current);
			char *temp = strstr(newPath, ".gz");
			int pid, status;

			if((pid = fork()) == -1){
				printf("Fork Error\n");
				exit(1);
			}
			else if(pid == 0){
				if(execlp("touch", "rouch", newPath, NULL) == -1){
					printf("Execlp Exception\n");
					exit(1);
				}
			}

			wait(&status);

			if(!retrieveFileContents(diFile, startingBlock*BLOCK_SIZE, blocksOccupied, bytesUsed, newPath)){
				printf("File Extraction Error\n");
			}

			if(temp != NULL){

				if((pid = fork()) == -1){
					printf("Fork Error\n");
					exit(1);
				}
				else if(pid == 0){
					if(execlp("gzip", "gzip", "-d", newPath, NULL) == -1){
						printf("Execlp Exception\n");
						exit(1);
					}
				}

				wait(&status);
			}
		}
	}

	if(getFilename(current) != NULL){
		free(newPath);
	}

	return;
}