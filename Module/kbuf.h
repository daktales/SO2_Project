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
};

inline int kb_init (struct kb *kbuf);

inline int kb_isempty (struct kb *kbuf);

/*	Extract an element from kb	*/
int kb_push (char *data,struct kb *kbuf);

/*	Insert an element into kb	*/
int kb_pop(char *data,struct kb *kbuf);

/*	Split a long string in substring using a kb buffer	*/
void kb_split(char *data,int maxl,struct kb *kbuf);

/* Join substring from a kb buffer into a string	*/
void kb_join(char *data,int maxl, struct kb *kbuf);
#endif
