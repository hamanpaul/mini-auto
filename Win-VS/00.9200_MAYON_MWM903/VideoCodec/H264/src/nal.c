/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	nal.c
	
Abstract:

   	Handles the operations on converting String of Data Bits (SODB)
    to Raw Byte Sequence Payload (RBSP), and then 
    onto Encapsulate Byte Sequence Payload (EBSP).

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/09/05	Lsk	Create	

*/




#include "general.h"
#include "nalu.h"
#include "vlc.h"

//Start code and Emulation Prevention need this to be defined in identical manner at encoder and decoder
#define ZEROBYTES_SHORTSTARTCODE 2 //indicates the number of zero bytes in the short start-code prefix

//static u8 NAL_Payload_buffer[LENGTH_OF_SPS_PPS_NALU];

//////////////////////////////////////////////////////////
//
// H264 encoder functions
//
//////////////////////////////////////////////////////////

 /*!
 ************************************************************************
 * \brief
 *    Converts String Of Data Bits (SODB) to Raw Byte Sequence 
 *    Packet (RBSP)
 * \param currStream
 *        Bitstream which contains data bits.
 * \return None
 * \note currStream is byte-aligned at the end of this function
 *    
 ************************************************************************
*/


void SODBtoRBSP(Bitstream *currStream)
{
  currStream->byte_buf <<= 1;
  currStream->byte_buf |= 1;
  currStream->bits_to_go--;
  currStream->byte_buf <<= currStream->bits_to_go;
  currStream->streamBuffer[currStream->byte_pos++] = currStream->byte_buf;
  currStream->bits_to_go = 8;
  currStream->byte_buf = 0;
}


/*!
************************************************************************
*  \brief
*     This function converts a RBSP payload to an EBSP payload
*     
*  \param streamBuffer
*       pointer to data bits
*  \param begin_bytepos
*            The byte position after start-code, after which stuffing to
*            prevent start-code emulation begins.
*  \param end_bytepos
*           Size of streamBuffer in bytes.
*  \param min_num_bytes
*           Minimum number of bytes in payload. Should be 0 for VLC entropy
*           coding mode. Determines number of stuffed words for CABAC mode.
*  \return 
*           Size of streamBuffer after stuffing.
*  \note
*      NAL_Payload_buffer is used as temporary buffer to store data.
*
*
************************************************************************
*/

int RBSPtoEBSP(u8 *streamBuffer, u32 begin_bytepos, u32 end_bytepos)
{

    u32 i, j, count;
    u8  NAL_Payload_buffer[LENGTH_OF_SPS_PPS_NALU];

    memcpy(&NAL_Payload_buffer[begin_bytepos],&streamBuffer[begin_bytepos], (end_bytepos - begin_bytepos) * sizeof(unsigned char));

    count = 0;
    j = begin_bytepos;
    for(i = begin_bytepos; i < end_bytepos; i++) 
    {
        if(count == ZEROBYTES_SHORTSTARTCODE && !(NAL_Payload_buffer[i] & 0xFC)) 
        {
          streamBuffer[j] = 0x03;
          j++;
          count = 0;   
        }
        streamBuffer[j] = NAL_Payload_buffer[i];
        if(NAL_Payload_buffer[i] == 0x00)      
          count++;
        else 
          count = 0;
        j++;
    }    
    return j;
}

//////////////////////////////////////////////////////////
//
// H264 decoder functions
//
//////////////////////////////////////////////////////////

 /*!
 ************************************************************************
 * \brief
 *    Converts RBSP to string of data bits
 * \param streamBuffer
 *          pointer to buffer containing data
 *  \param last_byte_pos
 *          position of the last byte containing data.
 * \return last_byte_pos
 *          position of the last byte pos. If the last-byte was entirely a stuffing byte,
 *          it is removed, and the last_byte_pos is updated.
 *  
************************************************************************/
#if 0
int RBSPtoSODB(byte *streamBuffer, int last_byte_pos)
{
  int ctr_bit, bitoffset;
  
  bitoffset = 0; 
  //find trailing 1
  ctr_bit = (streamBuffer[last_byte_pos-1] & (0x01<<bitoffset));   // set up control bit
  
  while (ctr_bit==0)
  {                 // find trailing 1 bit
    bitoffset++;
    if(bitoffset == 8) 
    {
      if(last_byte_pos == 0)
        printf(" Panic: All zero data sequence in RBSP \n");
      assert(last_byte_pos != 0);
      last_byte_pos -= 1;
      bitoffset = 0;
    }
    ctr_bit= streamBuffer[last_byte_pos-1] & (0x01<<(bitoffset));
  }
  
  
  // We keep the stop bit for now
/*  if (remove_stop)
  {
    streamBuffer[last_byte_pos-1] -= (0x01<<(bitoffset));
    if(bitoffset == 7)
      return(last_byte_pos-1);
    else
      return(last_byte_pos);
  }
*/
  return(last_byte_pos);
  
}


/*!
************************************************************************
* \brief
*    Converts Encapsulated Byte Sequence Packets to RBSP
* \param streamBuffer
*    pointer to data stream
* \param end_bytepos
*    size of data stream
* \param begin_bytepos
*    Position after beginning 
************************************************************************/


int EBSPtoRBSP(byte *streamBuffer, int end_bytepos, int begin_bytepos)
{
  int i, j, count;
  count = 0;
  
  if(end_bytepos < begin_bytepos)
    return end_bytepos;
  
  j = begin_bytepos;
  
  for(i = begin_bytepos; i < end_bytepos; i++) 
  { //starting from begin_bytepos to avoid header information
    //Lsk : skip 0x000003
    if(count == ZEROBYTES_SHORTSTARTCODE && streamBuffer[i] == 0x03) 
    {
      i++;
      count = 0;
    }
    streamBuffer[j] = streamBuffer[i];
    if(streamBuffer[i] == 0x00)
      count++;
    else
      count = 0;
    j++;
  }
  
  return j;
}
#endif
