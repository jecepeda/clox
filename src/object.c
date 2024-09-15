#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType)                                         \
  (type *)allocateObject(sizeof(type), objectType)

#define ALLOCATE_STR(type, totalSize, objectType)                              \
  (type *)allocateObject(totalSize, objectType)

static Obj *allocateObject(size_t size, ObjType type) {
  Obj *object = (Obj *)reallocate(NULL, size);
  object->type = type;
  object->next = vm.objects;
  vm.objects = object;
  return object;
}

static ObjString *allocateString(int length) {
  ObjString *string =
      ALLOCATE_STR(ObjString, sizeof(ObjString) + length, OBJ_STRING);
  string->length = length;
  return string;
}

ObjString *createString(int lenght) { return allocateString(lenght); }

ObjString *copyString(const char *chars, int length) {
  ObjString *string = allocateString(length);
  memcpy(string->chars, chars, length);
  string->chars[length] = '\0';
  return string;
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
  case OBJ_STRING:
    printf("%s", AS_CSTRING(value));
    break;
  }
}