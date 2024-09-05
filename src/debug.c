#include "../include/debug.h"
#include "../include/chunk.h"
#include "../include/line.h"
#include <stdint.h>
#include <stdio.h>

void disassembleChunk(Chunk* chunk, const char* name) {
  printf("== %s ==\n", name);

  for (int offset = 0; offset < chunk->count;) {
    offset = disassembleInstruction(chunk, offset);
  }
}

static int simpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  printValue(chunk->constants.values[constant]);
  printf("'\n");
  // 1 byte for instruction
  // 1 byte for operand
  return offset + 2;
}

int disassembleInstruction(Chunk* chunk, int offset) {
  // print the number of the operation
  printf("%04d ", offset);
  // print line number if it's different than the previous line
  int line = getLine(&chunk->lines, offset);
  if (line == -1) {
    printf("   | ");
  }
  else {
    printf("%4d ", line);
  }

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
  case OP_CONSTANT:
    return constantInstruction("OP_CONSTANT", chunk, offset);
  case OP_RETURN:
    return simpleInstruction("OP_RETURN", offset);
  default:
    printf("Unknown opcode %d\n", instruction);
    return offset + 1;
  }
}
