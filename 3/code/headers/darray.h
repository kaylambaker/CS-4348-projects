#ifndef DARRAY_H
#define DARRAY_H

#include <sys/types.h>

typedef struct {
  uint size, capacity, typeSize;
  void *arr;
} DynamicArray;

DynamicArray *darray_create(uint typeSize);

void darray_destroy(DynamicArray *d);

void darray_insert(DynamicArray *d, uint index, void *data);

// return copy of item at index
void darray_get(DynamicArray *d, uint index, void *dst);

// return address of item at index
void *darray_at(DynamicArray *d, uint index);

#endif
