/***** Includes *****/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//Linux or MacOS
#ifdef UNIX
	#include <pthread.h>
	#include <unistd.h>
	#include <semaphore.h>
#endif

//Windows
#ifdef WINDOWS
	#include <windows.h>
	#include <semaphore.h>
#endif

/***** Defines: Constants in seconds *****/
#define PRODUCER_SLEEP_S	1
#define CONSUMER_SLEEP_S	1
#define QUEUESIZE 			5
#define LOOP 				10


/***** Function prototypes *****/
void *producer (void *args);
void *consumer (void *args);

#ifdef UNIX
sem_t semaphore;
#endif

#ifdef WINDOWS
HANDLE hMutex;
#endif 

/***** Queue struct *****/
typedef struct 
{
	int buf[QUEUESIZE];
	long head, tail;
	int full, empty;  //you may need or may not
	/*TODO for students: Declare the locks here */
	#ifdef UNIX
	sem_t semaphore;
	#endif
	#ifdef WINDOWS
	HANDLE hMutex;
	#endif 
} queue;

/***** Queue function prototypes  *****/
queue *queueInit (void);
void queueDelete (queue *q);
void queueAdd (queue *q, int in);
void queueDel (queue *q, int *out);

/***** main *****/
int main ()
{
	queue *fifo;
	int i;

	//for Consumer's random coming
	unsigned int iseed = (unsigned int)time(NULL);
	//seeds the random number generator
	srand(iseed);
	
    /******one producer and multiple consumer********/
	
	//an array of consumers and one producer
	#ifdef UNIX
	pthread_t pro, con[LOOP];
	#endif
	
	//an array of consumers and one producer
	#ifdef WINDOWS
	HANDLE pro, con[LOOP];
	#endif
	
	fifo = queueInit ();
	if (fifo ==  NULL) 
	{
		fprintf (stderr, "main: Queue Init failed.\n");
		exit (1);
	}
	
	#ifdef UNIX
		pthread_create (&pro, NULL, producer, fifo);
		for(i=0; i<LOOP; i++)
		{
			pthread_create (&con[i], NULL, consumer, fifo);
		}
	#endif
	
	#ifdef WINDOWS
		pro = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE) producer, fifo, 0, NULL);
		for(i=0; i<LOOP; i++)
		{
	#ifdef WINDOWS
			/* TODO for students: Simulate Consumers' random arrival */
			Sleep((rand()%2)*1000);
	#endif
		    con[i] = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE) consumer, fifo, 0, NULL);		
		}
	#endif
	
	
	#ifdef UNIX
		pthread_join (pro, NULL);
		for(i=0; i<LOOP; i++)
		{
			pthread_join (con[i], NULL);	
		}
	#endif
	
	#ifdef WINDOWS
		WaitForSingleObject(pro, INFINITE);
        /*******Wait for all the threads complete**********/      
		WaitForMultipleObjects(LOOP, con, TRUE, INFINITE);	

	#endif
	
	queueDelete (fifo);

    #ifdef WINDOWS
	system("pause");
	#endif
	
	return 0;
}

/***** producer *****/
void *producer (void *q)
{

	queue *fifo;
	int i;

	fifo = (queue *)q;

    //TODO for students: obtain the lock and release the lock somewhere

	for (i = 0; i < LOOP; i++) 
	{

        // TODO for students: Obtain the locks somewhere here
		#ifdef UNIX
		//pthread_mutex_lock(&lock);
		while (fifo->full) 
		{
			sem_wait(&semaphore);
		}
		#endif
		
        #ifdef WINDOWS
		WaitForSingleObject(hMutex,INFINITE);
		while(fifo->full)
		{
			WaitForSingleObject(hMutex,INFINITE);
		}
        #endif
			
			
		queueAdd (fifo, i+1);
		printf ("producer: produced %d th.\n",i+1);
		
		/* sleep */
		#ifdef UNIX
			usleep ( PRODUCER_SLEEP_S * 1000000); 
		#endif
		
		#ifdef WINDOWS
			Sleep ( PRODUCER_SLEEP_S * 1000);	
		#endif

        /*******Release the locks**********/
        #ifdef UNIX
		sem_post(&semaphore);
        #endif

        
        #ifdef WINDOWS
		ReleaseMutex(hMutex);
        #endif
	}

	return (NULL);
}

/***** consumer *****/
void *consumer (void *q)
{
	queue *fifo;
	int d;

	fifo = (queue *)q;

	/* Simulate Consumers' random arrival */
	#ifdef UNIX
		usleep ( (rand()%LOOP) * 1000000); 
	#endif
		

    // TODO for students: obtain the lock and release the lock somewhere

    #ifdef UNIX
	sem_wait(&semaphore);
	while (fifo->empty) 
	{
	sem_wait(&semaphore);
	}
	#endif
	
	
    #ifdef WINDOWS
	WaitForSingleObject(hMutex,INFINITE);
	while (fifo->empty) 
	{
	WaitForSingleObject(hMutex,INFINITE);
	}
	#endif
	
	/* sleep */
	#ifdef UNIX
		usleep ( CONSUMER_SLEEP_S * 1000000); 
	#endif

	#ifdef WINDOWS
		Sleep ( CONSUMER_SLEEP_S * 1000);	
	#endif

    
	queueDel (fifo, &d);
	printf ("------------------------------------>consumer: recieved %d.\n", d);		

/** Release the locks here    **/
    #ifdef UNIX
	//Not needed for semaphores
    #endif


    #ifdef WINDOWS
    //ReleaseMutex(&hMutex);
	#endif
	

	return (NULL);
}


/***** queueInit *****/
queue *queueInit (void)
{
	queue *q;
	int i;
	
	q = (queue *)malloc (sizeof (queue));
	if (q == NULL) return (NULL);

	for(i=0;i<QUEUESIZE;i++)
	{
		q->buf[i]=0;
	}
	q->empty = 1;
	q->full = 0;
	q->head = 0;
	q->tail = 0;

    //TODO for students: Initialize the locks here
	#ifdef UNIX
	sem_init(&semaphore, 0,1);
	#endif

	#ifdef WINDOWS
	//ghMutex = CreateMutex(NULL,FALSE,NULL);
	hMutex = CreateSemaphore(NULL,0,10,NULL);
	#endif

	return (q);
}


/***** queueDelete *****/
void queueDelete (queue *q)
{

	//TODO for students: free the locks here
	#ifdef UNIX
	sem_destroy(&semaphore);
	#endif

	#ifdef WINDOWS
	//ReleaseMutex(ghMutex);
	CloseHandle(&hMutex);
	#endif

	/* free memory used for queue */
	free (q);
}

/***** queueAdd *****/
void queueAdd (queue *q, int in)
{
	q->buf[q->tail] = in;
	q->tail++;
	if (q->tail == QUEUESIZE)
		q->tail = 0;
	if (q->tail == q->head)
		q->full = 1;
	q->empty = 0;

	return;
}

/***** queueDel *****/
void queueDel (queue *q, int *out)
{
	*out = q->buf[q->head];
	q->buf[q->head]=0;

	q->head++;

	if (q->head == QUEUESIZE)
		q->head = 0;
	if (q->head == q->tail)
		q->empty = 1;
	q->full = 0;

	return;
}
