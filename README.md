# adderall
A Brainf*ck Interpreter

| Brainf*ck Command | Description                       | C Equivalent |
|-------------------|-----------------------------------|--------------|
| `>`               | Increments the data pointer by one (moves the data pointer to the right) | `data_pointer++` |
| `<`               | Decrements the data pointer by one (moves the data pointer to the left) | `data_pointer--` |
| `+`               | Increments the byte at the data pointer by one                         | `(*data_pointer)++` |
| `-`               | Decrements the byte at the data pointer by one                         | `(*data_pointer)--` |
| `.`               | Outputs the byte at the data pointer                                   | `putchar(*data_pointer)` |
| `,`            | Accepts one byte as input, and stores it in the byte at the data pointer  | `*data_pointer = getchar()` |
| `[`            | If the byte at the data pointer is zero, jump it to the next command after `]` command | `while (*data_pointer) {` |
| `]`            | If the byte at the data pointer is nonzero, jump it back to the command after 
`[` command |`}` | 

## How to use 
```
gcc adderall.c -o adderall 
./adderall hello_world.bf
```
The above commands should print `"Hello World!"` to the terminal.

## References:
- [Wiki](https://en.wikipedia.org/wiki/Brainfuck#:~:text=The%20language%20takes%20its%20name,the%20boundaries%20of%20computer%20programming.)
- [Esolang Wiki](https://esolangs.org/wiki/Brainfuck)
