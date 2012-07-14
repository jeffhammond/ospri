#include <stdio.h>
#include <stdlib.h>
#include <upc.h>

int main(int argc, char* argv[])
{
    if( MYTHREAD % 2 )
         printf("Hello world: I am thread %d and I am even.\n", MYTHREAD);
    else
         printf("Hello world: I am thread %d and I am odd.\n", MYTHREAD);

    return 0;
}

