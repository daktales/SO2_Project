#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#include "read.h"
#include "buffer.h"
#include "fun.h"

/* * * VARIABLES * * */

int usleep (unsigned int value);

/*	FIFO for raw data	*/
static wbuf rbuffer; 
static pthread_mutex_t rmutex;
static pthread_cond_t rcv;

/*	FIFO for elaborated data	*/
static wbuf ebuffer; 
static pthread_mutex_t emutex;
static pthread_cond_t ecv;

/*	Variable for device	*/
static int fd; 

/*	Threads Lists	*/
static pthread_t* rthreads;
static int nrt;
static pthread_t* ethreads;
static int net;
static pthread_t* p_threads;
static int npt;

/*	For looping	*/
static int read_loop; /* protected with rmutex */
static int todo; /* protected with rmutex */

/*	Fun	*/
static int control;

/* * * FUNCTIONS * * */

/* * Threads * */

/* Take raw data (non-capital letters) and converts them into uppercase
 * letters.
 * Use two mutexes to syncronize with other (all) threads.
 * rmutex protects raw data buffer (rbuffer)
 * emutex protects eleaborated data buffer (ebuffer)
 *
 * It uses a condition variable to check if the raw buffer is void,
 * if the buffer is void, but read_dev thread has not finished (buffer.done),
 * this thread waits for "input" from read_dev
 *
 * Processing data is not performed in mutual exclusion in order to prevent
 * a long elaboration task excludes other threads from accessing the buffers.
 *
 * When elaboration is finished it will release a pending write_dev thread
 *
 * Die when there's nothing else to do (empty rbuffer and .done = 1 )
 */
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
			usleep(count*400000);
			to_upper(data);
			pthread_mutex_lock(&emutex);
			wbuf_ins(data,&ebuffer);
			/* Awake print T */
			pthread_cond_signal(&ecv);
			pthread_mutex_unlock(&emutex);
		}
	}
	return NULL;
}

/* Print data to stdout
 * Use one mutex to syncronize with other print and elaborate threads.
 * Like elaborate threads it will protect source buffer (ebuffer) with
 * a mutex (emutex). Use a c.v. to sleep if ebuffer is empty.
 *
 * Die when there is nothing else to do
 */
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
		usleep(1000000);
	}
	return NULL;
}
/* Read data from device
 *
 * Infinite loop until 00 (done command) or fixed number of read
 *
 * Mutex for rbuffer
 * Awake elaborate thread with c.v.
 */
static void *read_dev(void *name){
	int stop = 0;
	int tmp_todo;
	int myname = (int) name;
	char *tmp = (char*) malloc((size_t)READ_BYTE);
	while(!stop){
		pthread_mutex_lock(&rmutex);
		tmp_todo = (read_loop) ? 1 : todo--;
		if (tmp_todo<=0){
			stop = 1;
		} else {
			if (read(fd,tmp,READ_BYTE)){
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
			/* Awake elaborate T */
			pthread_cond_signal(&rcv);
		}
		pthread_mutex_unlock(&rmutex);
		usleep(2700000);
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

/* *	General functions * */

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
	control++;
	if (control>1){
		printf("I've said wait..!");
		return;
	}
	printf("Closing, please wait..(signum: %d)\n",signum);
	/* Using rmutex because create_data check todo and read loop variables
	 * within rmutex mutual exclusion
	 */
	pthread_mutex_lock(&rmutex);
	todo = 0;
	read_loop = 0;
	rbuffer.head = NULL;
	rbuffer.count = 0;
	rbuffer.done = 1;
	pthread_mutex_lock(&emutex);
	pthread_mutex_unlock(&rmutex);
	ebuffer.head = NULL;
	ebuffer.count = 0;
	ebuffer.done = 1;
	pthread_mutex_unlock(&emutex);

	return;
	}



/* Main */

int main (int argc, char *argv[]){

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
		fprintf(stderr,"Syntax: %s <number of information>\n\t 0 for reading until terminator found\n",argv[0]);
		return -1;
	}

	/* Set read loop	*/
	read_loop = (todo) ? 0 : 1;

	/* handle SIGINT	*/
	signal(SIGINT, catcher_SIGINT);
	control = 0;

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
	if(DEB){printf("Closing: All Read threads has stopped\n");}
	release_t(&rmutex,&rbuffer,&rcv);
	if(DEB){printf("Closing: Raw buffer closed\n");}

	/*	Waiting for elaborate_data Thread	*/
	wait_for(ethreads,net);
	if(DEB){printf("Closing: All Elaborate threads has stopped\n");}
	release_t(&emutex,&ebuffer,&ecv);
	if(DEB){printf("Closing: Elaborate buffer closed\n");}

	/*	Waiting for print Thread	*/
	wait_for(p_threads,npt);
	if(DEB){printf("Closing: All Write threads has stopped\n");}

	/*	Close device	*/
	dev_close(&fd);
	
	/* * Cleanup * */
	cleanup();

	return 0;
}
