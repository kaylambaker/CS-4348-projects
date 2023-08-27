#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <sys/types.h>

typedef struct Node {
  // point to data
  void *data;
  // point to next node
  struct Node *next;
} Node;

typedef struct {
  uint len, typeSize;
  Node *head;
  Node *tail;
} Queue;

// create queue that stores elements that are typeSize bytes
extern Queue *queue_create(uint typeSize);
// nuke queue
extern void queue_destroy(Queue *q);
// enqueue value at data
extern void queue_enqueue(Queue *q, void *data);
// dequeued element gets stored in data
extern void queue_dequeue(Queue *q, void *data);

extern bool queue_empty(Queue *q);

#endif
