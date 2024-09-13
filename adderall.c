/* Adderall: A BrainFuck Interpreter */ 

/* 
Command: 
- `>`: increments the data pointer by one (moves the data pointer to the right) 
- `<`: decrements the data pointer by one (moves the data pointer to the left)
- `+`: increments the byte at the data pointer by one 
- `-`: decrements the byte at the data pointer by one 
- `.`: outputs the byte at the data pointer 
- `,`: accepts one byte as input, and stores it in the byte at the data pointer 
- `[`: if the byte at the data pointer is zero, jump it to the next command after `]` command 
- `]`: if the byte at the data pointer is nonzero, jump it back to the command after `[` command
*/

#include <stdio.h>

#define MEM_SIZE 30000                               // max memory size for the program
#define SRC_SIZE  30000                              // max source file size 

char memory[MEM_SIZE];                               // program memory
char* ptr = memory;                                  // data pointer 

char source[SRC_SIZE];                               // source file buffer 
char* ip = source;                                   // instruction pointer 

int loop;                                            // loop counter

void execute() {
    while (*ip) {
        switch (*ip) {
            case '>': ++ptr; break;                  // increments the memory pointer 
            case '<': --ptr; break;                  // decrements the memory pointer 
            case '+': ++*ptr; break;                 // increments the memory byte
            case '-': --*ptr; break;                 // decrements the memory byte 
            case '.': putc(*ptr, stdout); break;     // output the char 
            case ',': *ptr = getc(stdin); break;     // get char as input 

            case '[': {                              // loop start 
                if (!*ptr) {                         // if cell at the pointer is zero    
                    loop = 1;                        // set loop counter to 1 
                    while (loop) {                   
                        ip++;                        // increment instruction pointer 
                        if (*ip == '[') loop++;      // increment loop counter if `[` cmd 
                        if (*ip == ']') loop--;      // decrement loop counter if `]` cmd
                    }
                }
                break;
            }
            case ']': {                              // loop end 
                loop = 1;                            // set loop counter to 1
                while (loop) {
                    ip--;                            // decrement instruction pointer 
                    if (*ip == '[') loop--;          // decrement loop counter if `[` cmd 
                    if (*ip == ']') loop++;          // increment loop counter if `]` cmd 
                }
                ip--;                                // decrement instruction pointer 
                break;
            }
        }
        ip++;                                        // increment instruction pointer 
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("\n **** Adderall: A Brainfuck Interpreter ****\n\n");
        printf("  1. Type or paste in brainfuck source code\n");
        printf("  2. Use Ctrl-D to run the code\n");
        printf("  3. Use ./adderall [filename] to execute source file\n"); 
        
        fread(source, 1, SRC_SIZE - 1, stdin);       // read source code from stdin 
        execute();                                   // execute the program 
        printf("\n");
    } else if (argc == 2) {
        FILE *file = fopen(argv[1], "r");            // open the file stream 
        fread(source, 1, SRC_SIZE, file);            // read source from file 
        fclose(file);                                // close the file 
        execute();                                   // execute the program 
    } 
    return 0;
}
