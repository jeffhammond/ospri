/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/* ================================================================ */
/*                                                                  */
/* Licensed Materials - Property of IBM                             */
/*                                                                  */
/* Blue Gene/Q                                                      */
/*                                                                  */
/* (C) Copyright IBM Corp.  2010, 2012                              */
/*                                                                  */
/* US Government Users Restricted Rights -                          */
/* Use, duplication or disclosure restricted                        */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/* This software is available to you under the                      */
/* Eclipse Public License (EPL).                                    */
/*                                                                  */
/* ================================================================ */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

//
// torus_test_remoteget_atomic
//
// RemoteGet an atomic, performing a fetch and inc on it.
//
// 1. Programs a Descriptor to send one remote-get message to node B
//    - The remote get contains a DirectPut descriptor
// 2. The MU receives the remote get packet and injects its payload
//    (the DirectPut descriptor).
// 3. The DirectPut descriptor points at a payload of 8 bytes.  The
//    pointer is an atomic address with a fetch-and-inc opcode.
// 4. The MU sends the value in the 8 bytes back to us, and then
//    increments the value on node B.
// 5. Steps 1-4 are repeated, and the value received is checked to
//    ensure it is incrementing each time.
// 

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

//#include "callthru_config.h"

#ifdef __FWEXT__
#define TRACE(x)
#include <firmware/include/fwext/fwext.h>
#include <firmware/include/Firmware.h>
#include <firmware/include/fwext/fwext_lib.h>
#include <firmware/include/fwext/fwext_nd.h>
#include <firmware/include/fwext/fwext_mu.h>
#else
#define TRACE(x) printf x;
#endif

#include <spi/include/mu/Descriptor.h>
#include <spi/include/mu/Descriptor_inlines.h>
#include <spi/include/mu/InjFifo.h>
#include <spi/include/mu/Addressing.h>
#include <spi/include/mu/Addressing_inlines.h>
#include <spi/include/kernel/MU.h>

#include "torus_test_inlines.h"


//#define PRINT_DEBUG_MESSAGES 1

#define TORUS_MAX_PAYLOAD_SIZE    512

#define PACKET_HEADER_SIZE     32

// Let send a message that contains exactly 64 bytes (Descriptor size)
#define MESSAGE_SIZE_REMOTE_GET (64)

// Message lenght (Direct Put)
#define MESSAGE_SIZE_DIRECT_PUT (8)

// Arbitrary value to be the initial value of the counter that is being
// remotely incremented.
#define ATOMIC_COUNTER_INITIAL_VALUE (12345678)
#define RECEIVE_BUFFER_INITIAL_VALUE (5)

#define NUM_ITERATIONS 100
int num_iterations = NUM_ITERATIONS;

//
// Note: Torus wrap in mambo
//       A+ ---> A-
//       B+ ---> B-
//          ...   


uint64 packet_payload_size_in_bytes;
uint64 packet_size_in_bytes;
uint64 packet_size_in_chunks;


#define INJ_MEMORY_FIFO_SIZE              0x0000007FFFULL  // 32K bytes

// ///////////////  MemoryFIFO ///////////////////

#define REC_MEMORY_FIFO_SIZE_1          0x00000FFFFULL  // 64K bytes

#define REC_MEMORY_FIFO_SIZE_2          0x00000FFFFULL  // 64K bytes

#define REC_FIFO_ID_1                   1
#define REC_FIFO_ID_2                   2

// ////////////////////////////////////////////////

// Descriptor (RemoteGet)
__attribute((aligned(64))) MUHWI_Descriptor_t mu_iRemoteGetDescriptor;

// Descriptor (DirectPut)
MUHWI_Descriptor_t mu_iDirectPutDescriptor;

////////////////////////////////////////////////////////////////////////

/**
 * \brief Malloc with memory alignment
 *
 * \param[out]  memory  Pointer to the output pointer of the memory that was malloc'd.
 *                      This should be passed to free() later.
 * \param[out]  buffer  Pointer to the output pointer of the aligned buffer within
 *                      the malloc'd memory.
 * \param[in]   alignment  The requested alignment.
 * \param[in]   size       The requested number of bytes in the buffer.
 *
 * \retval  0  Success
 * \retval  -1 Error
 */
int malloc_memalign ( void **memory,
		      void **buffer,
		      uint64_t alignment,
		      uint32_t size )
{
  // Malloc the memory to be the size of the buffer plus space for alignment.
  void *myMemory = malloc ( size + alignment );
  if ( !myMemory ) return -1;

  *memory = myMemory;
  *buffer = (void*)( ((uint64_t)myMemory + alignment)  & ~(alignment-1) );

  return 0;
}


#ifdef __FWEXT__

int is_mambo = 0; // Indicates whether mambo is being used when in firmware extension mode.

int test_main ( void ) {
  
  if (PhysicalThreadIndex() > 0)       // run  a single core test.
    test_exit(0);

  int rc=0;
  printf("Torus Remote Get Atomic Test\n");

  // Perform initialization of the network and mu
  Personality_t *pers;

  pers = fwext_getPersonality();
  uint64_t p1 = pers->Kernel_Config.NodeConfig & PERS_ENABLE_Mambo;
  if (p1) is_mambo = 1;

  // ND and MU init is done in firmware, but we disable it in svchost and 
  // call it directly here because it performs much better.
  // #if 0
  fw_nd_set_verbose(0);  // if 1, prints all dcr commands, don't use on cycle sim
                         // on cycle sim, can have DcrMonitory trace DCR commands

  rc = fw_nd_reset_release(pers);
  
  if(rc)
    {
      TRACE(("fw_nd_reset_release failed with rc=%d\n",rc));
      test_exit (rc);
    }
    
  fw_mu_set_verbose(0);  // if 1, prints all dcr commands, don't use on cycle sim
                         // on cycle sim, can have DcrMonitory trace DCR commands

  rc = fw_mu_reset_release(pers);
  
  if(rc)
    {
      printf("fw_mu_reset_release failed with rc=%d\n",rc);
      test_exit (rc);
    }
  // #endif // if 0  

  uint64_t max_value = ~0; 
 
  fw_mu_set_sys_range(0, /* range_id */
		      0, /* min_value */
		      max_value);

  fw_mu_set_usr_range(0, /* range_id */
		      0, /* min_value */
		      max_value);  
  
/*   fw_mu_set_imfifo_rget   (1, 1); */
/*   fw_mu_set_imfifo_system (1, 0); */

  TRACE(("Network and MU Initialization is complete\n"));

#else
int main(int argc, char **argv)
{
    int rc;
#endif

    uint i = 0;
    // Destination for Remote Get packet
    MUHWI_Destination_t dest;
    MUSPI_SetUpDestination ( &dest, 0, 0, 0, 0, 0 );

    MUSPI_InjFifoSubGroup_t   fifo_subgroup;  

    uint64 message_size_in_bytes_remote_get = MESSAGE_SIZE_REMOTE_GET;
    uint64 message_size_in_bytes_direct_put = MESSAGE_SIZE_DIRECT_PUT;

    TRACE(("main(): Injection Memory FIFO (0,0,0), Send Remote Get Message with Atomic Increment\n"));


//#ifdef PRINT_DEBUG_MESSAGES
	printf("Start!\n");
//#endif

    // ------------------------------------------------------
	// allocates area for message_sent_remote_get[] buffer (RemoteGet)
    // ------------------------------------------------------
    uint64 *message_sent_remote_get = (uint64 *)malloc(message_size_in_bytes_remote_get);
    uint64 *message_sent_direct_put = (uint64 *)malloc(message_size_in_bytes_direct_put);

    TRACE(("message_sent_remote_get (address) = %p\n", message_sent_remote_get));
    TRACE(("message_size_in_bytes_remote_get = %lld\n", message_size_in_bytes_remote_get));

    TRACE(("message_sent_direct_put (address) = %p\n", message_sent_direct_put));
    TRACE(("message_size_in_bytes_direct_put = %lld\n", message_size_in_bytes_direct_put));

    // Initializes the message_sent_remote_get[] buffer
    for (i=0;i<message_size_in_bytes_remote_get/8;i++) 
        message_sent_remote_get[i] = 0x00ull;  // 8-bytes
    
    Kernel_MemoryRegion_t  mregionSentRemoteGet;
    rc = Kernel_CreateMemoryRegion ( &mregionSentRemoteGet,
				     message_sent_remote_get,
				     message_size_in_bytes_remote_get );

    if ( rc != 0)
    {
      printf("Kernel_CreateMemoryRegion failed for message_sent_remote_get with rc=%d\n",rc);
#ifdef __FWEXT__      
      test_exit(1);
#else
      exit(1);
#endif
    }


    // Initializes the message_sent[] buffer
    *message_sent_direct_put = (uint64)ATOMIC_COUNTER_INITIAL_VALUE;  // 8-bytes
    uint64_t expected_counter_value = ATOMIC_COUNTER_INITIAL_VALUE + RECEIVE_BUFFER_INITIAL_VALUE;
    
    Kernel_MemoryRegion_t  mregionSentDirectPut;
    rc = Kernel_CreateMemoryRegion ( &mregionSentDirectPut,
				     message_sent_direct_put,
				     message_size_in_bytes_direct_put );

    if ( rc != 0)
    {
      printf("Kernel_CreateMemoryRegion failed for message_sent_direct_put with rc=%d\n",rc);
#ifdef __FWEXT__
      test_exit(1);
#else
      exit(1);
#endif
    }

    // Get an atomic address for the message_sent buffer.

    uint64_t message_sent_atomic_address = MUSPI_GetAtomicAddress ( 
				      (uint64_t)message_sent_direct_put - 
				      (uint64_t)mregionSentDirectPut.BaseVa + 
				      (uint64_t)mregionSentDirectPut.BasePa,
				      MUHWI_ATOMIC_OPCODE_LOAD_INCREMENT );

    TRACE(("message_sent_direct_put (atomic address) = 0x%llx\n", 
	   (long long unsigned int)message_sent_atomic_address));

    /////////////////////////////////////////////////
      
    typedef struct recvArea
    {
      volatile uint64 counter;
      unsigned char   recvBuffer[MESSAGE_SIZE_DIRECT_PUT];
    } recvArea_t;

    // Allocate space for the reception counter and the receive buffer
    
    recvArea_t *recvAreaPtr = (recvArea_t*)malloc ( sizeof(recvArea_t) );
    if ( !recvAreaPtr )
    {
      printf("Allocating recvArea failed\n");
#ifdef __FWEXT__
      test_exit(1);
#else
      exit(1);
#endif
    }

    volatile uint64 *counterAddress    = (volatile uint64*)&(recvAreaPtr->counter);
    unsigned char   *recvBufferAddress = (unsigned char  *)&(recvAreaPtr->recvBuffer[0]);

    *((uint64*)recvBufferAddress) = RECEIVE_BUFFER_INITIAL_VALUE;

    // Get a memory region for the recvArea.
    
    Kernel_MemoryRegion_t  recvAreaMemRegion;
    rc = Kernel_CreateMemoryRegion ( &recvAreaMemRegion,
				     recvAreaPtr,
				     sizeof(recvArea_t) );

    if ( rc != 0)
    {
      printf("Kernel_CreateMemoryRegion failed for recvAreaMemRegion with rc=%d\n",rc);
#ifdef __FWEXT__
      test_exit(1);
#else
      exit(1);
#endif
    }

    // Calculate the offsets of the counter and receive buffer from the base address.

    uint64_t recvAreaBasePA   = (uint64_t)recvAreaMemRegion.BasePa;
    uint64_t counterOffset    = (uint64_t)counterAddress - (uint64_t)recvAreaMemRegion.BaseVa;
    uint64_t recvBufferOffset = (uint64_t)recvBufferAddress - (uint64_t)recvAreaMemRegion.BaseVa;

    TRACE(("counterAddress=%p, recvBufferAddress=%p, recvAreaBasePA=0x%llx, counterOffset=0x%llx, recvBufferOffset=0x%llx\n",counterAddress, recvBufferAddress, 
	   (long long unsigned int)recvAreaBasePA,
	   (long long unsigned int)counterOffset,
	   (long long unsigned int)recvBufferOffset));

    //////////////////////////////////////////////////////////////
    // Initialize base address table and atomic counter info
    //////////////////////////////////////////////////////////////
    /* Set up the base address table */
    uint32_t batids[1] = {0};
    MUSPI_BaseAddressTableSubGroup_t bat;
    rc = Kernel_AllocateBaseAddressTable ( 0,
					   &bat,
					   1,
					   batids,
					   0 /* "User" use */ );
    if (rc != 0)
      {
	printf("Kernel_AllocateBaseAddressTable failed with rc=%d\n",rc);
#ifdef __FWEXT__
	test_exit(1);
#else
	exit(1);
#endif
      }
    
    rc = MUSPI_SetBaseAddress ( &bat,
				0,
				(uint64_t)recvAreaMemRegion.BasePa );
    if (rc != 0)
      {
	printf("MUSPI_SetBaseAddress failed with rc=%d\n",rc);
#ifdef __FWEXT__
	test_exit(1);
#else
	exit(1);
#endif
      }

    TRACE(("Set BaseAddressTable entry slot 0 to 0x%llx\n",
	   (long long unsigned int)recvAreaMemRegion.BasePa));

    uint64_t muAtomicCounterOffset = MUSPI_GetAtomicOffsetFromBaseAddress ( 
						   &bat, 
						   0, 
						   recvAreaBasePA + counterOffset,
						   MUHWI_ATOMIC_OPCODE_STORE_ADD );

    uint64_t muAtomicRecvBufferOffset = MUSPI_GetAtomicOffsetFromBaseAddress ( 
						   &bat, 
						   0, 
						   recvAreaBasePA + recvBufferOffset,
						   MUHWI_ATOMIC_OPCODE_STORE_ADD );

    TRACE(("main(): recvCounterVa=%p, recvAreaBasePA=0x%llx, muAtomicCounterOffset=0x%llx, muAtomicRecvBufferOffset=0x%llx\n",
	   &(recvAreaPtr->counter), 
	   (long long unsigned int)recvAreaBasePA, 
	   (long long unsigned int)muAtomicCounterOffset,
	   (long long unsigned int)muAtomicRecvBufferOffset));
    
    //////////////////////////////////////////////////////////////
    // Create a DirectPut Descriptor and copy it into the
    // message payload
    //////////////////////////////////////////////////////////////
    TRACE(("main(): Configures direct put descriptor\n"));

    MUSPI_Pt2PtDirectPutDescriptorInfo_t mu_iDirectPutDescriptorInfo;
    mu_iDirectPutDescriptorInfo.Base.Pre_Fetch_Only  = MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO;
    mu_iDirectPutDescriptorInfo.Base.Payload_Address = message_sent_atomic_address;
    mu_iDirectPutDescriptorInfo.Base.Message_Length  = message_size_in_bytes_direct_put;
    mu_iDirectPutDescriptorInfo.Base.Torus_FIFO_Map  = MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP;
    mu_iDirectPutDescriptorInfo.Base.Dest            = dest;
    mu_iDirectPutDescriptorInfo.Pt2Pt.Hints_ABCD = MUHWI_PACKET_HINT_AP;
    mu_iDirectPutDescriptorInfo.Pt2Pt.Misc1      = MUHWI_PACKET_HINT_E_NONE |
                                                   MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE |
                                                   MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
                                                   MUHWI_PACKET_DO_NOT_DEPOSIT;
    mu_iDirectPutDescriptorInfo.Pt2Pt.Misc2      = MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC;
    mu_iDirectPutDescriptorInfo.Pt2Pt.Skip       = 0;

    mu_iDirectPutDescriptorInfo.DirectPut.Rec_Payload_Base_Address_Id = 0;
    mu_iDirectPutDescriptorInfo.DirectPut.Rec_Payload_Offset          = muAtomicRecvBufferOffset;
    mu_iDirectPutDescriptorInfo.DirectPut.Rec_Counter_Base_Address_Id = 0;
    mu_iDirectPutDescriptorInfo.DirectPut.Rec_Counter_Offset          = muAtomicCounterOffset;
    mu_iDirectPutDescriptorInfo.DirectPut.Pacing                      = MUHWI_PACKET_DIRECT_PUT_IS_NOT_PACED;

    rc = MUSPI_CreatePt2PtDirectPutDescriptor( &mu_iDirectPutDescriptor,
					       &mu_iDirectPutDescriptorInfo
					     );
    if (rc != 0)
      {
        printf("MUSPI_CreatePt2PtDirectPutDescriptor failed with rc=%d\n",rc);
#ifdef __FWEXT__
	test_exit(1);
#else
	exit(1);
#endif
      }

    //MUSPI_DescriptorDumpHex("Direct Put Descriptor",
    //		    &mu_iDirectPutDescriptor);

    // Copy Descriptor into RemoteGet message payload

    memcpy((char *)((void *)message_sent_remote_get), (char *)((void *)(&mu_iDirectPutDescriptor)), message_size_in_bytes_remote_get);


    /////////////////////////////////////////////////////////////
    // RemoteGet message
    // Create a remote get descriptor
    /////////////////////////////////////////////////////////////
    TRACE(("main(): Configures remote get descriptor\n"));

    MUSPI_Pt2PtRemoteGetDescriptorInfo_t mu_iRemoteGetDescriptorInfo;
    mu_iRemoteGetDescriptorInfo.Base.Pre_Fetch_Only  = MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO;
    mu_iRemoteGetDescriptorInfo.Base.Payload_Address = (uint64_t)message_sent_remote_get - (uint64_t)mregionSentRemoteGet.BaseVa + (uint64_t)mregionSentRemoteGet.BasePa;
    mu_iRemoteGetDescriptorInfo.Base.Message_Length  = message_size_in_bytes_remote_get;
    mu_iRemoteGetDescriptorInfo.Base.Torus_FIFO_Map  = MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AP;
    mu_iRemoteGetDescriptorInfo.Base.Dest            = dest;
    mu_iRemoteGetDescriptorInfo.Pt2Pt.Hints_ABCD = MUHWI_PACKET_HINT_AP;
    mu_iRemoteGetDescriptorInfo.Pt2Pt.Misc1      = MUHWI_PACKET_HINT_E_NONE |
                                                   MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE |
                                                   MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
                                                   MUHWI_PACKET_DO_NOT_DEPOSIT;
    mu_iRemoteGetDescriptorInfo.Pt2Pt.Misc2      = MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC;
    mu_iRemoteGetDescriptorInfo.Pt2Pt.Skip       = 0;
    mu_iRemoteGetDescriptorInfo.RemoteGet.Type             = MUHWI_PACKET_TYPE_GET;
    mu_iRemoteGetDescriptorInfo.RemoteGet.Rget_Inj_FIFO_Id = 1; // Fifo 1 is for remote get use

    // Prepares Injection Memory FIFO Descriptor (RemoteGet)
    rc = MUSPI_CreatePt2PtRemoteGetDescriptor( &mu_iRemoteGetDescriptor,
					       &mu_iRemoteGetDescriptorInfo
					     );
    if (rc != 0)
      {
        printf("MUSPI_CreatePt2PtRemoteGetDescriptor failed with rc=%d\n",rc);
#ifdef __FWEXT__
	test_exit(1);
#else
	exit(1);
#endif
      }

    //    MUSPI_DescriptorDumpHex("Remote Get Descriptor",
    //		    &mu_iRemoteGetDescriptor);

    /////////////////////////////////////////////////////////////////
    // Configures Injection Memory FIFO Registers
    // - fifo 0 that the core injects descriptors into
    // - fifo 1 that the MU injects remote get payload into
    /////////////////////////////////////////////////////////////////
    TRACE(("main(): Configures Injection Memory FIFO Registers\n"));

    void *injMemoryFifoPtr, *memoryForInjMemoryFifoPtr;
    rc = malloc_memalign ( &memoryForInjMemoryFifoPtr,
			   &injMemoryFifoPtr, 
			   64, 
			   INJ_MEMORY_FIFO_SIZE+1 );
    if (rc)
    {
      printf("inj_memory_fifo malloc failed with rc=%d\n",rc);
#ifdef __FWEXT__      
      test_exit(1);
#else
      exit(1);
#endif
    }

    void *rgetMemoryFifoPtr, *memoryForRgetMemoryFifoPtr;
    rc = malloc_memalign ( &memoryForRgetMemoryFifoPtr,
			   &rgetMemoryFifoPtr, 
			   64, 
			   INJ_MEMORY_FIFO_SIZE+1 );
    if (rc)
    {
      printf("rget_memory_fifo malloc failed with rc=%d\n",rc);
#ifdef __FWEXT__      
      test_exit(1);
#else
      exit(1);
#endif
    }

    uint32_t fifoid[2] = { 0, 1 };

    Kernel_InjFifoAttributes_t injFifoAttrs[2];
    injFifoAttrs[0].RemoteGet = 0;
    injFifoAttrs[0].System    = 0;
    injFifoAttrs[1].RemoteGet = 1;
    injFifoAttrs[1].System    = 0;
    
    rc = Kernel_AllocateInjFifos (0, &fifo_subgroup, 2, 
				  fifoid, injFifoAttrs);
    if ( rc != 0)
    {
      printf("Kernel_AllocateInjFifos failed with rc=%d\n",rc);
#ifdef __FWEXT__
      test_exit(1);
#else
      exit(1);
#endif
    }
    
    Kernel_MemoryRegion_t  mregionInj;
    rc = Kernel_CreateMemoryRegion ( &mregionInj,
				     injMemoryFifoPtr,
				     INJ_MEMORY_FIFO_SIZE + 1 );

    if ( rc != 0)
    {
      printf("Kernel_CreateMemoryRegion failed for injMemoryFifoPtr with rc=%d\n",rc);
#ifdef __FWEXT__
      test_exit(1);
#else
      exit(1);
#endif
    }
    
    Kernel_MemoryRegion_t  mregionRget;
    rc = Kernel_CreateMemoryRegion ( &mregionRget,
				     rgetMemoryFifoPtr,
				     INJ_MEMORY_FIFO_SIZE + 1 );

    if ( rc != 0)
    {
      printf("Kernel_CreateMemoryRegion failed for rgetMemoryFifoPtr with rc=%d\n",rc);
#ifdef __FWEXT__
      test_exit(1);
#else
      exit(1);
#endif
    }
    
    rc = Kernel_InjFifoInit (&fifo_subgroup, fifoid[0], &mregionInj,
			     (uint64_t)injMemoryFifoPtr -
			     (uint64_t)mregionInj.BaseVa,
			     INJ_MEMORY_FIFO_SIZE);    
    if (rc != 0)
      {
        printf("Kernel_InjFifoInit Inj failed with rc=%d, errno=%d\n",rc,errno);
#ifdef __FWEXT__
	test_exit(1);
#else
	exit(1);
#endif
      }
    
    rc = Kernel_InjFifoInit (&fifo_subgroup, fifoid[1], &mregionRget,
			     (uint64_t)rgetMemoryFifoPtr -
			     (uint64_t)mregionRget.BaseVa,
			     INJ_MEMORY_FIFO_SIZE);    
    if (rc != 0)
      {
        printf("Kernel_InjFifoInit Rget failed with rc=%d, errno=%d\n",rc,errno);
#ifdef __FWEXT__
	test_exit(1);
#else
	exit(1);
#endif
      }

    rc = Kernel_InjFifoActivate (&fifo_subgroup, 2, fifoid, KERNEL_INJ_FIFO_ACTIVATE);    
    if (rc != 0)
      {
        printf("Kernel_InjFifoActivate Inj failed with rc=%d, errno=%d\n",rc,errno);
#ifdef __FWEXT__
	test_exit(1);
#else
	exit(1);
#endif
      }


    // ---------------------------------------------
    //    Reception Side
    // ---------------------------------------------

    
/*     *data_counter_base_address = REC_PAYLOAD_BASE_ADDRESS; */
/*     printf("data_counter_base_address = %p\n", data_counter_base_address); */

    // Loop, sending the remote get, waiting for the reception counter to hit zero,
    // and verifying the received counter's value.
    // for (i=0; i<num_iterations; i++)  /** disable loop **/
    {
      
      // Let's initialize the Counter for corresponding Counter Id
      // Note: counter is initialized with the message size
      // updates counter with number of bytes sent
      *counterAddress = MESSAGE_SIZE_DIRECT_PUT;
      
      // -----------------------------------------------------------
      // Processor Advances Tail pointer - Descriptor is 64-bytes
      // MU should Inject (RemoteGet) message into the Torus
      // -----------------------------------------------------------
      
      // Let's Inject the (RemoteGet) Descriptor into the Injection Memory FIFO
#if 1
      printf("main(): Inject Descriptor into Injection Memory FIFO\n");
#endif
      
      rc = MUSPI_InjFifoInject (MUSPI_IdToInjFifo(fifoid[0], &fifo_subgroup), 
				(void *)(&mu_iRemoteGetDescriptor) );
      
      if (rc < 0) // Should have injected 1 descriptor
	{
	  printf("MUSPI_InjFifoInject failed with rc=%d\n",rc);
#ifdef __FWEXT__
	  test_exit(1);
#else
	  exit(1);
#endif
	}
      
#ifndef __FWEXT__
      printf("main(): Successful injection of remote get descriptor\n");
#endif
      
      // //////////////////////////////////////////////////
      //      Reception side, check counter value
      // //////////////////////////////////////////////////
      
      uint64 volatile counter_value;
      
      // wait for the counter to reach ZERO
      while (1) {

        counter_value = *counterAddress;
	
        if (counter_value == 0) 
	  {
            //
#if 1
	    printf("counter is now ZERO !!!!\n");
#endif
	    
	    break;
	  }	
      }
      _bgq_msync(); // Ensure data is available to all cores.
 
      
      // Let's print the Received Message contents
      
      //put_offset = (uint64)mu_pktHdrDirectPut.Put_Offset_LSB;
      
#ifndef __FWEXT__
      printf("recvBufferAddress = %p\n", recvBufferAddress);
      printf("---Prints Received Message contents\n");
      
      Print_Message((unsigned char *)recvBufferAddress, message_size_in_bytes_direct_put);
      
      printf("---Where Received Message is being stored: recvBufferAddress = %p\n", recvBufferAddress);
      printf("---Checks Received Message contents(size = %lld)\n", message_size_in_bytes_direct_put);
#endif
      
      uint64_t receivedCounterValue = *((uint64_t*)recvBufferAddress);
      
      if ( receivedCounterValue == expected_counter_value )
	{
	  printf("---Received Counter Value = %llu\n",
		 (long long unsigned int)receivedCounterValue);
	}
      else
	{
	  printf("ERROR: Received Counter Value = %llu, expected %llu\n",
		 (long long unsigned int)receivedCounterValue,
		 (long long unsigned int)expected_counter_value);
#ifdef __FWEXT__
	  test_exit(1);
#else
	  exit(1);
#endif
	}


      if ( *message_sent_direct_put == ATOMIC_COUNTER_INITIAL_VALUE+1 )
	{
	  printf("---Sent Counter Value = %llu\n",
		 (long long unsigned int)*message_sent_direct_put);
	}
      else
	{
	  printf("ERROR: Sent Counter Value = %llu, expected %llu\n",
		 (long long unsigned int)*message_sent_direct_put,
		 (long long unsigned int)(ATOMIC_COUNTER_INITIAL_VALUE+1));
#ifdef __FWEXT__
	  test_exit(1);
#else
	  exit(1);
#endif
	}

    }
    
    //printf("All counter values passed\n");

#ifdef __FWEXT__

    if ( is_mambo == 0 ) // Termination checks don't work in mambo.  ErrInt DCRs are not zero.
      {
	rc = fw_nd_term_check(pers);
	if (rc)
	  {
	    printf("ERROR: fw_nd_term_check failed with rc=%d\n",rc);
	    test_exit(1);
	  }

	rc = fw_mu_term_check(pers);
	if (rc)
	  {
	    printf("ERROR: fw_mu_term_check failed with rc=%d\n",rc);
	    test_exit(1);
	  }
      }

#endif

    printf("Done!\n");

#ifdef  __FWEXT__
    test_exit (0);
#endif   

    return 0;	
}

