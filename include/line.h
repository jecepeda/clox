#ifndef clox_line_h
#define clox_line_h

#include "common.h"

typedef struct {
    int count;
    int line;
} Line;

typedef struct {
    int count;
    int capacity;
    Line* lines;
} LineArray;

void initLineArray(LineArray* lineArray);
void writeLineArray(LineArray* lineArray, int line);
void freeLineArray(LineArray* lineArray);
int getLine(LineArray* lineArray, int offset);

#endif