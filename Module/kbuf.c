#include <linux/module.h>
#include "kbuf.h"

#define DEF_NODE 10;

int kb_init (struct kb *kbuf){
	kbuf->head = NULL;
	kbuf->tail = NULL;
	kbuf->count = 0;
	kbuf->maxnode = DEF_NODE;
	return 0;
}

int kb_isfull (struct kb *kbuf){
	if (kbuf->count == kbuf->maxnode){
		return 1;
	}
	return 0;
}

int kb_isempty (struct kb *kbuf){
	if (kbuf->head == NULL){
		return 1;
	}
	return 0;
}

int kb_push (char *data,struct kb *kbuf){
	struct kb_node *node = kmalloc(sizeof(struct kb_node),GFP_USER);

	node->data = data;
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

int kb_pop(char *data,struct kb *kbuf){
	struct kb_node *node;

	node = kbuf->head;
	kbuf->head = node->next;
	if (kbuf->head == NULL){ /*	if last member	*/
		kbuf->tail = NULL;
	};
	kbuf->count--;
	data = node->data;
	kfree(node); /* destroy extracted element	*/
	

	return 0;
}
