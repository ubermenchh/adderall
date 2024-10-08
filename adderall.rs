use std::env;
use std::fs::File;
use std::io::{self, Read};

const MEM_SIZE: usize = 30000;
const SRC_SIZE: usize = 30000;

struct Interpreter {
    memory: [u8; MEM_SIZE],
    ptr: usize,
    source: Vec<u8>,
    ip: usize,
}

impl Interpreter {
    fn new() -> Self {
        Interpreter {
            memory: [0; MEM_SIZE],
            ptr: 0,
            source: Vec::with_capacity(SRC_SIZE),
            ip: 0,
        }
    }

    fn execute(&mut self) {
        while self.ip < self.source.len() {
            match self.source[self.ip] as char {
                '>' => self.ptr = (self.ptr + 1) % MEM_SIZE,
                '<' => self.ptr = (self.ptr + MEM_SIZE - 1) % MEM_SIZE,
                '+' => self.memory[self.ptr] = self.memory[self.ptr].wrapping_add(1),
                '-' => self.memory[self.ptr] = self.memory[self.ptr].wrapping_sub(1),
                '.' => print!("{}", self.memory[self.ptr] as char),
                ',' => {
                    let mut input = [0; 1];
                    if io::stdin().read_exact(&mut input).is_ok() {
                        self.memory[self.ptr] = input[0];
                    }
                }
                '[' => {
                    if self.memory[self.ptr] == 0 {
                        let mut loop_count = 1;
                        while loop_count > 0 {
                            self.ip += 1;
                            match self.source[self.ip] as char {
                                '[' => loop_count += 1,
                                ']' => loop_count -= 1,
                                _ => {}
                            }
                        }
                    }
                }
                ']' => {
                    if self.memory[self.ptr] != 0 {
                        let mut loop_count = 1;
                        while loop_count > 0 {
                            self.ip -= 1;
                            match self.source[self.ip] as char {
                                '[' => loop_count -= 1,
                                ']' => loop_count += 1,
                                _ => {}
                            }
                        }
                        self.ip -= 1;
                    }
                }
                _ => {}
            }
            self.ip += 1;
        }
    }
}

fn main() -> io::Result<()> {
    let args: Vec<String> = env::args().collect();
    let mut interpreter = Interpreter::new();

    if args.len() == 1 {
        println!("\n **** Adderall: A Brainfuck Interpreter ****\n");
        println!("  1. Type or paste in brainfuck source code");
        println!("  2. Use Ctrl-D (Unix) or Ctrl-Z (Windows) to run the code");
        println!("  3. Use rustc adderall.rs && ./adderall [filename] to execute source file");

        io::stdin().read_to_end(&mut interpreter.source)?;
        interpreter.execute();
        println!();
    } else if args.len() == 2 {
        let mut file = File::open(&args[1])?;
        file.read_to_end(&mut interpreter.source)?;
        interpreter.execute();
    }
    Ok(())
}
