#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"
#include "value.h"

#define ALLOCATE(type, count)                                                  \
  (type *)reallocate(NULL, 0, sizeof(type) * (count), true)

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0, true)

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY_NO_GC(type, pointer, oldCount, newCount)                    \
  (type *)reallocate(pointer, sizeof(type) * (oldCount),                       \
                     sizeof(type) * (newCount), false)

#define GROW_ARRAY(type, pointer, oldCount, newCount)                          \
  (type *)reallocate(pointer, sizeof(type) * (oldCount),                       \
                     sizeof(type) * (newCount), true)

#define FREE_ARRAY(type, pointer, oldCount)                                    \
  reallocate(pointer, sizeof(type) * (oldCount), 0, false)

void *reallocate(void *pointer, size_t oldSize, size_t newSize, bool triggerGC);

void freeObjects();
void collectGarbage();
void markValue(Value value);
void markObject(Obj *object);

#endif
