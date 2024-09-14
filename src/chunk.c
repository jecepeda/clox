#include "../include/chunk.h"
#include "../include/memory.h"
#include <stdint.h>
#include <stdlib.h>

void initChunk(Chunk *chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  initValueArray(&chunk->constants);
  initLineArray(&chunk->lines);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line) {
  writeLineArray(&chunk->lines, line);
  if (chunk->capacity < chunk->count + 1) {
    chunk->capacity = GROW_CAPACITY(chunk->capacity);
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, chunk->capacity);
  }
  chunk->code[chunk->count] = byte;
  chunk->count++;
}

void freeChunk(Chunk *chunk) {
  FREE_ARRAY(uint8_t, chunk->code);
  freeLineArray(&chunk->lines);
  freeValueArray(&chunk->constants);
  initChunk(chunk);
}

void writeConstant(Chunk *chunk, Value value, int line) {
  size_t size = chunk->constants.count;
  if (size >= 256u) {
    writeChunk(chunk, OP_CONSTANT_LONG, line);
    writeValueArray(&chunk->constants, value);
    uint8_t a, b, c;
    int temp = size;
    // now we iteratively get the first byte
    // and right-shift it to remove it
    c = temp & 0xFF;
    temp >>= 0x8;
    b = temp & 0xFF;
    temp >>= 0x8;
    a = temp & 0xFF;

    writeChunk(chunk, a, line);
    writeChunk(chunk, b, line);
    writeChunk(chunk, c, line);
    return;
  }
  writeChunk(chunk, OP_CONSTANT, line);
  writeChunk(chunk, size, line);
  writeValueArray(&chunk->constants, value);
}