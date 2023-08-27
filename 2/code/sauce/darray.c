#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/darray.h"

DynamicArray *darray_create(uint typeSize) {
  DynamicArray *d = malloc(sizeof(DynamicArray));
  d->typeSize = typeSize;
  d->arr = calloc(1, d->typeSize);
  d->capacity = 1;
  d->size = 0;
  return d;
}

void darray_destroy(DynamicArray *d) {
  free(d->arr);
  free(d);
}

void darray_insert(DynamicArray *d, uint index, void *data) {
  if (index >= d->capacity) {
    while (index >= d->capacity)
      d->capacity *= 2;
    void *tempArr = d->arr;
    d->arr = calloc(d->capacity, d->typeSize);      // make new array
    memcpy(d->arr, tempArr, d->typeSize * d->size); // copy over old array
    memcpy(d->arr + index * d->typeSize, data,
           d->typeSize); // insert data into new array
    d->size = index + 1;
    free(tempArr);
  } else {
    memcpy(d->arr + index * d->typeSize, data, d->typeSize);
    if (d->size <= index) {
      d->size = index + 1;
    }
  }
}

// set dst to a copy of item at index
void darray_get(DynamicArray *d, uint index, void *dst) {
  memcpy(dst, d->arr + (index * d->typeSize), d->typeSize);
}

// return address of item at index
void *darray_at(DynamicArray *d, uint index) {
  return d->arr + index * d->typeSize;
}
