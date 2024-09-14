#include "../include/vm.h"
#include "../include/common.h"
#include "../include/compiler.h"
// #include "../include/debug.h"
#include "../include/stack.h"

#include <stdarg.h>
#include <stdio.h>

VM vm;

void initVM(Chunk *c) {
  vm.chunk = c;
  initChunk(vm.chunk);
  initStack(&vm.stack);
}

void freeVM() {
  // for now, ignore
  // if (vm.chunk->code != NULL) {
  //   freeChunk(vm.chunk);
  // }
  // if (vm.stack.items != NULL) {
  //   freeStack(&vm.stack);
  // }
}

static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void runtimeError(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = vm.ip - vm.chunk->code - 1;
  int line = getLine(&vm.chunk->lines, instruction);
  fprintf(stderr, "[line %d] in script\n", line);
  resetStack(&vm.stack);
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define BINARY_OP(valueType, op)                                               \
  do {                                                                         \
    if (!IS_NUMBER(stackPeek(&vm.stack, 0)) ||                                 \
        !IS_NUMBER(stackPeek(&vm.stack, 1))) {                                 \
      runtimeError("Operands must be numbers.");                               \
      return INTERPRET_RUNTIME_ERROR;                                          \
    }                                                                          \
    double b = AS_NUMBER(stackPop(&vm.stack));                                 \
    double a = AS_NUMBER(stackPop(&vm.stack));                                 \
    stackPush(&vm.stack, valueType(a op b));                                   \
  } while (false)
#define READ_CONSTANT(isLong)                                                  \
  (vm.chunk->constants.values[isLong ? ((READ_BYTE() << 16) |                  \
                                        (READ_BYTE() << 8) | (READ_BYTE()))    \
                                     : READ_BYTE()])
  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    if (vm.stack.top > 0) {
      printf("          ");
      for (int i = 0; i < vm.stack.top; i++) {
        printf("[ ");
        printValue(vm.stack.items[i]);
        printf(" ]");
      }
      printf("\n");
    }
    disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
    uint8_t instruction;
    bool isLong = false;
    switch (instruction = READ_BYTE()) {
    case OP_CONSTANT_LONG:
      isLong = true; // don't break, fall through
    case OP_CONSTANT: {
      Value constant = READ_CONSTANT(isLong);
      stackPush(&vm.stack, constant);
      break;
    }
    case OP_ADD:
      BINARY_OP(NUMBER_VAL, +);
      break;
    case OP_SUBTRACT:
      BINARY_OP(NUMBER_VAL, -);
      break;
    case OP_MULTIPLY:
      BINARY_OP(NUMBER_VAL, *);
      break;
    case OP_DIVIDE:
      BINARY_OP(NUMBER_VAL, /);
      break;
    case OP_NOT:
      stackPush(&vm.stack, BOOL_VAL(isFalsey(stackPop(&vm.stack))));
      break;
    case OP_EQUAL: {
      Value b = stackPop(&vm.stack);
      Value a = stackPop(&vm.stack);
      stackPush(&vm.stack, BOOL_VAL(valuesEqual(a, b)));
      break;
    }
    case OP_NEGATE:
      if (!IS_NUMBER(stackPeek(&vm.stack, 0))) {
        runtimeError("Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      stackPush(&vm.stack, NUMBER_VAL(-AS_NUMBER(stackPop(&vm.stack))));
      break;
    case OP_GREATER:
      BINARY_OP(BOOL_VAL, >);
      break;
    case OP_LESS:
      BINARY_OP(BOOL_VAL, <);
      break;
    case OP_RETURN:
      printValue(stackPop(&vm.stack));
      printf("\n");
      return INTERPRET_OK;
      break;
    case OP_NIL:
      stackPush(&vm.stack, NIL_VAL);
      break;
    case OP_TRUE:
      stackPush(&vm.stack, BOOL_VAL(true));
      break;
    case OP_FALSE:
      stackPush(&vm.stack, BOOL_VAL(false));
      break;
    default: {
      printf("Unknown opcode %d\n", instruction);
      return INTERPRET_RUNTIME_ERROR;
    }
    }
  }
}

#undef READ_CONSTANT
#undef BINARY_OP
#undef READ_BYTE

InterpretResult interpret(const char *source) {
  Chunk chunk;
  initChunk(&chunk);

  if (!compile(source, &chunk)) {
    freeChunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  InterpretResult result = run();

  // freeChunk(&chunk);
  // freeStack(&vm.stack);
  return result;
}