#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>

int main(int argc, char* argv[])
{
    int world_rank = 0;

    size_t size = ( argc>1 ? atoi(argv[1]) : getpagesize() ); 
    printf("%d: size = %ld \n", world_rank, (long)size);

    char * filename = "/dev/local/foo";    // works
    printf("%d: filename = %s \n", world_rank, filename);

    FILE * file = fopen(filename, "rw");
    if (file==NULL) printf("%d: fopen failed: %p \n", world_rank, file);
    else            printf("%d: fopen succeeded: %p \n", world_rank, file);

    int n = -1;
    {
      char * tmp = malloc(100);
      assert(tmp!=NULL);

      n = sprintf(tmp, "Team America, Fuck Yeah!");
      if (n<0) printf("%d: sprintf failed \n", world_rank);
      else     printf("%d: sprintf succeeded \n", world_rank);

      printf("%d: tmp = %s after sprintf \n", world_rank, tmp);

      int rc = fputs(tmp, file);
      if (rc==EOF) printf("%d: fputs failed \n", world_rank);
      else         printf("%d: fputs succeeded \n", world_rank);
    }

    {
      char * tmp = malloc(n+1);
      assert(tmp!=NULL);

      void * ptr = fgets(tmp, n, file);
      if (ptr==NULL) printf("%d: fgets failed \n", world_rank);
      else           printf("%d: fgets succeeded \n", world_rank);

      printf("%d: tmp = %s after fgets \n", world_rank, tmp);
    }

    {
      int rc = fclose(file);
      if (rc==0) printf("%d: fclose succeeded \n", world_rank);
      else       printf("%d: fclose failed \n", world_rank);
    }

    printf("%d: test finished \n", world_rank);

    return 0;
}
