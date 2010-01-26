#ifndef _STAT_H_
#define _STAT_H_


struct ds {
	int ds_dim;
	int *ds_list[];
};

inline void ds_init(struct ds *ds_var,int dim);

float ds_med(struct ds *ds_var);

int ds_max(struct ds *ds_var);

int ds_min(struct ds *ds_var);

#endif
