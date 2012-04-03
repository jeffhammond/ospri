#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

#include <process.h>
#include <location.h>
#include <personality.h>

struct BGQ_Torus_t
{
    int32_t Coords[6];    
    int32_t PartitionSize[6];    
    int32_t PartitionTorus[6];    
    int32_t JobSize[6];    
    int32_t JobTorus[6];    
} 
BGQ_Torus_t
