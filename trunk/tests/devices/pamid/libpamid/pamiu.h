#ifndef PAMIU_H
#define PAMIU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int posix_memalign(void ** memptr, size_t alignment, size_t size);

#define ALIGNMENT 128

void * PAMIU_Malloc(size_t size);
void PAMIU_Free(void * ptr);

#endif /* PAMIU_H */
