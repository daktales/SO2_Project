#ifndef _KBUF_H_
#define _KBUF_H_

/*	Kbuf node	*/
struct kb_node {
	char *data;
	struct kb_node *next;
};

/*	Kbuf struct, linked list	*/
struct kb{ 
	struct kb_node *head;
	struct kb_node *tail;
	int count;
};

/*	Initialize Kbuf Struct	*/
int kb_init (struct kb *kbuf);

/*	Check if Kbuf is full (number of node >= max)	*/
int kb_isfull (struct kb *kbuf,int max);

/*	Check if Kbuf is empty	*/
int kb_isempty (struct kb *kbuf);

/*	Extract an element from kb	*/
int kb_push (char *data,struct kb *kbuf);

/*	Insert an element into kb	*/
int kb_pop(char *data,struct kb *kbuf);

/*	Populate an array with all data's lenghts */
void kb_scan(int *data,struct kb *kbuf);

#endif
