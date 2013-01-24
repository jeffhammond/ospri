// -*- c++ -*-
// To build:
//    (need to use C++ because of pass-by-reference in swap)
//    wrap.py mpiarbrpn.w > mpiarbrpn.C
//    mpicc -c mpiarbrpn.C
//    ar cr libmpiarbrpn.a mpiarbrpn.o
//    ranlib libmpiarbrpn.a
#include <unistd.h>
#include <mpi.h>

#if defined(__bgq__)
#  include </bgsys/drivers/ppcfloor/spi/include/kernel/process.h>
#  include </bgsys/drivers/ppcfloor/spi/include/kernel/location.h>
#  include </bgsys/drivers/ppcfloor/firmware/include/personality.h>
#elif defined(__bgp__)
#  error Blue Gene/P support has not been implemented yet.
#elif defined(__crayxe)
#  error Cray Gemini support has not been implemented yet.
#else
#  error This only works on special supercomputers.
#endif

static MPI_Comm altworld;
static int sleep_and_abort;
static int marpn_debug;

inline void swap_world(MPI_Comm & world) {
   if (world == MPI_COMM_WORLD) {
      world = altworld;
   }
}

{{fn func MPI_Init}}{
   {{callfn}}

   int rank;
   PMPI_Comm_rank(MPI_COMM_WORLD, &rank);

#if defined(__bgq__)
   int maxrpn = Kernel_ProcessCount();
   int rpn    = maxrpn;
#else
   int maxrpn = 1;
   int rpn    = maxrpn;
#endif

   marpn_debug = 0;
   char * dbgenv;
   dbgenv = getenv ("MARPN_DEBUG");
   if (dbgenv!=NULL)
      marpn_debug = atoi(dbgenv);

   sleep_and_abort = 0;
   char * sabenv;
   sabenv = getenv ("MARPN_SLEEP_AND_ABORT");
   if (sabenv!=NULL)
      sleep_and_abort = atoi(sabenv);

   char * rpnenv;
   rpnenv = getenv ("MARPN_RANKS_PER_NODE");
   if (rpnenv!=NULL)
   {
      rpn = atoi(rpnenv);
      if (rpn>maxrpn)
      {
         printf("You have requested more ranks per node (%d) than are available (%d)! \n", 
                rpn, maxrpn);
         PMPI_Abort(MPI_COMM_WORLD, rpn);
         exit(rpn);
      }
      else if (rpn<maxrpn)
      {
         int coreid    = Kernel_ProcessorCoreID();   /* 0-15 */
         int corehwtid = Kernel_ProcessorThreadID(); /* 0-3  */

         int keep = 0;
         /* it is a good idea to spread the active ranks across all the cores 
          * one should not assume a particular rank layout when splitting world */
         if (rpn<=16)
            keep =                    ((coreid < (rpn   )) && (corehwtid == 0));
         else if (rpn<=32)
            keep = (corehwtid < 1) || ((coreid < (rpn-16)) && (corehwtid == 1));
         else if (rpn<=48)
            keep = (corehwtid < 2) || ((coreid < (rpn-32)) && (corehwtid == 2));
         else if (rpn<=64)
            keep = (corehwtid < 3) || ((coreid < (rpn-48)) && (corehwtid == 3));

         if (marpn_debug)
            printf("rank %d (core %d, hwtid %d) is %s from the new world \n", rank, coreid, corehwtid, keep ? "included" : "excluded" );

         PMPI_Comm_split(MPI_COMM_WORLD, keep, rank, &altworld);
         if (!keep) 
         {
            if (sleep_and_abort)
            {
               if (marpn_debug)
                  printf("rank %d (core %d, hwtid %d) going to sleep \n", rank, coreid, corehwtid );
               /* 1B seconds is a long time */
               sleep(1000000000);
            }
            else
            {
               if (marpn_debug)
                  printf("rank %d (core %d, hwtid %d) calling MPI_Finalize \n", rank, coreid, corehwtid );
               PMPI_Finalize();
               exit(0);
            }
         }
      }
      else /* trivial case */
      {
         PMPI_Comm_dup(MPI_COMM_WORLD, &altworld);
      }
   } 

}{{endfn}}

{{fn func MPI_Finalize}}{
   if (sleep_and_abort)
   {
      /* returning exit code 0 ~should~ behave like MPI_Finalize here */
      PMPI_Abort(MPI_COMM_WORLD, 0);
      exit(0);
   }
   {{callfn}}
}{{endfn}}

{{fnall func MPI_Init MPI_Finalize}}{
   {{apply_to_type MPI_Comm swap_world}}
   {{callfn}}
}{{endfnall}}
