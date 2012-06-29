#include <stdio.h>
#include <stdlib.h>
#include <pmi.h>
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
  for(int i = 0; i<rpn; i++)
    printf("rank %d clique[%d] = %d \n", rank, i, clique_ranks[i]); 

  int nid;
  rc = PMI_Get_nid(rank, &nid);
  if (rc!=PMI_SUCCESS) 
    PMI_Abort(rc,"PMI_Get_nid failed");
  printf("rank %d PMI_Get_nid gives nid %d \n", rank, nid);

  rca_mesh_coord_t xyz;
  rca_get_meshcoord( (uint16_t) nid, &xyz);
  printf("rank %d rca_get_meshcoord returns (%2u,%2u,%2u)\n", rank, xyz.mesh_x, xyz.mesh_y, xyz.mesh_z);

  fflush(stdout);
  return 0;
}
