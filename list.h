#define TRUE 1
#define FALSE 0

struct ListNode;
typedef struct ListNode* listPointer;
typedef int (*comparisonFunction)(void*, void*);
typedef void (*freeFunction)(void*);

listPointer initList();
int insert(listPointer*, void*, char*, char*);
int delete(listPointer*, void*, comparisonFunction);
int deleteAndFree(listPointer*, void*, comparisonFunction, freeFunction);
void deleteAllAndFree(listPointer*, freeFunction);
void* search(listPointer, void*, comparisonFunction);
int listLength(listPointer list);
void* searchByRealName(listPointer list, char* realname);

void printList(listPointer);
void printDinodesList(listPointer list);
void printDirectoriesList(listPointer list);

void* getNthValue(listPointer list, int n);
char* getRealNameNth(listPointer list, int n);
int getPositionByRealName(listPointer list, char* realName);
void* getValueByRealName(listPointer list, char* realName);
int insertAtStart(listPointer* list, void* insValue, char* name, char* real);
void deleteAndFreeTheFirst(listPointer* list, freeFunction);

int setFilename(listPointer *, char*);
char* getFilename(listPointer);
int setRealname(listPointer *, char*);
char* getrealname(listPointer);
void* getValue(listPointer* node);