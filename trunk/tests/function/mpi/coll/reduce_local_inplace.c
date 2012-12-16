#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv)
{
    int comm_rank, comm_size;

    MPI_Init(&argc,&argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    int t = (argc>1) ? atoi(argv[1]) : 1;
    int c = (argc>2) ? atoi(argv[2]) : 10000;
    
    double * in  = (double *) malloc(c*sizeof(double));
    double * out = (double *) malloc(c*sizeof(double));
    if (in==NULL || out==NULL) exit(1);

    for (int i=0 ; i<c; i++)
        in [i] = (double)i;

    for (int i=0 ; i<c; i++)
        out[i] = (double)i;

    int root = 0;

    double scale = 1.0;

    switch (t)
    {
        case 1:
            MPI_Reduce( in , out, c, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_SELF );
            break;

        case 2:
            MPI_Reduce( MPI_IN_PLACE , out, c, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_SELF );
            break;

        case 3:
            MPI_Reduce_local( in , out, c, MPI_DOUBLE, MPI_SUM );
            scale = 2.0;
            break;

        case 4:
            MPI_Reduce_local( MPI_IN_PLACE , out, c, MPI_DOUBLE, MPI_SUM );
            scale = 2.0;
            break;

        default:
            printf("valid options are 1, 2, 3, 4 \n");
            exit(1);
            break;
    }


    int errors = 0;
    for (int i=0 ; i<c; i++)
        errors += (int) ( out[i] != i*scale );

    if (errors>0)
    {
        printf("MPI_Reduce had %d errors! \n", errors);
        for (int i=0 ; i<c; i++)
            printf("%d: out[%d] = %lf (correct is %lf) \n",
                    comm_rank, i, out[i], i*scale );
        exit(1);
    }

    free(out);
    free(in );

    printf("test finished successfully \n");

    return 0;
}

