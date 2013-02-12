#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pmi.h>
#include <pmi2.h>
#include <rca_lib.h>
#include <mpi.h>

int main(int argc, char * argv[])
{
  int rc;
  int rank, size;

  MPI_Init(&argc, &argv);

  int verbose = ( argc > 1 ? atoi(argv[1]) : 0 );

  PMI_BOOL initialized;
  rc = PMI_Initialized(&initialized);
  if (rc!=PMI_SUCCESS) 
    PMI_Abort(rc,"PMI_Initialized failed");

  if (initialized!=PMI_TRUE)
  {
    int spawned;
    rc = PMI_Init(&spawned);
    if (rc!=PMI_SUCCESS) 
      PMI_Abort(rc,"PMI_Init failed");
  }

  rc = PMI_Get_rank(&rank);
  if (rc!=PMI_SUCCESS) 
    PMI_Abort(rc,"PMI_Get_rank failed");

  rc = PMI_Get_size(&size);
  if (rc!=PMI_SUCCESS) 
    PMI_Abort(rc,"PMI_Get_size failed");

  int rpn; /* rpn = ranks per node */
  rc = PMI_Get_clique_size(&rpn); 
  if (rc!=PMI_SUCCESS)  
    PMI_Abort(rc,"PMI_Get_clique_size failed");

  int * clique_ranks = malloc( rpn * sizeof(int) );
  if (clique_ranks==NULL) 
    PMI_Abort(rpn,"malloc failed");

  rc = PMI_Get_clique_ranks(clique_ranks, rpn); 
  if (rc!=PMI_SUCCESS)  
    PMI_Abort(rc,"PMI_Get_clique_ranks failed");

  int nid;
  rc = PMI_Get_nid(rank, &nid);
  if (rc!=PMI_SUCCESS) 
    PMI_Abort(rc,"PMI_Get_nid failed");

  rca_mesh_coord_t xyz, mxyz;
  rc = rca_get_meshcoord( (uint16_t) nid, &xyz);
  if (rc!=0) 
    PMI_Abort(rc,"rca_get_meshcoord");

  rc = rca_get_max_dimension(&mxyz);
  if (rc!=0) 
    PMI_Abort(rc,"rca_get_max_dimension");

  int len = 255;
  char hostname[len];
  rc = gethostname(hostname, len);
  if (rc!=PMI_SUCCESS) 
    PMI_Abort(rc,"gethostname");

  int mpilen;
  char mpiname[MPI_MAX_PROCESSOR_NAME];
  MPI_Get_processor_name(mpiname, &mpilen);

  printf("rank %d of %d, nid = %d, xyz = (%u,%u,%u) of (%u,%u,%u), hostname = %s mpiname = %s \n", 
          rank, size, nid, 
          xyz.mesh_x, xyz.mesh_y, xyz.mesh_z, 
          mxyz.mesh_x, mxyz.mesh_y, mxyz.mesh_z, 
          hostname, mpiname);

  if (verbose>0)
  {
      printf("rank %d, rpn = %d, clique[] = ", rank, rpn); 
      for(int i = 0; i<rpn; i++)
        printf("%d ", clique_ranks[i]); 
      printf("\n");
  }

#if 0
  int flag = 0;
  char value[PMI2_MAX_VALLEN];
  PMI2_Info_GetJobAttr( "physTopology", value, PMI2_MAX_VALLEN, &flag );
  if (!flag) 
    PMI_Abort(flag,"PMI2_Info_GetJobAttr failed on physTopology");
  else
    printf("physTopology = %s \n", value);

  if (strcmp( value, "hierarchical" ) == 0) {
    char *p=0;
    PMI2_Info_GetJobAttr( "phyTopologyLevels", value, PMI2_MAX_VALLEN, &flag );
    if (!flag) 
      PMI_Abort(flag,"PMI2_Info_GetJobAttr failed on phyTopologyLevels");
    else
      printf("physTopologyLevels = %s \n", value);

    PMI2_Info_GetJobAttr( "localRanksCount", value, PMI2_MAX_VALLEN, &flag );
    if (!flag) 
      PMI_Abort(flag,"PMI2_Info_GetJobAttr failed on localRanksCount");

    /* Lets see if the bottom level is complete */
    p = strrchr( value, ',' );
    if (!p) 
      PMI_Abort(1,"error, malformed value");

    if ( strcmp(p+1,"complete") == 0) {
      int inlen = 128, outlen;
      int mranks[inlen];
      /* int PMI2_Info_GetNodeAttrIntArray(const char name[], int array[], int arraylen, int *outlen, int *found); */
      PMI2_Info_GetNodeAttrIntArray( "localRanks", mranks, inlen, &outlen, &flag );
      if (!flag)
        PMI_Abort(rc,"PMI2_Info_GetJobAttr failed on localRanks");
      /* else */
    
    }
  }
#endif

  free(clique_ranks);
  fflush(stdout);

  MPI_Finalize();

  return 0;
}
