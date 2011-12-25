CC=mpicc
CFLAGS=-g -std=c99 -O1
#BLAS=-lblas
BLAS=-L/software/goto -lgoto2_core2-r1.13 -lgfortran

all: mpi_rma_self_versus_memcpy.x mpi_acc_self_versus_daxpy.x

mpi_rma_self_versus_memcpy.x: mpi_rma_self_versus_memcpy.c
	$(CC) $(CFLAGS) $< -o $@

mpi_acc_self_versus_daxpy.x: mpi_acc_self_versus_daxpy.c
	$(CC) $(CFLAGS) $< $(BLAS) -o $@

refresh: realclean all

clean:
	rm -f *.o

realclean: clean
	rm -f *.x

