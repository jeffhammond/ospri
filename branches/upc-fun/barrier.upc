#include <stdio.h>
#include <stdlib.h>
#include <upc.h>

int main(int argc, char* argv[])
{
    printf("Hello world: I am thread %d.\n", MYTHREAD);
    upc_barrier;
    return 0;
}

