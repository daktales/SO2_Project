#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#include "write.h"
#include "buffer.h"
#include "fun.h"

/* */

int usleep (unsigned int value);

static wbuf rbuffer; /*	FIFO for raw data	*/
static pthread_mutex_t rmutex;
static pthread_cond_t rcv;

static wbuf ebuffer; /*	FIFO for elaborated data	*/
static pthread_mutex_t emutex;
static pthread_cond_t ecv;

static int fd; /* Variable for device	*/

static pthread_t* cthreads;
static int nct;
static pthread_t* ethreads;
static int net;
static pthread_t* wthreads;
static int nwt;

static int write_loop; /* protected with rmutex */
static int todo;

/* * Threads * */

/* Take raw data (capital letters) and converts them into lowercase
 * letters.
 * Use two mutex to syncronize with other (all) threads.
 * rmutex protects raw data buffer (rbuffer)
 * emutex protects eleaborated data buffer (ebuffer)
 *
 * It use a condition variables to check if raw buffer is void,
 * if buffer is void but create_data thread has not finished (buffer.done)
 * this thread wait for "input" from create_data
 *
 * Processing data is not done in mutual exclusion to prevent that a long
 * elaboration exclude other thread to access buffers.
 *
 * When elaboration is finished it will release a pending write_dev thread
 *
 * Die when there are nothing else to do (empty rbuffer and .done = 1 )
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
			usleep(4000000);
			to_lower(data);
			pthread_mutex_lock(&emutex);
			wbuf_ins(data,&ebuffer);
			/* Awake write T */
			pthread_cond_signal(&ecv);
			pthread_mutex_unlock(&emutex);
		}
	}
	return NULL;
}

/* Write data to device
 * Use one mutex to syncronize with other write and elaborate threads.
 * Like elaborate threads it will protect source buffer (ebuffer) with
 * a mutex (emutex). Use a c.v. to sleep if ebuffer is empty.
 *
 * It will check if write on device fails and do some retry if it fail.
 *
 * Die when there are nothing else to do
 */
 static void *write_dev(void *name){
	int stop = 0;
	char* data;
	int count;
	int myname = (int) name;
	int i;
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
			if(DEB){fprintf(stdout,"WRITE T%d (todo:%d): %s\n",myname,count,data);}
			i = 0;
			/*	write to device, strlen() do not count \0 so +1 is needed*/
			/* If writing fails it will retry for 5 times */
			while ((i<MAX_RETRY)&&(write(fd,data,strlen(data)+1))){  
				if(DEB){fprintf(stderr,"Writing attempt %d fail, retry..\n",i);}
				i++;
			}
			if (i==5){
				fprintf(stderr,"**FAIL writing on device, data lost");
			}
			pthread_mutex_unlock(&emutex);
		}
	}
	return NULL;
}

/* Generate random data (capital letters's string)
 * One mutex for raw data buffer
 *
 * Infinite or fixed data creation
 *
 * Use c.v. to awake elaborate threads
 */
static void *create_data(void *name){
	int stop = 0;
	int tmp_todo;
	int myname = (int) name;
	while(!stop){
		pthread_mutex_lock(&rmutex);
		tmp_todo = (write_loop) ? 1 : todo--;
		if (tmp_todo<=0){
			stop = 1;
		} else {
			wbuf_ins(gen_data(),&rbuffer);
			/* Awake elaborate T */
			pthread_cond_signal(&rcv);
			if(DEB){fprintf(stdout,"CREATE T%d (todo: %d)\n",myname,tmp_todo);}
		}
		pthread_mutex_unlock(&rmutex);
		usleep(2000000);
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

/* */

/* Destroy mutex, c.v. and free threads arrays */
void cleanup(){
	pthread_mutex_destroy(&rmutex);
	pthread_mutex_destroy(&emutex);

	pthread_cond_destroy(&rcv);
	pthread_cond_destroy(&ecv);

	free(cthreads);
	free(ethreads);
	free(wthreads);
}

/* Open device */
void dev_open(int *f){
	int status;
	status = *f = open(DEV,O_WRONLY);
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

/* Send 'done' command to device */
int dev_done(int *f){
	int i = 0;
	while((i<=MAX_RETRY)&&(write(*f,"00\0",3))){
		if(DEB){fprintf(stderr,"Error sending \'done\' command\n");}
		i++;
	}
	if (i==MAX_RETRY){
		fprintf(stderr,"Critical Error sending \'done\' command\n");
		return 1;
	}
	return 0;
}

/*	Intercepts CTRL-C	*/
void catcher_SIGINT(int signum) {
	printf("Closing, please wait..(signum: %d)\n",signum);
	/* Using rmutex because create_data check todo and write loop variables
	 * within rmutex mutual exclusion
	 */
	pthread_mutex_lock(&rmutex);
	todo = 0;
	write_loop = 0;
	pthread_mutex_unlock(&rmutex);

	if (&fd != NULL){
		wait_for(cthreads,nct);
		if(DEB){printf("CTRL-C: All Create threads has stopped\n");}	
		release_t(&rmutex,&rbuffer,&rcv);
		if(DEB){printf("CTRL-C: Raw buffer closed\n");}
		wait_for(ethreads,net);
		if(DEB){printf("CTRL-C: All Elaborate threads has stopped\n");}
		release_t(&emutex,&ebuffer,&ecv);
		if(DEB){printf("CTRL-C: Elaborate buffer closed\n");}
		wait_for(wthreads,nwt);
		if(DEB){printf("CTRL-C: All Write threads has stopped\n");}
		dev_done(&fd);
		dev_close(&fd);
	}
	cleanup();
	exit(1);
	}


/* * Main * */

int main (int argc, char *argv[]){

	int i;
	int tmp_nct = 0;
	int tmp_net = 0;
	int tmp_nwt = 0;

	/*	Init random seed	*/
	srand(time(NULL));

	/*	Basic arguments check	*/
	if (argc==2){
		todo = atoi(argv[1]);
	} else {
		fprintf(stderr,"Syntax: %s <number of information>\n\t 0 for infinite loop (ctrl-c to terminate)\n",argv[0]);
		return -1;
	}

	/* Set write loop	*/
	write_loop = (todo) ? 0 : 1;

	/* handle SIGINT	*/
	signal(SIGINT, catcher_SIGINT);

	/* Open Device	*/
	dev_open(&fd);

	/*	Automagically choose best number of thread	*/
	if (!write_loop){
		tmp_nct = (todo/S_CT)+1;
		tmp_net = (todo/S_ET)+1;
		tmp_nwt = (todo/S_WT)+1;
		nct = (tmp_nct<MAX_CT) ? tmp_nct : MAX_CT; /*	creation threads	*/
		net = (tmp_net<MAX_ET) ? tmp_net : MAX_ET; /*	elaboration threads	*/
		nwt = (tmp_nwt<MAX_WT) ? tmp_nwt : MAX_WT; /*	write threads	*/
	} else {
		nct = MAX_CT;
		net = MAX_ET;
		nwt = MAX_WT;
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

	/* * Closing * */

	/*		Waiting for create_data Thread	*/
	wait_for(cthreads,nct);
	release_t(&rmutex,&rbuffer,&rcv);

	/*	Waiting for elaborate_data Thread	*/
	wait_for(ethreads,net);
	release_t(&emutex,&ebuffer,&ecv);

	/*	Waiting for write_dev Thread	*/
	wait_for(wthreads,nwt);

	/* Say to device that i'm done	*/
	dev_done(&fd);
	
	/*	Close device	*/
	dev_close(&fd);
	
	/* * Cleanup * */
	cleanup();

	return 0;
}
