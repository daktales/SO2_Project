#include <linux/module.h>

#include "kbuf.h"
#include "stat.h"

inline void ds_init(struct ds *ds_var,int dim){
	ds_var->ds_dim = dim;
	ds_var->ds_list = (int *) kmalloc(sizeof(int)*dim,GFP_KERNEL);
	}

inline void ds_reset(struct ds *ds_var){
	kfree(ds_var->ds_list);
	ds_var->ds_dim = 0;
}

inline int ds_populate(struct ds *ds_var, struct kb *kb_var){
	int len = kb_var->count;
	if (len){
		ds_init(ds_var,len);
		kb_scan(ds_var->ds_list,kb_var);
	}
	return len;
}
	
	

inline int ds_med(struct ds *ds_var){
	int tmp = 0;
	int i;
	for(i=0;i<(ds_var->ds_dim);i++){
		tmp = tmp + ds_var->ds_list[i];
	}
	tmp = tmp / ds_var->ds_dim;
	return tmp;
}

inline int ds_max(struct ds *ds_var){
	int tmp = 0;
	int i;
	for(i=0;i<(ds_var->ds_dim);i++){
		if (tmp<(ds_var->ds_list[i])){
			tmp = (ds_var->ds_list[i]);
		}
	}
	return tmp;
}

inline int ds_min(struct ds *ds_var,int max){
	int tmp = max;
	int i;
	if (!max) {
		return 0;
	}
	for(i=0;i<(ds_var->ds_dim);i++){
		if (tmp>(ds_var->ds_list[i])){
			tmp = (ds_var->ds_list[i]);
		}
	}
	return tmp;
}
