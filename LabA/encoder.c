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
    char *key = "0";
    char type = '+';
    FILE *inFile = stdin;
    FILE *outFile = stdout;

    for (int i = 0; i < argc; i++) {
        // Debug mode
        if (debug == 1){
            fprintf(stderr, "debug: argument %d: %s\n", i, argv[i]);
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
            inFile = fopen(&argv[i][2], "r");
            if(inFile == NULL){
                fprintf(stderr, "Cannot open input file\n");
                return 1;
            }
        }
        if(argv[i][0] == '-' &&  argv[i][1] == 'o'){
            outFile = fopen(&argv[i][2], "w");
            if(outFile == NULL){
                fprintf(stderr, "Cannot open output file\n");
                return 1;
            }
        }
    }
    

    // Encryption mode
    int charInput;
    int i = 0;
    int keySize = len(key);
    int charOutput; 
    
    while ((charInput = fgetc(inFile)) != EOF) {
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
        fputc(charOutput, outFile);
        i++;
    }


    return 0;
}
