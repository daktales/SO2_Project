#ifndef _KBUF_H_
#define _KBUF_H_

struct kb_node {
	char *data;
	struct kb_node *next;
};

struct kb{ 
	struct kb_node *head;
	struct kb_node *tail;
	int count;
};

inline int kb_init (struct kb *kbuf);

int kb_isfull (struct kb *kbuf,int max);

int kb_isempty (struct kb *kbuf);

/*	Extract an element from kb	*/
inline int kb_push (char *data,struct kb *kbuf);

/*	Insert an element into kb	*/
inline int kb_pop(char *data,struct kb *kbuf);

/* Populate an array with all data lenght */
inline void kb_scan(int *data,struct kb *kbuf);

#endif
