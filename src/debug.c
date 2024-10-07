#include "debug.h"
#include "chunk.h"
#include "line.h"
#include <stdint.h>
#include <stdio.h>

void disassembleChunk(Chunk *chunk, const char *name) {
  printf("== %s ==\n", name);

  for (int offset = 0; offset < chunk->count;) {
    offset = disassembleInstruction(chunk, offset);
  }
  printf("== %s end ==\n", name);
}

static int simpleInstruction(const char *name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

static int constantInstruction(bool isLong, const char *name, Chunk *chunk,
                               int offset) {
  uint32_t constant = chunk->code[offset + 1];
  if (isLong) {
    constant = (constant << 16) | (chunk->code[offset + 2] << 8) |
               chunk->code[offset + 3];
  }

  printf("%-16s %4d '", name, constant);
  printValue(chunk->constants.values[constant]);
  printf("'\n");
  // if long: 1 byte for the opcode, 3 bytes for the operand
  // if short: 1 byte for the opcode, 1 byte for the operand
  return offset + (isLong ? 4 : 2);
}

static int byteInstruction(bool isLong, const char *name, Chunk *chunk,
                           int offset) {
  uint32_t slot = chunk->code[offset + 1];
  if (isLong) {
    slot =
        (slot << 16) | (chunk->code[offset + 2] << 8) | chunk->code[offset + 3];
  }
  printf("%-16s %4d\n", name, slot);
  return offset + (isLong ? 4 : 2);
}

int disassembleInstruction(Chunk *chunk, int offset) {
  // print the number of the operation
  printf("#%04d ", offset);
  // print line number if it's different than the previous line
  int line = getLine(&chunk->lines, offset);
  if (line == -1) {
    printf("   | ");
  } else {
    printf("%4d ", line);
  }

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
  case OP_DEFINE_GLOBAL_LONG:
    return constantInstruction(true, "OP_DEFINE_GLOBAL_LONG", chunk, offset);
  case OP_DEFINE_GLOBAL:
    return constantInstruction(false, "OP_DEFINE_GLOBAL", chunk, offset);
  case OP_GET_GLOBAL_LONG:
    return constantInstruction(true, "OP_GET_GLOBAL_LONG", chunk, offset);
  case OP_GET_GLOBAL:
    return constantInstruction(false, "OP_GET_GLOBAL", chunk, offset);
  case OP_SET_GLOBAL_LONG:
    return constantInstruction(true, "OP_SET_GLOBAL_LONG", chunk, offset);
  case OP_SET_GLOBAL:
    return constantInstruction(false, "OP_SET_GLOBAL", chunk, offset);
  case OP_CONSTANT_LONG:
    return constantInstruction(true, "OP_CONSTANT_LONG", chunk, offset);
  case OP_CONSTANT:
    return constantInstruction(false, "OP_CONSTANT", chunk, offset);
  case OP_GET_LOCAL_LONG:
    return byteInstruction(true, "OP_GET_LOCAL_LONG", chunk, offset);
  case OP_GET_LOCAL:
    return byteInstruction(false, "OP_GET_LOCAL", chunk, offset);
  case OP_SET_LOCAL_LONG:
    return byteInstruction(true, "OP_SET_LOCAL_LONG", chunk, offset);
  case OP_SET_LOCAL:
    return byteInstruction(false, "OP_SET_LOCAL", chunk, offset);
  case OP_RETURN:
    return simpleInstruction("OP_RETURN", offset);
  case OP_NEGATE:
    return simpleInstruction("OP_NEGATE", offset);
  case OP_PRINT:
    return simpleInstruction("OP_PRINT", offset);
  case OP_POP:
    return simpleInstruction("OP_POP", offset);
  case OP_ADD:
    return simpleInstruction("OP_ADD", offset);
  case OP_SUBTRACT:
    return simpleInstruction("OP_SUBTRACT", offset);
  case OP_MULTIPLY:
    return simpleInstruction("OP_MULTIPLY", offset);
  case OP_DIVIDE:
    return simpleInstruction("OP_DIVIDE", offset);
  case OP_NIL:
    return simpleInstruction("OP_NIL", offset);
  case OP_TRUE:
    return simpleInstruction("OP_TRUE", offset);
  case OP_FALSE:
    return simpleInstruction("OP_FALSE", offset);
  case OP_NOT:
    return simpleInstruction("OP_NOT", offset);
  case OP_EQUAL:
    return simpleInstruction("OP_EQUAL", offset);
  case OP_GREATER:
    return simpleInstruction("OP_GREATER", offset);
  case OP_LESS:
    return simpleInstruction("OP_LESS", offset);
  default:
    printf("Unknown opcode %d\n", instruction);
    return offset + 1;
  }
}
