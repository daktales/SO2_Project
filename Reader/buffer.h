#ifndef _BUFFER_H_
#define _BUFFER_H_

/* Linked List Element*/
struct wbe_struct{
	char* data;
	struct wbe_struct* next;
};
typedef struct wbe_struct wbe;

/* Linked List */
struct wbuf_struct{
	wbe* head;
	wbe* tail;
	int count;
	int done;
};
typedef struct wbuf_struct wbuf;


/* Inits wbuf */
void wbuf_init(wbuf* mybuffer);

/* Inserts an element */
void wbuf_ins(char *value,wbuf* mybuffer);

/* Extracts the first element of wbuf */
char* wbuf_ext(wbuf* mybuffer);

/* How many elements */
int wbuf_count(wbuf* mybuffer);

#endif
