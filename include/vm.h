#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "object.h"
#include "stack.h"
#include "table.h"

#define FRAMES_MAX 64

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

typedef struct {
  ObjClosure *closure;
  uint8_t *ip;
  int base;
} CallFrame;

typedef struct {
  CallFrame frames[FRAMES_MAX];
  int frameCount;

  Stack stack;
  Obj *objects;
  Table strings;
  ObjString *initString;
  Table globals;
  ObjUpvalue *openUpvalues;

  size_t bytesAllocated;
  size_t nextGC;
  // GC
  int grayCount;
  int grayCapacity;
  Obj **grayStack;
} VM;

extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char *source);

#endif