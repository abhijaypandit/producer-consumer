#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

// Header containing configuration for the program - MODIFY AS NEEDED
#include "config.h"

// Semaphores and mutex
sem_t empty; // Indicates buffer is empty
sem_t ready; // Indicates buffer has data
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Controls access to shared resource

#define BUFFER_SIZE NUM_MSG

// Global buffer for messages
struct message {
	int data; // random data
	int time; // timestamp
} buffer[BUFFER_SIZE];

// Indices of first and last elements of buffer
int in = 0;
int out = 0;

// Producer and consumer function declarations
void *producer(void *arg);
void *consumer(void *arg);

// Producer and consumer thread arguments
struct tid {
		int id;	// thread id
		int count;	// message count
} pid[NUM_PRO], cid[NUM_CON];


int main() {
	pthread_t pthread[NUM_PRO]; // producer threads
	pthread_t cthread[NUM_CON]; // consumer threads

	// Initialize mutex
	pthread_mutex_init(&mutex, NULL);
	
	// Initialize semaphores
	sem_init(&empty, 0, BUFFER_SIZE);
	sem_init(&ready, 0, 0);

	int pi = 0, ci = 0;
	for(int i=0; i<NUM_MSG; i++) {
		// Initialize message count for each producer thread
		if(i < NUM_PRO) pid[pi].count = 1;
		else pid[pi].count += 1;
		pi = (pi+1)%NUM_PRO; 

		// Initialize message count for each consumer thread
		if(i < NUM_CON) cid[ci].count = 1;
		else cid[ci].count += 1;
		ci = (ci+1)%NUM_CON; 
	}

	for(int i=0; i<NUM_PRO; i++) {
		pid[i].id = i; // Initialize thread id
		pthread_create(&pthread[i], NULL, producer, &pid[i]); // Create producer threads
	}

	for(int i=0; i<NUM_CON; i++) {	
		cid[i].id = i; // Initialize thread id
		pthread_create(&cthread[i], NULL, consumer, &cid[i]); // Create consumer threads
	}

	// Wait for producer and consumer threads to complete
	for(int i=0; i<NUM_PRO; i++) pthread_join(pthread[i], NULL);
	for(int i=0; i<NUM_CON; i++) pthread_join(cthread[i], NULL);

	// Destroy mutex and semaphores
	pthread_mutex_destroy(&mutex);
	sem_destroy(&empty);
	sem_destroy(&ready);

	return 0;
}


void *producer(void *arg) {
	int id = ((struct tid *)arg)->id;
	int count = ((struct tid *)arg)->count;
	//printf("Producer %d started, produces %d message(s).\n", id, count);
	printf("Producer %d started.\n", id);

	int data;
	struct timeval time;

	for(int i=0; i<count; i++) {
		// Decrement count of empty slots in buffer
		sem_wait(&empty);
		
		// Acquire lock to buffer
		pthread_mutex_lock(&mutex);
		
		// Insert data item into buffer
		data = rand();
		gettimeofday(&time, NULL);
		buffer[in].data = data%1000000;
		buffer[in].time = time.tv_usec;
		printf("Producer %d - Insert item %d: data = %d, timestamp = %d\n", id, in, buffer[in].data, buffer[in].time);
		in = in + 1;
		
		// Release lock to buffer
		pthread_mutex_unlock(&mutex);
		
		// Increment count of available data in buffer
		sem_post(&ready);
	}
	
	printf("Producer %d ended.\n", id);

	return (void *)NULL;
}


void *consumer(void *arg) {
	int id = ((struct tid *)arg)->id;
	int count = ((struct tid *)arg)->count;
	//printf("Counsumer %d started, consumes %d message(s).\n", id, count);
	printf("Consumer %d started.\n", id);

	struct message item;

	for(int i=0; i<count; i++) {
		// Decrement count of available data in buffer
		sem_wait(&ready);
		
		// Acquire lock to buffer
		pthread_mutex_lock(&mutex);
		
		// Remove data item from buffer
		item = buffer[out];
		printf("Consumer %d - Remove item %d: data = %d, timestamp = %d\n", id, out, item.data, item.time);
		out = out + 1;
		
		// Release lock to buffer
		pthread_mutex_unlock(&mutex);
	
		// Increment count of empty slots in buffer
		sem_post(&empty);
	}
	
	printf("Consumer %d ended.\n", id);

	return (void *)NULL;
}
