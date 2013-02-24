#include <stdio.h>

#ifdef __bgq__
#include <hwi/include/bqc/A2_inlines.h>
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/memory.h>
#endif

#define PRINT_SUCCESS 0

//#define SLEEP sleep
#define SLEEP usleep

#ifdef DEBUG
#define TEST_ASSERT(c,m) \
        do { \
        if (!(c)) { \
                    printf(m" FAILED on rank %ld\n", world_rank); \
                    fflush(stdout); \
                  } \
        else if (PRINT_SUCCESS) { \
                    printf(m" SUCCEEDED on rank %ld\n", world_rank); \
                    fflush(stdout); \
                  } \
        SLEEP(1); \
        assert(c); \
        } \
        while(0);
#else
#define TEST_ASSERT(c,m) 
#endif

static size_t world_size, world_rank = -1;

static void cb_done (void *ctxt, void * clientdata, pami_result_t err)
{
  int * active = (int *) clientdata;
  (*active)--;
}

static void print_meminfo(FILE * output, char * message)
{
#ifdef __bgq__
  uint64_t shared, persist, heapavail, stackavail, stack, heap, guard, mmap;

  Kernel_GetMemorySize(KERNEL_MEMSIZE_SHARED, &shared);
  Kernel_GetMemorySize(KERNEL_MEMSIZE_PERSIST, &persist);
  Kernel_GetMemorySize(KERNEL_MEMSIZE_HEAPAVAIL, &heapavail);
  Kernel_GetMemorySize(KERNEL_MEMSIZE_STACKAVAIL, &stackavail);
  Kernel_GetMemorySize(KERNEL_MEMSIZE_STACK, &stack);
  Kernel_GetMemorySize(KERNEL_MEMSIZE_HEAP, &heap);
  Kernel_GetMemorySize(KERNEL_MEMSIZE_GUARD, &guard);
  Kernel_GetMemorySize(KERNEL_MEMSIZE_MMAP, &mmap);

  fprintf(output, "MEMORY INFORMATION: %s \n", message);
  fprintf(output, "Allocated heap: %.2f MB, avail. heap: %.2f MB\n", 
          (double)heap/(1024*1024), (double)heapavail/(1024*1024));
  fprintf(output, "Allocated stack: %.2f MB, avail. stack: %.2f MB\n", 
          (double)stack/(1024*1024), (double)stackavail/(1024*1024));
  fprintf(output, "Memory: shared: %.2f MB, persist: %.2f MB, guard: %.2f MB, mmap: %.2f MB\n", 
          (double)shared/(1024*1024), (double)persist/(1024*1024), (double)guard/(1024*1024), (double)mmap/(1024*1024));
  fflush(stdout);
#endif

  return;
}
