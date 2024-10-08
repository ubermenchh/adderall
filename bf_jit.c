// JiT Compiler for Brainfuck

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>

typedef struct Vector {
    int size;
    int capacity;
    char* data;
} Vector;

int vector_create(Vector* vec, int capacity) {
    vec->size = 0;
    vec->capacity = capacity;
    vec->data = malloc(capacity * sizeof(char));
    return vec->data == NULL ? -1 : 0;
}

int vector_destroy(Vector* vec) {
    if (vec == NULL || vec->data == NULL) {
        return -1;
    }
    vec->size = 0;
    vec->capacity = 0;
    free(vec->data);
    vec->data = NULL;
    vec = NULL;
    return 0;
}

int vector_push(Vector* vec, char* bytes, int len) {
    if (vec->size + len > vec->capacity) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, vec->capacity * sizeof(char));
        if (vec->data == NULL) {
            return -1;
        }
    }
    memcpy(vec->data + vec->size, bytes, len);
    vec->size += len;
    return 0;
}

int vector_write32LE(Vector* vec, int offset, int value) {
    if (offset >= vec->size) {
        return -1;
    }
    vec->data[offset + 3] = (value & 0xff000000) >> 24;
    vec->data[offset + 2] = (value & 0x00ff0000) >> 16;
    vec->data[offset + 1] = (value & 0x0000ff00) >> 8;
    vec->data[offset + 0] = (value & 0x000000ff);
    return 0;
}

#define STACK_SIZE 100
#define SRC_SIZE 30000

typedef struct Stack_t {
    int size;
    int items[STACK_SIZE];
} Stack_t;

int stack_push(Stack_t* p, int x) {
    if (p->size == STACK_SIZE) {
        return -1;
    } else {
        p->items[p->size++] = x;
        return 0;
    }
}

int stack_pop(Stack_t* p, int* x) {
    if (p->size == 0) {
        return -1;
    } else {
        *x = p->items[--p->size];
        return 0;
    }
}

// return 0 is success
#define GUARD(expr) assert(!(expr))

typedef void* fn_memset(void*, int, size_t);
typedef int fn_putchar(int);
typedef int fn_getchar();

void jit(const char* file_contents) {
    Vector instruction_stream;
    Stack_t relocation_table = {.size = 0, .items = {0}};
    int relocation_site = 0;
    int relative_offset = 0;

    GUARD(vector_create(&instruction_stream, 100));
    char prologue[] = {
        0x55, // push rbp 
        0x48, 0x89, 0xe5, // mov rsp, rbp 
        // backup callee saved registers 
        0x41, 0x54, // pushq %r12 
        0x41, 0x55, // pushq %r13 
        0x41, 0x56, // pushq %r14 
        // %rdi = memset 
        // %rsi = putchar 
        // %rdx = getchar
        // backup args to callee saved registers
        0x49, 0x89, 0xfc, // movq %rdi, %r12
        0x49, 0x89, 0xf5, // movq %rdi, %r13
        0x49, 0x89, 0xd6, // movq %rdi, %r14
        // %r12 = memset 
        // %r13 = putchar 
        // %r14 = getchar
        // allocate 30,008 B on stack 
        0x48, 0x81, 0xec, 0x38, 0x75, 0x00, 0x00, // subq $30000, %rsp 
        // address of beginning of tape 
        0x48, 0x8d, 0x3c, 0x24, // leaq (%rsp), %rdi 
        // fill with 0's
        0xbe, 0x00, 0x00, 0x00, 0x00, // movl $0, %esi 
        // length 30,000 B 
        0x48, 0xc7, 0xc2, 0x30, 0x75, 0x00, 0x00, // movq $30000, %rdi
        // memset
        0x41, 0xff, 0xd4, // callq *%r12
        0x49, 0x89, 0xe4, // movq %rsp, %r12 
        // %r12, = &tape[0];
    };
    GUARD(vector_push(&instruction_stream, prologue, sizeof(prologue)));

    for (unsigned long i = 0; file_contents[i] != '\0'; ++i) {
        switch (file_contents[i]) {
            case '>': {
                char opcodes[] = {0x49, 0xff, 0xc4};
                GUARD(vector_push(&instruction_stream, opcodes, sizeof(opcodes)));
            }
            break;
            case '<': {
                char opcodes[] = {0x49, 0xff, 0xcc};
                GUARD(vector_push(&instruction_stream, opcodes, sizeof(opcodes)));
            }
            break;
            case '+': {
                char opcodes[] = {0x41, 0xfe, 0x04, 0x24};
                GUARD(vector_push(&instruction_stream, opcodes, sizeof(opcodes)));
            }
            break;
            case '-': {
                char opcodes[] = {0x41, 0xfe, 0x0c, 0x24};
                GUARD(vector_push(&instruction_stream, opcodes, sizeof(opcodes)));
            }
            break;
            case '.': {
                char opcodes[] = {
                    0x41, 0x0f, 0xb6, 0x3c, 0x24, // movzbl (%r12), %edi 
                    0x41, 0xff, 0xd5 // callq *%r13 
                };
                GUARD(vector_push(&instruction_stream, opcodes, sizeof(opcodes)));
            }
            break;
            case ',': {
                char opcodes[] = {
                    0x41, 0xff, 0xd6, // callq *%r12 
                    0x41, 0x88, 0x04, 0x24, // movb %al, (%r12)
                };
                GUARD(vector_push(&instruction_stream, opcodes, sizeof(opcodes)));
            }
            break;
            case '[': {
                char opcodes[] = {
                    0x41, 0x80, 0x3c, 0x24, 0x00, // cmpb $0, (%r12)
                    0x0f, 0x84, 0x00, 0x00, 0x00, 0x00 // je <32b relative offset, 2's complement, LE>'
                };
                GUARD(vector_push(&instruction_stream, opcodes, sizeof(opcodes)));
            }
            GUARD(stack_push(&relocation_table, instruction_stream.size)); // create a label after 
            break;
            case ']': {
                char opcodes[] = {
                    0x41, 0x80, 0x3c, 0x24, 0x00, // cmpb $0, (%r12)
                    0x0f, 0x85, 0x00, 0x00, 0x00, 0x00 // jne <32b relative offset, 2's complement, LE>
                };
                GUARD(vector_push(&instruction_stream, opcodes, sizeof(opcodes)));
            }
            // patches self and matching open bracket 
            GUARD(stack_pop(&relocation_table, &relocation_site));
            relative_offset = instruction_stream.size - relocation_site;
            vector_write32LE(&instruction_stream, instruction_stream.size - 4, -relative_offset);
            vector_write32LE(&instruction_stream, relocation_site - 4, relative_offset);
            break;
        }
    }
    char epilogue[] = {
        0x48, 0x81, 0xc4, 0x38, 0x75, 0x00, 0x00, // addq $30000, %rsp 
        // restore callee saved registers 
        0x41, 0x5e, // popq %r14
        0x41, 0x5d, // popq %r13
        0x41, 0x5c, // popq %r12
        0x5d, // pop rbp 
        0xc3, // ret
    };
    GUARD(vector_push(&instruction_stream, epilogue, sizeof(epilogue)));

    void* mem = mmap(NULL, instruction_stream.size, PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
    memcpy(mem, instruction_stream.data, instruction_stream.size);
    void (*jitted_func) (fn_memset, fn_putchar, fn_getchar) = mem;
    jitted_func(memset, putchar, getchar);
    munmap(mem, instruction_stream.size);
    vector_destroy(&instruction_stream);
}

int main(int argc, char* argv[]) {
    char source[SRC_SIZE];
    if (argc == 1) {
        fread(source, 1, SRC_SIZE - 1, stdin);       // read source code from stdin 
        jit(source);                                   // execute the program 
        printf("\n");
    } else if (argc == 2) {
        FILE *file = fopen(argv[1], "r");            // open the file stream 
        fread(source, 1, SRC_SIZE, file);            // read source from file 
        fclose(file);                                // close the file 
        jit(source);                                   // execute the program 
    } 
    return 0;
}
