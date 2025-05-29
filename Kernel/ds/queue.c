#include <memoryManager.h>
#include <queue.h>

typedef struct Node {
    void* data;
    struct Node* next;
} Node;

typedef struct Queue {
    Node* front;
    Node* rear;
    int size;
}Queue;

QueueADT createQueue() {
    QueueADT queue = (QueueADT)allocMemory(sizeof(Queue));
    if (queue == NULL) {
        return NULL; 
    }
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
    return queue;
}

void freeQueue(QueueADT queue) {
    if (queue == NULL) {
        return;
    }
    Node* current = queue->front;
    while (current != NULL) {
        Node* next = current->next;
        freeMemory(current);
        current = next;
    }
    freeMemory(queue);
}

int enqueue(QueueADT queue, void* data) {
    if (queue == NULL) {
        return -1;
    }
    Node* newNode = (Node*)allocMemory(sizeof(Node));
    if (newNode == NULL) {
        return -1; 
    }
    newNode->data = data;
    newNode->next = NULL;

    if (queue->rear == NULL) {
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
    queue->size++;
    return 0;
}

void* dequeue(QueueADT queue) {
    if (queue == NULL || queue->front == NULL) {
        return NULL; 
    }
    Node* temp = queue->front;
    void* data = temp->data;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    freeMemory(temp);
    queue->size--;
    return data;
}

int queueSize(QueueADT queue) {
    if (queue == NULL) {
        return 0; 
    }
    return queue->size;
}

int isQueueEmpty(QueueADT queue) {
    if (queue == NULL) {
        return 1; 
    }
    return queue->size == 0;
}

int isEmptyQueue(QueueADT queue) {
    return isQueueEmpty(queue);
}

void* peekQueue(QueueADT queue) {
    if (queue == NULL || queue->front == NULL) {
        return NULL; 
    }
    return queue->front->data;
}

void clearQueue(QueueADT queue) {
    if (queue == NULL) {
        return;
    }
    Node* current = queue->front;
    while (current != NULL) {
        Node* next = current->next;
        freeMemory(current);
        current = next;
    }
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
}

void* containsQueue(QueueADT queue, void* data, int (*compare)(void*, void*)) {
    if (queue == NULL || compare == NULL) {
        return NULL; 
    }
    Node* current = queue->front;
    while (current != NULL) {
        if (compare(current->data, data) == 0) {
            return current->data; 
        }
        current = current->next;
    }
    return NULL; 
}