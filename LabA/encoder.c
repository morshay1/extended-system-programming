#include <stdio.h>

int len(char* s){
    int i = 0;
    while(s[i] != '\0'){
        i++;
    }
    return i;
}

char calcOutputChar(char type, int inputChar, char base, int shift, int range){
        char output;
        int offset = inputChar - base;
        shift = shift - '0';
        if(type == '+'){
            int newOffset = (offset + shift) % range;
            output = base + newOffset;
        }
        else if(type == '-'){
            int newOffset = (offset - shift + range) % range;
            output = base + newOffset;
        }
        return output;
    }
    

int main(int argc, char *argv[]){
    int debug = 0;
    char *key = NULL;
    char type = '\0';
    int charInput;

    for (int i = 0; i < argc; i++) {
        // Debug mode
        if (debug == 1){
            fprintf(stderr, "Debug: argument %d: %s\n", i, argv[i]);
        }
        if (argv[i][0] == '+' && argv[i][1] == 'D') {
            debug = 1;
        }
        if(argv[i][0] == '-' &&  argv[i][1] == 'D'){
            debug = 0;
        }
        if(argv[i][0] == '+' &&  argv[i][1] == 'E'){
            type = '+';
            key = &argv[i][2];
        }
        if(argv[i][0] == '-' &&  argv[i][1] == 'E'){
            type = '-';
            key = &argv[i][2];
        }
        if(argv[i][0] == '-' &&  argv[i][1] == 'i'){

        }
    }
    

    // Encryption mode
    int charOutput; 
    int i = 0;
    int keySize = len(key);

    printf("Write something:\n");
    while ((charInput = fgetc(stdin)) != EOF) {
        int shift = key[i % keySize];
        if(charInput >= 48 && charInput <= 57){ // 0-9
            charOutput = calcOutputChar(type, charInput, '0', shift, 10);
        } 
        else if(charInput >= 65 && charInput <= 90){ // A-Z
            charOutput = calcOutputChar(type, charInput, 'A', shift, 26);
        } 
        else if(charInput >= 97 && charInput <= 122){ // a-z
            charOutput = calcOutputChar(type, charInput, 'a', shift, 26);
        } 
        else{
            charOutput = charInput;
        }
        printf("%c", charOutput);
        i++;
    }


    return 0;
}
