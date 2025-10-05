#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("HELLO!");

    int i = 0;
    while(i < 10000)
    {
        i = 1;
    }

    return 0;
}