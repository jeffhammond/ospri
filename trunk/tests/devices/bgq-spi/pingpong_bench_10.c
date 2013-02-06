/* Heechang Na and James Osborn wrote this code */
#define CONTROL
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include <mpi.h>
#include <mpix.h>

#include "spi_simple.h"

typedef uint64_t timebase_t;
#define timebase_get(t) (t) = GetTimeBase()
#define timebase_seconds(t0,t1) ((1./1.6e9)*((double)((t1)-(t0))))


int
main(int argc, char **argv)
{
#ifndef USE_MPI
  uint32_t  fifoID;
  nfifos = 1;
  spi_init();
#else
  MPI_Status status;
#endif
  MPI_Init(&argc,&argv);

  int size, rank;
  size = nranks();
  rank = myrank();
  int coords[NDIM];
  mycoords(coords);

  int srcrank = 0;
  int destrank;
  int tag = 0;

	int i,NN,k,kk;
double j=2.1;

int m_size;
int Nn = 10;
double ave[Nn];

    NN = 4;
    srcrank = 0; destrank = 2;
    timebase_t t0,t1,t2,t3,t4,t5,t6,t01;

  for(NN=1;NN<=1024;NN=NN*2){
  for(destrank=1;destrank<size;destrank++){
  for(kk=0;kk<Nn;kk++){
    barrier();

    if(rank==srcrank) {

      double message[NN];
      double recvbuf[NN];
      for(i=0;i<NN;i++)message[i] = i + 0.1;
      for(i=0;i<NN;i++)recvbuf[i] = 0.0;
      m_size = (int) sizeof(message);
timebase_get(t0);
#ifndef USE_MPI
      fifoID = 0;
      set_send( destrank, message, m_size, fifoID);
      set_recv( destrank, recvbuf ,m_size ,fifoID);
      barrier();
#else
      MPI_Request send_req, recv_req;
      MPI_Send_init(message, m_size, MPI_BYTE, destrank, tag, MPI_COMM_WORLD, &send_req);
      MPI_Recv_init(recvbuf, m_size, MPI_BYTE, destrank, tag, MPI_COMM_WORLD, &recv_req);
      barrier();
#endif
for(k=0;k<4;k++){
for(i=0;i<NN;i++)recvbuf[i] = 0.0;
timebase_get(t1);
#ifndef USE_MPI
      send(fifoID);
      send_wait(fifoID);
#else
      MPI_Start(&send_req);
      MPI_Wait(&send_req, &status);
#endif
timebase_get(t2);
#ifndef USE_MPI
      recv_wait(fifoID,m_size);
#else
      MPI_Start(&recv_req);
      MPI_Wait(&recv_req, &status);
#endif
timebase_get(t3);
}
#ifndef USE_MPI
      send_deact(fifoID);
#else
      MPI_Request_free(&send_req);
#endif

#ifndef USE_MPI
      recv_free(fifoID);
#else
      MPI_Request_free(&recv_req);
#endif
timebase_get(t4);

ave[kk] = 1e6*timebase_seconds(t1,t3);

    } else
      if(rank==destrank) {
        double recvbuf[NN];
        double sendbuf[NN];
        for(i=0;i<NN;i++)recvbuf[i] = 0.0;
        for(i=0;i<NN;i++)sendbuf[i] = destrank + 1.3;
        m_size = (int) sizeof(recvbuf);
        //printf("recv buf %f size %i \n",recvbuf[2],m_size);
timebase_get(t0);
#ifndef USE_MPI
        fifoID = 0;
        set_send( srcrank, sendbuf, m_size, fifoID);
        set_recv(srcrank, recvbuf,m_size ,fifoID);
        barrier();
#else
        MPI_Request send_req, recv_req;
        MPI_Send_init(sendbuf, m_size, MPI_BYTE, srcrank, tag, MPI_COMM_WORLD, &send_req);
        MPI_Recv_init(recvbuf, m_size, MPI_BYTE, srcrank, tag, MPI_COMM_WORLD, &recv_req);
        barrier();
#endif
for(k=0;k<4;k++){
timebase_get(t1);
#ifndef USE_MPI
        recv_wait(fifoID,m_size);
#else
        MPI_Start(&recv_req);
        MPI_Wait(&recv_req, &status);
#endif
timebase_get(t2);
#ifndef USE_MPI
        send(fifoID);
        send_wait(fifoID);
#else
        MPI_Start(&send_req);
        MPI_Wait(&send_req, &status);
#endif
timebase_get(t3);
//printf("Time 13 = %g usecs, k= %i\n", 1e6*timebase_seconds(t1,t3),k);
}
#ifndef USE_MPI
        recv_free(fifoID);
#else
        MPI_Request_free(&recv_req);
#endif

#ifndef USE_MPI
        send_deact(fifoID);
#else
        MPI_Request_free(&send_req);
#endif
timebase_get(t4);

ave[kk] = 1e6*timebase_seconds(t1,t3);

      } else {
        barrier();
      }
  } /* kk: iteration */


/* print out averages */
if(rank==srcrank) {
double Ave=0,sigma=0,error=0;
for(i=0;i<Nn;i++)Ave += ave[i];
Ave = Ave/Nn;
for(i=0;i<Nn;i++){
  sigma += (Ave - ave[i]) * (Ave - ave[i]);
}
error = sqrt(sigma/Nn/(Nn-1));
printf("Time 13 = %g +- %g usecs, dest= %i , size = %i byte \n", Ave/2.0, error/2.0,destrank,m_size);

} else
if(rank==destrank) {
double Ave=0,sigma=0,error=0;
for(i=0;i<Nn;i++)Ave += ave[i];
Ave = Ave/Nn;
for(i=0;i<Nn;i++){
  sigma += (Ave - ave[i]) * (Ave - ave[i]);
}
error = sqrt(sigma/Nn/(Nn-1));
printf("time 13 = %g +- %g usecs, dest= %i , size = %i byte \n", Ave/2.0, error/2.0,destrank,m_size);

}


  } /*destrank */
  } /* NN : data size*/

#ifndef USE_MPI
spi_free();
#endif
MPI_Finalize();
  return 0;
}
