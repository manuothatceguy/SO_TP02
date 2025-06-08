#ifndef QUEUE_H
#define QUEUE_H

typedef struct Queue * QueueADT;

QueueADT createQueue();

void freeQueue(QueueADT queue);

int enqueue(QueueADT queue, void* data);

void* dequeue(QueueADT queue);

int queueSize(QueueADT queue);

int isQueueEmpty(QueueADT queue);

void* peekQueue(QueueADT queue);

void clearQueue(QueueADT queue);

void* containsQueue(QueueADT queue, void* data, int (*compare)(void*, void*));

void* removeFromQueue(QueueADT queue, void* data, int (*compare)(void*, void*));

void ** dumpQueue(QueueADT queue);

#endif