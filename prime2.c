#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>




struct threadArgs{ //the parameters of threads
	int* numbers;
	long threadId;
	int size;
	
};

struct threadReturnValue{ //the returned strut from thread
	int number;//which is controlled if it is prime or not
	long threadId;
	struct threadReturnValue* nextPtr;
};

pthread_mutex_t mutexModify; //to lock globals
int selectNumber; //index of the array of controlling numbers 

void removeNotPrimes(int selected, int* numbers,int size,long threadId){//setting zero multiples of numbers by using Sieve of Eratoshthenes method


	if (numbers[selected] != 0)
	{
		int x = selected * selected;
		while (x <= size)
		{
			numbers[x] = 0;
			x = x + selected;
		}
	}
}
int isnumeric(char *str) //controls if a string is numeric or not
{
	while(*str)
	{
		if(!isdigit(*str))
		return 0;
		str++;
	}

	return 1;
}
int isPrime(int number,long threadId) {// controls primality of the selected number by selected thread
	int i;
	for (i=2; i<number; i++) {
		if (number % i == 0 && i != number) return 0;// divisible so, not prime
	}
	return 1;//prime
}

void* FindPrimes(void* tArgs)// a method for threads to select numbers from number pool
{
	struct threadReturnValue *returnValueFirst=NULL;//the pointer for the linked list of the primes which is found by specific thread
	struct threadReturnValue *returnValueLast=NULL;
	
	int selected;//thee number which will be controlled
	pthread_mutex_lock (&mutexModify);//critical section/////////////////////////////////////////////////////
	struct threadArgs *myArgs = (struct threadArgs*)tArgs;//pointer for thread arguments
	while(selectNumber < myArgs->size){ //if selectNumber is still in bounds of array which constructed from number pool
			while( myArgs->numbers[selectNumber] ==0){
				if(selectNumber < myArgs->size)
				selectNumber++;
			}
			selected = myArgs->numbers[selectNumber];//global variable is changed by trade protected in critical section
			selectNumber++;
		
		pthread_mutex_unlock (&mutexModify);//critical section end//////////////////////////////////////////////////
		
		if(isPrime(selected,myArgs->threadId)==1){//if the number is prime add to the private linked list of thread
			if(returnValueFirst==NULL){
				returnValueFirst = malloc(sizeof(struct threadReturnValue));
				returnValueFirst->threadId = myArgs->threadId;
				returnValueFirst->number = selected;
				returnValueFirst->nextPtr = NULL;
				returnValueLast = returnValueFirst;
				}else{
					returnValueLast->nextPtr = malloc(sizeof(struct threadReturnValue));
					returnValueLast->nextPtr->threadId = myArgs->threadId;
					returnValueLast->nextPtr->number = selected;
					returnValueLast->nextPtr->nextPtr = NULL;
					returnValueLast = returnValueLast->nextPtr;
					
				}
		}
		
		pthread_mutex_lock (&mutexModify);
	}
	
	pthread_mutex_unlock (&mutexModify);
   pthread_exit((void*)returnValueFirst);//return the firstptr of the linked list of primes found
}

int main(int argc, char **argv){
	int i,rc,size,noOfThreads=2;
	long t;
	void *status;
	
	for (i = 0; i < argc; ++i)
	{
		printf("argv[%d]: %s\n", i, argv[i]);
		if( argv[1] == NULL || (strcmp(argv[1],"-t") && strcmp(argv[1],"-n"))){
			printf("You didn't specify any valid argument \n");
			return;
			}
		if(!strcmp(argv[i],"-n")){
			if(argv[i+1]!=NULL){
				if( isnumeric(argv[i+1]) ){
					sscanf(argv[i+1], "%d", &size);
				}else{
					printf("Your argument is not numeric\n");
					return;
					}
			}else{
				printf("You didn't specify the number interval\n");
				return;
				}

		}
		else if(!strcmp(argv[i],"-t")){
			if(argv[i+1]!=NULL){
				if( isnumeric(argv[i+1])){
					sscanf(argv[i+1], "%d", &noOfThreads);
				}else{
					printf("Your argument is not numeric\n");
					return;
					}

			}else{
				printf("You didn't specify the number of threads\n");
				return;
			}
		}
		
	}
	if((argv[5]==NULL || argv[6]==NULL) || strcmp(argv[5],"-o")){
		printf("You didn't specify the file name\n");
		return;
		}
	int numbers[size];//getting from user
	pthread_t threads[noOfThreads];
	
	for (i = 2; i <= size; i++)//initilize number array
		numbers[i] = i;
		
	int in;
	struct threadArgs * tArgs;
	tArgs = malloc(noOfThreads*sizeof(struct threadArgs));//parameters of threads
	pthread_mutex_init(&mutexModify, NULL);//initilize mutex
	for(t=0;t<noOfThreads;t++){
		(tArgs+t)->threadId = t;
		(tArgs+t)->size = size;
		(tArgs+t)->numbers = numbers;
		printf("In main: creating thread %ld\n", t);

		rc = pthread_create(&threads[t], NULL, FindPrimes, (void *)(tArgs+t));
		
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	struct threadReturnValue *firstPtr = NULL;
	struct threadReturnValue *lastPtr = NULL;
	struct threadReturnValue *ret;
	for(i=0;i<noOfThreads;i++) {
		ret = NULL;
		pthread_join(threads[i], (void**)&ret);
				if(firstPtr==NULL){
				firstPtr=ret;
				lastPtr = firstPtr;
				while(lastPtr->nextPtr!=NULL){
					lastPtr = lastPtr ->nextPtr;
					}
			
				}else{
					lastPtr->nextPtr = ret;
					while(lastPtr->nextPtr!=NULL){
					lastPtr = lastPtr ->nextPtr;
					}
				}
	}
	lastPtr = firstPtr;
	FILE * fp;
	fp = fopen (argv[6], "w+");
	
	while(lastPtr!= NULL){
		fprintf(fp,"**thread id is : %ld, and prime number is: %d\n",lastPtr->threadId,lastPtr->number);
		printf("**thread id is : %ld, and prime number is: %d\n",lastPtr->threadId,lastPtr->number);
		lastPtr = lastPtr->nextPtr;
		
		}

	
	
	
	pthread_exit(NULL);
}


