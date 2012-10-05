#ifndef DRIVER_H
#define DRIVER_H

void bcast_only(FILE * output, MPI_Comm comm, int max_mem);
void bcast_vs_scatter_allgather(FILE * output, MPI_Comm comm, int max_mem);
void allgather_only(FILE * output, MPI_Comm comm, int max_mem);
void alltoall_only(FILE * output, MPI_Comm comm, int max_mem);
void reducescatterblock_only(FILE * output, MPI_Comm comm, int max_mem);
void reducescatterblock_vs_reduceandscatter(FILE * output, MPI_Comm comm, int max_mem);
void reducescatterblock_vs_allreduce(FILE * output, MPI_Comm comm, int max_mem);

#endif // DRIVER_H
