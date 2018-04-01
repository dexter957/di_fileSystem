#include "diHeader.h"



typedef struct dFileEntry *pointerToDFileEntry;
typedef struct dFile *pointerToDFile;


/*Functions for the directory entries in a directory file*/
int createDFileEntry(pointerToDFileEntry *theDFileEntry);
void destroyDFileEntry(pointerToDFileEntry *theDFileEntry);
void setINodeNumber(pointerToDFileEntry *theDFileEntry, int inodeNumber);
int setFileName(pointerToDFileEntry *theDFileEntry, char* fileName);
int getINodeNumber(pointerToDFileEntry *theDFileEntry);
char* getFileName(pointerToDFileEntry *theDFileEntry);
int getFileNameLength(pointerToDFileEntry *theDFileEntry);
long int getSizeOfStructDFileEntry();
void printDFileEntry(pointerToDFileEntry theEntry);

/*Functions for the directory files*/
int createDFile(pointerToDFile *theDFile);
int allocateDFileEntries(pointerToDFile *theDFile, int numOfEntries);
int getNumberOfDFileEntries(pointerToDFile *theDFile);
pointerToDFileEntry getDFileEntry_nth(pointerToDFile *theDFile, int n);
void destroyDFile(pointerToDFile *theDFile);
int addDFileEntry(pointerToDFile *theDFile, int n,int inodeNum, char* fileName);
long int getSizeOfStructDFile();
void printDFile(pointerToDFile theDFile);


/*Functions for writing a directory file to a .di file*/
void copyDFileContent(pointerToDFile *theDFile, char* difileName, int* bytesWritten);

/*Function to retrieve a directory file*/
int retrieveDFile(pointerToDFile *theDFile, int offset, int sizeInBytes,char* difileName);
