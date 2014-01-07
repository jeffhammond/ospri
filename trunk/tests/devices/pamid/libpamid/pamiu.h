#ifndef PAMIU_H
#define PAMIU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int posix_memalign(void ** memptr, size_t alignment, size_t size);

#define ALIGNMENT 128

void * PAMIU_Malloc(size_t size);
void PAMIU_Free(void * ptr);

#define PAMIU_SAFE_CAST(l,i) \
        if (l>INT_MAX || l<INT_MIN) \
            fprintf(stderr,"PAMIU_SAFE_CAST: loss of precision has occurred in casting %ld from long to int ", l); \
        i = ( (l<INT_MAX || l>INT_MIN) ? l : INT_MIN);

#endif /* PAMIU_H */
