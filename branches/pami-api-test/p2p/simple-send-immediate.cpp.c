unsigned gVerbose = 1;

enum opNum
{
  OP_COPY,
  OP_NOOP,
  OP_MAX,
  OP_MIN,
  OP_SUM,
  OP_PROD,
  OP_LAND,
  OP_LOR,
  OP_LXOR,
  OP_BAND,
  OP_BOR,
  OP_BXOR,
  OP_MAXLOC,
  OP_MINLOC,
  OP_COUNT,
};
static const int op_count = OP_COUNT;
pami_data_function op_array[OP_COUNT];
const char *op_array_str[OP_COUNT];

enum dtNum
{
  DT_NULL,
  DT_BYTE,
  DT_SIGNED_CHAR,
  DT_UNSIGNED_CHAR,
  DT_SIGNED_SHORT,
  DT_UNSIGNED_SHORT,
  DT_SIGNED_INT,
  DT_UNSIGNED_INT,
  DT_SIGNED_LONG,
  DT_UNSIGNED_LONG,
  DT_SIGNED_LONG_LONG,
  DT_UNSIGNED_LONG_LONG,
  DT_FLOAT,
  DT_DOUBLE,
  DT_LONG_DOUBLE,
  DT_LOGICAL1,
  DT_LOGICAL2,
  DT_LOGICAL4,
  DT_LOGICAL8,
  DT_SINGLE_COMPLEX,
  DT_DOUBLE_COMPLEX,
  DT_LOC_2INT,
  DT_LOC_2FLOAT,
  DT_LOC_2DOUBLE,
  DT_LOC_SHORT_INT,
  DT_LOC_FLOAT_INT,
  DT_LOC_DOUBLE_INT,
  DT_LOC_LONG_INT,
  DT_LOC_LONGDOUBLE_INT,
  DT_COUNT,
};
int dt_count = DT_COUNT;
pami_type_t dt_array[DT_COUNT];
const char * dt_array_str[DT_COUNT];

size_t ** alloc2DContig(int nrows, int ncols)
{
  int i;
  size_t **array;
  array = (size_t**)malloc(nrows * sizeof(size_t*));
  assert(array);
  array[0] = (size_t *)calloc(nrows * ncols, sizeof(size_t));
  assert(array[0]);

  for (i = 1; i < nrows; i++)
      array[i] = array[0] + i * ncols;

  return array;
}

void init_tables()
{
  op_array[0]=PAMI_DATA_COPY;
  op_array[1]=PAMI_DATA_NOOP;
  op_array[2]=PAMI_DATA_MAX;
  op_array[3]=PAMI_DATA_MIN;
  op_array[4]=PAMI_DATA_SUM;
  op_array[5]=PAMI_DATA_PROD;
  op_array[6]=PAMI_DATA_LAND;
  op_array[7]=PAMI_DATA_LOR;
  op_array[8]=PAMI_DATA_LXOR;
  op_array[9]=PAMI_DATA_BAND;
  op_array[10]=PAMI_DATA_BOR;
  op_array[11]=PAMI_DATA_BXOR;
  op_array[12]=PAMI_DATA_MAXLOC;
  op_array[13]=PAMI_DATA_MINLOC;

  op_array_str[0]="PAMI_DATA_COPY";
  op_array_str[1]="PAMI_DATA_NOOP";
  op_array_str[2]="PAMI_DATA_MAX";
  op_array_str[3]="PAMI_DATA_MIN";
  op_array_str[4]="PAMI_DATA_SUM";
  op_array_str[5]="PAMI_DATA_PROD";
  op_array_str[6]="PAMI_DATA_LAND";
  op_array_str[7]="PAMI_DATA_LOR";
  op_array_str[8]="PAMI_DATA_LXOR";
  op_array_str[9]="PAMI_DATA_BAND";
  op_array_str[10]="PAMI_DATA_BOR";
  op_array_str[11]="PAMI_DATA_BXOR";
  op_array_str[12]="PAMI_DATA_MAXLOC";
  op_array_str[13]="PAMI_DATA_MINLOC";

  dt_array[0]=PAMI_TYPE_NULL;
  dt_array[1]=PAMI_TYPE_BYTE;
  dt_array[2]=PAMI_TYPE_SIGNED_CHAR;
  dt_array[3]=PAMI_TYPE_UNSIGNED_CHAR;
  dt_array[4]=PAMI_TYPE_SIGNED_SHORT;
  dt_array[5]=PAMI_TYPE_UNSIGNED_SHORT;
  dt_array[6]=PAMI_TYPE_SIGNED_INT;
  dt_array[7]=PAMI_TYPE_UNSIGNED_INT;
  dt_array[8]=PAMI_TYPE_SIGNED_LONG;
  dt_array[9]=PAMI_TYPE_UNSIGNED_LONG;
  dt_array[10]=PAMI_TYPE_SIGNED_LONG_LONG;
  dt_array[11]=PAMI_TYPE_UNSIGNED_LONG_LONG;
  dt_array[12]=PAMI_TYPE_FLOAT;
  dt_array[13]=PAMI_TYPE_DOUBLE;
  dt_array[14]=PAMI_TYPE_LONG_DOUBLE;
  dt_array[15]=PAMI_TYPE_LOGICAL1;
  dt_array[16]=PAMI_TYPE_LOGICAL2;
  dt_array[17]=PAMI_TYPE_LOGICAL4;
  dt_array[18]=PAMI_TYPE_LOGICAL8;
  dt_array[19]=PAMI_TYPE_SINGLE_COMPLEX;
  dt_array[20]=PAMI_TYPE_DOUBLE_COMPLEX;
  dt_array[21]=PAMI_TYPE_LOC_2INT;
  dt_array[22]=PAMI_TYPE_LOC_2FLOAT;
  dt_array[23]=PAMI_TYPE_LOC_2DOUBLE;
  dt_array[24]=PAMI_TYPE_LOC_SHORT_INT;
  dt_array[25]=PAMI_TYPE_LOC_FLOAT_INT;
  dt_array[26]=PAMI_TYPE_LOC_DOUBLE_INT;
  dt_array[27]=PAMI_TYPE_LOC_LONG_INT;
  dt_array[28]=PAMI_TYPE_LOC_LONGDOUBLE_INT;

  dt_array_str[0]="PAMI_TYPE_NULL";
  dt_array_str[1]="PAMI_TYPE_BYTE";
  dt_array_str[2]="PAMI_TYPE_SIGNED_CHAR";
  dt_array_str[3]="PAMI_TYPE_UNSIGNED_CHAR";
  dt_array_str[4]="PAMI_TYPE_SIGNED_SHORT";
  dt_array_str[5]="PAMI_TYPE_UNSIGNED_SHORT";
  dt_array_str[6]="PAMI_TYPE_SIGNED_INT";
  dt_array_str[7]="PAMI_TYPE_UNSIGNED_INT";
  dt_array_str[8]="PAMI_TYPE_SIGNED_LONG";
  dt_array_str[9]="PAMI_TYPE_UNSIGNED_LONG";
  dt_array_str[10]="PAMI_TYPE_SIGNED_LONG_LONG";
  dt_array_str[11]="PAMI_TYPE_UNSIGNED_LONG_LONG";
  dt_array_str[12]="PAMI_TYPE_FLOAT";
  dt_array_str[13]="PAMI_TYPE_DOUBLE";
  dt_array_str[14]="PAMI_TYPE_LONG_DOUBLE";
  dt_array_str[15]="PAMI_TYPE_LOGICAL1";
  dt_array_str[16]="PAMI_TYPE_LOGICAL2";
  dt_array_str[17]="PAMI_TYPE_LOGICAL4";
  dt_array_str[18]="PAMI_TYPE_LOGICAL8";
  dt_array_str[19]="PAMI_TYPE_SINGLE_COMPLEX";
  dt_array_str[20]="PAMI_TYPE_DOUBLE_COMPLEX";
  dt_array_str[21]="PAMI_TYPE_LOC_2INT";
  dt_array_str[22]="PAMI_TYPE_LOC_2FLOAT";
  dt_array_str[23]="PAMI_TYPE_LOC_2DOUBLE";
  dt_array_str[24]="PAMI_TYPE_LOC_SHORT_INT";
  dt_array_str[25]="PAMI_TYPE_LOC_FLOAT_INT";
  dt_array_str[26]="PAMI_TYPE_LOC_DOUBLE_INT";
  dt_array_str[27]="PAMI_TYPE_LOC_LONG_INT";
  dt_array_str[28]="PAMI_TYPE_LOC_LONGDOUBLE_INT";
}

int pami_init(pami_client_t * client,
              pami_context_t * context,
              char * clientname,
              size_t * num_contexts,
              pami_configuration_t * configuration,
              size_t num_config,
              pami_task_t * task_id,
              size_t * num_tasks)
{
  pami_result_t result = PAMI_ERROR;
  char cl_string[] = "TEST";
  pami_configuration_t l_configuration;
  size_t max_contexts;



  if(clientname == NULL)
    clientname = cl_string;


  result = PAMI_Client_create (clientname, client, NULL, 0);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to initialize pami client %s: result = %d\n",
                 clientname,result);
        return 1;
      }
  init_tables();



  l_configuration.name = PAMI_CLIENT_NUM_CONTEXTS;
  result = PAMI_Client_query(*client, &l_configuration,1);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to query configuration.name=(%d): result = %d\n",
                 l_configuration.name, result);
        return 1;
      }
  max_contexts = l_configuration.value.intval;
  *num_tasks = (*num_tasks<max_contexts)?*num_tasks:max_contexts;

  l_configuration.name = PAMI_CLIENT_TASK_ID;
  result = PAMI_Client_query(*client, &l_configuration,1);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to query configuration.name=(%d): result = %d\n",
                 l_configuration.name, result);
        return 1;
      }
  *task_id = l_configuration.value.intval;

  l_configuration.name = PAMI_CLIENT_NUM_TASKS;
  result = PAMI_Client_query(*client, &l_configuration,1);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to query configuration.name=(%d): result = %d\n",
                 l_configuration.name, result);
        return 1;
      }
  *num_tasks = l_configuration.value.intval;



  result = PAMI_Context_createv(*client, configuration, num_config, context, *num_contexts);
  if (result != PAMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to create pami context: result = %d\n",
                 result);
        return 1;
      }


  return 0;
}

int pami_shutdown(pami_client_t * client,
                  pami_context_t * context,
                  size_t * num_contexts)
{
  pami_result_t result;

  result = PAMI_Context_destroyv(context, *num_contexts);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
      return 1;
    }

  result = PAMI_Client_destroy(client);
  if (result != PAMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
      return 1;
    }

  return 0;
}

unsigned gNumRoots = -1;
unsigned gFull_test = 0;
unsigned gMax_datatype_sz= 32;
unsigned gMax_byte_count = 65536;
unsigned gMin_byte_count = 1;
unsigned gBuffer_offset = 0;
unsigned gNiterlat = 100;
size_t gNum_contexts = 1;
size_t** gValidTable = NULL;
unsigned gSelector = 1;
char* gSelected ;
unsigned gParentless = 1;
int gOptimize = -1;

void setup_op_dt(size_t ** validTable,char* sDt, char* sOp);

void setup_env()
{



  char* sVerbose = getenv("TEST_VERBOSE");

  if(sVerbose) gVerbose=atoi(sVerbose);



  gSelected = getenv("TEST_PROTOCOL");

  if (!gSelected) gSelected = (char*)"";
  else if (gSelected[0] == '-')
  {
    gSelector = 0 ;
    ++gSelected;
  }


  char* sDt = getenv("TEST_DT");


  char* sOp = getenv("TEST_OP");


  if (sDt || sOp) gFull_test = 0;
  if ((sDt && !strcmp(sDt, "ALL")) || (sOp && !strcmp(sOp, "ALL"))) gFull_test = 1;
  if ((sDt && !strcmp(sDt, "SHORT")) || (sOp && !strcmp(sOp, "SHORT")))
  {
    sDt = sOp = NULL;
    gFull_test = 0;
  }


  char* sNRoots = getenv("TEST_NUM_ROOTS");


  if (sNRoots) gNumRoots = atoi(sNRoots);


  char* sCount = getenv("TEST_BYTES");


  if (sCount) gMax_byte_count = atoi(sCount);


  char* sCountOnly = getenv("TEST_BYTES_ONLY");


  if (sCountOnly) gMin_byte_count = gMax_byte_count = atoi(sCountOnly);


  char* sOffset = getenv("TEST_OFFSET");


  if (sOffset) gBuffer_offset = atoi(sOffset);


  char* sIter = getenv("TEST_ITER");


  if (sIter) gNiterlat = atoi(sIter);


  char* snum_contexts = getenv("TEST_NUM_CONTEXTS");

  if (snum_contexts) gNum_contexts = atoi(snum_contexts);


  gValidTable = alloc2DContig(op_count, dt_count);
  setup_op_dt(gValidTable,sDt,sOp);




  char* sParentless = getenv("TEST_PARENTLESS");

  if (sParentless) gParentless = atoi(sParentless);





  char* sOptimize = getenv("TEST_OPTIMIZE");

  if (sOptimize) gOptimize = atoi(sOptimize);


}

void setup_op_dt(size_t ** validTable,char* sDt, char* sOp)
{



  int i,j;
  unsigned force = 0;

  init_tables();

  if (gFull_test)
  {
    for (i = 0; i < op_count; i++)
      for (j = 0; j < dt_count; j++)
        validTable[i][j] = 1;

  }
  else if (sDt && sOp)
  {
    force = 1;
    for (i = 0; i < op_count; i++)
      for (j = 0; j < dt_count; j++)
          if (!strcmp(sDt, dt_array_str[j]) &&
              !strcmp(sOp, op_array_str[i]))
            validTable[i][j] = 1;
          else
            validTable[i][j] = 0;
  }
  else if (sOp)
  {
    for (i = 0; i < op_count; i++)
      for (j = 0; j < dt_count; j++)
        if (!strcmp(sOp, op_array_str[i]))
          validTable[i][j] = 1;
        else
          validTable[i][j] = 0;
  }
  else if (sDt)
  {
    for (i = 0; i < op_count; i++)
      for (j = 0; j < dt_count; j++)
        if (!strcmp(sDt, dt_array_str[j]))
          validTable[i][j] = 1;
        else
          validTable[i][j] = 0;
  }
  else
  {
    for (i = 0; i < op_count; i++)
      for (j = 0; j < dt_count; j++)
        validTable[i][j] = 0;

      validTable[OP_SUM][DT_SIGNED_INT] = 1;
      validTable[OP_MAX][DT_SIGNED_INT] = 1;
      validTable[OP_MIN][DT_SIGNED_INT] = 1;
      validTable[OP_SUM][DT_UNSIGNED_INT] = 1;
      validTable[OP_MAX][DT_UNSIGNED_INT] = 1;
      validTable[OP_MIN][DT_UNSIGNED_INT] = 1;
      validTable[OP_SUM][DT_DOUBLE] = 1;
      validTable[OP_MAX][DT_DOUBLE] = 1;
      validTable[OP_MIN][DT_DOUBLE] = 1;

  }
  if(!force)
  {



    for (i = 0, j = DT_SINGLE_COMPLEX; i < OP_COUNT; i++)if(i!=OP_SUM && i!=OP_PROD) validTable[i][j] = 0;
    for (i = 0, j = DT_DOUBLE_COMPLEX; i < OP_COUNT; i++)if(i!=OP_SUM && i!=OP_PROD) validTable[i][j] = 0;



    for (i = 0, j = DT_NULL; i < OP_COUNT; i++) validTable[i][j] = 0;
    for (i = 0, j = DT_BYTE; i < OP_COUNT; i++) validTable[i][j] = 0;
    for (j = 0, i = OP_COPY; j < DT_COUNT; j++) validTable[i][j] = 0;
    for (j = 0, i = OP_NOOP; j < DT_COUNT; j++) validTable[i][j] = 0;



    for (i = 0, j = DT_LOC_2INT ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
    for (i = 0, j = DT_LOC_SHORT_INT ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
    for (i = 0, j = DT_LOC_FLOAT_INT ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
    for (i = 0, j = DT_LOC_DOUBLE_INT; i < OP_MAXLOC; i++)validTable[i][j] = 0;
    for (i = 0, j = DT_LOC_LONG_INT ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
    for (i = 0, j = DT_LOC_LONGDOUBLE_INT; i < OP_MAXLOC; i++)validTable[i][j] = 0;
    for (i = 0, j = DT_LOC_2FLOAT ; i < OP_MAXLOC; i++)validTable[i][j] = 0;
    for (i = 0, j = DT_LOC_2DOUBLE ; i < OP_MAXLOC; i++)validTable[i][j] = 0;



    for (j = 0, i = OP_MAXLOC; j < DT_LOC_2INT; j++) validTable[i][j] = 0;
    for (j = 0, i = OP_MINLOC; j < DT_LOC_2INT; j++) validTable[i][j] = 0;




    for (i = 0, j = DT_LOGICAL1; i < OP_LAND ; i++) validTable[i][j] = 0;
    for (i = OP_BXOR+1, j = DT_LOGICAL1; i < OP_COUNT; i++) validTable[i][j] = 0;
    for (i = 0, j = DT_LOGICAL2; i < OP_LAND ; i++) validTable[i][j] = 0;
    for (i = OP_BXOR+1, j = DT_LOGICAL2; i < OP_COUNT; i++) validTable[i][j] = 0;
    for (i = 0, j = DT_LOGICAL4; i < OP_LAND ; i++) validTable[i][j] = 0;
    for (i = OP_BXOR+1, j = DT_LOGICAL4; i < OP_COUNT; i++) validTable[i][j] = 0;
    for (i = 0, j = DT_LOGICAL8; i < OP_LAND ; i++) validTable[i][j] = 0;
    for (i = OP_BXOR+1, j = DT_LOGICAL8; i < OP_COUNT; i++) validTable[i][j] = 0;




    for (i = OP_PROD+1, j = DT_LONG_DOUBLE; i < OP_COUNT; i++) validTable[i][j] = 0;
  }
}

void get_split_method(size_t *num_tasks,
                      pami_task_t task_id,
                      int *rangecount,
                      pami_geometry_range_t *range,
                      pami_task_t *local_task_id,
                      size_t set[2],
                      int *id,
                      pami_task_t *root,
                      int non_root[2])
{
  size_t half = *num_tasks / 2;
  char *method = getenv("TEST_SPLIT_METHOD");


  if ((!method || !strcmp(method, "0")))
  {
    if (task_id < half)
    {
      range[0].lo = 0;
      range[0].hi = half - 1;
      set[0] = 1;
      set[1] = 0;
      *id = 1;
      *root = 0;
      *num_tasks = half;
      *local_task_id = task_id;
      non_root[0] = *root +1;
      non_root[1] = half-1;
    }
    else
    {
      range[0].lo = half;
      range[0].hi = *num_tasks - 1;
      set[0] = 0;
      set[1] = 1;
      *id = 2;
      *root = half;
      *local_task_id = task_id - *root;
      non_root[0] = *root +1;
      non_root[1] = *num_tasks-1;
      *num_tasks = *num_tasks - half;
    }

    *rangecount = 1;
  }

  else if ((method && !strcmp(method, "-1")))
  {
    unsigned i = 0;
    int iter = 0;;

    if ((task_id % 2) == 0)
    {
      for (i = 0; i < *num_tasks; i++)
      {
        if ((i % 2) == 0)
        {
          range[iter].lo = i;
          range[iter].hi = i;
          iter++;
        }
     }

      set[0] = 1;
      set[1] = 0;
      *id = 1;
      *root = 0;
      *rangecount = iter;
      non_root[0] = range[1].lo;
      non_root[1] = range[iter-1].lo;
    }
    else
    {
      for (i = 0; i < *num_tasks; i++)
      {
        if ((i % 2) != 0)
        {
          range[iter].lo = i;
          range[iter].hi = i;
          iter++;
        }
      }

      set[0] = 0;
      set[1] = 1;
      *id = 2;
      *root = 1;
      *rangecount = iter;
      non_root[0] = range[1].lo;
      non_root[1] = range[iter-1].lo;
    }

    *num_tasks = iter;
    *local_task_id = task_id/2;
  }

  else if ((!method || !strcmp(method, "-2")))
  {
    int iter = 0;;
    if (task_id < half)
    {
      signed i = 0;
      for (i = half - 1; i >=0; i--)
      {
        range[iter].lo = i;
        range[iter].hi = i;
        if(task_id == (unsigned)i)
          *local_task_id = iter;
        iter++;
      }

      set[0] = 1;
      set[1] = 0;
      *id = 1;
      *root = half-1;
      *rangecount = iter;
      non_root[0] = range[1].lo;
      non_root[1] = range[iter-1].lo;
    }
    else
    {
      unsigned i = 0;
      for (i = *num_tasks - 1; i >=half; i--)
      {
        range[iter].lo = i;
        range[iter].hi = i;
        if(task_id == i)
          *local_task_id = iter;
        iter++;
      }

      set[0] = 0;
      set[1] = 1;
      *id = 2;
      *root = *num_tasks - 1;
      *rangecount = iter;
      non_root[0] = range[1].lo;
      non_root[1] = range[iter-1].lo;
    }
    *num_tasks = iter;
  }

  else
  {
    half = atoi(method);
    if(*num_tasks <= half)
    {
      fprintf(stderr, "assert(*num_tasks > half)");
      assert(*num_tasks > half);
    }
    if (task_id < half)
    {
      range[0].lo = 0;
      range[0].hi = half - 1;
      set[0] = 1;
      set[1] = 0;
      *id = 1;
      *root = 0;
      *num_tasks = half;
      *local_task_id = task_id;
      non_root[0] = *root +1;
      non_root[1] = half-1;
    }
    else
    {
      range[0].lo = half;
      range[0].hi = *num_tasks - 1;
      set[0] = 0;
      set[1] = 1;
      *id = 2;
      *root = half;
      *local_task_id = task_id - *root;
      non_root[0] = *root +1;
      non_root[1] = *num_tasks-1;
      *num_tasks = *num_tasks - half;
    }

    *rangecount = 1;
  }



}
void get_next_root(size_t num_tasks,
                   pami_task_t *root)
{
  size_t half = num_tasks / 2;
  char *method = getenv("TEST_SPLIT_METHOD");


  if ((!method || !strcmp(method, "0")))
  {
    if (*root < half)
    {
      *root = *root + 1;
      if(*root >= half) *root = 0;
    }
    else
    {
      *root = *root + 1;
      if(*root >= num_tasks) *root = half;
    }
  }

  else if ((method && !strcmp(method, "-1")))
  {
    if ((*root % 2) == 0)
    {
      *root = *root + 2;
      if(*root >= num_tasks) *root = 0;
    }
    else
    {
      *root = *root + 2;
      if(*root >= num_tasks) *root = 1;
    }
  }

  else
  {
    half = atoi(method);
    if (*root < half)
    {
      *root = *root + 1;
      if(*root >= half) *root = 0;
    }
    else
    {
      *root = *root + 1;
      if(*root >= num_tasks) *root = half;
    }
  }



}

unsigned validate (const void * addr, size_t bytes)
{
  unsigned status = 1;
  uint8_t * byte = (uint8_t *) addr;
  uint8_t i;

  for (i = 0; i < bytes; i++)
    {
      if (byte[i] != i)
        {
          fprintf (stderr, "validate(%p,%zu) .. ERROR .. byte[%d] != %d (value is %d)\n",
                   addr, bytes, i, i, byte[i]);
          status = 0;
        }
    }

  return status;
}







void test_dispatch (
  pami_context_t context,
  void * cookie,
  const void * header,
  size_t header_size,
  const void * data,
  size_t data_size,
  pami_endpoint_t origin,
  pami_recv_t * recv)
{
  volatile size_t * active = (volatile size_t *) cookie;
  (*active)--;

  if (validate (header, header_size))
    fprintf (stderr, ">>> header validated.\n");
  else
    fprintf (stderr, ">>> header ERROR !!\n");

  if (validate (data, data_size))
    fprintf (stderr, ">>> payload validated.\n");
  else
    fprintf (stderr, ">>> payload ERROR !!\n");

  return;
}



int main (int argc, char ** argv)
{
  pami_client_t client;
  pami_context_t context[1];
  size_t num_contexts = 1;
  pami_task_t task_id;
  size_t num_tasks;
  pami_result_t result;

  { int rc = pami_init (&client, context, NULL, &num_contexts, NULL, 0, &task_id, &num_tasks); if (rc != PAMI_SUCCESS) { printf("pami_init (&client, context, NULL, &num_contexts, NULL, 0, &task_id, &num_tasks)" " rc = %d, line %d\n", rc,
  ); exit(-1); } } ; 

  volatile size_t recv_active[2];
  recv_active[0] = 1;
  recv_active[1] = 1;

  size_t dispatch = 10;
  pami_dispatch_callback_function fn;
  fn.p2p = test_dispatch;
  pami_dispatch_hint_t options = {};

  size_t i;

  for (i = 0; i < num_contexts; i++)
    {
      result = PAMI_Dispatch_set (context[i],
                                  dispatch,
                                  fn,
                                  (void *) & recv_active[i],
                                  options);

      if (result != PAMI_SUCCESS)
        {
          fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
          return 1;
        }
    }



  uint8_t header[1024];
  uint8_t data[1024];

  for (i = 0; i < 1024; i++)
    {
      header[i] = i;
      data[i] = i;
    }


  if (task_id == 0)
    {
      size_t target = 1 % num_contexts;

      pami_send_immediate_t parameters;
      parameters.dispatch = dispatch;
      parameters.header.iov_base = header;
      parameters.header.iov_len = 4;
      parameters.data.iov_base = data;
      parameters.data.iov_len = 32;
      { int rc = PAMI_Endpoint_create (client, 1, target, &parameters.dest); if (rc != PAMI_SUCCESS) { printf("PAMI_Endpoint_create (client, 1, target, &parameters.dest)" " rc = %d, line %d\n", rc, 155); exit(-1); } };

      fprintf (stdout, "PAMI_Send_immediate() functional test [%scrosstalk]\n", (num_contexts == 1) ? "no " : "");
      fprintf (stdout, "\n");

      { int rc = PAMI_Send_immediate (context[0], &parameters); if (rc != PAMI_SUCCESS) { printf("PAMI_Send_immediate (context[0], &parameters)" " rc = %d, line %d\n", rc, 160); exit(-1); } };

      while (recv_active[0] != 0)
        {
          result = PAMI_Context_advance (context[0], 100);

          if ( (result != PAMI_SUCCESS) && (result != PAMI_EAGAIN) )
            {
              fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
              return 1;
            }
        }
    }




  else if (task_id == 1)
    {
      size_t target = 1 % num_contexts;

      while (recv_active[target] != 0)
        {
          result = PAMI_Context_advance (context[target], 100);

          if ( (result != PAMI_SUCCESS) && (result != PAMI_EAGAIN) )
            {
              fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
              return 1;
            }
        }

      pami_send_immediate_t parameters;
      parameters.dispatch = dispatch;
      parameters.header.iov_base = header;
      parameters.header.iov_len = 4;
      parameters.data.iov_base = data;
      parameters.data.iov_len = 32;
      { int rc = PAMI_Endpoint_create (client, 0, 0, &parameters.dest); if (rc != PAMI_SUCCESS) { printf("PAMI_Endpoint_create (client, 0, 0, &parameters.dest)" " rc = %d, line %d\n", rc, 198); exit(-1); } };

      result = PAMI_Send_immediate (context[target], &parameters);

      if (result != PAMI_SUCCESS)
        {
          fprintf (stderr, "Error. Unable to send immediate. result = %d\n", result);
          return 1;
        }

      result = PAMI_Context_advance (context[target], 100);

      if ( (result != PAMI_SUCCESS) && (result != PAMI_EAGAIN) )
        {
          fprintf (stderr, "Error. Unable to advance pami context. result = %d\n", result);
          return 1;
        }
    }



  { int rc = pami_shutdown(&client, context, &num_contexts); if (rc != PAMI_SUCCESS) { printf("pami_shutdown(&client, context, &num_contexts)" " rc = %d, line %d\n", rc, 219); exit(-1); } };

  return 0;
};
