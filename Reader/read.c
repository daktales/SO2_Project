#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#include "buffer.h"
#include "fun.h"

#define S_ET 5 /*	Step for new eleborate_data Thread	*/
#define MAX_ET 10 /*	Max ET	*/
#define S_RT 1 /*	Step for new read_dev Thread	*/
#define MAX_RT 1 /*	Max RT	*/
#define S_PT 20 /*	Step for print_data Thread	*/
#define MAX_PT 5 /*	Max PT	*/

#define DEV "/dev/mydev"

#define DEB 1 /*	Enable stdout output	*/

int usleep (unsigned int value);

static wbuf rbuffer; /*	FIFO for raw data	*/
static pthread_mutex_t rmutex;
static pthread_cond_t rcv;

static wbuf ebuffer; /*	FIFO for elaborated data	*/
static pthread_mutex_t emutex;
static pthread_cond_t ecv;

static int fd; /* Variable for device	*/

static pthread_t* rthreads;
static int nrt;
static pthread_t* ethreads;
static int net;
static pthread_t* p_threads;
static int npt;

static int read_loop; /* protected with rmutex */
static int todo;

/* Threads */

static void *elaborate_data(void *name){
	int stop = 0;
	int count;
	char* data;
	int myname = (int) name;
	while(!stop){
		pthread_mutex_lock(&rmutex);
		while((rbuffer.head==NULL)&&(!rbuffer.done)){
			pthread_cond_wait(&rcv, &rmutex);
		}
		if((rbuffer.done)&&(rbuffer.head==NULL)){
			pthread_mutex_unlock(&rmutex);
			stop = 1;
		} else {
			data = wbuf_ext(&rbuffer);
			count = wbuf_count(&rbuffer);
			if (DEB){fprintf(stdout,"ELABORATE T%d (todo:%d): %s\n",myname,count,data);}
			pthread_mutex_unlock(&rmutex);
			usleep(2000000);
			to_upper(data);
			pthread_mutex_lock(&emutex);
			wbuf_ins(data,&ebuffer);
			pthread_cond_signal(&ecv);
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
			pthread_cond_wait(&ecv, &emutex);
		}
		if((ebuffer.done)&&(ebuffer.head==NULL)){
			pthread_mutex_unlock(&emutex);
			stop = 1;
		} else {
			data = wbuf_ext(&ebuffer);
			count = wbuf_count(&ebuffer);
			fprintf(stdout,"PRINT T%d (todo:%d): %s\n",myname,count,data);
			pthread_mutex_unlock(&emutex);
		}
	}
	return NULL;
}

static void *read_dev(void *name){
	int stop = 0;
	int tmp_todo;
	int myname = (int) name;
	char *tmp = (char*) malloc((size_t)40);
	while(!stop){
		pthread_mutex_lock(&rmutex);
		tmp_todo = (read_loop) ? 1 : todo--;
		if (tmp_todo<=0){
			stop = 1;
		} else {
			if (read(fd,tmp,40)){
				fprintf(stderr,"Read from device failed\n");
			} else {
				if (!strcmp(tmp,"00\0")){
					stop = 1;
					read_loop = 0;
					if(DEB){fprintf(stdout,"READ T%d : Last Read\n",myname);}
				} else {
					wbuf_ins(tmp,&rbuffer);
					if(DEB){fprintf(stdout,"READ T%d (todo: %d): %s\n",myname,tmp_todo,tmp);}
				}
			}
			pthread_cond_signal(&rcv);
		}
		pthread_mutex_unlock(&rmutex);
		usleep(1000000);
	}
	return NULL;
}

/* Wait until a kind of thread die */
void wait_for(pthread_t *t_array, int t_num){
	int i;
	for(i=0;i<t_num;i++){
		pthread_join(t_array[i], NULL);
	}
}

/*	Release pending threads	*/
void release_t(pthread_mutex_t *mut,wbuf *wb,pthread_cond_t *cv){
	pthread_mutex_lock(mut);
	wb->done = 1; /* Set "writing done" on the buffer */
	pthread_cond_broadcast(cv); /* Awake all T waiting on the empty buffer */
	pthread_mutex_unlock(mut);
}


/* Destroy mutex, c.v. and free threads arrays */
void cleanup(){
	pthread_mutex_destroy(&rmutex);
	pthread_mutex_destroy(&emutex);

	pthread_cond_destroy(&rcv);
	pthread_cond_destroy(&ecv);

	free(rthreads);
	free(ethreads);
	free(p_threads);
}

/* Open device */
void dev_open(int *f){
	int status;
	status = *f = open(DEV,O_RDONLY);
	if (status == -1){
		fprintf(stderr,"Error opening device %s\n",DEV);
	 	exit(1);
	}
	if(DEB){printf("Device Opened\n");};
}

/* Close device */
void dev_close(int *f){
	int status;
	status = close(*f);
	if (status == -1){
		fprintf(stderr,"Errore closing device %s\n",DEV);
		cleanup();
		exit(1);
	}
	if(DEB){printf("Device Closed\n");}
}

/*	Intercepts CTRL-C	*/
void catcher_SIGINT(int signum) {
	printf("Closing, please wait..(signum: %d)\n",signum);
	/* Using rmutex because create_data check todo and read loop variables
	 * within rmutex mutual exclusion
	 */
	pthread_mutex_lock(&rmutex);
	todo = 0;
	read_loop = 0;
	pthread_mutex_unlock(&rmutex);

	if (&fd != NULL){
		wait_for(rthreads,nrt);
		if(DEB){printf("CTRL-C: All Read threads has stopped\n");}
		release_t(&rmutex,&rbuffer,&rcv);
		if(DEB){printf("CTRL-C: Raw buffer closed\n");}
		wait_for(ethreads,net);
		if(DEB){printf("CTRL-C: All Elaborate threads has stopped\n");}
		release_t(&emutex,&ebuffer,&ecv);
		if(DEB){printf("CTRL-C: Elaborate buffer closed\n");}
		wait_for(p_threads,npt);
		if(DEB){printf("CTRL-C: All Print threads has stopped\n");}
		dev_close(&fd);
	}
	cleanup();
	exit(1);
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

	/*	Init random seed	*/
	srand(time(NULL));

	/*	Basic arguments check	*/
	if (argc==2){
		todo = atoi(argv[1]);
	} else {
		fprintf(stderr,"Syntax: %s <number of information>\n",argv[0]);
		return -1;
	}

	/* Set read loop	*/
	read_loop = (todo) ? 0 : 1;

	/* handle SIGINT	*/
	signal(SIGINT, catcher_SIGINT);

	/* Open Device	*/
	dev_open(&fd);

	
	/*	Automagically choose best number of thread	*/
	if (!read_loop){
		tmp_nrt = (todo/S_RT)+1;
		tmp_net = (todo/S_ET)+1;
		tmp_npt = (todo/S_PT)+1;
		nrt = (tmp_nrt<MAX_RT) ? tmp_nrt : MAX_RT; /*	read threads	*/
		net = (tmp_net<MAX_ET) ? tmp_net : MAX_ET; /*	elaboration threads	*/
		npt = (tmp_npt<MAX_PT) ? tmp_npt : MAX_PT; /*	print threads	*/
	} else {
		nrt = MAX_RT;
		net = MAX_ET;
		npt = MAX_PT;
	}

	/*	Init Mutex	*/
	pthread_mutex_init(&rmutex,NULL);
	pthread_mutex_init(&emutex,NULL);

	/*	Init Condition Variables	*/
	pthread_cond_init(&rcv,NULL);
	pthread_cond_init(&ecv,NULL);

	/*	Init Buffers	*/
	wbuf_init(&rbuffer);
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
	wait_for(rthreads,nrt);
	release_t(&rmutex,&rbuffer,&rcv);

	/*	Waiting for elaborate_data Thread	*/
	wait_for(ethreads,net);
	release_t(&emutex,&ebuffer,&ecv);

	/*	Waiting for print Thread	*/
	wait_for(p_threads,npt);

	/*	Close device	*/
	dev_close(&fd);
	
	/* * Cleanup * */
	cleanup();

	return 0;
}
