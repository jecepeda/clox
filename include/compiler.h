#ifndef clox_compile_h
#define clox_compile_h

#include "object.h"
#include "vm.h"

#define DEBUG_PRINT_CODE

bool compile(const char *source, Chunk *chunk);

#endif