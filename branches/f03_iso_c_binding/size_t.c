#include <stdio.h>
#include <math.h>

int main(int argc, char * argv[])
{
    int i;
    size_t j;

    for (i=0; i<=64; i++)
    {
        j = pow(2,i);
        printf(" i =  %d , j =  %zu \n", i, j );
    }

    return 0;
}
     
