#ifndef clox_stack_h
#define clox_stack_h

#include "common.h"
#include "value.h"

typedef struct {
    Value* items;
    int top;
    int capacity;
} Stack;

void stackPush(Stack* stack, Value value);
Value stackPop(Stack* stack);
void initStack(Stack* stack);
void freeStack(Stack* stack);

#endif