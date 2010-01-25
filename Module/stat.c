#include "stat.h"

inline int ds_isfull(struct ds *ds_var){
	if (ds_var->ds_index < 9){
		return 0;
	}
	return 1;
}

inline void ds_init(struct ds *ds_var){
	ds_var->ds_index = 0;
	}
