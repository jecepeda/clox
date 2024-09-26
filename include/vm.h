#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "stack.h"
#include "table.h"

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

typedef struct {
  Chunk *chunk;
  uint8_t *ip; // instruction pointer
  Stack stack;
  Obj *objects;
  Table strings;
} VM;

extern VM vm;

void initVM(Chunk *c);
void freeVM();
InterpretResult interpret(const char *source);

#endif