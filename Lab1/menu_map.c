#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "functions.h"

char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  for(int i = 0; i < array_length; i++)
  {
    mapped_array[i] = (*f)(array[i]);
  }
  return mapped_array;
}

char my_get(char c){
    return fgetc(stdin);
}

char cprt(char c){
    if(c >= 0x20 && c <= 0x7E){
        printf("%c\n", (char)c);
    }
    else{
        printf(".\n");
    }
    return c;
}

char encrypt(char c){
    return (c >= 0x1F && c <= 0x7E) ? (char)(c + 1) : c;
}

char decrypt(char c){
    return (c >= 0x21 && c <= 0x7F) ? (char)(c - 1) : c;
}

char xprt(char c){
    printf("%x\n", c);
    return c;
}

char dprt(char c){
    printf("%d\n", c);
    return c;
}


struct fun_desc {
    char* name;
    char (*fun) (char);
};

    
struct fun_desc menu[] = {{"my_get" ,my_get},
                          {"cprt",cprt}, 
                          {"encrypt",encrypt}, 
                          {"decrypt",decrypt}, 
                          {"xprt",xprt}, 
                          {"dprt",dprt},
                          {NULL,NULL}};

int main(int argc, char** argv) {
    int array_size = 5;
    char* carray = malloc(array_size * sizeof(char));
    if (!carray) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < array_size; i++) {
        carray[i] = '\0';
    }

    char buffer[128];
    int lower = 0;
    int upper = 0;

    while (menu[upper].name != NULL) {
        upper++;
    }

    while (1) {
        printf("Select operation from the following menu (ctrl^D for exit):\n");

        int i = 0;
        while (menu[i].name != NULL) {
            printf("%d) %s\n", i, menu[i].name);
            i++;
        }

        printf("Option:\n");
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break; 
        }

        int selection = atoi(buffer); 

        if (selection >= lower && selection < upper) {
            printf("Within bounds\n");
            carray = map(carray, array_size, menu[selection].fun);
        } else {
            printf("Not within bounds\n");
            break;
        }
    }

    free(carray);  
    return 0;
}
