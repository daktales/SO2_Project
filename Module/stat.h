#ifndef _STAT_H_
#define _STAT_H_

#define KB_OCC 75 /* Kernel buffer Occupation in % */

struct ds {
	int ds_dim;
	int *ds_list;
	int ds_kb_dim;
	int ds_kb_max;
	int ds_kb_sum;
};

int my_div(int a, int b);

inline int calculate_per(int a, int b);

void ds_init_stat(struct ds *ds_var,int dim);

void ds_init_alloc(struct ds *ds_var,int max);

void ds_reset_stat(struct ds *ds_var);

void ds_reset_alloc(struct ds *ds_var);

void ds_add_kb_value(struct ds *ds_var, int value);

int ds_get_kb_occupation(struct ds *ds_var);

void ds_set_kb_occupation(struct ds *ds_var, int value);

int ds_populate(struct ds *ds_var, struct kb *kb_var);

int ds_med(struct ds *ds_var);

int ds_max(struct ds *ds_var);

int ds_min(struct ds *ds_var,int max);

#endif
