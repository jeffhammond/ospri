#include "pamid.h"
#include <hwi/include/bqc/A2_inlines.h>

int main(int argc, char * argv[])
{
    PAMID_Initialize();

    size_t rank = PAMID_World_rank();
    size_t size = PAMID_World_size();

    size_t n = (argc>1 ? atoi(argv[1]) : 1000);

    void * ptr = malloc(n);
    if (ptr==NULL) abort();
    memset(ptr, '\0', n);

    void ** baseptrs = malloc(sizeof(void*)*size);
    if (baseptrs==NULL) abort();

    PAMID_Allgather_world(sizeof(void*), &ptr, baseptrs);

    //printf("%ld: ptr = %p baseptrs[%ld] = %p \n", rank, ptr, rank, baseptrs[rank] );

    void * src = malloc(n);
    if (src==NULL) abort();
    memset(src, '\1', n);

    size_t target = ( (rank==0) ? (size-1) : (rank-1) );
    printf("trying PAMID_Put_endtoend from %ld to %ld of %ld bytes \n", rank, target, n);

    uint64_t t0 = GetTimeBase();
    PAMID_Put_endtoend(n, src, target, baseptrs[target]);
    uint64_t t1 = GetTimeBase();
    uint64_t dt = t1-t0;
    printf("PAMID_Put_endtoend from rank %ld to %ld of %ld bytes took %llu cycles (%lf MB/s) \n",
            rank, target, n, (unsigned long long)dt, 1.0e-6 * n / dt);

    for (int i=0; i<100; i++)
        PAMID_Progress_poke();

    PAMID_Barrier_world();

    int same = memcmp( src, baseptrs[rank], n);
    printf("%ld: same = %d", rank, same);

    free(src);
    free(ptr);
    free(baseptrs);

    PAMID_Finalize();

    if (rank==0) printf("TEST DONE \n");
    fflush(stdout);

    return 0;
}
