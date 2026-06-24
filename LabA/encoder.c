#include <stdio.h>

int main()
{
    printf("Enter something: ");
    fgets(something, sizeof(something), stdin);
    printf("%s", something);
    return 0;
}
