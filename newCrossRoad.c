#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
//We use pthread library which matches the POSIX standard, so we include the head <pthread.h> and <unistd.h>
//-------------------------------------------------------------------------------------------------------------------
#include <pthread.h>
#include <unistd.h>

//These macros map the directions to the numbers, the numbers are the index of the mutex and condition.
//NORTH->WEST->SOUTH->EAST(CouterClockwise)   
//The right direction of the current direction is currentDirection+1. (e.g. NORTH+1=WEST)
//The right direction of the current direction is (currentDirection+3)%4. (e.g. (NORTH+3)%4=EAST)
//Set the max number of carThread to MAXSIZE(default 100. Change it if you want.)
//--------------------------------------------------------------------------------------------------------------------
#define NORTH 0			
#define WEST  1			 
#define SOUTH 2			 
#define EAST  3			
#define MAXSIZE 100	

//The 2-D array save the string of direction to be printed.
//e.g. NWSE[0][]=NWSE[NORTH][]="North"
//--------------------------------------------------------------------------------------------------------------------
char NWSE[4][8]=
{
	"North",
	"West",
	"South",
	"East"
};

//The Car struct,pass the infomation of the cars to the thread, which will be created later.
//If we use gcc to compile, it's more convenient to use typedef.
//A car has two infomation, the direction(from) and the number.
//typedef struct Car* PointerCar;
//--------------------------------------------------------------------------------------------------------------------
struct Car
{
	int direction;
	int number;
};

//Use a self-defined data structure Queue for c. If we use cpp, we could just #include<queue>
//--------------------------------------------------------------------------------------------------------------------
struct Queue
{
	int capacity;   //The capacity of the queue
	int size;       //The Number of element in the queue now
	int front;      //The index of the head 
	int rear;       //The index of the rear 
	int *element;   //The pointer to a intger array which is used to save the element of the queue
};

struct Queue* QueueInit(int maxElements);  //Initial a queue with the capacity of maxElements
void Queueclear(struct Queue* q);          //Clear the queue 
void enqueue(int x,struct Queue* q);       //Push the element to the back of the queue
int dequeue(struct Queue* q);              //Pop the first element of the queue, return the value of the element
int front(struct Queue* q);                //Get the element of the queue without popping it


//Gloabal varible,conditon,mutex, accessed by all the thread(include car thread and the deadlock check thread)
//--------------------------------------------------------------------------------------------------------------------

//Conditional variable for 4 directions car queue.
//--------------------------------------------------------------------------------------------------------------------
pthread_cond_t carQueue[4];	   //Activated if the car int the current queue pass the cross.
pthread_cond_t first[4];	   //Activated if some car pass the cross and it is the other queue's turn to pass.

							  
//The waitCarQueue and the mutex to access the waitCarQueue for 4 directions.
//--------------------------------------------------------------------------------------------------------------------
struct Queue* waitCarQueue[4];		   //The queue for cars waiting for pass the cross.
pthread_mutex_t waitQMut[4];   //The mutex to proctect the waiting queue.
                               //If some thread access the waiting queue, other thread can't do that simultaneously.

pthread_mutex_t printLock;

//The public part and the deadlock check
//--------------------------------------------------------------------------------------------------------------------
pthread_mutex_t cross;		  //Public department, in other words the cross.
pthread_t deadLockCheck;	  //The thread to check the deadlock

//The 0,1 flag,use bitmap is also okay
//--------------------------------------------------------------------------------------------------------------------
int someDudeInCross;		  //Some dude with the same direction is in the cross
							  //Set to 1 before entering the cross and set back to 0 when leaves
int waiting[4];		          //1 means in the direction has a car/cars waiting, 0 means no car in that direction is waiting

pthread_cond_t outQueue[4];
pthread_cond_t outCross; 

pthread_mutex_t waitingLock[4];


//Thread function for the cars from 4 directions
//The parameter "info" is the fourth parameter in the function pthread_create(), here is the car's direction and number
//--------------------------------------------------------------------------------------------------------------------
void* carFrom(void* info)
{
	//Pass the parameters to the thread
	//-----------------------------------------------------------------------------------------------------------------
	struct Car* thisCar=(struct Car*)info;	           //The pointer to the information passed in
	int direction=thisCar->direction;	               //The direciton 
	int number=thisCar->number;	                       //The number 
	char* dirS;	                                       //The pointer to the direction string 
	dirS=NWSE[direction];	                           //Get the stirng from the 2-D array we initialized at the begging of the file 

	pthread_mutex_lock(&waitQMut[direction]);
	enqueue(number,waitCarQueue[direction]);
	pthread_mutex_unlock(&waitQMut[direction]);

	pthread_mutex_lock(&waitQMut[direction]);
	while(front(waitCarQueue[direction])!=number)pthread_cond_wait(&outQueue[direction],&waitQMut[direction]);
	pthread_mutex_lock(&printLock);
	printf("Car %d from %s arrives at crossing.\n", number, dirS);
	pthread_mutex_unlock(&printLock);
	pthread_mutex_unlock(&waitQMut[direction]);

	pthread_mutex_lock(&waitQMut[(direction+1)%4]);
	if(waitCarQueue[(direction+1)%4]->size!=0)
	{
		pthread_mutex_lock(&waitingLock[direction]);
		waiting[direction]=1;
		pthread_mutex_unlock(&waitingLock[direction]);

		pthread_cond_wait(&first[direction],&waitQMut[(direction+1)%4]);

		pthread_mutex_lock(&waitingLock[direction]);
		waiting[direction]=0;
		pthread_mutex_unlock(&waitingLock[direction]);
	}
	pthread_mutex_unlock(&waitQMut[(direction+1)%4]);


	pthread_mutex_lock(&cross);
	while(someDudeInCross)pthread_cond_wait(&outCross,&cross);

	someDudeInCross=1;
	//At this time we are crossing
	//sleep(1);
	//At this time we are crossing

	pthread_mutex_lock(&printLock);
	printf("Car %d from %s leaving crossing.\n",number,dirS);
	pthread_mutex_unlock(&printLock);

	pthread_mutex_lock(&waitQMut[direction]);
	pthread_cond_signal(&first[(direction+3)%4]);
	pthread_mutex_unlock(&waitQMut[direction]);
	someDudeInCross=0;
	pthread_cond_signal(&outCross);
	
	
	pthread_mutex_unlock(&cross);

	pthread_mutex_lock(&waitQMut[direction]);
	dequeue(waitCarQueue[direction]);
	pthread_cond_signal(&outQueue[direction]);
	pthread_mutex_unlock(&waitQMut[direction]);
}
//Thread function for the deadLock check
//--------------------------------------------------------------------------------------------------------------------
void* checkDeadLock()
{
	while(1)
	{
		//When we handle deadLock, we don't want any dude to enter the public seciton(the cross), so we lock it
		//------------------------------------------------------------------------------------------------------------
		pthread_mutex_lock(&cross);
		while(someDudeInCross)pthread_cond_wait(&outCross,&cross);

		int i;
		for(i=0;i<4;++i)pthread_mutex_lock(&waitingLock[i]);
		if(!(waiting[NORTH]==1&&waiting[WEST]==1&&waiting[SOUTH]==1&&waiting[EAST]==1))
		{
			for(i=3;i>=0;--i)pthread_mutex_unlock(&waitingLock[i]);
			pthread_mutex_unlock(&cross);
			continue;
		}

		pthread_mutex_lock(&printLock);
		printf("DEADLOCK: car jam detected, signalling NORTH to go.\n");
		pthread_mutex_unlock(&printLock);

		for(i=3;i>=0;--i)pthread_mutex_unlock(&waitingLock[i]);

		pthread_mutex_lock(&waitQMut[WEST]);
		pthread_cond_signal(&first[NORTH]);
		pthread_mutex_unlock(&waitQMut[WEST]);

		pthread_mutex_unlock(&cross);
	}
}

//Main function
//Handle the input and creat the thread
//--------------------------------------------------------------------------------------------------------------------
int main(int argc,char* argv[])
{
	int no=0;										//The Number to assign,while a thread created, no=no+1
	int i;                                          //The conter
	char directions[MAXSIZE+1];                       //The string to save the input parameters argv[1] 
	pthread_t car[MAXSIZE];                         //The thread for car

	//Mutex,conditional varible and flag initial
	//----------------------------------------------------------------------------------------------------------------
	for(i=0;i<4;++i)
	{
		pthread_cond_init(&carQueue[i],NULL);  //"carQueue" initial
		pthread_cond_init(&first[i],NULL);     //"first" initial
		pthread_cond_init(&outQueue[i],NULL);
		pthread_mutex_init(&waitingLock[i],NULL);   //direction mutex initial
		pthread_mutex_init(&waitQMut[i],NULL); //wait queue mutex inital
		waitCarQueue[i]=QueueInit(MAXSIZE);    //queue inital with capacity of MAXSIZE                  //Obviously there is no car in the cross at first
		waiting[i]=0;                          //Obviously there is no car is waiting at first
	}
	//pthread_cond_init(&deadLock,NULL);                                         //No deadlock happens at first 
	//pthread_mutex_init(&deadLockMut,NULL);                                     //deadlock mutex initial 
	pthread_mutex_init(&cross,NULL);
	pthread_mutex_init(&printLock,NULL);                                             //cross mutex initial
	pthread_cond_init(&outCross,NULL); 
	someDudeInCross=0;
	
	//create the thread to check deadlock 
	//----------------------------------------------------------------------------------------------------------------
	int error1=pthread_create(&deadLockCheck,NULL,checkDeadLock,NULL);          
	if(error1!=0)printf("Creat deadLockCheck thread failed! %s\n",strerror(error1));

	//Input handle 
	//----------------------------------------------------------------------------------------------------------------
	strcpy(directions,argv[1]);
	directions[MAXSIZE]='\0';

	//Car thread create
	//----------------------------------------------------------------------------------------------------------------
	for(i=0;i<strlen(directions);++i)
	{
		struct Car* thisCar=(struct Car*)malloc(sizeof(struct Car));   //Create a Car pointer to the new memory address
		switch (directions[i])                                          //Handle the input direction (e.g. 'n'->NORTH)
		{
			case 'n':
				thisCar->direction=NORTH;
				break;
			case 'e':
				thisCar->direction=EAST;
				break;
			case 's':
				thisCar->direction=SOUTH;
				break;
			case 'w':
				thisCar->direction=WEST;
				break;
			default:
				break;
		}
		thisCar->number=++no;                                            //Assign a number to the car thread

		//truely creat the car thread, passing the information of car (direction and number) 
		//------------------------------------------------------------------------------------------------------------
		int error2=pthread_create(&car[i],NULL,carFrom,thisCar);          
		if(error2!=0)printf("Can't create car %d! %s\n",thisCar->number,strerror(error2));
	}

	//Waiting process end and clear the carQueue
	//----------------------------------------------------------------------------------------------------------------
	for(i=0;i<strlen(directions);i++)pthread_join(car[i],NULL);
	for(i=0;i<4;i++)Queueclear(waitCarQueue[i]);
	
	return 0;
} 


//These are the queue function, features are listed at the front of the file
//Use the circular queue struct,two index point the head and the rear
//The succ(int value,struct Queue *q) function can get the next index of the input index of x
//----------------------------------------------------------------------------------------------------------------

//Initial a queue with the capacity of maxElements
//----------------------------------------------------------------------------------------------------------------
struct Queue* QueueInit(int maxElements)
{
	struct Queue* q=(struct Queue*)malloc(sizeof(struct Queue));
	q->element=(int*)malloc(sizeof(int)*maxElements);
	q->capacity=maxElements;
	q->size=0;
	q->front=1;
	q->rear=0;
	return q; 
}

//Clear the queue
//----------------------------------------------------------------------------------------------------------------
void Queueclear(struct Queue* q)
{
	free(q->element);
	free(q);
} 

//Get the next index of input index x
//If the input index equal to the max, then the next index will be 0
//----------------------------------------------------------------------------------------------------------------
static int succ(int value,struct Queue* q)
{
	if(++value==q->capacity)value=0;
	return value;
}

//Push the element to the rear of the queue
//----------------------------------------------------------------------------------------------------------------
void enqueue(int x,struct Queue* q)
{
	if(q->size==q->capacity)return;		
	else
	{
		q->size++;	
		q->rear=succ(q->rear,q);
		q->element[q->rear]=x;
	}
}

//Pop the element from the head of the queue
//Return the value of the element popped
//----------------------------------------------------------------------------------------------------------------
int dequeue(struct Queue* q)
{
	int res;
	if(q->size==0)return -1;	
	else
	{
		res=q->element[q->front];
		q->size--;	
		q->front=succ(q->front,q);
		return res;
	}
}

//Get the value of the element of the top with out pop the element
//----------------------------------------------------------------------------------------------------------------
int front(struct Queue* q)
{
	if(q->size==0)return -1;
	else return q->element[q->front];
}
