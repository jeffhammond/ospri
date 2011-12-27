CC=mpicc
CFLAGS=-g -std=c99 -O1

all: test_isend.x

%.x: %.c
	$(CC) $(CFLAGS) $< -o $@

refresh: realclean all

clean:
	rm -f *.o

realclean: clean
	rm -f *.x

