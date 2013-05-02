#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include <spi/include/kernel/memory.h>
#include <spi/include/l2/barrier.h>

int num_threads;
pthread_t * pool;

static L2_Barrier_t barrier = L2_BARRIER_INITIALIZER;

int debug = 0;

int get_thread_id(void)
{
    for (int i=0; i<num_threads; i++)
        if (pthread_self()==pool[i])
            return i;

    return -1;
}

void * fight(void * input)
{
    L2_Barrier(&barrier, num_threads);
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char * argv[])
{
    num_threads = (argc>1) ? atoi(argv[1]) : 1;
    printf("Pthread test using %d threads \n", num_threads );

    /* this "activates" the L2 atomic data structure */
    Kernel_L2AtomicsAllocate(&barrier, sizeof(L2_Barrier_t) );

    pool = (pthread_t *) malloc( num_threads * sizeof(pthread_t) );
    assert(pool!=NULL);

    uint64_t dt;

    dt = 0;
    for (int i=0; i<num_threads; i++) {
        uint64_t t0 = GetTimeBase();
        int rc = pthread_create(&(pool[i]), NULL, &fight, NULL);
        uint64_t t1 = GetTimeBase();
        dt += (t1-t0);
        if (rc!=0) {
            printf("pthread error \n");
            fflush(stdout);
            sleep(1);
            assert(rc==0);
        }
    }

    printf("%d threads created in %llu cycles per call \n", num_threads, dt/num_threads);
    fflush(stdout);

    dt = 0;
    for (int i=0; i<num_threads; i++) {
        void * junk;
        uint64_t t0 = GetTimeBase();
        int rc = pthread_join(pool[i], &junk);
        uint64_t t1 = GetTimeBase();
        dt += (t1-t0);
        if (rc!=0) {
            printf("pthread error \n");
            fflush(stdout);
            sleep(1);
            assert(rc==0);
        }
    }
    
    printf("%d threads joined in %llu cycles per call \n", num_threads, dt/num_threads);
    fflush(stdout);

    free(pool);
 
    return 0;   
}
