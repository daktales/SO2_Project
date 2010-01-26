#include <linux/module.h>
#include "stat.h"

inline void ds_init(struct ds *ds_var,int dim){
	ds_var->ds_dim = dim;
	*(ds_var->ds_list) = (int *) kcalloc((size_t) dim, sizeof(int),GFP_KERNEL);
	}

inline void ds_destroy(struct ds *ds_var){
	kfree(ds_var->ds_list);
}

float ds_med(struct ds *ds_var){
	float tmp = 0.0;
	int i;
	for(i=0;i<(ds_var->ds_dim);i++){
		tmp = tmp + (float) *(ds_var->ds_list[i]);
	}
	tmp = tmp / ds_var->ds_dim;
	return tmp;
}

int ds_max(struct ds *ds_var){
	int tmp = 0;
	int i;
	for(i=0;i<(ds_var->ds_dim);i++){
		if (tmp<*(ds_var->ds_list[i])){
			tmp = *(ds_var->ds_list[i]);
		}
	}
	return tmp;
}

int ds_min(struct ds *ds_var){
	int tmp = 0;
	int i;
	for(i=0;i<(ds_var->ds_dim);i++){
		if (tmp>*(ds_var->ds_list[i])){
			tmp = *(ds_var->ds_list[i]);
		}
	}
	return tmp;
}
