#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include "stattools.h"


int hierarchyZip(char*);


int main(int argc, char *argv[]){
	int i, pid, status;
	char cur[1024], *curWithDel, *entity;

	for(i=0;i<argc;++i)
	{
		printf("%s\n",argv[i] );
	}




	if(getcwd(cur, sizeof(cur)) == NULL){
		printf("GetCWD Exception\n");
		exit(1);
	}

	printf("%s\n", cur);

	curWithDel = malloc(sizeof(char)*(strlen(cur)+2));
	if(curWithDel == NULL){
		printf("Allocation Exception\n");
		exit(1);
	}
	strcpy(curWithDel, cur);
	strcat(curWithDel, "/");

	printf("%s\n", curWithDel);

	for(i = 0; i < argc; i++){
		entity = malloc(sizeof(char)*(strlen(curWithDel) + strlen(argv[i]) + 1));
		if(entity == NULL){
			printf("Allocation Exception\n");
			free(curWithDel);
			exit(1);
		}
		strcpy(entity, curWithDel);
		strcat(entity, argv[i]);

		printf("folder in main %s\n", entity);

		if(isDir(entity)){
			if(hierarchyZip(entity)){
				printf("Zipping Exception\n");
				exit(1);	
			}
		}
		else{

			if((pid = fork())==-1){
				printf("Fork Exception\n");
				exit(1);
			}
			else if(pid == 0){
				if(execlp("gzip", "gzip", entity, NULL) == -1){
					printf("Execlp Exception\n");
					exit(1);
				}
			}

			wait(&status);

		}

		free(entity);
	}

	free(curWithDel);

	exit(0);
}



int hierarchyZip(char* entity){
	int pid, status;
	char *newEntity, *directory;
	DIR *dirDesc;
	struct dirent *content;

	if(!isDir(entity)){

		printf("file %s\n", entity);

		if((pid = fork())==-1){
			printf("Fork Exception\n");
			return 1;
		}
		else if(pid == 0){
			if(execlp("gzip", "gzip", entity, NULL) == -1){
				printf("Execlp Exception\n");
				return 1;
			}
		}

		wait(&status);
	}
	else{
		directory = malloc(sizeof(char)*(strlen(entity)+2));
		if(directory == NULL){
			printf("Allocation Exception\n");
			return 1;
		}
		strcpy(directory, entity);
		if(entity[strlen(entity)-1] != '/'){
			strcat(directory, "/");
		}

		printf("folder %s\n", entity);
		printf("folder %s\n", directory);


		if((dirDesc = opendir(entity)) == NULL){
			printf("Open Directory Exception\n");
			return 1;
		}
		while((content = readdir(dirDesc)) != NULL){
			if(strcmp(content->d_name, ".") == 0 || strcmp(content->d_name, "..") == 0){
				continue;
			}

			newEntity = malloc(sizeof(char)*(strlen(directory) + strlen(content->d_name) + 1));
			if(newEntity == NULL){
				printf("Allocation Exception\n");
				free(directory);
				return 1;
			}
			strcpy(newEntity, directory);
			strcat(newEntity, content->d_name);

			if(hierarchyZip(newEntity)){
				free(newEntity);
				free(directory);
				return 1;
			}

			free(newEntity);
		}

		closedir(dirDesc);

		free(directory);
	}

	return 0;
}
