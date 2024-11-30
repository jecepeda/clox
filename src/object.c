#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType)                                         \
  (type *)allocateObject(sizeof(type), objectType)

#define ALLOCATE_STR(type, totalSize, objectType)                              \
  (type *)allocateObject(totalSize, objectType)

static Obj *allocateObject(size_t size, ObjType type) {
  Obj *object = (Obj *)reallocate(NULL, 0, size);
  object->type = type;
  object->next = vm.objects;
  vm.objects = object;
  return object;
}

ObjFunction *newFunction() {
  ObjFunction *function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
  function->arity = 0;
  function->upvalueCount = 0;
  function->name = NULL;
  initChunk(&function->chunk);
  return function;
}

ObjClosure *newClosure(ObjFunction *function) {
  ObjUpvalue **upvalues = ALLOCATE(ObjUpvalue *, function->upvalueCount);

  for (int i = 0; i < function->upvalueCount; i++) {
    upvalues[i] = NULL;
  }

  ObjClosure *closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
  closure->function = function;
  closure->upvalues = upvalues;
  closure->upvalueCount = function->upvalueCount;
  return closure;
}

ObjNative *newNative(NativeFn function) {
  ObjNative *native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
  native->function = function;
  return native;
}

ObjUpvalue *newUpvalue(int pos) {
  ObjUpvalue *upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
  upvalue->location = pos;
  upvalue->next = NULL;
  upvalue->closed = NIL_VAL;
  return upvalue;
}

static uint32_t hashString(const char *key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

static ObjString *allocateString(int length) {
  ObjString *string =
      ALLOCATE_STR(ObjString, sizeof(ObjString) + length + 1, OBJ_STRING);
  string->length = length;
  return string;
}

ObjString *createString(int lenght) { return allocateString(lenght); }

ObjString *copyString(const char *chars, int length) {
  // we first try to look if the string already exists in the table
  uint32_t hash = hashString(chars, length);
  ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL) {
    return interned;
  }
  // if it doesn't exist, we create a new string
  ObjString *string = allocateString(length);
  memcpy(string->chars, chars, length);
  string->hash = hash;
  string->chars[length] = '\0';
  // we add the string to the hash table
  tableSet(&vm.strings, string, NIL_VAL);
  return string;
}

static void printFunction(ObjFunction *function) {
  if (function->name == NULL) {
    printf("<script>");
    return;
  }
  printf("<fn %s>", function->name->chars);
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
  case OBJ_CLOSURE:
    printFunction(AS_CLOSURE(value)->function);
    break;
  case OBJ_STRING: {
    ObjString *string = AS_STRING(value);
    printf("%s", AS_CSTRING(value));
    break;
  }
  case OBJ_FUNCTION:
    printFunction(AS_FUNCTION(value));
    break;
  case OBJ_NATIVE:
    printf("<native fn>");
    break;
  case OBJ_UPVALUE:
    printf("upvalue");
    break;
  }
}
