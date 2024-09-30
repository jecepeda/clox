#include "stack.h"
#include "memory.h"

void stackPush(Stack *stack, Value value) {
  if (stack->capacity <= stack->top) {
    int oldCapacity = stack->capacity;
    stack->capacity = GROW_CAPACITY(oldCapacity);
    stack->items =
        GROW_ARRAY(Value, stack->items, oldCapacity, stack->capacity);
  }
  stack->items[stack->top] = value;
  stack->top++;
}

Value stackPop(Stack *stack) {
  stack->top--;
  return stack->items[stack->top];
}

Value stackPeek(Stack *stack, int distance) {
  return stack->items[stack->top - 1 - distance];
}

void initStack(Stack *stack) {
  stack->capacity = 0;
  stack->top = 0;
  stack->items = NULL;
}

void freeStack(Stack *stack) {
  FREE_ARRAY(Value, stack->items, stack->capacity);
}

void resetStack(Stack *stack) { stack->top = 0; }