#ifndef _STAT_H_
#define _STAT_H_


struct ds {
	int ds_list[10];
	int ds_index;
};

inline int ds_isfull(struct ds *ds_var);

inline void ds_init(struct ds *ds_var);

#endif
