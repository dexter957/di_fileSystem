#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"


typedef struct ListNode{
	void* value;
	char* filename;
	char* realname;
	listPointer next, previous;
}ListNode;


listPointer initList(){
	return NULL;
}


int insert(listPointer* list, void* insValue, char* name, char* real){
	listPointer temp = malloc(sizeof(ListNode));
	if(temp == NULL){
		return FALSE;
	}
	temp->filename = NULL;
	temp->realname = NULL;

	if(name != NULL){
		temp->filename = malloc(sizeof(char)*(strlen(name)+1));
		if(temp->filename == NULL){
			free(temp);
			return FALSE;
		}
		strcpy(temp->filename, name);
	}

	if(real!=NULL){
		temp->realname = malloc(sizeof(char)*(strlen(real)+1));
		if(temp->realname == NULL){
			free(temp);
			return FALSE;
		}
		strcpy(temp->realname, real);
	}

	temp->value = insValue;
	
	if(*list == NULL){
		*list = temp;
	}
	else{
		listPointer oldEnd = (*list)->previous;
		temp->previous = oldEnd;
		oldEnd->next = temp;
	}
	(*list)->previous = temp;
	temp->next = *list;


	return TRUE;
}


int insertAtStart(listPointer* list, void* insValue, char* name, char* real){
	listPointer temp = malloc(sizeof(ListNode));
	if(temp == NULL){
		return FALSE;
	}
	temp->filename = NULL;
	temp->realname = NULL;

	if(name != NULL){
		temp->filename = malloc(sizeof(char)*(strlen(name)+1));
		if(temp->filename == NULL){
			free(temp);
			return FALSE;
		}
		strcpy(temp->filename, name);
	}

	if(real!=NULL){
		temp->realname = malloc(sizeof(char)*(strlen(real)+1));
		if(temp->realname == NULL){
			free(temp);
			return FALSE;
		}
		strcpy(temp->realname, real);
	}

	temp->value = insValue;
	
	if(*list == NULL){
		*list = temp;
	}
	else{

		listPointer end=(*list)->previous;
		temp->next=(*list);
		temp->previous=end;/////////
		end->next=temp;

	}
	(*list) = temp;

	return TRUE;
}

int delete(listPointer* list, void* delValue, comparisonFunction compare){
	listPointer previous = (*list)->previous, current = *list;

	if(current != NULL){
		do{
			if(compare(current->value, delValue)){
				if(current == *list){
					if((*list)->next == *list){
						*list = NULL;
					}
					else{
						*list = current->next;
					}
				}
				previous->next = current->next;
				current->next->previous = previous;
				if(current->filename != NULL){
					free(current->filename);
				}
				if(current->realname != NULL){
					free(current->realname);
				}
				free(current);
				return TRUE;
			}
			previous = current;
			current = current->next;
		}while(current != *list);
	}

	return FALSE;
}


int deleteAndFree(listPointer* list, void* delValue, comparisonFunction compare, freeFunction deallocate){
	listPointer previous = (*list)->previous, current = *list;

	if(current != NULL){
		do{
			if(compare(current->value, delValue)){
				if(current == *list){
					if((*list)->next == *list){
						*list = NULL;
					}
					else{
						*list = current->next;
					}
				}
				previous->next = current->next;
				current->next->previous = previous;
				if(deallocate != NULL){
					deallocate(current->value);
				}
				if(current->filename != NULL){
					free(current->filename);
				}
				if(current->realname != NULL){
					free(current->realname);
				}
				free(current);
				return TRUE;
			}
			previous = current;
			current = current->next;
		}while(current != *list);
	}

	return FALSE;
}

void deleteAndFreeTheFirst(listPointer* list, freeFunction deallocate)
{
	listPointer newfirst=(*list)->next;
	listPointer end=(*list)->previous;
	end->next=newfirst;
	newfirst->previous=end;
	listPointer toDelete=(*list);
	if(toDelete->realname!=NULL)
	{
		free(toDelete->realname);
	}
	if(toDelete->filename!=NULL)
	{
		free(toDelete->filename);
	}
	if(deallocate != NULL)
	{
		deallocate(&(toDelete->value));
	}
	(*list)=newfirst;
}

void deleteAllAndFree(listPointer* list, freeFunction deallocate){
	listPointer temp;
	
	(*list)->previous->next = NULL;
	
	while(*list != NULL){
		
		if(deallocate != NULL){
			deallocate(&((*list)->value));
		}
		
		temp = *list;
		*list = (*list)->next;
		if(temp->filename != NULL){
			free(temp->filename);
		}
		if(temp->realname != NULL){
			free(temp->realname);
		}
		free(temp);
	}
}


void* search(listPointer list, void* searchValue, comparisonFunction compare){
	listPointer current = list;

	if(current != NULL){
		do{
			if(compare(current->value, searchValue)){
			//	return current->value;
				return current;
			}
			current = current->next;
		}while(current != list);
	}

	return NULL;
}

void* searchByRealName(listPointer list, char* realname)
{
	
	listPointer current=list;

	if(current!=NULL)
	{
		
		do
		{
		//	printf("current real name %s and searching for %s\n",current->realname, realname );
			if(current->realname!=NULL)
			{
				if(strcmp(current->realname,realname)==0)
				{
					return current->value;
				}
				else
				{
					current=current->next;
				}
			}
			else
			{
				current=current->next;
			}
			
		} while (current!=list);
	}
	return NULL;
}

void* getNthValue(listPointer list, int n)
{
	listPointer current=list;
	int i, max = listLength(list);
	if(n > max){
		return NULL;
	}
	for(i=0;i<n;++i)
	{
		if(current!=NULL)
		{
			current=current->next;
		}
		else
		{
			return NULL;
		}
	}
	return current->value;
}

char* getRealNameNth(listPointer list, int n)
{
	listPointer current=list;
	int i, max = listLength(list);
	if(n > max){
		return NULL;
	}
	for(i=0;i<n;++i)
	{
		if(current!=NULL)
		{
			current=current->next;
		}
		else
		{
			return NULL;
		}
	}
	return current->realname;
}



void printList(listPointer list){
	listPointer current = list;

	printf("List:\n");
	if(current != NULL){
		do{
		//	printTester1(current->value);
			printf("%s\n", current->filename);

			current = current->next;
		}while(current != list);
	}
}


void printDinodesList(listPointer list)
{
	listPointer current=list;
	printf("List:\n");
	if(current!=NULL)
	{
		do
		{
			if(current->filename != NULL) printf("Name:%s\n",current->filename);
			//printf("Real name:%s\n",current->realname );
 			printDinode(current->value);
			current=current->next;
		} while (current!=list);
	}
}


void printDirectoriesList(listPointer list)
{
	listPointer current=list;
	printf("List:\n");
	if(current!=NULL)
	{
		do
		{
			printf("Name:%s\n",current->filename );
			printDFile(current->value);
			current=current->next;
		} while (current!=list);
	}

}


int listLength(listPointer list)
{
	int nodes=0;
	listPointer current=list;
	if(current!=NULL)
	{
		do
		{
			++nodes;
			current=current->next;
		} while (current!=list);
	}
	return nodes;
}


int setFilename(listPointer *node, char *name){
	if((*node)->filename != NULL){
		free((*node)->filename);
	}

	(*node)->filename = malloc(sizeof(char)*(strlen(name) + 1));
	if((*node)->filename == NULL){
		return FALSE;
	}

	strcpy((*node)->filename, name);

	return TRUE;
}


char* getFilename(listPointer node){
	return node->filename;
}


int setRealname(listPointer *node, char *real){
	if((*node)->realname != NULL){
		free((*node)->realname);
	}

	(*node)->realname = malloc(sizeof(char)*(strlen(real) + 1));
	if((*node)->realname == NULL){
		return FALSE;
	}

	strcpy((*node)->realname, real);

	return TRUE;
}


char* getrealname(listPointer node){
	return node->realname;
}

void* getValue(listPointer* node)
{
	return (*node)->value;
}

int getPositionByRealName(listPointer list, char* realName)
{
	listPointer current=list;
	int i=0;
	if(current!=NULL)
	{
		do
		{
			if(current->realname!=NULL)
			{
				if(strcmp(current->realname,realName)==0)
				{
					return i;
				}
				else
				{
					++i;
					current=current->next;
				}
			}
			else
			{
				++i;
				current=current->next;
			}
			
		} while (current!=list);
	}
	return i;
}