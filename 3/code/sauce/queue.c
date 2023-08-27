#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/queue.h"

Queue *queue_create(unsigned int typeSize) {
  Queue *q = malloc(sizeof(Queue));
  q->len = 0;
  q->head = q->tail = NULL;
  q->typeSize = typeSize;
  return q;
}

void queue_destroy(Queue *q) {
  Node *n = q->head;
  while (n) {
    q->head = q->head->next;
    free(n->data);
    free(n);
    n = q->head;
  }
  free(q);
}

void queue_enqueue(Queue *q, void *data) {
  Node *newNode = malloc(sizeof(Node));
  newNode->data = malloc(q->typeSize);
  newNode->next = NULL;
  memcpy(newNode->data, data, q->typeSize);
  if (q->len == 0) {
    q->head = q->tail = newNode;
  } else {
    q->tail->next = newNode;
    q->tail = newNode;
  }
  q->len++;
}

void queue_dequeue(Queue *q, void *data) {
  if (q->len == 0)
    return;
  Node *n = q->head;
  memcpy(data, n->data, q->typeSize);
  if (q->len > 1) {
    q->head = q->head->next;
  } else {
    q->head = q->tail = NULL;
  }
  free(n->data);
  free(n);
  q->len--;
}

bool queue_empty(Queue *q) { return q->len == 0; }
