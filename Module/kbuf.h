#ifndef _KBUF_H_
#define _KBUF_H_

struct kb_node {
	char *data;
	struct kb_node *next;
};

struct kb{ 
	struct kb_node *head;
	struct kb_node *tail;
	short count;
	short maxnode;
};

int kb_init (struct kb *kbuf);

int kb_isfull (struct kb *kbuf);

int kb_isempty (struct kb *kbuf);

int kb_push (char *data,struct kb *kbuf);

int kb_pop(char *data,struct kb *kbuf);

#endif
