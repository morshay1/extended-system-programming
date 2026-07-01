#include <stdio.h>
#include <string.h>
#include <malloc.h>

/* return string "word" if the count is 1 or "words" otherwise */
char *words(int count)
{
  char *words = malloc(strlen("words") + 1);
  if(words == NULL){
    fprintf(stderr, "Memory allocation failed!\n");
    return NULL;
  }
  strcpy(words, "words");
  if (count == 1)
    words[strlen(words)-1] = '\0';
  
  return words;
}

/* print a message reportint the number of words */
int print_word_count(char **argv)
{
  int count = 0;
  char **a = argv;
  while (*(a++))
    ++count;
  char *wordss = words(count);
  if (wordss == NULL)
    return -1;
  printf("The sentence contains %d %s.\n", count, wordss);
  free(wordss);
  return count;
}

/* print the number of words in the command line and return the number as the exit code */
int main(int argc, char **argv)
{
  print_word_count(argv + 1);
  return 0;
}