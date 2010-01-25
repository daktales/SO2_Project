#include <linux/module.h>
#include "kbuf.h"

#define DEF_NODE 10;


inline int kb_init (struct kb *kbuf){
	kbuf->head = NULL;
	kbuf->tail = NULL;
	kbuf->count = 0;
	kbuf->maxnode = DEF_NODE;
	return 0;
}

inline int kb_isfull (struct kb *kbuf){
	if (kbuf->count == kbuf->maxnode){
		return 1;
	}
	return 0;
}

inline int kb_isempty (struct kb *kbuf){
	if (kbuf->head == NULL){
		return 1;
	}
	return 0;
}

int kb_push (char *data,struct kb *kbuf){
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

int kb_pop(char *data,struct kb *kbuf){
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

/* REDO using strncpy	*/
void kb_split(char *data,int maxl,struct kb *kbuf){

	int i = 0;
	int j = 0;
	int len = strlen(data)+1;
	int tmpl = (len < maxl) ? len : maxl;
	char *tmp;
	tmp = kmalloc(tmpl,GFP_KERNEL);

	while (data[i] != '\0'){
		tmp[j] = data[i];
		j++;
		i++;
		if (j == maxl){
			kb_push(tmp,kbuf);
			j=0;
			if ((len-i)<maxl){
				tmpl = len-i;
				tmp = kmalloc(tmpl,GFP_KERNEL);
			}
		}
	}
	tmp[j] = data[i];
	kb_push(tmp,kbuf);
	kfree(tmp);
	}

void kb_join(char *data,int maxl, struct kb *kbuf){
	char *tmp = kmalloc(maxl,GFP_KERNEL);
	char *tmp_full = kmalloc((maxl*(kbuf->count)),GFP_KERNEL);
	
	while(kb_isempty(kbuf)){
		kb_pop(tmp,kbuf);
		strncat(tmp_full,tmp,strlen(tmp));/*	strncat append a \0 and strlen do not count \0	*/
	}
	data = kmalloc(strlen(tmp_full)+1,GFP_KERNEL); /*	sort of mem_trim(data)	*/
	strcpy(data,tmp_full);
	kfree(tmp);
	kfree(tmp_full);
}
