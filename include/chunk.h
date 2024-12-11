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
  // long enums consists of 4 bytes
  // whereas normal ones consists of 1/2 bytes,
  // depending if they need to extract data from a constant table or an index
  OP_CONSTANT,
  OP_CONSTANT_LONG,
  // globals
  OP_DEFINE_GLOBAL,
  OP_DEFINE_GLOBAL_LONG,
  OP_SET_GLOBAL,
  OP_SET_GLOBAL_LONG,
  OP_GET_GLOBAL,
  OP_GET_GLOBAL_LONG,
  // locals
  OP_GET_LOCAL,
  OP_GET_LOCAL_LONG,
  OP_SET_LOCAL,
  OP_SET_LOCAL_LONG,
  OP_CLOSURE,
  OP_CLOSURE_LONG,
  // upvalues
  OP_CLOSE_UPVALUE,
  OP_GET_UPVALUE,
  OP_GET_UPVALUE_LONG,
  OP_SET_UPVALUE,
  OP_SET_UPVALUE_LONG,
  // classes
  OP_CLASS,
  OP_CLASS_LONG,
  OP_GET_PROPERTY,
  OP_GET_PROPERTY_LONG,
  OP_SET_PROPERTY,
  OP_SET_PROPERTY_LONG,
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
  OP_LOOP,
  OP_CALL,
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_RETURN,
} OpCode;

typedef struct {
  // same as a byte
  uint8_t *code;
  uint32_t count;
  uint32_t capacity;
  LineArray lines;
  ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
void freeChunk(Chunk *chunk);
// make constant saves a variable to the chunk
uint32_t makeConstant(Chunk *chunk, Value value);
// write constant writes a constant to the chunk alongside OP_CONSTANT
void writeConstant(Chunk *chunk, OpCode code, Value value, int line);

#endif
