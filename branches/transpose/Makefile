ifdef FRONTEND
    CC=xlc_r
    CFLAGS=-g -O5 -qhot -qipa=list="transpose.x.lst" -qlanglvl=stdc99 -qreport
    OMPFLAGS=-qsmp=omp -DOMP
    LIBS=safemalloc.o
else
    CC=mpixlc_r
    CFLAGS=-g -O5 -qhot -qipa=list="transpose.x.lst" -qtune=450 -qarch=450d -qlanglvl=stdc99 -qreport -DMPI
    OMPFLAGS=-qsmp=omp -DOMP
    HPM=-L/soft/apps/UPC/lib -lhpm
    LIBS=$(HPM) safemalloc.o
endif

all: transpose.x transpose-omp.x

safemalloc.o: safemalloc.c safemalloc.h
	$(CC) $(CFLAGS) -c safemalloc.c -o safemalloc.o

transpose.x: transpose.c safemalloc.o
	$(CC) $(CFLAGS) transpose.c $(LIBS) -o transpose.x

transpose-omp.x: transpose.c safemalloc.o
	$(CC) $(CFLAGS) $(OMPFLAGS) transpose.c $(LIBS) -o transpose-omp.x

ifdef FRONTEND
submit: all
	OMP_NUM_THREADS=1 ./transpose.x
	OMP_NUM_THREADS=4 ./transpose-omp.x
else
submit: all
		qsub --mode smp -n 1 -t 55 --env OMP_NUM_THREADS=1 ./transpose.x 1 550 1
		qsub --mode smp -n 1 -t 55 --env OMP_NUM_THREADS=4 ./transpose-omp.x 1 550 1
		qsub --mode smp -n 1 -t 55 --env OMP_NUM_THREADS=1 ./transpose.x 16 3072 16
		qsub --mode smp -n 1 -t 55 --env OMP_NUM_THREADS=4 ./transpose-omp.x 16 3072 16
endif

clean:
	rm -f *.o 

realclean: clean
	rm -f *.x *.lst

