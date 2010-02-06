#ifndef _STAT_H_
#define _STAT_H_

#define KB_OCC 75 /* Kernel buffer Occupation in % */

struct ds {
	/* Stat section */
	int ds_dim;
	int *ds_list;
	/* Alloc section */
	int ds_kb_dim;
	int ds_kb_max;
	int ds_kb_sum;
};

/* Division */
int my_div(int a, int b);

/* Get % ( a : x = b : 100) */
inline int calculate_per(int a, int b);

/* Init struct's stat section */
void ds_init_stat(struct ds *ds_var,int dim);

/* Init struct's alloc section */
void ds_init_alloc(struct ds *ds_var,int max);

/* Reset struct's stat section */
void ds_reset_stat(struct ds *ds_var);

/* Reset struct's alloc section */
void ds_reset_alloc(struct ds *ds_var);

/* Add value to alloc section */
void ds_add_kb_value(struct ds *ds_var, int value);

/* Get kbuf occupation in % */
int ds_get_kb_occupation(struct ds *ds_var);

/* Set kbuf max size */
void ds_set_kb_occupation(struct ds *ds_var, int value);

/* Populate stat's struct	*/
int ds_populate(struct ds *ds_var, struct kb *kb_var);

/* average */
int ds_med(struct ds *ds_var);

/* max */
int ds_max(struct ds *ds_var);

/* min */
int ds_min(struct ds *ds_var,int max);

#endif
