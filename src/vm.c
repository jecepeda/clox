#include "../include/common.h"
#include "../include/vm.h"
#include "../include/debug.h"
#include "../include/stack.h"

#include <stdio.h>

VM vm;

void initVM(Chunk* c) {
    vm.chunk = c;
    initChunk(vm.chunk);
    initStack(&vm.stack);
}

void freeVM() {
    freeChunk(vm.chunk);
    freeStack(&vm.stack);
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define BINARY_OP(op) \
    do { \
        double b = stackPop(&vm.stack);   \
        double a = stackPop(&vm.stack);   \
        stackPush(&vm.stack, a op b);     \
    } while (false);
#define READ_CONSTANT(isLong) (vm.chunk->constants.values[isLong ? ((READ_BYTE()<<16) | (READ_BYTE()<<8) | (READ_BYTE())) : (READ_BYTE())])
    //
    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        if (vm.stack.top > 0) {
            printf("          ");
            for (int i = 0; i < vm.stack.top; i++) {
                printf("[ ");
                printValue(vm.stack.items[i]);
                printf(" ]");
            }
            printf("\n");
        }
        disassembleInstruction(vm.chunk,
            (int)(vm.ip - vm.chunk->code));
#endif
        uint8_t instruction;
        bool isLong = false;
        switch (instruction = READ_BYTE()) {
        case OP_CONSTANT_LONG:
            isLong = true; // don't break, fall through
        case OP_CONSTANT: {
            Value constant = READ_CONSTANT(isLong);
            stackPush(&vm.stack, constant);
            break;
        }
        case OP_ADD:      BINARY_OP(+); break;
        case OP_SUBTRACT: BINARY_OP(-); break;
        case OP_MULTIPLY: BINARY_OP(*); break;
        case OP_DIVIDE:   BINARY_OP(/ ); break;
        case OP_NEGATE: {
            stackPush(&vm.stack, -stackPop(&vm.stack));
            break;
        }
        case OP_RETURN: {
            printValue(stackPop(&vm.stack));
            printf("\n");
            return INTERPRET_OK;
            break;
        }
        default: {
            printf("Unknown opcode %d\n", instruction);
            return INTERPRET_RUNTIME_ERROR;
        }
        }
    }

}

#undef READ_CONSTANT 
#undef BINARY_OP
#undef READ_BYTE


InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}