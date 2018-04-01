#include "diHeader.h"
#include <time.h>

#define DIRECT 			12 /*How many direct pointers to data blocks an inode will/should have*/
#define SINGLE_INDIRECT 1  /*Number of single indirect pointers*/
#define DOUBLE_INDIRECT 1  /*Number of double indirect pointers*/
#define TRIPLE_INDIRECT 1  /*Number of triple indirect pointers*/


typedef struct dinode *pointerToDinode;
typedef struct dinodeFile *pointerToDinodeFile;

int createDinode(pointerToDinode* theDinode);
void setDinodeNum(pointerToDinode* theDinode, int num);
void setOwner(pointerToDinode* theDinode, int owner);
void setGroup(pointerToDinode *theDinode, int group);
void setDirectory(pointerToDinode* theDinode, int yesORno);
void setOwnerAccessRights(pointerToDinode* theDinode, int accessRights);
void setGroupAccessRights(pointerToDinode* theDinode, int accessRights);
void setUniverseAccessRights(pointerToDinode* theDinode, int accessRights);
void setFileSizeInBytes(pointerToDinode* theDinode, long int sizeInBytes);
void setBlocksOccupying(pointerToDinode* theDinode, int blocksOccupying);
void setLastAccess(pointerToDinode* theDinode, time_t lastAccess);
void setLastModification(pointerToDinode* theDinode, time_t lastModification);
void setLastStatusChange(pointerToDinode* theDinode, time_t lastStatusChange);
void setStartingBlock(pointerToDinode* theDinode, int startingBlock);
int getDinodeNum(pointerToDinode *theDinode);
int getOwner(pointerToDinode* theDinode);
int getGroup(pointerToDinode *theDinode);
int isDirectory(pointerToDinode* theDinode);
int getOwnerAccessRights(pointerToDinode* theDinode);
int getGroupAccessRights(pointerToDinode* theDinode);
int getUniverseAccessRights(pointerToDinode* theDinode);
long int getFileSizeInBytes(pointerToDinode *theDinode);
int getBlocksOccupying(pointerToDinode *theDinode);
time_t getLastAccess(pointerToDinode *theDinode);
time_t getLastModification(pointerToDinode *theDinode);
time_t getLastStatusChange(pointerToDinode *theDinode);
int getStartingBlock(pointerToDinode *theDinode);
void destroyDinode(pointerToDinode* theDinode);
long int getSizeOfStructDinode();
void printDinode(pointerToDinode theDinode);
int compareDinodeNum(pointerToDinode theDinode, int num);

/*Functions for the dinode file*/


int createDinodeFile(pointerToDinodeFile *theDinodeFile);
int allocateBlocksForDinodeFile(pointerToDinodeFile *theDinodeFile, int numOfDiNodes);
void copyDinodeToDinodeFile(pointerToDinodeFile *theDinodeFile, pointerToDinode *theDinode);
int copyDinodesContent(pointerToDinodeFile *theDinodeFile, char* difileName, int offset);
void destroyDinodeFile(pointerToDinodeFile *theDinodeFile);
long int getSizeOfStructDinodeFile();
int retrieveDinode(pointerToDinode *theDinode, int offset, char* difileName);