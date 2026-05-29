#ifndef task2
#define task2

#include <stdio.h>
#include <stdlib.h>

char* map(char *array, int array_length, char (*f)(char));

char my_get(char c);
/* Ignores c, reads and returns a character from stdin using fgetc. */

char cprt(char c);
/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */

char encrypt(char c);
/* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x1F and 0x7E it is returned unchanged */

char decrypt(char c);
/* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x21 and 0x7F it is returned unchanged */

char xprt(char c);
/* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */

char dprt(char c);

/* dprt prints the value of c in a decimal representation followed by a new line, and returns c unchanged. */ 

#endif