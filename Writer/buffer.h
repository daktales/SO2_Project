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
	int todo;
};
typedef struct wbuf_struct wbuf;

void wbuf_init(wbuf* mybuffer);

void wbuf_ins(char *value,wbuf* mybuffer);

char* wbuf_ext(wbuf* mybuffer);

int wbuf_count(wbuf* mybuffer);

#endif
