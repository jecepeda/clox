#include "vm.h"
#include "chunk.h"
#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "stack.h"
#include "value.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// #include "debug.h"

static void defineNative(const char *name, NativeFn function);

VM vm;

static Value clockNative(int argCount, Value *args) {
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

void initVM() {
  initStack(&vm.stack);
  initTable(&vm.strings);
  initTable(&vm.globals);
  defineNative("clock", clockNative);
  vm.openUpvalues = NULL;
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
  vm.frameCount = 0;
  vm.openUpvalues = NULL;
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

  for (int i = vm.frameCount - 1; i >= 0; i--) {
    CallFrame *frame = &vm.frames[i];
    ObjFunction *function = frame->closure->function;
    int instruction = frame->ip - function->chunk.code - 1;
    int line = getLine(&frame->closure->function->chunk.lines, instruction);
    fprintf(stderr, "[line %d] in ", line);
    if (function->name == NULL) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->chars);
    }
  }

  resetStack(&vm.stack);
}

static void defineNative(const char *name, NativeFn function) {
  stackPush(&vm.stack, OBJ_VAL(copyString(name, (int)strlen(name))));
  stackPush(&vm.stack, OBJ_VAL(newNative(function)));
  tableSet(&vm.globals, AS_STRING(stackPeek(&vm.stack, 1)),
           stackPeek(&vm.stack, 0));
  stackPop(&vm.stack);
  stackPop(&vm.stack);
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

static bool call(ObjClosure *closure, int argCount) {
  if (argCount != closure->function->arity) {
    runtimeError("Expected %d arguments but got %d.", closure->function->arity,
                 argCount);
    return false;
  }
  if (vm.frameCount == FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }
  CallFrame *frame = &vm.frames[vm.frameCount++];
  frame->closure = closure;
  frame->ip = closure->function->chunk.code;
  frame->base = vm.stack.top - argCount - 1;
  return true;
}

static bool callValue(Value callee, int argCount) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
    case OBJ_CLOSURE:
      return call(AS_CLOSURE(callee), argCount);
    case OBJ_NATIVE: {
      NativeFn native = AS_NATIVE(callee);
      Value result = native(argCount, &vm.stack.items[vm.stack.top - argCount]);
      vm.stack.top -= argCount + 1;
      stackPush(&vm.stack, result);
      return true;
    }
    default:
      break; // non-callable object type
    }
  }
  runtimeError("Can only call functions and classes.");
  return false;
}

static ObjUpvalue *captureUpvalue(int local) {
  ObjUpvalue *prevUpvalue = NULL;
  ObjUpvalue *upvalue = vm.openUpvalues;
  while (upvalue != NULL && upvalue->location > local) {
    prevUpvalue = upvalue;
    upvalue = upvalue->next;
  }

  if (upvalue != NULL && upvalue->location == local) {
    return upvalue;
  }

  ObjUpvalue *createdUpvalue = newUpvalue(local);
  createdUpvalue->next = upvalue;

  if (prevUpvalue == NULL) {
    vm.openUpvalues = createdUpvalue;
  } else {
    prevUpvalue->next = createdUpvalue;
  }
  return createdUpvalue;
}

static void closeUpvalues(int last) {
  while (vm.openUpvalues != NULL && vm.openUpvalues->location >= last) {
    ObjUpvalue *upvalue = vm.openUpvalues;
    upvalue->closed = vm.stack.items[upvalue->location];
    upvalue->location = -1;
    vm.openUpvalues = upvalue->next;
  }
}

static InterpretResult run() {
  CallFrame *frame = &vm.frames[vm.frameCount - 1];
#define READ_BYTE() (*frame->ip++)
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
  (frame->closure->function->chunk.constants                                   \
       .values[isLong ? ((READ_BYTE() << 16) | (READ_BYTE() << 8) |            \
                         (READ_BYTE()))                                        \
                      : READ_BYTE()])
#define READ_SHORT()                                                           \
  (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_SLOT(isLong)                                                      \
  isLong ? ((READ_BYTE() << 16) | (READ_BYTE() << 8) | (READ_BYTE()))          \
         : READ_BYTE()
#define READ_STRING(isLong) AS_STRING(READ_CONSTANT(isLong))
  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("--------------------------\n");
    if (vm.stack.top > 0) {
      printf("          ");
      for (int i = 0; i < vm.stack.top; i++) {
        printf("[ ");
        printValue(vm.stack.items[i]);
        printf(" ]");
      }
      printf("\n");
    }
    disassembleInstruction(
        &frame->closure->function->chunk,
        (int)(frame->ip - frame->closure->function->chunk.code));
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
      stackPush(&vm.stack, vm.stack.items[frame->base + slot]);
      break;
    }
    case OP_SET_LOCAL_LONG:
      isLong = true; // don't break, fall through
    case OP_SET_LOCAL: {
      uint32_t slot = READ_SLOT(isLong);
      vm.stack.items[frame->base + slot] = stackPeek(&vm.stack, 0);
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
    case OP_SET_UPVALUE_LONG:
      isLong = true; // don't break, fall through
    case OP_SET_UPVALUE: {
      uint32_t slot = READ_SLOT(isLong);
      int pos = frame->closure->upvalues[slot]->location;
      if (pos == -1) {
        frame->closure->upvalues[slot]->closed = stackPeek(&vm.stack, 0);
      } else {
        vm.stack.items[pos] = stackPeek(&vm.stack, 0);
      }
      break;
    }
    case OP_DEFINE_GLOBAL_LONG:
      isLong = true; // don't break, fall through
    case OP_DEFINE_GLOBAL: {
      ObjString *name = READ_STRING(isLong);
      tableSet(&vm.globals, name, stackPeek(&vm.stack, 0));
      stackPop(&vm.stack);
      break;
    }
    case OP_GET_UPVALUE_LONG:
      isLong = true; // don't break, fall through
    case OP_GET_UPVALUE: {
      uint32_t slot = READ_SLOT(isLong);
      int pos = frame->closure->upvalues[slot]->location;
      if (pos == -1) {
        stackPush(&vm.stack, frame->closure->upvalues[slot]->closed);
      } else {
        stackPush(&vm.stack, vm.stack.items[pos]);
      }
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
      frame->ip += offset;
      break;
    }
    case OP_JUMP_IF_FALSE: {
      uint16_t offset = READ_SHORT();
      if (isFalsey(stackPeek(&vm.stack, 0))) {
        frame->ip += offset;
      }
      break;
    }
    case OP_LOOP: {
      uint16_t offset = READ_SHORT();
      frame->ip -= offset;
      break;
    }
    case OP_GREATER:
      BINARY_OP(BOOL_VAL, >);
      break;
    case OP_LESS:
      BINARY_OP(BOOL_VAL, <);
      break;
    case OP_CALL: {
      int argCount = READ_BYTE();
      if (!callValue(stackPeek(&vm.stack, argCount), argCount)) {
        return INTERPRET_RUNTIME_ERROR;
      }
      frame = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_CLOSE_UPVALUE:
      closeUpvalues(vm.stack.top - 1);
      stackPop(&vm.stack);
      break;
    case OP_RETURN: {
      Value result = stackPop(&vm.stack);
      closeUpvalues(frame->base);
      vm.frameCount--;
      if (vm.frameCount == 0) {
        stackPop(&vm.stack);
        return INTERPRET_OK;
      }

      // put the stack pointer back to before the function call
      vm.stack.top = frame->base;
      stackPush(&vm.stack, result);
      frame = &vm.frames[vm.frameCount - 1];
      break;
    }
    case OP_NIL:
      stackPush(&vm.stack, NIL_VAL);
      break;
    case OP_TRUE:
      stackPush(&vm.stack, BOOL_VAL(true));
      break;
    case OP_FALSE:
      stackPush(&vm.stack, BOOL_VAL(false));
      break;
    case OP_CLOSURE_LONG:
      isLong = true; // don't break, fall through
    case OP_CLOSURE: {
      ObjFunction *function = AS_FUNCTION(READ_CONSTANT(isLong));
      ObjClosure *closure = newClosure(function);
      int count = closure->upvalueCount;
      for (int i = 0; i < closure->upvalueCount; i++) {
        uint8_t isLocal = READ_BYTE();
        uint32_t index = READ_SLOT(true);
        if (isLocal) {
          int pos = frame->base + index;
          closure->upvalues[i] = captureUpvalue(pos);
        } else {
          ObjUpvalue *upValue = frame->closure->upvalues[index];
          closure->upvalues[i] = upValue;
        }
      }
      stackPush(&vm.stack, OBJ_VAL(closure));
      break;
    }
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
  ObjFunction *function = compile(source);
  if (function == NULL)
    return INTERPRET_COMPILE_ERROR;

  stackPush(&vm.stack, OBJ_VAL(function));
  ObjClosure *closure = newClosure(function);
  stackPop(&vm.stack);
  stackPush(&vm.stack, OBJ_VAL(closure));
  call(closure, 0);

  InterpretResult result = run();
  resetStack(&vm.stack);
  return result;
}