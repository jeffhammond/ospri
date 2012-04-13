CC     = clang
CFLAGS = -g -Wall -O2 -DREALTYPE=double -DRESTRICT=restrict # -std=c99

OBJECTS := vec_copy_c.o      vec_scale_c.o
TESTS   := vec_copy_driver.x vec_scale_driver.x

all: $(TESTS)

%.x: %.o $(OBJECTS)
	$(CC) $(CFLAGS) $< $(OBJECTS) safemalloc.o -o $@

%.o: %.c alcf-vector.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f $(OBJECTS) safemalloc.o

distclean: clean
	-rm -f $(TESTS)

