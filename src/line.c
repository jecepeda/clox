#include "../include/line.h"
#include "../include/memory.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void initLineArray(LineArray* lineArray) {
    lineArray->count = 0;
    lineArray->capacity = 0;
    lineArray->lines = NULL;
}

void increaseLineArrayCapacity(LineArray* lineArray) {
    int oldCapacity = lineArray->capacity;
    lineArray->capacity = GROW_CAPACITY(oldCapacity);
    lineArray->lines = GROW_ARRAY(Line, lineArray->lines, oldCapacity, lineArray->capacity);
}

void writeLineArray(LineArray* chunk, int line) {
    // check for length
    if (chunk->capacity < chunk->count + 1) {
        increaseLineArrayCapacity(chunk);
    }

    // if the line is equal to the previous line written
    if (chunk->count > 0 && chunk->lines[chunk->count - 1].line == line) {
        chunk->lines[chunk->count - 1].count++;
    }
    else {
        chunk->lines[chunk->count].count = 1;
        chunk->lines[chunk->count].line = line;
        chunk->count++;
    }

}

int getLine(LineArray* chunk, int offset) {
    int offsetCount = 0;
    for (int i = 0; i < chunk->count; i++) {
        for (int j = 0; j < chunk->lines[i].count; j++) {
            if (offsetCount == offset) {
                return j == 0 ? chunk->lines[i].line : -1;
            }
            offsetCount++;
        }
    }
    printf("\n Found error while getting offset. aborting");
    exit(1);
    return -1;
}

void freeLineArray(LineArray* lineArray) {
    FREE_ARRAY(Line, lineArray->lines, lineArray->capacity);
    initLineArray(lineArray);
}