#include <stdio.h>

int main(int argc, char *argv[])
{
    // Debug Mode
    for (int i = 0; i < argc; i++) {
        if (argv[1][0] == '+' && argv[1][1] == 'D') {
            printf(argv[i])
        }
    }

    // int c;
    // while ((c = fgetc(stdin)) != '\n') {
    //     printf("Read character: %c\n", c);
    // }
    return 0;
}
