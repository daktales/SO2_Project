#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

#include "buffer.h"
#include "fun.h"

#define S_ET 5 /*	Step for new eleborate_data Thread	*/
#define MAX_ET 10 /*	Max ET	*/
#define S_CT 10 /*	Step for new create_data Thread	*/
#define MAX_CT 5 /*	Max CT	*/
#define S_WT 1 /*	Step for new write_dev Thread	*/
#define MAX_WT 2 /*	Max WT	*/
#define DEV "/dev/mydev"

#define DEB 1 /*	Enable stdout output	*/

int usleep (unsigned int value);

static wbuf rbuffer; /*	FIFO for raw data	*/
static pthread_mutex_t rmutex;

static wbuf ebuffer; /*	FIFO for elaborated data	*/
static pthread_mutex_t emutex;

static int fd; /* Variable for device	*/

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
		tmp_todo = rbuffer.todo--;
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


/* Main */

int main (int argc, char *argv[]){

	int nct = MAX_CT; /*	creation threads	*/
	int net = MAX_ET; /*	elaboration threads	*/
	int nwt = MAX_WT; /*	write threads	*/
	int i;
	int tmp_nct = 0;
	int tmp_net = 0;
	int tmp_nwt = 0;
	int tmp_todo = 0;

	int status = 0;
	
	pthread_t* cthreads;
	pthread_t* ethreads;
	pthread_t* wthreads;


	/*	Init random seed	*/
	srand(time(NULL));

	/*	Basic arguments check	*/
	if (argc==2){
		tmp_todo = atoi(argv[1]);
	} else {
		fprintf(stderr,"Syntax: %s <number of information>\n",argv[0]);
		return -1;
	}

	/* Open Device	*/

	status = fd = open(DEV,O_WRONLY);
	if (status == -1){
		fprintf(stderr,"Error opening device %s\n",DEV);
		return -1;
	}

	/*	Automagically choose best number of thread	*/
	tmp_nct = (tmp_todo/S_CT)+1;
	tmp_net = (tmp_todo/S_ET)+1;
	tmp_nwt = (tmp_todo/S_WT)+1;
	if (tmp_nct<nct){nct = tmp_nct;}; 
	if (tmp_net<net){net = tmp_net;};
	if (tmp_nwt<nwt){nwt = tmp_nwt;};

	
	
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
	for(i=0;i<nct;i++){
		pthread_join(cthreads[i], NULL);
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
	pthread_cond_broadcast(&ebuffer.cv); /*	Release all wthreads	*/
	pthread_mutex_unlock(&emutex);

	/*	Waiting for write_dev Thread	*/
	for(i=0;i<nwt;i++){
		pthread_join(wthreads[i], NULL);
	}
	/* Say to device that i'm done	*/

	write(fd,"",0);
	
	/*	Close device	*/

	status = close(fd);
	if (status == -1){
		fprintf(stderr,"Errore closing device %s\n",DEV);
	}

	/* Cleanup */

	pthread_mutex_destroy(&rmutex);
	pthread_mutex_destroy(&emutex);

	wbuf_destroy(&rbuffer);
	wbuf_destroy(&ebuffer);

	free(cthreads);
	free(ethreads);
	free(wthreads);

	return 0;
}
