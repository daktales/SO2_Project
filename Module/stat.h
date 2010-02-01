#ifndef _STAT_H_
#define _STAT_H_


struct ds {
	int ds_dim;
	int *ds_list;
};

inline void ds_init(struct ds *ds_var,int dim);

inline void ds_reset(struct ds *ds_var);

inline int ds_populate(struct ds *ds_var, struct kb *kb_var);

inline int ds_med(struct ds *ds_var);

inline int ds_max(struct ds *ds_var);

inline int ds_min(struct ds *ds_var,int max);

#endif
