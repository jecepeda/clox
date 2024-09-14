#ifndef clox_compile_h
#define clox_compile_h

#include "vm.h"

bool compile(const char *source, Chunk *chunk);

#endif