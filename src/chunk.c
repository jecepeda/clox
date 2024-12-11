#include "chunk.h"
#include "memory.h"
#include "stack.h"
#include "vm.h"

void initChunk(Chunk *chunk) {
  chunk->code = NULL;
  chunk->count = 0;
  chunk->capacity = 0;
  initValueArray(&chunk->constants);
  initLineArray(&chunk->lines);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line) {
  writeLineArray(&chunk->lines, line);
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(chunk->capacity);
    chunk->code =
        GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
  }
  chunk->code[chunk->count] = byte;
  chunk->count++;
}

void freeChunk(Chunk *chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  freeLineArray(&chunk->lines);
  freeValueArray(&chunk->constants);
  initChunk(chunk);
}

uint32_t makeConstant(Chunk *chunk, Value value) {
  stackPush(&vm.stack, value);
  writeValueArray(&chunk->constants, value);
  stackPop(&vm.stack);
  return chunk->constants.count - 1;
}

void writeConstant(Chunk *chunk, OpCode code, Value value, int line) {
  stackPush(&vm.stack, value);

  uint32_t size = chunk->constants.count;
  if (size >= 256u) {
    code++;
    writeChunk(chunk, code, line);
    writeValueArray(&chunk->constants, value);
    WRITE_CHUNK_LONG(chunk, size, line);
    return;
  }
  writeValueArray(&chunk->constants, value);
  writeChunk(chunk, code, line);
  writeChunk(chunk, size, line);
  stackPop(&vm.stack);
}