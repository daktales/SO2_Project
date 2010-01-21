#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include "buffer.h"
#include "fun.h"

#define S_ET 5 /*	Step for new eleborate_data Thread	*/
#define MAX_ET 10 /*	Max ET	*/
#define S_RT 10 /*	Step for new read_dev Thread	*/
#define MAX_RT 5 /*	Max RT	*/
#define S_PT 20 /*	Step for print_data Thread	*/
#define MAX_PT 5 /*	Max PT	*/


#define DEB 1 /*	Enable stdout output	*/

int usleep (unsigned int value);

static wbuf rbuffer; /*	FIFO for raw data	*/
static pthread_mutex_t rmutex;

static wbuf ebuffer; /*	FIFO for elaborated data	*/
static pthread_mutex_t emutex;

/* Threads */

static void *elaborate_data(void *name){
	int stop = 0;
	int count;
	char* data;
	int myname = (int) name;
	while(!stop){
		pthread_mutex_lock(&rmutex);
		while((rbuffer.head==NULL)&&(!rbuffer.done)){
			pthread_cond_wait(&rbuffer.cv, &rmutex);
		}
		if((rbuffer.done)&&(rbuffer.head==NULL)){
			pthread_mutex_unlock(&rmutex);
			stop = 1;
		} else {
			data = wbuf_ext(&rbuffer);
			count = wbuf_count(&rbuffer);
			pthread_mutex_unlock(&rmutex);
			usleep(2000000);
			to_upper(data);
			pthread_mutex_lock(&emutex);
			wbuf_ins(data,&ebuffer);
			if (DEB){fprintf(stdout,"Elaborate %d: %s\n\tRemain %d value pending\n",myname,data,count);};
			pthread_cond_signal(&ebuffer.cv);
			pthread_mutex_unlock(&emutex);
		}
	}
	return NULL;
}

static void *print_data(void *name){
	int stop = 0;
	char* data;
	int count;
	int myname = (int) name;
	while(!stop){
		pthread_mutex_lock(&emutex);
		while((ebuffer.head==NULL)&&(!ebuffer.done)){
			pthread_cond_wait(&ebuffer.cv, &emutex);
		}
		if((ebuffer.done)&&(ebuffer.head==NULL)){
			pthread_mutex_unlock(&emutex);
			stop = 1;
		} else {
			data = wbuf_ext(&ebuffer);
			count = wbuf_count(&ebuffer);
			if(DEB){fprintf(stdout,"Print %d: %s\n\tRemain %d value pending\n",myname,data,count);}
			pthread_mutex_unlock(&emutex);
		}
	}
	return NULL;
}

static void *read_dev(void *name){
	int stop = 0;
	int tmp_todo;
	int myname = (int) name;
	while(!stop){
		pthread_mutex_lock(&rmutex);
		tmp_todo = rbuffer.todo--;
		if(DEB){fprintf(stderr,"Read %d, todo:%d\n",myname,tmp_todo);}
		if (tmp_todo<=0){
			stop = 1;
		} else {
			wbuf_ins(gen_data2(),&rbuffer);
			pthread_cond_signal(&rbuffer.cv);
		}
		pthread_mutex_unlock(&rmutex);
		usleep(1000000);
	}
	return NULL;
}


/* Main */

int main (int argc, char *argv[]){

	int nrt = MAX_RT; /*	read threads	*/
	int net = MAX_ET; /*	elaboration threads	*/
	int npt = MAX_PT; /*	print threads	*/
	int i;
	int tmp_nrt = 0;
	int tmp_net = 0;
	int tmp_npt = 0;
	int tmp_todo = 0;

	pthread_t* rthreads;
	pthread_t* ethreads;
	pthread_t* p_threads;


	/*	Init random seed	*/
	srand(time(NULL));

	/*	Basic arguments check	*/
	if (argc==2){
		tmp_todo = atoi(argv[1]);
	} else {
		fprintf(stderr,"Syntax: %s <number of information>\n",argv[0]);
		return -1;
	}

	/*	Automagically choose best number of thread	*/
	tmp_nrt = (tmp_todo/S_RT)+1;
	tmp_net = (tmp_todo/S_ET)+1;
	tmp_npt = (tmp_todo/S_PT)+1;
	if (tmp_nrt<nrt){nrt = tmp_nrt;}; 
	if (tmp_net<net){net = tmp_net;};
	if (tmp_npt<npt){npt = tmp_npt;};


	/*	Init Mutex	*/
	pthread_mutex_init(&rmutex,NULL);
	pthread_mutex_init(&emutex,NULL);

	/*	Init Buffers	*/
	wbuf_init(&rbuffer);
	rbuffer.todo = tmp_todo; /*	Number of data to create	*/
	wbuf_init(&ebuffer);

	/*	Init Thread's arrays	*/
	rthreads = (pthread_t*)calloc((size_t)nrt,sizeof(pthread_t));
	ethreads = (pthread_t*)calloc((size_t)net,sizeof(pthread_t));
	p_threads = (pthread_t*)calloc((size_t)npt,sizeof(pthread_t));

	/*	Creating Threads	*/
	for(i=0;i<nrt;i++){
		pthread_create(&rthreads[i], NULL, read_dev, (void *)i);
	}
	for(i=0;i<net;i++){
		pthread_create(&ethreads[i], NULL, elaborate_data, (void*)i);
	}
	for(i=0;i<npt;i++){
		pthread_create(&p_threads[i], NULL, print_data, (void*)i);
	}

	/*	Closing */

	/*		Waiting for read_dev Thread	*/
	for(i=0;i<nrt;i++){
		pthread_join(rthreads[i], NULL);
	}
	pthread_mutex_lock(&rmutex);
	rbuffer.done = 1;
	pthread_cond_broadcast(&rbuffer.cv); /*	Release all ethreads	*/
	pthread_mutex_unlock(&rmutex);

	/*	Waiting for elaborate_data Thread	*/
	for(i=0;i<net;i++){
		pthread_join(ethreads[i], NULL);
	}
	pthread_mutex_lock(&emutex);
	ebuffer.done = 1;
	pthread_cond_broadcast(&ebuffer.cv); /*	Release all p_threads	*/
	pthread_mutex_unlock(&emutex);

	/*	Waiting for print_data Thread	*/
	for(i=0;i<npt;i++){
		pthread_join(p_threads[i], NULL);
	}

	/* Cleanup */

	pthread_mutex_destroy(&rmutex);
	pthread_mutex_destroy(&emutex);

	wbuf_destroy(&rbuffer);
	wbuf_destroy(&ebuffer);

	free(rthreads);
	free(ethreads);
	free(p_threads);

	return 0;
}
