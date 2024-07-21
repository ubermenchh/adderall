#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

typedef enum {
    OP_INC,
    OP_DEC,
    OP_LEFT,
    OP_RIGHT,
    OP_INPUT,
    OP_OUTPUT,
    OP_JUMP_IF_ZERO,
    OP_JUMP_IF_NONZERO
} OP_KIND;

typedef struct {
    OP_KIND kind;
    size_t operand;
} Op;

typedef struct {
    Op* items;
    size_t count;
    size_t capacity;
} Ops;

void add_op(Ops* ops, OP_KIND kind, size_t operand) {
    if (ops->count == ops->capacity) {
        ops->capacity = ops->capacity ? ops->capacity * 2 : 1;
        ops->items = realloc(ops->items, ops->capacity * sizeof(Op));
    }
    ops->items[ops->count++] = (Op){kind, operand};
}

Ops parse_bf(const char* source) {
    Ops ops = {0};
    size_t source_len = strlen(source);

    for (size_t i = 0; i < source_len; i++) {
        switch (source[i]) {
            case '+': add_op(&ops, OP_INC, 0); break;
            case '-': add_op(&ops, OP_DEC, 0); break;
            case '<': add_op(&ops, OP_LEFT, 0); break;
            case '>': add_op(&ops, OP_RIGHT, 0); break;
            case ',': add_op(&ops, OP_INPUT, 0); break;
            case '.': add_op(&ops, OP_OUTPUT, 0); break;
            case '[': add_op(&ops, OP_JUMP_IF_ZERO, 0); break;
            case ']': add_op(&ops, OP_JUMP_IF_NONZERO, 0); break;
        }
    }
    return ops;
}

int main(void) {
    const char* source = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";
    Ops ops = parse_bf(source);

    free(ops.items);
    return 0;
}
