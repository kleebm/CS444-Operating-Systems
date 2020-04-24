#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

#define MAXGRIDSIZE 	10
#define MAXTHREADS	1000
#define NO_SWAPS	20

extern int errno;

typedef enum {GRID, ROW, CELL, NONE} grain_type;

int gridsize = 0;

//You definitely need locks to protect the grid because
//it is shared state accessed by read/write by all the threads
int grid[MAXGRIDSIZE][MAXGRIDSIZE];

/***** Variables needed for Locking ******/
//ThreadsLeft lock
pthread_mutex_t threadsLeftLock;

//grid lock
pthread_mutex_t gridLock = PTHREAD_MUTEX_INITIALIZER;

//row level lock
pthread_mutex_t rowLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;

//cell level lock
pthread_mutex_t cellLock = PTHREAD_MUTEX_INITIALIZER;

struct QNode {
        int data;
        struct QNode* next;
};

struct Queue {
        struct QNode *front, *rear;
};

struct QNode* newNode(int k){
        struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
        temp->data = k;
        temp->next = NULL;
        return temp;
}

struct Queue* createQueue(){
        struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
        q->front = q->rear = NULL;
        return q;
}

void enQueue(struct Queue* q,int k){
        struct QNode* temp = newNode(k);
        if(q->rear == NULL){
                q->front = q->rear = temp;
                return;
        }
        q->rear->next = temp;
        q->rear =temp;
}

void deQueue(struct Queue* q){
        if(q->front == NULL)
                return;

        struct QNode* temp = q->front;
        q->front = q->front->next;
        if(q->front == NULL)
                q->rear = NULL;

        free(temp);
}

//queues to track for lower level locking
struct Queue* rowQ;//rows currently being used
struct Queue* lockedRow;//rows currently locked out

struct Queue* cellQ;//cells currently being used
struct Queue* lockedCell;//cells currently locked out

/**  End of queue neccessary stuff   **/


//what about threads_left ... does this need to be protected? What happens if you don't?
int threads_left = 0;

time_t start_t, end_t;

int PrintGrid(int grid[MAXGRIDSIZE][MAXGRIDSIZE], int gridsize)
{
	int i;
	int j;
	
	for (i = 0; i < gridsize; i++)
	{
		for (j = 0; j < gridsize; j++)
			fprintf(stdout, "%d\t", grid[i][j]);
		fprintf(stdout, "\n");
	}
	return 0;
}


long InitGrid(int grid[MAXGRIDSIZE][MAXGRIDSIZE], int gridsize)
{
	int i;
	int j;
	long sum = 0;
	int temp = 0;

	srand( (unsigned int)time( NULL ) );


	for (i = 0; i < gridsize; i++)
		for (j = 0; j < gridsize; j++) {
			temp = rand() % 100;			
			grid[i][j] = temp;
			sum = sum + temp;
		}

	return sum;

}

long SumGrid(int grid[MAXGRIDSIZE][MAXGRIDSIZE], int gridsize)
{
	int i;
	int j;
	long sum = 0;


	for (i = 0; i < gridsize; i++){
		for (j = 0; j < gridsize; j++) {
			sum = sum + grid[i][j];
		}
	}
	return sum;

}

/**  Utility methods used for row and cell level locking  **/

// Methods used for row level checking
bool findRowElement(struct Queue* searchQueue, int row1, int row2){
        if(searchQueue->front == NULL)
                return false;
        struct QNode* current = searchQueue->front;
        while(current != NULL){
                if(current->data == row1 || current->data == row2)
                        return true;
                current = current->next;
        }
        return false;
}


//If element is found that means it is currently being used
bool findQueueElement(struct Queue* searchQueue,struct Queue* lockedQueue ){
        if(searchQueue->front == NULL )
                return true;
        struct QNode* current = searchQueue->front; 
        struct QNode* locked = lockedQueue->front;
        while(locked != NULL){
                if(current->data == locked->data)
                        return false;
                else if(current->next == NULL){
                        current = searchQueue->front;
                        locked = locked->next;
                }
                else
                        current = current->next;
        }
        return true;
}


//Methods for Cell level locking

bool findCellElement(struct Queue* searchQueue, int row1, int col1, int row2, int col2){
        if(searchQueue->front == NULL)
                return false;
        struct QNode* current = searchQueue->front;
        while(current != NULL && current->next != NULL){
                if((current->data == col1 && current->next->data == row1) || (current->data == col2 && current->next->data == row2)) 
                        return true;
                current = current->next->next;
        }
        return false;
}


//If element is found that means it is currently being used
bool findCellQueueElement(struct Queue* searchQueue,struct Queue* lockedQueue ){
        if(searchQueue->front == NULL || lockedQueue->front == NULL)
                return true;
        struct QNode* current = searchQueue->front; 
        struct QNode* locked = lockedQueue->front;
        while(locked != NULL && locked->next != NULL){
                if(current->data == locked->data  && current->next->data == locked->next->data)
                        return false;
                else if(current->next->next == NULL){
                        current = searchQueue->front;
                        locked = locked->next->next;
                }
                else
                        current = current->next->next;
        }
        return true;
}


/** End methods for lock checking   **/

void* do_swaps(void* args)
{

	int i, row1, column1, row2, column2;
	int temp;
	grain_type* gran_type = (grain_type*)args;

	threads_left++;

	for(i=0; i<NO_SWAPS; i++)
	{
		row1 = rand() % gridsize;
		column1 = rand() % gridsize;	
		row2 = rand() % gridsize;
		column2 = rand() % gridsize;


		if (*gran_type == ROW)
		{
		  /* obtain row level locks*/
		  /* *** FILL IN CODE HERE */
		  pthread_mutex_lock(&rowLock);
                  if(findRowElement(rowQ,row1,row2)){
                        for(int i = threads_left+1 ;i> 0; i--){
                                enQueue(lockedRow,row1);
                                enQueue(lockedRow,row2);
                                pthread_cond_wait(&cond1,&rowLock);
                                if(!findRowElement(rowQ,row1,row2))
                                        break;
                        }
                        enQueue(rowQ,row1);
                        enQueue(rowQ,row2);
                  }
                  else{
                        enQueue(rowQ,row1);
                        enQueue(rowQ,row2);
                 }
                  pthread_mutex_unlock(&rowLock);
		}
		else if (*gran_type == CELL)
		{
		  /* obtain cell level locks */
		  /* *** FILL IN CODE HERE  */
			pthread_mutex_lock(&cellLock);
                if(findCellElement(cellQ, row1,column1,row2,column2)){
                        for(int i = threads_left+1; i >0; i--){
                                enQueue(lockedCell,column1);
                                enQueue(lockedCell,row1);
                                enQueue(lockedCell,column2);
                                enQueue(lockedCell,row2);
                                pthread_cond_wait(&cond1,&cellLock);
                        //printf("unlocked down %d \n", threads_left);
                        //deQueue(lockedRow);
                        //deQueue(lockedRow);
                                if(!findCellElement(cellQ,row1,column1,row2,column2))
                                        break;
                        }
                }
                enQueue(cellQ,column1);
                enQueue(cellQ,row1);
                enQueue(cellQ,column2);
                enQueue(cellQ,row2);
                pthread_mutex_unlock(&cellLock);
                }

		
		else if (*gran_type == GRID)
		{
		  /* obtain grid level lock*/
		  /* *** FILL IN CODE HERE */		
		  pthread_mutex_lock(&gridLock);
		}

	  
		temp = grid[row1][column1];
		sleep(1);
		grid[row1][column1]=grid[row2][column2];
		grid[row2][column2]=temp;
//		printf("swapping: %d,%d with %d,%d",row1,column1, row2,column2);


		if (*gran_type == ROW)
		{
		  /* release row level locks */
		  /* *** FILL IN CODE HERE */
		  pthread_mutex_lock(&rowLock);
//                  printf("rowQ: ");  
//                printQueue(rowQ);
//                  printf("lockedRow: ");
//                  printQueue(lockedRow);
//                  printf("\n");
                  deQueue(rowQ);
                  deQueue(rowQ);
                  if(findQueueElement(rowQ, lockedRow) && lockedRow->front != NULL){
//                        printf("unlocked a row \n");
                        pthread_cond_signal(&cond1);
                        deQueue(lockedRow);
                        deQueue(lockedRow);
                  }
                  pthread_mutex_unlock(&rowLock);

		}
		else if (*gran_type == CELL)
		{
		  /* release cell level locks */
		  /* *** FILL IN CODE HERE */
		 pthread_mutex_lock(&cellLock);
		 deQueue(cellQ);
                 deQueue(cellQ);
                 deQueue(cellQ);
                 deQueue(cellQ);
                 if(findQueueElement(cellQ,lockedCell)){
                        pthread_cond_signal(&cond1);
                        deQueue(lockedCell);
                        deQueue(lockedCell);
                        deQueue(lockedCell);
                        deQueue(lockedCell);
                 }
                 pthread_mutex_unlock(&cellLock);

		}
		else if (*gran_type == GRID)
		{
		  /* release grid level lock */
		  /* *** FILL IN CODE HERE */
		  pthread_mutex_unlock(&gridLock);
		}


	}

	/* does this need protection? */
	//lock
	pthread_mutex_lock(&threadsLeftLock);
	threads_left--;
	if (threads_left == 0){  /* if this is last thread to finish*/
	  time(&end_t);         /* record the end time*/
	}
	pthread_mutex_unlock(&threadsLeftLock);
	return NULL;
}	




int main(int argc, char **argv)
{


	int nthreads = 0;
	pthread_t threads[MAXTHREADS];
	grain_type rowGranularity = NONE;
	long initSum = 0, finalSum = 0;
	int i;

	rowQ = createQueue();
	lockedRow = createQueue();

	cellQ = createQueue();
	lockedCell = createQueue();

	if (argc > 3)
	{
		gridsize = atoi(argv[1]);					
		if (gridsize > MAXGRIDSIZE || gridsize < 1)
		{
			printf("Grid size must be between 1 and 10.\n");
			return(1);
		}
		nthreads = atoi(argv[2]);
		if (nthreads < 1 || nthreads > MAXTHREADS)
		{
			printf("Number of threads must be between 1 and 1000.");
			return(1);
		}

		if (argv[3][1] == 'r' || argv[3][1] == 'R')
			rowGranularity = ROW;
		if (argv[3][1] == 'c' || argv[3][1] == 'C')
			rowGranularity = CELL;
		if (argv[3][1] == 'g' || argv[3][1] == 'G')
		  rowGranularity = GRID;
			
	}
	else
	{
		printf("Format:  gridapp gridSize numThreads -cell\n");
		printf("         gridapp gridSize numThreads -row\n");
		printf("         gridapp gridSize numThreads -grid\n");
		printf("         gridapp gridSize numThreads -none\n");
		return(1);
	}

	printf("Initial Grid:\n\n");
	initSum =  InitGrid(grid, gridsize);
	PrintGrid(grid, gridsize);
	printf("\nInitial Sum:  %d\n", initSum);
	printf("Executing threads...\n");

	/* better to seed the random number generator outside
	   of do swaps or all threads will start with same
	   choice
	*/
	srand((unsigned int)time( NULL ) );
	
	time(&start_t);
	for (i = 0; i < nthreads; i++)
	{
		if (pthread_create(&(threads[i]), NULL, do_swaps, (void *)(&rowGranularity)) != 0)
		{
			perror("thread creation failed:");
			exit(-1);
		} 
	}


	for (i = 0; i < nthreads; i++)
		pthread_detach(threads[i]);


	while (1)
	{
		sleep(2);

		if (threads_left == 0)
		  {
		    fprintf(stdout, "\nFinal Grid:\n\n");
		    PrintGrid(grid, gridsize);
		    finalSum = SumGrid(grid, gridsize); 
		    fprintf(stdout, "\n\nFinal Sum:  %d\n", finalSum);
		    if (initSum != finalSum){
		      fprintf(stdout,"DATA INTEGRITY VIOLATION!!!!!\n");
		    } else {
		      fprintf(stdout,"DATA INTEGRITY MAINTAINED!!!!!\n");
		    }
		    fprintf(stdout, "Secs elapsed:  %g\n", difftime(end_t, start_t));

		    exit(0);
		  }
	}	
	
	
	return(0);
	
}






