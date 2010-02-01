#include <linux/module.h>
#include "kbuf.h"

inline int kb_init (struct kb *kbuf){
	kbuf->head = NULL;
	kbuf->tail = NULL;
	kbuf->count = 0;
	return 0;
}

int kb_isempty (struct kb *kbuf){
	if (kbuf->head == NULL){
		return 1;
	}
	return 0;
}

int kb_isfull (struct kb *kbuf,int max){
	if (kbuf->count < max){
		return 0;
	}
	return 1;
}

inline int kb_push (char *data,struct kb *kbuf){
	struct kb_node *node = kmalloc(sizeof(struct kb_node),GFP_KERNEL);
	node->data = kmalloc(strlen(data)+1,GFP_KERNEL);
	
	strcpy(node->data,data);
	node->next = NULL;
	if (kbuf->head == NULL){
		kbuf->head = node;
	} else {
		kbuf->tail->next = node;
	}
	kbuf->tail = node;
	kbuf->count++;

	return 0;
}

inline int kb_pop(char *data,struct kb *kbuf){
	struct kb_node *node;
	if (kbuf->head == NULL){	/* if empty	*/
		return 1;
	}
	node = kbuf->head;
	kbuf->head = node->next;
	if (kbuf->head == NULL){ /*	if last member	*/
		kbuf->tail = NULL;
	};
	kbuf->count--;
	strcpy(data,node->data);
	kfree(node->data);
	kfree(node); /* destroy extracted element	*/
	

	return 0;
}

inline void kb_scan(int *data,struct kb *kbuf){
	struct kb_node *p = kbuf->head;
	int i = 0;
	while (p != NULL){
		data[i] = strlen(p->data);
		i++;
		p = p->next;
	}
}
