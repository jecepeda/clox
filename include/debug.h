#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"

#define DEBUG_TRACE_EXECUTION
#define DEBUG_PRINT_CODE

void disassembleChunk(Chunk *chunk, const char *name);
int disassembleInstruction(Chunk *chunk, int offset);

#endif
