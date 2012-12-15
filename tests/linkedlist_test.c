#include <assert.h>
#include <stdio.h>

#include "linkedlist.h"

List *list = NULL;
char *test1="test1 data";
char *test2="test2 data";

typedef struct{
	int a;
	int b;
}str_test;


void testCreate();
void testDestroy();
void testClear();

int main(int argc, char const *argv[])
{
	str_test *test1=malloc(sizeof(str_test));
	str_test *test2=malloc(sizeof(str_test));
	
	testCreate();
	listPushEnd(list, test1);
	listPushEnd(list, test2);
	printf("listCount %i\n", listCount(list));

	printf("List first location %p\n", listFirst(list));
	printf("List last location %p\n", listLast(list));

	listClear(list);
	listDestroy(list);

	return 0;
}


void testCreate(){
	list=listCreate();
	assert(list!=NULL);
	printf("List created at memory location %p\n", list);
	printf("first node %p\n", list->first);

}

void testDestroy(){
	listClearDestroy(list);
	printf("List destroyed at memory location %p\n", list);
}

