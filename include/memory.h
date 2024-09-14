#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"

// macro that tells you by how much do you increase the capacity of the array
#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

// macro that automatically calls the function that increases
// the capacity of the array
#define GROW_ARRAY(type, pointer, newCount)                                    \
  ((type *)reallocate(pointer, sizeof(type) * (newCount)))

// macro that calls the function to free the array
#define FREE_ARRAY(type, pointer) reallocate(pointer, 0)

void *reallocate(void *pointer, size_t newSize);

#endif
