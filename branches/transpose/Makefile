ifdef FRONTEND
    CC=xlc_r
    CFLAGS=-g -O5 -qhot -qipa=list="tranpose.x.lst" -qlanglvl=stdc99 -qreport
    LIBS=safemalloc.o
else
    CC=mpixlc_r
    CFLAGS=-g -O5 -qhot -qipa=list="tranpose.x.lst" -qtune=450 -qarch=450d -qsmp=omp -qlanglvl=stdc99 -qreport -DMPI
    HPM=-L/soft/apps/UPC/lib -lhpm
    LIBS=$(HPM) safemalloc.o
endif

all: realclean transpose.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o safemalloc.o
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

ifdef FRONTEND
submit: all
	OMP_NUM_THREADS=4 ./transpose.x
else
submit: all
	qsub --mode smp -n 1 -t 55 --env OMP_NUM_THREADS=4 ./transpose.x 1 550 1
	qsub --mode smp -n 1 -t 55 --env OMP_NUM_THREADS=4 ./transpose.x 16 3072 16
endif

clean:
	rm -f *.o 

realclean: clean
	rm -f *.x *.lst

