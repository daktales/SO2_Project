#include <linux/module.h>

#include "kbuf.h"
#include "stat.h"


int my_div(int a, int b){
	int res = a % b;
	int mod = a / b;
	if (res >= (b/2)){
		mod++;
	}
	return mod;
}

int calculate_per(int a, int b){
	return my_div((a*100),b);
}


void ds_init_stat(struct ds *ds_var,int dim){
	ds_var->ds_dim = dim;
	ds_var->ds_list = (int *) kmalloc(sizeof(int)*dim,GFP_KERNEL);
}

void ds_init_alloc(struct ds *ds_var,int max){
	ds_var->ds_kb_dim = 0;
	ds_var->ds_kb_sum = 0;
	ds_var->ds_kb_max = max;
}

void ds_reset_stat(struct ds *ds_var){
	kfree(ds_var->ds_list);
	ds_var->ds_dim = 0;
}

void ds_reset_alloc(struct ds *ds_var){
	ds_var->ds_kb_dim = 0;
	ds_var->ds_kb_sum = 0;
}

void ds_add_kb_value(struct ds *ds_var, int value){
	ds_var->ds_kb_dim++;
	ds_var->ds_kb_sum = ds_var->ds_kb_sum + value;
}

int ds_get_kb_occupation(struct ds *ds_var){
	int a = my_div((ds_var->ds_kb_sum),(ds_var->ds_kb_dim));
	int b = (ds_var->ds_kb_max);
	int res = calculate_per(a,b);
	return res;
}

void ds_set_kb_occupation(struct ds *ds_var, int value){
	/* Min 2 otherwise stuck on 1 */
	ds_var->ds_kb_max = (value<=1) ? 2 : my_div((value*100),KB_OCC);
}

int ds_populate(struct ds *ds_var, struct kb *kb_var){
	int len = kb_var->count;
	if (len){
		ds_init_stat(ds_var,len);
		kb_scan(ds_var->ds_list,kb_var);
	}
	return len;
}

int ds_med(struct ds *ds_var){
	int tmp = 0;
	int i;
	for(i=0;i<(ds_var->ds_dim);i++){
		tmp = tmp + ds_var->ds_list[i];
	}
	tmp = tmp / ds_var->ds_dim;
	return tmp;
}

int ds_max(struct ds *ds_var){
	int tmp = 0;
	int i;
	for(i=0;i<(ds_var->ds_dim);i++){
		if (tmp<(ds_var->ds_list[i])){
			tmp = (ds_var->ds_list[i]);
		}
	}
	return tmp;
}

int ds_min(struct ds *ds_var,int max){
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
