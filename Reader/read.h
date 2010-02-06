#ifndef _READ_H_
#define _READ_H_

#include "buffer.h"

#define S_ET 5 /*	Step for new eleborate_data Thread	*/
#define MAX_ET 4 /*	Max ET	*/
#define S_RT 1 /*	Step for new read_dev Thread	*/
#define MAX_RT 1 /*	Max RT	*/
#define S_PT 10 /*	Step for print_data Thread	*/
#define MAX_PT 2 /*	Max PT	*/

#define DEV "/dev/mydev"

#define DEB 1 /*	Enable stdout output	*/
#define MAX_RETRY 5 /*	Number of write retry	*/
#define READ_BYTE 40 /* Byte to read from device */

static void *elaborate_data(void *name);

static void *read_dev(void *name);

static void *print_data(void *name);

void wait_for(pthread_t *t_array, int t_num);

void release_t(pthread_mutex_t *mut, wbuf *wb,pthread_cond_t *cv);

void cleanup();

void dev_open(int *f);

void dev_close(int *f);

void catcher_SIGINT(int signum);

#endif
