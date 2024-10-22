#ifndef clox_compile_h
#define clox_compile_h

#include "object.h"
#include "vm.h"

ObjFunction *compile(const char *source);

#endif