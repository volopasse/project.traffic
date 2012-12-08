#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdlib.h>

typedef struct ListNode{
	struct ListNode *next;
	struct ListNode *prev;
	void *value;
} ListNode;

typedef struct List{
	int count;
	ListNode *first;
	ListNode *last;
} List;

List *listCreate();
void listDestroy(List *list);
void listClear(List *list);
void listClearDestroy(List *list);

#define listCount(A)	( (A)->count)
#define listFirst(A)	( (A)->first != NULL ? (A)->first->value : NULL)
#define listLast(A)		( (A)->last != NULL ? (A)->last->value : NULL)

void listPushFront(List *list, void *value);
void *listPopFront(List *list);

void listPushEnd(List *list, void *value);
void *listPopEnd(List *list);

void listPushMid(List *list, int pos, void *value);
void *listPopMid(List *list, int pos);

void *listRemove(List *list, ListNode *node);

#define LIST_FOREACH(L, F, N, C) ListNode *_node=NULL;\
			ListNode *C=NULL;\
			for(C=_node = L->F; _node!=NULL; C=_node=_node->N)
				

#endif