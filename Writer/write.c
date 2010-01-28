#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#include "buffer.h"
#include "fun.h"

#define S_ET 5 /*	Step for new eleborate_data Thread	*/
#define MAX_ET 2 /*	Max ET	*/
#define S_CT 10 /*	Step for new create_data Thread	*/
#define MAX_CT 2 /*	Max CT	*/
#define S_WT 1 /*	Step for new write_dev Thread	*/
#define MAX_WT 2 /*	Max WT	*/
#define DEV "/dev/mydev"

#define DEB 1 /*	Enable stdout output	*/
#define MAX_RETRY 5 /*	Number of write retry	*/

int usleep (unsigned int value);

static wbuf rbuffer; /*	FIFO for raw data	*/
static pthread_mutex_t rmutex;

static wbuf ebuffer; /*	FIFO for elaborated data	*/
static pthread_mutex_t emutex;

static int fd; /* Variable for device	*/

static pthread_t* cthreads;
static int nct;
static pthread_t* ethreads;
static int net;
static pthread_t* wthreads;
static int nwt;

static int write_loop; /* protected with rmutex */

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
			to_lower(data);
			pthread_mutex_lock(&emutex);
			wbuf_ins(data,&ebuffer);
			if (DEB){fprintf(stdout,"Elaborate %d: %s\n\tRemain %d value pending\n",myname,data,count);};
			pthread_cond_signal(&ebuffer.cv);
			pthread_mutex_unlock(&emutex);
		}
	}
	return NULL;
}

static void *write_dev(void *name){
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
			if(DEB){fprintf(stdout,"Write %d: %s\n\tRemain %d value pending\n",myname,data,count);}
			while (write(fd,data,strlen(data)+1)){  /*	write to device, strlen() do not count \0 so +1 is needed*/
				fprintf(stderr,"FAIL writing on device");
			}
			pthread_mutex_unlock(&emutex);
		}
	}
	return NULL;
}


static void *create_data(void *name){
	int stop = 0;
	int tmp_todo;
	int myname = (int) name;
	while(!stop){
		pthread_mutex_lock(&rmutex);
		tmp_todo = (write_loop) ? 1 : rbuffer.todo--;
		if(DEB){fprintf(stderr,"Create %d, todo:%d\n",myname,tmp_todo);}
		if (tmp_todo<=0){
			stop = 1;
		} else {
			wbuf_ins(gen_data(),&rbuffer);
			pthread_cond_signal(&rbuffer.cv);
		}
		pthread_mutex_unlock(&rmutex);
		usleep(1000000);
	}
	return NULL;
}

void wait_for(pthread_t *t_array, int t_num){
	int i;
	for(i=0;i<t_num;i++){
		pthread_join(t_array[i], NULL);
	}
}

void release_t(pthread_mutex_t *mut,wbuf *wb){
	pthread_mutex_lock(mut);
	wb->done = 1;
	pthread_cond_broadcast(&(wb->cv)); /*	Release all pending threads	*/
	pthread_mutex_unlock(mut);
}

/* */

void cleanup(){
	pthread_mutex_destroy(&rmutex);
	pthread_mutex_destroy(&emutex);

	wbuf_destroy(&rbuffer);
	wbuf_destroy(&ebuffer);

	free(cthreads);
	free(ethreads);
	free(wthreads);
}



void dev_open(int *f){
	int status;
	status = *f = open(DEV,O_WRONLY);
	if (status == -1){
		fprintf(stderr,"Error opening device %s\n",DEV);
	 	exit(1);
	}
}

void dev_close(int *f){
	int status;
	status = close(*f);
	if (status == -1){
		fprintf(stderr,"Errore closing device %s\n",DEV);
		cleanup();
		exit(1);
	}
}

int dev_done(int f){
	int i = 0;
	while((i<=MAX_RETRY)&&(write(f,"00",2))){
		if(DEB){fprintf(stderr,"Error sending \'done\' command\n");};
		i++;
	}
	if (i==MAX_RETRY){
		fprintf(stderr,"Critical Error sending \'done\' command\n");
		return 1;
	}
	return 0;
}

void catcher_SIGINT(int signum) {
	printf("Closing, please wait..\n");
	pthread_mutex_lock(&rmutex);
	rbuffer.todo = 0;
	write_loop = 0;
	pthread_mutex_unlock(&rmutex);
	
	release_t(&rmutex,&rbuffer);
	if(DEB){printf("CTRL-Z: Closing Raw buffer\n");}
	wait_for(cthreads,nct);
	if(DEB){printf("CTRL-Z: All Create threads has stopped\n");}
	wait_for(ethreads,net);
	if(DEB){printf("CTRL-Z: All Elaborate threads has stopped\n");}
	release_t(&emutex,&ebuffer);
	if(DEB){printf("CTRL-Z: Closing Elaborate buffer\n");}
	wait_for(wthreads,nwt);
	if(DEB){printf("CTRL-Z: All Write threads has stopped\n");}

	if (&fd != NULL){
		dev_done(fd);
		dev_close(&fd);
	}
	cleanup();
	exit(1);
	}


/* Main */

int main (int argc, char *argv[]){

	int i;
	int tmp_nct = 0;
	int tmp_net = 0;
	int tmp_nwt = 0;
	int tmp_todo = 0;

	/*	Init random seed	*/
	srand(time(NULL));

	/*	Basic arguments check	*/
	if (argc==2){
		tmp_todo = atoi(argv[1]);
	} else {
		fprintf(stderr,"Syntax: %s <number of information>\n",argv[0]);
		return -1;
	}

	/* Set write loop	*/

	write_loop = (tmp_todo) ? 0 : 1;

	/* handle SIGINT	*/
	signal(SIGINT, catcher_SIGINT);

	/* Open Device	*/

	dev_open(&fd);

	/*	Automagically choose best number of thread	*/
	tmp_nct = (tmp_todo/S_CT)+1;
	tmp_net = (tmp_todo/S_ET)+1;
	tmp_nwt = (tmp_todo/S_WT)+1;
	nct = (tmp_nct<MAX_CT) ? tmp_nct : MAX_CT; /*	creation threads	*/
	net = (tmp_net<MAX_ET) ? tmp_net : MAX_ET; /*	elaboration threads	*/
	nwt = (tmp_nwt<MAX_WT) ? tmp_nwt : MAX_WT; /*	write threads	*/

	
	
	/*	Init Mutex	*/
	pthread_mutex_init(&rmutex,NULL);
	pthread_mutex_init(&emutex,NULL);

	/*	Init Buffers	*/
	wbuf_init(&rbuffer);
	rbuffer.todo = tmp_todo; /*	Number of data to create	*/
	wbuf_init(&ebuffer);

	/*	Init Thread's arrays	*/
	cthreads = (pthread_t*)calloc((size_t)nct,sizeof(pthread_t));
	ethreads = (pthread_t*)calloc((size_t)net,sizeof(pthread_t));
	wthreads = (pthread_t*)calloc((size_t)nwt,sizeof(pthread_t));

	/*	Creating Threads	*/
	for(i=0;i<nct;i++){
		pthread_create(&cthreads[i], NULL, create_data, (void *)i);
	}
	for(i=0;i<net;i++){
		pthread_create(&ethreads[i], NULL, elaborate_data, (void*)i);
	}
	for(i=0;i<nwt;i++){
		pthread_create(&wthreads[i], NULL, write_dev, (void*)i);
	}

	/*	Closing */

	/*		Waiting for create_data Thread	*/

	wait_for(cthreads,nct);

	release_t(&rmutex,&rbuffer);

	/*	Waiting for elaborate_data Thread	*/

	wait_for(ethreads,net);
	
	release_t(&emutex,&ebuffer);

	/*	Waiting for write_dev Thread	*/

	wait_for(wthreads,nwt);
	
	/* Say to device that i'm done	*/

	dev_done(fd);
	
	/*	Close device	*/

	dev_close(&fd);
	
	/* Cleanup */

	cleanup();

	return 0;
}
