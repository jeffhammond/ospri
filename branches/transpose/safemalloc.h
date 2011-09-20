#define safemalloc((a)) malloc((a)); assert( (a) != NULL );
