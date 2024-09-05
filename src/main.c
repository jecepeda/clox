#include "../include/common.h"
#include "../include/chunk.h"
#include "../include/debug.h"

int main(int _, const char* argv[]) {
  Chunk chunk;
  initChunk(&chunk);
  for (long long i = 0; i < 10000; i++) {
    writeConstant(&chunk, i, i);
  }
  writeChunk(&chunk, OP_RETURN, 10000);
  disassembleChunk(&chunk, "test chunk");
  freeChunk(&chunk);
  return 0;
}
