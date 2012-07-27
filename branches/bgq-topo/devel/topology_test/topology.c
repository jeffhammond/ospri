
/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* (C) Copyright IBM Corp.  2011, 2011                              */
/* IBM CPL License                                                  */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */


///////////////////////////////////////////////////////
////  Alltoall Test via MU Rmote Put operations
///////////////////////////////////////////////////////

/////////////////////////////////////////
/// Test library common code
////////////////////////////////////////
#include "msg_common.h"

// Maximum number of ndoes this test will run on
#define MAX_NUM_NODES          1024

// struct to hold torus coordinates or dimensions data
typedef struct { uint8_t a,b,c,d,e; } torus_t;

// number of participating processes (1 proc/node)
unsigned numNodes;
int myA, myB, myC, myD, myE;

// rank of this process
unsigned myRank, myRank_test, myCoord, myCoreID, myHWTID;

int main(int argc, char **argv)
{
    BG_CoordinateMapping_t coord;
    BG_JobCoords_t job;
    Personality_t pers;
    Kernel_GetPersonality(&pers, sizeof(pers));
    myRank=Kernel_GetRank();
    myCoord=Kernel_MyTcoord();
    Kernel_JobCoords(&job);
//    myCoreID=Kernel_ProcessorCoreID();
//    myHWTID=Kernel_ProcessorThreadID();
//    myPhysicalID=Kernel_PhysicalProcessorID();
  torus_t tcoords =
    {
       myA=pers.Network_Config.Acoord,
       myB=pers.Network_Config.Bcoord,
       myC=pers.Network_Config.Ccoord,
       myD=pers.Network_Config.Dcoord,
       myE=pers.Network_Config.Ecoord
     };

  torus_t tdims =
    {
      pers.Network_Config.Anodes,
      pers.Network_Config.Bnodes,
      pers.Network_Config.Cnodes,
      pers.Network_Config.Dnodes,
      pers.Network_Config.Enodes
    };

  numNodes = tdims.a * tdims.b * tdims.c * tdims.d * tdims.e;

  unsigned my_com_A=job.shape.a;
  unsigned my_com_B=job.shape.b;
  unsigned my_com_C=job.shape.c;
  unsigned my_com_D=job.shape.d;
  unsigned my_com_E=job.shape.e;

  unsigned my_com_Acoord=job.corner.a;
  unsigned my_com_Bcoord=job.corner.b;
  unsigned my_com_Ccoord=job.corner.c;
  unsigned my_com_Dcoord=job.corner.d;
  unsigned my_com_Ecoord=job.corner.e;

    if ( myRank == 763 )
      {
        printf("number of nodes:%d \n", numNodes);
        printf("number of processes per node:%d \n",Kernel_ProcessCount());
        printf("number of hardware threads per process:%d \n",Kernel_ProcessorCount());
        printf("MPI rank %d has 5D torus coordinates <%d,%d,%d,%d,%d> \n", myRank, myA, myB, myC, myD, myE);
        printf("job has 5D torus dimensions <%d,%d,%d,%d,%d> \n", tdims.a, tdims.b, tdims.c, tdims.d, tdims.e);
        printf("MPI rank %d has dimensions <%d,%d,%d,%d,%d> \n", myRank, my_com_A,my_com_B, my_com_C, my_com_D, my_com_E);
        printf("MPI rank %d has coordinates <%d,%d,%d,%d,%d> \n",myRank, my_com_Acoord, my_com_Bcoord, my_com_Ccoord, my_com_Dcoord, my_com_Ecoord); 
      }

//        printf("rank %d has coordinates 
  unsigned a,b,c,d,e;
  unsigned a_mult = tdims.b * tdims.c * tdims.d * tdims.e;
  unsigned b_mult = tdims.c * tdims.d * tdims.e;
  unsigned c_mult = tdims.d * tdims.e;
  unsigned d_mult = tdims.e;

  for ( a = 0; a < tdims.a; a++ )
    for ( b = 0; b < tdims.b; b++ )
      for ( c = 0; c < tdims.c; c++ )
        for ( d = 0; d < tdims.d; d++ )
          for ( e = 0; e < tdims.e; e++ )
            {
               unsigned rank = a * a_mult + b * b_mult + c * c_mult + d * d_mult + e;

               if ( a == tcoords.a &&
                    b == tcoords.b &&
                    c == tcoords.c &&
                    d == tcoords.d &&
                    e == tcoords.e )

               myRank_test=rank;
            }

//         if (myRank == myRank_test) printf("MPI rank %d returns 1 \n", myRank);
}
