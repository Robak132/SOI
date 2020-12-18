#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>

struct Queue{
	int length;
	char* *head;
	char* *vip;
	char* *usr;
};
struct Queue queue;
int queue_size;
int queue_entity_size;
int max_size;
int iter, iterfull;
char* active_message;

sem_t QueueLock, QueueFreeSpace, QueueNotEmpty;

void InitSemaphores() {
    sem_init(&QueueLock, 0, 1);
    sem_init(&QueueFreeSpace, 0, queue_size);
	sem_init(&QueueNotEmpty, 0, 0);
}
void RemoveSemaphores() {
    sem_destroy(&QueueLock);
    sem_destroy(&QueueFreeSpace);
    sem_destroy(&QueueNotEmpty);
}

int int_message_queue (int queue_size, int queue_entity_size) {
	queue.head = (char **)malloc(queue_size * sizeof(char*));
	queue.length = 0;
	queue.vip = queue.head;
	queue.usr = queue.head;
	if(queue.head == NULL)
		return -1;
	for (int i =0; i < queue_size; i++)	{
		queue.head[i] = (char *)malloc(queue_entity_size * sizeof(char));
		if(queue.head[i] == NULL)
			return -1;
	}
	return 0;
}
void show_queue(){
	printf("Elements in bufor: %i\n", queue.length);
	for (int i=0;i<queue.length;i++)
		printf("%s, ", queue.head[i]);
	printf("\n");
}

int put_vip_message(char *message, int message_size) {
	if(message_size > queue_entity_size || queue.length >= queue_size-1)
		return -1;
	int diff = queue.vip - queue.head;
	for (int i=queue.length;i>diff;i--)
		queue.head[i] = queue.head[i-1];
	*queue.vip = message;
	queue.vip++;
	queue.usr++;
	queue.length++;
	return 0;
}
int put_normal_message(char *message, int message_size) {
	if(message_size > queue_entity_size || queue.length >= queue_size -1) {
		return -1;
	}
	*queue.usr = message;
	queue.usr++;
	queue.length++;
	return 0;
}
int get_message(int max_message_size) {
	if(queue.length == 0)
		return -1;
	active_message = *queue.head;

	char* cutmess = (char *)malloc(max_message_size * sizeof(char));
	for(int i = 0; i<strlen(active_message);i++)
		cutmess[i] = active_message[i];
	cutmess[max_message_size] = '\0';
	active_message = cutmess;

	for (int i = 0; i < queue.length-1; i++)
		queue.head[i] = queue.head[i+1];
	queue.length--;
	queue.usr--;
	if(queue.vip>queue.head)
		queue.vip--;
	return 0;	
}

void* Writer(void* arg) {
	long int MessagesProcessed = 0;
    int myid = pthread_self() % 1000000;

	printf("[Writer %d] Start\n\n", myid);
	while(MessagesProcessed < iter) {	
		srand(time(NULL));	
		usleep((rand() % 500000));

		sem_wait(&QueueFreeSpace);
		sem_wait(&QueueLock);
		
		char* mess = (char *)malloc(queue_entity_size * 2 * sizeof(char));
		sprintf(mess, "%i:x%li", myid, MessagesProcessed);
		int status = put_normal_message(mess, strlen(mess)+1);
		printf("[Writer %i] %i:x%li\n", myid, myid, MessagesProcessed);
		printf("[Writer %i] Elements in queue: %i\n", myid, queue.length);

		sem_post(&QueueLock);
		if (status == 0)
			sem_post(&QueueNotEmpty);

		MessagesProcessed++;
	}
	printf("\n[Writer %i] Stop\n\n", myid);
}
void* WriterVIP(void* arg) {
	long int MessagesProcessed = 0;
    int myid = pthread_self() % 1000000;

	printf("[VWriter %d] Start\n\n", myid);
	while(MessagesProcessed < iter) {
		srand(time(NULL));	
		usleep((rand() % 500000));

		sem_wait(&QueueFreeSpace);
		sem_wait(&QueueLock);
		
		char* mess = (char *)malloc(queue_entity_size * 2 * sizeof(char));
		sprintf(mess, "%i:v%li", myid, MessagesProcessed);
		int status = put_vip_message(mess, strlen(mess)+1);
		printf("[VWriter %i] %i:v%li\n", myid, myid, MessagesProcessed);
		printf("[VWriter %i] Elements in queue: %i\n", myid, queue.length);

		sem_post(&QueueLock);
		if (status == 0)
			sem_post(&QueueNotEmpty);

		MessagesProcessed++;
	}
	printf("\n[VWriter %i] Stop\n\n", myid);
}
void* Reader(void* arg) {
	long int MessagesProcessed = 0;

	printf("[Reader] Start\n\n");
	while(MessagesProcessed <= iterfull) {
		srand(time(NULL));	
		usleep((rand() % 500000));

		sem_wait(&QueueNotEmpty);
		sem_wait(&QueueLock);
	
		int status = get_message(max_size);

		sem_post(&QueueLock);
		if (status == 0)
			sem_post(&QueueFreeSpace);
		
		printf("[Reader] Reading %s\n", active_message);
		printf("[Reader] Elements in queue: %i\n", queue.length);
		printf("[Reader] Iteration: %li\n\n", MessagesProcessed);
		MessagesProcessed++;
	}
	printf("\n[Reader] End\n\n");
	show_queue();
}


int main(int argc, char *argv[]) {
	queue_size = atoi(argv[1]);
	int n_wrts = atoi(argv[2]);
	int n_vips = atoi(argv[3]);
	queue_entity_size = 15;
	max_size = atoi(argv[4]);
	iter = atoi(argv[5]);
	iterfull = (n_wrts + n_vips) * (iter-1);

 	int_message_queue(queue_size, queue_entity_size);
	InitSemaphores();

	int i;
	for (i=0;i<n_wrts;i++) {
		pthread_t thread;
		pthread_create(&thread, NULL, &Writer, NULL);
	}
	for (i=0;i<n_vips;i++) {
		pthread_t thread;
		pthread_create(&thread, NULL, &WriterVIP, NULL);
	}
	pthread_t thread;
	pthread_create(&thread, NULL, &Reader, NULL);

	RemoveSemaphores();
	pthread_exit(NULL); 
	exit(0);
}