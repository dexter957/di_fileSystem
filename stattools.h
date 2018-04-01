#define TRUE 1
#define FALSE 0

long getInode(char*);
int getUserId(char*);
long getSize(char*);
int isDir(char*);
int getUserRights(char*);
int getGroupRights(char*);
int getOtherRights(char*);
long getAccessTime(char*);
long getModificationTime(char*);
long getStatusChangeTime(char*);