#include "../include/common.h"
#include "../include/chunk.h"
#include "../include/debug.h"

int main(int _, const char* argv[]) {
  Chunk chunk;
  initChunk(&chunk);
  int constant = addConstant(&chunk, 1.2);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);
  writeChunk(&chunk, OP_RETURN, 124);
  writeChunk(&chunk, OP_RETURN, 125);
  disassembleChunk(&chunk, "test chunk");
  freeChunk(&chunk);
  return 0;
}
