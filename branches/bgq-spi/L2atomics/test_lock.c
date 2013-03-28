#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include <spi/include/l2/barrier.h>
#include <spi/include/l2/lock.h>

int num_threads;
pthread_t * pool;

int64_t counter;

L2_Barrier_t barrier = L2_BARRIER_INITIALIZER;

int get_thread_id(void)
{
    for (int i=0; i<num_threads; i++)
        if (pthread_self()==pool[i])
            return i;

    return -1;
}

void * fight(void * input)
{
    int tid = get_thread_id();

    printf("%d: before L2_Barrier 1 \n", tid);
    L2_Barrier(&barrier, num_threads);
    printf("%d: after  L2_Barrier 1 \n", tid);
    fflush(stdout);

    L2_Lock_t * lock_ptr = (L2_Lock_t *)input;

    int64_t mycounter = 0;

    while (mycounter<100000)
    {
        L2_LockAcquire(lock_ptr);
        counter++;
        mycounter++;
        L2_LockRelease(lock_ptr);

        if (mycounter%10000) 
            printf("%d: mycounter = %lld \n", tid, (long long int)mycounter);
    }

    printf("%d: before L2_Barrier 2 \n", tid);
    L2_Barrier(&barrier, num_threads);
    printf("%d: after  L2_Barrier 2 \n", tid);
    fflush(stdout);
    
    pthread_exit(NULL);

    return NULL;
}

int main(int argc, char * argv[])
{
    L2_Lock_t lock;

    num_threads = (argc>1) ? atoi(argv[1]) : 1;
    printf("L2 barrier test using %d threads \n", num_threads );

    Kernel_L2AtomicsAllocate(&barrier, sizeof(L2_Barrier_t) );
    Kernel_L2AtomicsAllocate(&lock, sizeof(L2_Lock_t));
    L2_LockInit(&lock);

    pool = (pthread_t *) malloc( num_threads * sizeof(pthread_t) );
    assert(pool!=NULL);

    counter = 0;

    for (int i=0; i<num_threads; i++) {
        int rc = pthread_create(&(pool[i]), NULL, &fight, NULL);
        if (rc!=0) {
            printf("pthread error \n");
            fflush(stdout);
            sleep(1);
        }
        assert(rc==0);
    }

    printf("threads created \n");
    fflush(stdout);

    for (int i=0; i<num_threads; i++) {
        void * junk;
        int rc = pthread_join(pool[i], &junk);
        if (rc!=0) {
            printf("pthread error \n");
            fflush(stdout);
            sleep(1);
        }
        assert(rc==0);
    }
    
    printf("threads joined \n");
    fflush(stdout);

    free(pool);
 
    return 0;   
}
