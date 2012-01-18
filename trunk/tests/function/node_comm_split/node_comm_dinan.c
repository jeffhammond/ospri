/* This code was authored by Jim Dinan */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void MPIX_Comm_split_node(MPI_Comm parent_in, MPI_Comm *out) {
  char     my_name[MPI_MAX_PROCESSOR_NAME];
  MPI_Comm node_comm = MPI_COMM_NULL;
  MPI_Comm parent_comm;
  int      len;

  // Dup so we don't leak communicators
  MPI_Comm_dup(parent_in, &parent_comm);
  MPI_Get_processor_name(my_name, &len);

  while (node_comm == MPI_COMM_NULL) {
    char     root_name[MPI_MAX_PROCESSOR_NAME];
    int      rank;
    MPI_Comm old_parent;

    MPI_Comm_rank(parent_comm, &rank);

    if (rank == 0) {
      MPI_Bcast(my_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, parent_comm);
      strncpy(root_name, my_name, MPI_MAX_PROCESSOR_NAME);
    } else {
      MPI_Bcast(root_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, parent_comm);
    }

    old_parent = parent_comm;

    if (strncmp(my_name, root_name, MPI_MAX_PROCESSOR_NAME) == 0) {
      //  My group splits off, I'm done after this
      MPI_Comm_split(parent_comm, 1, rank, &node_comm);
    } else {
      // My group keeps going, separate from the others
      MPI_Comm_split(parent_comm, 0, rank, &parent_comm);
    }

    // Old parent is no longer needed
    MPI_Comm_free(&old_parent);
  }

  *out = node_comm;
}

int main(int argc, char **argv)
{
  char my_name[MPI_MAX_PROCESSOR_NAME];
  int  node_rank, node_size, len;
  int  world_rank, world_size;
  MPI_Comm node_comm;

  MPI_Init(&argc, &argv);

  MPIX_Comm_split_node(MPI_COMM_WORLD, &node_comm);

  MPI_Get_processor_name(my_name, &len);
  MPI_Comm_rank(node_comm, &node_rank);
  MPI_Comm_size(node_comm, &node_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  printf("%s: %d of %d on node, %d of %d on world \n", my_name, node_rank, node_size, world_rank, world_size );

  MPI_Comm_free(&node_comm);

  MPI_Finalize();

  return 0;
}
