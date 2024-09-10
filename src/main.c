#include "../include/common.h"
#include "../include/chunk.h"
#include "../include/vm.h"

#include <stdio.h>

int main(int _, const char* argv[]) {
  Chunk chunk;
  initVM(&chunk);

  for (int i = 0; i < 256; i++) {
    writeConstant(&chunk, i, 123);
  }

  writeConstant(&chunk, 6, 123);
  writeConstant(&chunk, 4, 123);
  writeChunk(&chunk, OP_ADD, 123);

  writeConstant(&chunk, 5, 123);
  writeChunk(&chunk, OP_DIVIDE, 123);

  writeChunk(&chunk, OP_NEGATE, 123);

  writeChunk(&chunk, OP_RETURN, 10000);
  interpret(&chunk);
  freeVM();
  return 0;
}
