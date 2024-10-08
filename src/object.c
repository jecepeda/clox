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
      ALLOCATE_STR(ObjString, sizeof(ObjString) + length, OBJ_STRING);
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

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
  case OBJ_STRING:
    printf("%s", AS_CSTRING(value));
    break;
  }
}
