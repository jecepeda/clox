#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "line.h"
#include "value.h"

typedef enum {
  OP_CONSTANT,      // 2 bytes (code, operand)
  OP_CONSTANT_LONG, // 4 bytes (1 code, 3 operand)
  // 1 byte
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_NOT,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NEGATE,
  OP_RETURN,
} OpCode;

typedef struct {
  // same as a byte
  uint8_t *code;
  int count;
  int capacity;
  LineArray lines;
  ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
void freeChunk(Chunk *chunk);
void writeConstant(Chunk *chunk, Value value, int line);

#endif
