#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stattools.h"


long getInode(char *file){
	struct stat st = { 0 };
	if(stat(file, &st) == 0){
		printf("%ld\n", st.st_ino);
		return st.st_ino;
	}
	return -1;
}


int getUserId(char *file){
	struct stat st = { 0 };
	if(stat(file, &st) == 0){
		return st.st_uid;
	}
	return -1;
}


long getSize(char *file){
	struct stat st = { 0 };
	if(stat(file, &st) == 0){
		return st.st_size;
	}
	return -1;
}


int isDir(char *file){
	struct stat st = { 0 };
	if(stat(file, &st) == 0){
		return (S_ISDIR(st.st_mode) ? TRUE : FALSE);
	}
	return -1;
}


int getUserRights(char *file){
	int user = 0;

	struct stat st = { 0 };
	if(stat(file, &st) == 0){
		user += ((st.st_mode & S_IRUSR) ? 0400 : 0);
    	user += ((st.st_mode & S_IWUSR) ? 0200 : 0);
    	user += ((st.st_mode & S_IXUSR) ? 0100 : 0);
    	return user;
	}
	return -1;
}


int getGroupRights(char *file){
	int group = 0;

	struct stat st = { 0 };
	if(stat(file, &st) == 0){
		group += ((st.st_mode & S_IRGRP) ? 0040 : 0);
 		group += ((st.st_mode & S_IWGRP) ? 0020 : 0);
	    group += ((st.st_mode & S_IXGRP) ? 0010 : 0);
		return group;
	}
	return -1;
}


int getOtherRights(char *file){
	int other = 0;

	struct stat st = { 0 };
	if(stat(file, &st) == 0){
		other += ((st.st_mode & S_IROTH) ? 0004 : 0);
    	other += ((st.st_mode & S_IWOTH) ? 0002 : 0);
    	other += ((st.st_mode & S_IXOTH) ? 0001 : 0);
		return other;
	}
	return -1;
}


long getAccessTime(char *file){
	struct stat st = { 0 };
	if(stat(file, &st) == 0){
		return st.st_atime;
	}
	return -1;
}


long getModificationTime(char *file){
	struct stat st = { 0 };
	if(stat(file, &st) == 0){
		return st.st_mtime;
	}
	return -1;
}


long getStatusChangeTime(char *file){
	struct stat st = { 0 };
	if(stat(file, &st) == 0){
		return st.st_ctime;
	}
	return -1;
}