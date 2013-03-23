#include <stdio.h>
#include <math.h>

int main(int argc, char * argv[])
{
    size_t i, j;

    for (i=0; i<=64; i++) {
        j = pow(2,i);
        printf(" i =  %zu , j =  %zu \n", i, j );
    }

    return 0;
}
     
