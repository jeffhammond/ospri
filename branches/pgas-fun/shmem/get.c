#include "myshmem.h"

#define CHECK_SHEAP_IS_SYMMETRIC

#ifdef CHECK_SHEAP_IS_SYMMETRIC
long sheap_base;
#endif

int main(int argc, char* argv[])
{
    shmem_init();
    int mype = my_pe();
    int npes = num_pes();

    int i;
    int n = ( argc>1 ? atoi(argv[1]) : 5);
    int * sheap = shmalloc(n*sizeof(int));
    if (sheap==NULL) exit(1);
    for (i=0; i<n; i++)
        sheap[i] = mype;
    /* apparently Cray SHMEM doesn't call a barrier in shmalloc */
    shmem_barrier_all();

#ifdef CHECK_SHEAP_IS_SYMMETRIC
    int errors = 0;
    sheap_base = (long)sheap;
    long remote_sheap_base;
    /* this is an inefficient N^2 implementation of what should be a reduction */
    for (i=0; i<npes; i++)
    {
        shmem_long_get(&remote_sheap_base, &sheap_base, (size_t)1, i);
        if (sheap_base != remote_sheap_base)
        {
            printf("PE %d: the symmetric heap is not actually symmetric: my base = %p, PE %d base = %p \n",
                   mype, sheap_base, i, remote_sheap_base);
        }
    }
    if (errors==0)
            printf("PE %d: the symmetric heap is symmetric: my base = %p \n",
                   mype, sheap_base);
    shmem_barrier_all();
#endif

    int * local = malloc(n*sizeof(int));
    for (i=0; i<n; i++)
        local[i] = -mype;
    shmem_barrier_all();

    int target = (mype+1)%npes;
    shmem_int_get(local, sheap, (size_t)n, target);
    for (i=0; i<n; i++)
        if (local[i] != target)
            printf("PE %d, element %d: correct = %d, got %d \n", mype, i, target, local[i]);
    

    free(local);

    /* it is possible that Cray SHMEM doesn't call a barrier in shfree */
    shmem_barrier_all();
    shfree(sheap);
    
    printf("PE %d is done \n", mype);

    shmem_finalize();
    return 0;
}
