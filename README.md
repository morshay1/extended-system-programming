# Extended System Programming

A collection of low-level programming projects developed in C and x86 Assembly as part of an Extended System Programming Laboratory course at Ben-Gurion University.  
The projects explore operating system concepts, process management, executable formats, binary analysis, linking, loading, and direct interaction with the Linux kernel through system calls.

## Lab A- Encoder

The program processes text from either standard input or an input file and applies a cyclic Caesar-style encryption/decryption using a numeric key supplied via command-line arguments.

### Features

- Command-line argument parsing
- Debug mode (`+D` / `-D`)
- Encryption (`+E`) and decryption (`-E`) using cyclic numeric keys
- Input from keyboard or file (`-i<input_file>`)
- Output to terminal or file (`-o<output_file>`)
- Character-by-character processing using C standard I/O streams (`stdin`, `stdout`, `FILE*`)
- Support for lowercase letters, uppercase letters, and digits with wrap-around logic

### Example

```bash
./encoder +E123 -iinput.txt -ooutput.txt
```
