/***************************************************************************

                  COPYRIGHT

The following is a notice of limited availability of the code, and disclaimer
which must be included in the prologue of the code and in all source listings
of the code.

Copyright Notice
 + 2009 University of Chicago

Permission is hereby granted to use, reproduce, prepare derivative works, and
to redistribute to others.  This software was authored by:

Jeff R. Hammond
Leadership Computing Facility
Argonne National Laboratory
Argonne IL 60439 USA
phone: (630) 252-5381
e-mail: jhammond@mcs.anl.gov

                  GOVERNMENT LICENSE

Portions of this material resulted from work developed under a U.S.
Government Contract and are subject to the following license: the Government
is granted for itself and others acting on its behalf a paid-up, nonexclusive,
irrevocable worldwide license in this computer software to reproduce, prepare
derivative works, and perform publicly and display publicly.

                  DISCLAIMER

This computer code material was prepared, in part, as an account of work
sponsored by an agency of the United States Government.  Neither the United
States, nor the University of Chicago, nor any of their employees, makes any
warranty express or implied, or assumes any legal liability or responsibility
for the accuracy, completeness, or usefulness of any information, apparatus,
product, or process disclosed, or represents that its use would not infringe
privately owned rights.

 ***************************************************************************/

#include "comm_bench.h"

int main(int argc, char **argv)
{
    int me;
    int nproc;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&me);
    MPI_Comm_size(MPI_COMM_WORLD,&nproc);

    int nodeid = -1;
    PMI_Portals_get_nid(me, &nodeid);
    printf("%5d: PMI_Portals_get_nid returns %6d\n",me,nodeid);
    fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

    rca_mesh_coord_t xyz;
    rca_get_meshcoord((uint16_t)nodeid, &xyz);
    unsigned short xTorus = xyz.mesh_x;
    unsigned short yTorus = xyz.mesh_y;
    unsigned short zTorus = xyz.mesh_z;
    printf("%5d: rca_get_meshcoord returns (%2u,%2u,%2u)\n",me,xTorus,yTorus,zTorus);
    fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    return(0);
}

