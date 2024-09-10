#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"
#include "line.h"

typedef enum {
  OP_CONSTANT, // 2 bytes (code, operand)
  OP_CONSTANT_LONG, // 4 bytes (1 code, 3 operand)
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NEGATE,
  OP_RETURN, // 1 byte
} OpCode;

typedef struct {
  // same as a byte
  uint8_t* code;
  int count;
  int capacity;
  LineArray lines;
  ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
void freeChunk(Chunk* chunk);
int addConstant(Chunk* chunk, Value value);
void writeConstant(Chunk* chunk, Value value, int line);

#endif
