#include "vm.h"
#include "chunk.h"
#include "common.h"
#include "compiler.h"
// #include "debug.h"
#include "memory.h"
#include "stack.h"
#include "value.h"

#include <_printf.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

VM vm;

void initVM(Chunk *c) {
  vm.chunk = c;
  initChunk(vm.chunk);
  initStack(&vm.stack);
  initTable(&vm.strings);
  initTable(&vm.globals);
  vm.objects = NULL;
}

void freeVM() {
  freeObjects();
  freeTable(&vm.strings);
  freeTable(&vm.globals);
  // for now, ignore
  // if (vm.chunk->count > 0) {
  //   freeChunk(vm.chunk);
  // }
  if (vm.stack.capacity > 0) {
    freeStack(&vm.stack);
  }
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

static void concatenate() {
  ObjString *b = AS_STRING(stackPop(&vm.stack));
  ObjString *a = AS_STRING(stackPop(&vm.stack));

  int length = a->length + b->length;
  ObjString *result = createString(length);

  memcpy(result->chars, a->chars, a->length);
  memcpy(result->chars + a->length, b->chars, b->length);
  result->chars[length] = '\0';

  stackPush(&vm.stack, OBJ_VAL(result));
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
#define READ_SHORT() (vm.ip += 2, (uint16_t)((vm.ip[-2] << 8) | vm.ip[-1]))
#define READ_SLOT(isLong)                                                      \
  isLong ? ((READ_BYTE() << 16) | (READ_BYTE() << 8) | (READ_BYTE()))          \
         : READ_BYTE()
#define READ_STRING(isLong) AS_STRING(READ_CONSTANT(isLong))
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
    case OP_ADD: {
      if (IS_STRING(stackPeek(&vm.stack, 0)) &&
          IS_STRING(stackPeek(&vm.stack, 1))) {
        concatenate();
      } else if (IS_NUMBER(stackPeek(&vm.stack, 0)) &&
                 IS_NUMBER(stackPeek(&vm.stack, 1))) {
        double b = AS_NUMBER(stackPop(&vm.stack));
        double a = AS_NUMBER(stackPop(&vm.stack));
        stackPush(&vm.stack, NUMBER_VAL(a + b));
      } else {
        runtimeError("Operands must be two numbers or two strings.");
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
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
    case OP_GET_LOCAL_LONG:
      isLong = true; // don't break, fall through
    case OP_GET_LOCAL: {
      uint32_t slot = READ_SLOT(isLong);
      stackPush(&vm.stack, vm.stack.items[slot]);
      break;
    }
    case OP_SET_LOCAL_LONG:
      isLong = true; // don't break, fall through
    case OP_SET_LOCAL: {
      uint32_t slot = READ_SLOT(isLong);
      vm.stack.items[slot] = stackPeek(&vm.stack, 0);
      break;
    }
    case OP_NEGATE:
      if (!IS_NUMBER(stackPeek(&vm.stack, 0))) {
        runtimeError("Operand must be a number.");
        return INTERPRET_RUNTIME_ERROR;
      }
      stackPush(&vm.stack, NUMBER_VAL(-AS_NUMBER(stackPop(&vm.stack))));
      break;
    case OP_PRINT: {
      printValue(stackPop(&vm.stack));
      printf("\n");
      break;
    }
    case OP_POP:
      stackPop(&vm.stack);
      break;
    case OP_DEFINE_GLOBAL_LONG:
      isLong = true; // don't break, fall through
    case OP_DEFINE_GLOBAL: {
      ObjString *name = READ_STRING(isLong);
      tableSet(&vm.globals, name, stackPeek(&vm.stack, 0));
      stackPop(&vm.stack);
      break;
    }
    case OP_GET_GLOBAL_LONG:
      isLong = true; // don't break, fall through
    case OP_GET_GLOBAL: {
      ObjString *name = READ_STRING(isLong);
      Value value;
      if (!tableGet(&vm.globals, name, &value)) {
        runtimeError("Undefined variable '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      stackPush(&vm.stack, value);
      break;
    }
    case OP_SET_GLOBAL_LONG:
      isLong = true; // don't break, fall through
    case OP_SET_GLOBAL: {
      ObjString *name = READ_STRING(isLong);
      if (tableSet(&vm.globals, name, stackPeek(&vm.stack, 0))) {
        tableDelete(&vm.globals, name);
        runtimeError("Undefined variable '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
      }
      break;
    }
    case OP_JUMP: {
      uint16_t offset = READ_SHORT();
      vm.ip += offset;
      break;
    }
    case OP_JUMP_IF_FALSE: {
      uint16_t offset = READ_SHORT();
      if (isFalsey(stackPeek(&vm.stack, 0))) {
        vm.ip += offset;
      }
      break;
    }
    case OP_LOOP: {
      uint16_t offset = READ_SHORT();
      vm.ip -= offset;
      break;
    }
    case OP_GREATER:
      BINARY_OP(BOOL_VAL, >);
      break;
    case OP_LESS:
      BINARY_OP(BOOL_VAL, <);
      break;
    case OP_RETURN:
      // exit interpreter
      return INTERPRET_OK;
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

#undef READ_SHORT
#undef READ_SLOT
#undef READ_CONSTANT
#undef READ_STRING
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

  freeChunk(&chunk);
  return result;
}