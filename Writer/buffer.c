#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

/* Inits wbuf */
void wbuf_init(wbuf* mybuffer){
	mybuffer->head = NULL;
	mybuffer->tail = NULL;
	mybuffer->count = 0;
	mybuffer->done = 0;
}

/* Inserts an element */
void wbuf_ins(char *value,wbuf* mybuffer){
	wbe* new_wbe;

	new_wbe = (wbe*)malloc(sizeof(wbe));
	new_wbe->data = malloc(strlen(value)+1);
	strcpy(new_wbe->data,value);
	free(value);
	new_wbe->next = NULL;
	
	if (mybuffer->head == NULL){ /*	if empty list	*/
		mybuffer->head = new_wbe;
		} else {
			mybuffer->tail->next=new_wbe;
		};
	mybuffer->tail = new_wbe;
	mybuffer->count++;
}

/* Extracts the first element of wbuf */
char* wbuf_ext(wbuf* mybuffer){ 
	wbe* ext_wbe;
	char* ret_value;

	ext_wbe = mybuffer->head;
	mybuffer->head = ext_wbe->next;
	if (mybuffer->head == NULL){ /*	if last member	*/
		mybuffer->tail = NULL;
	};
	mybuffer->count--;
	ret_value = malloc(strlen(ext_wbe->data)+1); 
	strcpy(ret_value,ext_wbe->data);
	free(ext_wbe->data);
	free(ext_wbe); /* destroy extracted element	*/
	return ret_value;
}

/* How many elements */
int wbuf_count(wbuf* mybuffer){
	return mybuffer->count;
}
