#ifndef _WRITE_H_
#define _WRITE_H_

#include "buffer.h"

#define S_ET 5 /*	Step for new eleborate_data Thread	*/
#define MAX_ET 10 /*	Max ET	*/
#define S_CT 10 /*	Step for new create_data Thread	*/
#define MAX_CT 4 /*	Max CT	*/
#define S_WT 10 /*	Step for new write_dev Thread	*/
#define MAX_WT 4 /*	Max WT	*/
#define DEV "/dev/mydev"

#define DEB 1 /*	Enable stdout output	*/
#define MAX_RETRY 5 /*	Number of write retry	*/

static void *elaborate_data(void *name);

static void *write_dev(void *name);

static void *create_data(void *name);

void wait_for(pthread_t *t_array, int t_num);

void release_t(pthread_mutex_t *mut, wbuf *wb,pthread_cond_t *cv);

void cleanup();

void dev_open(int *f);

void dev_close(int *f);

int dev_done(int *f);

void catcher_SIGINT(int signum);

#endif

