#include <assert.h>

#include "linkedlist.h"

List *listCreate(){
	return calloc(1,sizeof(List));
}

void listDestroy(List *list){
	LIST_FOREACH(list, first, next, cur){
		if (cur->prev){
			free(cur->prev);
		}
	}
	free(list->last);
	free(list);
}

void listClear(List *list){
	LIST_FOREACH(list, first, next, cur){
		free(cur->value);
	}
}

void listClearDestroy(List *list){
	listClear(list);
	listDestroy(list);
}

void listPushEnd(List *list, void *value){
	ListNode *node=calloc(1,sizeof(ListNode));
	assert(node!=NULL);

	node->value=value;

	if (list->last==NULL){
		list->first=node;
		list->last=node;
	} else {
		list->last->next=node;
		node->prev=list->last;
		list->last=node;
	}
	list->count++;
}

void *listPopEnd(List *list){
	ListNode *node=list->last;
	return node!=NULL ? listRemove(list, node) : NULL;
}

void listPushFront(List *list){
	ListNode *node=calloc(1,sizeof(ListNode));
	assert(node!=NULL);

	node->value=value;

	if (list->first==NULL){
		list->first=node;
		list->last=node;
	} else {
		list->first->prev=node;
		node->next=list->first;
		list->first=node;
	}
	list->count++;	
}

void *listPopFront(List *list){
	ListNode *node=list->first;
	return node!=NULL ? listRemove(list, node) : NULL;
}

void *listRemove(List *list, ListNode *node){
	void *result=NULL;

	//TODO check for empty

	if (node==list->first && node==list->last){
		list->first=NULL;
		list->last=NULL;
	} else if (node==list->first){
		list->first=node->next;
		list->first->prev=NULL;
	} else if (node==list->last){
		list->last=node->prev;
		list->last->next=NULL;
	} else {
		ListNode *after=node->next;
		ListNode *before=node->prev;
		after->prev=before;
		before->next=after;
	}
	list->count--;
	result=node->value;
	free(node);

	return result;
}