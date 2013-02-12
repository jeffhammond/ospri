#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pmi.h>
#include <pmi2.h>
#include <rca_lib.h>

int main(void)
{
  int rc;
  int rank, size;

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

  printf("rank %d of %d \n", rank, size);

  int rpn; /* rpn = ranks per node */
  rc = PMI_Get_clique_size(&rpn); 
  if (rc!=PMI_SUCCESS)  
    PMI_Abort(rc,"PMI_Get_clique_size failed");
  printf("rank %d clique size %d \n", rank, rpn);

  int * clique_ranks = malloc( rpn * sizeof(int) );
  if (clique_ranks==NULL) 
    PMI_Abort(rpn,"malloc failed");

  rc = PMI_Get_clique_ranks(clique_ranks, rpn); 
  if (rc!=PMI_SUCCESS)  
    PMI_Abort(rc,"PMI_Get_clique_ranks failed");
  printf("rank %d clique[] = ", rank); 
  for(int i = 0; i<rpn; i++)
    printf("%d ", clique_ranks[i]); 
  printf("\n");

  int nid;
  rc = PMI_Get_nid(rank, &nid);
  if (rc!=PMI_SUCCESS) 
    PMI_Abort(rc,"PMI_Get_nid failed");
  printf("rank %d PMI_Get_nid gives nid %d \n", rank, nid);

  rca_mesh_coord_t xyz;
  rca_get_meshcoord( (uint16_t) nid, &xyz);
  printf("rank %d rca_get_meshcoord returns (%3u,%3u,%3u)\n", rank, xyz.mesh_x, xyz.mesh_y, xyz.mesh_z);

  int HOST_NAME_MAX = 255;
  char hostname[HOST_NAME_MAX];
  rc = gethostname(hostname, HOST_NAME_MAX);
  if (rc!=PMI_SUCCESS) 
    PMI_Abort(rc,"gethostname");
  else
    printf("hostname = %s \n", hostname);

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

  fflush(stdout);
  return 0;
}
