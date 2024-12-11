#ifndef clox_object_h
#define clox_object_h

#include "chunk.h"
#include "common.h"
#include "table.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_CLASS(value) isObjType(value, OBJ_CLASS)
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)

#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CLASS(value) ((ObjClass *)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)
#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))

typedef enum {
  OBJ_FUNCTION,
  OBJ_INSTANCE,
  OBJ_UPVALUE,
  OBJ_NATIVE,
  OBJ_STRING,
  OBJ_CLASS,
  OBJ_CLOSURE,
} ObjType;

struct Obj {
  ObjType type;
  bool isMarked;
  struct Obj *next;
};

typedef struct {
  Obj obj;
  int arity;
  int upvalueCount;
  Chunk chunk;
  ObjString *name;
} ObjFunction;

typedef struct ObjUpvalue {
  Obj obj;
  int location;
  Value closed;
  struct ObjUpvalue *next;
} ObjUpvalue;

typedef Value (*NativeFn)(int argCount, Value *args);

typedef struct {
  Obj obj;
  NativeFn function;
} ObjNative;

typedef struct {
  Obj obj;
  ObjFunction *function;
  ObjUpvalue **upvalues;
  int upvalueCount;
} ObjClosure;

typedef struct {
  Obj obj;
  ObjString *name;
} ObjClass;

typedef struct {
  Obj obj;
  ObjClass *klass;
  Table fields;
} ObjInstance;

struct ObjString {
  Obj obj;
  int length;
  uint32_t hash;
  char chars[];
};

ObjClosure *newClosure(ObjFunction *function);
ObjInstance *newInstance(ObjClass *klass);
ObjClass *newClass(ObjString *name);
ObjFunction *newFunction();
ObjNative *newNative(NativeFn function);
ObjUpvalue *newUpvalue(int pos);
ObjString *copyString(const char *chars, int length);
ObjString *createString(int length);
void printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
