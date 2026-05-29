#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *key = NULL; 
size_t key_length = 0;
int encoding_mode = 1;
static size_t key_index = 0;

char wrap_char(char c, int offset, char min, char max) {
    int range = max - min + 1;
    return (char)((((c - min) + offset + range) % range) + min);
}

char add_key(char c) {
    if (isupper(c)) {
        c = wrap_char(c, key[key_index] - '0', 'A', 'Z');
    } else if (islower(c)) {
        c = wrap_char(c, key[key_index] - '0', 'a', 'z');
    } else if (isdigit(c)) {
        c = wrap_char(c, key[key_index] - '0', '0', '9');
    }
    key_index = (key_index + 1) % key_length; 
    return c;
}

char subtract_key(char c) {
    if (isupper(c)) {
        c = wrap_char(c, -(key[key_index] - '0'), 'A', 'Z');
    } else if (islower(c)) {
        c = wrap_char(c, -(key[key_index] - '0'), 'a', 'z');
    } else if (isdigit(c)) {
        c = wrap_char(c, -(key[key_index] - '0'), '0', '9');
    }
    key_index = (key_index + 1) % key_length;
    return c;
}

char encode(char c) {
    if (encoding_mode == 1) { // +E
        return add_key(c);
    } else { // -E
        return subtract_key(c);
    }
}

int main(int argc, char **argv) {
    int debug_mode = 1; // Debug mode ON by default
    FILE *infile = stdin;
    FILE *outfile = stdout;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '+' && argv[i][1] == 'E') {
            encoding_mode = 1;
            key_length = 0;
            for (const char *p = argv[i] + 2; *p; ++p) {
                if (isdigit(*p)) {
                    key_length++;
                }
            }
            key = malloc(key_length + 1);
            size_t index = 0;
            for (const char *p = argv[i] + 2; *p; ++p) {
                if (isdigit(*p)) {
                    key[index++] = *p;
                }
            }
            key[key_length] = '\0';
        }
        else if (argv[i][0] == '-' && argv[i][1] == 'E') {
            encoding_mode = 0;
            key_length = 0;
            for (const char *p = argv[i] + 2; *p; ++p) {
                if (isdigit(*p)) {
                    key_length++;
                }
            }
            key = malloc(key_length + 1);
            size_t index = 0;
            for (const char *p = argv[i] + 2; *p; ++p) {
                if (isdigit(*p)) {
                    key[index++] = *p;
                }
            }
            key[key_length] = '\0';
        } 
        else if (argv[i][0] == '+' && argv[i][1] == 'D' && argv[i][2] == '\0') {
            debug_mode = 1;
        } 
        else if (argv[i][0] == '-' && argv[i][1] == 'D' && argv[i][2] == '\0') {
            debug_mode = 0;
        }
        else if (argv[i][0] == '-' && argv[i][1] == 'i') {
            const char *fname = argv[i] + 2; 
            if (*fname == '\0') { 
                if (i + 1 < argc) { 
                    fname = argv[++i];
                } else {
                    fprintf(stderr, "Error: No file specified after -i\n");
                    return 1;
                }
            }
            infile = fopen(fname, "r");
            if (!infile) {
                perror("Error opening input file");
                return 1;
            }
        } 
        else if (argv[i][0] == '-' && argv[i][1] == 'o') {
            const char *fname = argv[i] + 2; 
            if (*fname == '\0') { 
                if (i + 1 < argc) { 
                    fname = argv[++i];
                } else {
                    fprintf(stderr, "Error: No file specified after -o\n");
                    return 1;
                }
            }
            outfile = fopen(fname, "w");
            if (!outfile) {
                perror("Error opening output file");
                return 1;
            }
        }
        if (debug_mode == 1) {
            fprintf(stderr, "Argument: %s\n", argv[i]);
        }
    }

    if (key == NULL) {
        key = malloc(2);
        strcpy(key, "0");
        key_length = 1;
    }

    char c;
    while ((c = fgetc(infile)) != EOF) {
        char encoding_char = encode(c);
        fputc(encoding_char, outfile);
    }

    if (infile != stdin) fclose(infile);
    if (outfile != stdout) fclose(outfile);
    free(key);
    return 0;
}

