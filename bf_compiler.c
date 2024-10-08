// Compiler for Brainfuck

#include <stdio.h>
#include <stdlib.h>

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

void compile(char* file_contents) {
    int num_brackets = 0;
    int matching_brackets = 0;
    Stack_t stack = {.size = 0, .items = {0}};
    char* prologue = 
        ".text\n"
        ".globl _main\n"
        "_main:\n"
        "   pushq %rbp\n"
        "   movq %rsp, %rbp\n"
        "   pushq %r12\n"         // store callee saved register
        "   subq $30008, %rsp\n"  // allocate 30,008 B on stack, and realign
        "   leaq (%rsp), %rdi\n"  // address of beginning of tape 
        "   movl $0, %esi\n"      // fill with 0s 
        "   movq $30000, %rdx\n"  // length 30,000 B 
        "   call _memset\n"       // memset 
        "   movq %rsp, %r12";
    puts(prologue);

    for (unsigned long i = 0; file_contents[i] != '\0'; ++i) {
        switch (file_contents[i]) {
            case '>': 
                puts("   inc %r12");
                break;
            case '<':
                puts("   dec %r12");
                break;
            case '+':
                puts("   incb (%r12)");
                break;
            case '-':
                puts("   decb (%r12)");
                break;
            case '.':
                puts("   movzbl (%r12), %edi");
                puts("   call _putchar");
                break;
            case ',':
                puts("   call _getchar");
                puts("   movb %al, (%r12)");
                break;
            case '[':
                if (stack_push(&stack, num_brackets) == 0) {
                    puts("   cmpb $0, (%r12)");
                    printf("    je bracket_%d_end\n", num_brackets);
                    printf("bracket_%d_start:\n", num_brackets++);
                } else {
                    fprintf(stderr, "Out of stack space, too much nesting.");
                    exit(1);
                }
                break;
            case ']':
                if (stack_pop(&stack, &matching_brackets) == 0) {
                    puts("   cmpb $0, (%r12)");
                    printf("    jne bracket_%d_start\n", matching_brackets);
                    printf("bracket_%d_end:\n", matching_brackets);
                } else {
                    fprintf(stderr, "Stack underflow, unmatched brackets.");
                    exit(1);
                }
                break;
        }
    }
    const char* epilogue = 
        "   addq $30008, %rsp\n" // clean up tape from stack 
        "   popq %r12\n"         // restore callee saved register 
        "   popq %rbp\n"
        "   ret\n";
    puts(epilogue);
}

int main(int argc, char* argv[]) {
    char* source;
    if (argc == 1) {
        printf("\n **** Adderall: A Brainfuck Interpreter ****\n\n");
        printf("  1. Type or paste in brainfuck source code\n");
        printf("  2. Use Ctrl-D to run the code\n");
        printf("  3. Use ./adderall [filename] to execute source file\n"); 
        
        fread(source, 1, SRC_SIZE - 1, stdin);       // read source code from stdin 
        compile(source);                                   // execute the program 
        printf("\n");
    } else if (argc == 2) {
        FILE *file = fopen(argv[1], "r");            // open the file stream 
        fread(source, 1, SRC_SIZE, file);            // read source from file 
        fclose(file);                                // close the file 
        compile(source);                                   // execute the program 
    } 
    return 0;
}
