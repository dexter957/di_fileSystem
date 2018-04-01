#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct stat st = { 0 };


int main(int argc, char* argv[]){
	int flagMissing = 1, diFileMissing = 1, filesMissing = 1, compression = 0; //for the missing stuff
	int flagPos, diFilePos, filesPos; //positions inside argv
	int i, pid, status, flag = 0;
	char** vector;

	for(i = 1; i < argc; i++){ //{-c|-a|-x|-m|-d|-p|-j}
		if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "-q") == 0){
			flagPos = i;
			flagMissing = 0;
		}
		else if(strcmp(argv[i], "-j") == 0){
			compression = 1;
		}
		else if(strstr(argv[i], ".di") != NULL && diFileMissing == 1 && filesMissing == 1){
			diFileMissing = 0;
			diFilePos = i;
		}
		else if(filesMissing == 1){
			filesMissing = 0;
			filesPos = i;
		}
		else{
			break;
		}
	}

	if(flagMissing){
		printf("Incorrect number of parameters (Operation Flag Missing {-c|-a|-x|-m|-d|-p|-j|-q})\n");
		return -1;
	}
	else if(!flagMissing && compression && strcmp(argv[flagPos], "-c") != 0 && strcmp(argv[flagPos], "-a") != 0){
		printf("Incorrect use of compression flag -j\n");
		return -1;
	}
	if(diFileMissing){
		printf("Incorrect number of parameters (No <archive-file> given)\n");
		return -1;
	}
	if(filesMissing && strcmp(argv[flagPos], "-x") != 0 && strcmp(argv[flagPos], "-m") != 0 && strcmp(argv[flagPos], "-p") != 0){
		printf("Incorrect number of parameters (No <list-of-files/dirs> given)\n");
		return -1;
	}
	if(!filesMissing && (strcmp(argv[flagPos], "-m") == 0 || strcmp(argv[flagPos], "-p") == 0)){
		printf("Incorrect use of metadata flag %s (No <list-of-files/dirs> needed)\n", argv[flagPos]);
		return -1;
	}


	if(strcmp(argv[flagPos], "-c") != 0 && strcmp(argv[flagPos], "-a") != 0){
		if(stat(argv[diFilePos], &st) != 0){
			printf("No file %s exists\n", argv[diFilePos]);
			return -1;
		}
	}

	if(!filesMissing && (strcmp(argv[flagPos], "-c") == 0 || strcmp(argv[flagPos], "-a") == 0)){
		for(i = filesPos; i < argc; i++){
			if(stat(argv[i], &st) != 0){
				printf("No file/folder %s exists\n", argv[i]);
				return -1;
			}
		}
	}


	if(strcmp(argv[flagPos], "-c") == 0 || strcmp(argv[flagPos], "-a") == 0){
		vector = malloc(sizeof(char*)*(argc - filesPos + 3));
		if(vector == NULL){
			printf("Allocation Exception\n");
			return -1;
		}
		vector[0] = malloc(sizeof(char)*(strlen(argv[diFilePos])+1));
		vector[1] = malloc(sizeof(char)*2);
		if(vector[0] == NULL || vector[1] == NULL){
			free(vector[0]);
			free(vector[1]);
			free(vector);
			printf("Allocation Exception\n");
			return -1;
		}
		for(i = 2; i < argc - filesPos + 2; i++){
			vector[i] = malloc(sizeof(char)*(strlen(argv[filesPos+i-2])+1));
			if(vector[i] == NULL){
				flag = 1;
				break;
			}
		}
		if(flag){
			int j;

			for(j = 0; j < i; j++){
				free(vector[j]);
			}
			free(vector);
			printf("Allocation Exception\n");
			return -1;
		}
		strcpy(vector[0], argv[diFilePos]);
		if(compression){
			strcpy(vector[1], "1");
		}
		else{	
			strcpy(vector[1], "0");
		}
		for(i = 2; i < argc - filesPos + 2; i++){
			strcpy(vector[i], argv[filesPos+i-2]);
		}
		vector[argc - filesPos + 2] = NULL;
	}


	if((pid = fork()) < 0){
		printf("Fork Exception\n");
		return -1;
	}
	else if(pid == 0){
		if(strcmp(argv[flagPos], "-c") == 0){
			execv("./create", vector);
		}
		else if(strcmp(argv[flagPos], "-a") == 0){
			if(stat(argv[diFilePos], &st) == 0){
				execv("./append", vector);
			}
			else{
				execv("./create", vector);
			}
		}
		else if(strcmp(argv[flagPos], "-x") == 0){
			execv("./export", (argv+diFilePos));
		}
		else if(strcmp(argv[flagPos], "-m") == 0){
			printf("Entering new exec!!!\n");
			execv("./metadata", (argv+diFilePos));
		}
		else if(strcmp(argv[flagPos], "-d") == 0){
			execv("./delete", (argv+diFilePos));
		}
		else if(strcmp(argv[flagPos], "-p") == 0){
			execv("./print", (argv+diFilePos));
		}
		else if(strcmp(argv[flagPos], "-q") == 0){
			execv("./query", (argv+diFilePos));
		}
		else{
			return -1;
		}
	}

	wait(&status);

	if(strcmp(argv[flagPos], "-c") == 0 || strcmp(argv[flagPos], "-a") == 0){
		for(i = 0; i < argc - filesPos + 2; i++){
			free(vector[i]);
		}
		free(vector);
	}
	
	return 0;
}
