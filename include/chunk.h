#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "line.h"
#include "value.h"

#define WRITE_CHUNK_LONG(chunk, variable, line)                                \
  do {                                                                         \
    uint8_t a, b, c;                                                           \
    c = variable & 0xFF;                                                       \
    variable >>= 0x8;                                                          \
    b = variable & 0xFF;                                                       \
    variable >>= 0x8;                                                          \
    a = variable & 0xFF;                                                       \
    writeChunk(chunk, a, line);                                                \
    writeChunk(chunk, b, line);                                                \
    writeChunk(chunk, c, line);                                                \
  } while (false)

typedef enum {
  // constants
  OP_CONSTANT,      // 2 bytes (code, operand)
  OP_CONSTANT_LONG, // 4 bytes (1 code, 3 operand)
  // globals
  OP_DEFINE_GLOBAL,      // 2 bytes (code, operand)
  OP_DEFINE_GLOBAL_LONG, // 4 bytes (1 code, 3 operand)
  OP_SET_GLOBAL,         // 2 bytes (code, operand)
  OP_SET_GLOBAL_LONG,    // 4 bytes (1 code, 3 operand)
  OP_GET_GLOBAL,         // 2 bytes (code, operand)
  OP_GET_GLOBAL_LONG,    // 4 bytes (1 code, 3 operand)
  // locals
  OP_GET_LOCAL, // 2 bytes (code, operand)
  // OP_GET_LOCAL_LONG, // 4 bytes (1 code, 3 operand)
  OP_SET_LOCAL, // 2 bytes (code, operand)
  // OP_SET_LOCAL_LONG, // 4 bytes (1 code, 3 operand)
  // operators, etc
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_NOT,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NEGATE,
  OP_PRINT,
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
// make constant saves a variable to the chunk
size_t makeConstant(Chunk *chunk, Value value);
// write constant writes a constant to the chunk alongside OP_CONSTANT
void writeConstant(Chunk *chunk, Value value, int line);

#endif
