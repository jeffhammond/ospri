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
// Inline functions for BGQ Torus and Messaging Unit
//

#include <stdint.h>

#define ASM asm volatile

#define uchar   unsigned char
#define uint16  unsigned short
#define uint    unsigned int
#define uint64  unsigned long long

// prototypes



inline int Check_Message(unsigned char *data_sent, unsigned char *data, uint64 size);
inline void Print_Message(unsigned char *data, uint64 size);


// implementation





/*
 * Function: Check_Message()
 * Description: compares the contents for packet payload sent, with the received message.
 * Input:  unsigned char *data_in
 *         unsigned char *data 
 *         uint64         size
 */
int Check_Message(unsigned char *data_in, unsigned char *data, uint64 size)
{
    int i=0;
    int mismatch_flag=0;

    printf("Check_Message(size = %lld)\n", size);

    /* compare the entire message  */
    for (i=0; i<size; i++)
    {
        //
        if (data_in[i] != data[i]) 
        {
            // 
            printf("Error: mismatch data_in[%d]=0x%02x, data[%d]=0x%02x\n", i, data_in[i], i, data[i]);
            mismatch_flag = 1;
            //break;
        }
    }

    return mismatch_flag;

}  /* endof function: Check_Message()  */





/*
 * Function: Print_Message()
 * Description: Displays a formatted version of a Message.
 * Input:  char *data
 *         int   size
 */
void Print_Message(unsigned char *data, uint64 size)
{
    int i,k=0;
    uint64 numOfBytes=0;
    uint64 numOfLines=0;
    uint64 sizeRounded=0;

    printf("Print_Message(size = %lld)\n", size);

    if ((size % 32) == 0 )
        numOfLines = size / 32;       /* number of lines */
    else
        numOfLines = size / 32 + 1;   /* number of lines */

    /* rounds size to immediate multiple of 32 */
    if ((size % 32) == 0 )
        sizeRounded = size;
    else
        sizeRounded = (numOfLines + 1) * 32;

    /* Prints 32 bytes per line  */
    for (k=0;k<numOfLines;k++)
        {
            /* initializes counter for next line */
            i=0;

            printf("%04llx: ", numOfBytes);

            for (i=0;i<32;i++)
                {
                    printf("%02x ", (data[numOfBytes] & 0xff));
                    numOfBytes++;

                    if (numOfBytes == size)
                        {
                            printf("\n");
                            return;
                        }
                }

            printf("\n");
        }

}  /* endof function: Print_Message()  */
