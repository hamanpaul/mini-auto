/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	nalu.c
	
Abstract:

   	Common NALU support functions.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/09/05	Lsk	Create	

*/

#include "general.h"
#include "board.h"
#include "nal.h"
#include "nalu.h"
#include "H264api.h"
#include "vlc.h"


//////////////////////////////////////////////////////////
//
// H264 encoder functions
//
//////////////////////////////////////////////////////////

/*! 
 *************************************************************************************
 * \brief
 *    Converts an RBSP to a NALU
 *
 * \param rbsp
 *    byte buffer with the rbsp
 * \param nalu
 *    nalu structure to be filled
 * \param rbsp_size
 *    size of the rbsp in bytes
 * \param nal_unit_type
 *    as in JVT doc
 * \param nal_reference_idc
 *    as in JVT doc
 * \param min_num_bytes
 *    some incomprehensible CABAC stuff
 * \param UseAnnexbLongStartcode
 *    when 1 and when using AnnexB bytestreams, then use a long startcode prefix
 *
 * \return
 *    length of the NALU in bytes
 *************************************************************************************
 */

int RBSPtoNALU (unsigned char *rbsp, NALU_t *nalu, int rbsp_size, int nal_unit_type, int nal_reference_idc)
{
    int len;

    nalu->forbidden_bit = 0;
    nalu->nal_reference_idc = nal_reference_idc;
    nalu->nal_unit_type = nal_unit_type;
    nalu->startcodeprefix_len = 4;
    nalu->buf[0] = 0;
    nalu->buf[1] = 0; 
    nalu->buf[2] = 0; 
    nalu->buf[3] = 1;    
    nalu->buf[4] = (nalu->forbidden_bit << 7) | (nalu->nal_reference_idc << 5) | (nalu->nal_unit_type);
    memcpy (&nalu->buf[5], rbsp, rbsp_size);    
    len = 5 + RBSPtoEBSP (&nalu->buf[5], 0, rbsp_size);
    nalu->len = len;
    return len;
}

//////////////////////////////////////////////////////////
//
// H264 decoder functions
//
//////////////////////////////////////////////////////////

/*!
 ************************************************************************
 * \brief
 *    Returns the size of the NALU (bits between start codes in case of
 *    Annex B.  nalu->buf and nalu->len are filled.  Other field in
 *    nalu-> remain uninitialized (will be taken care of by NALUtoRBSP.
 *
 * \return
 *     0 if there is nothing any more to read (EOF)
 *    -1 in case of any error
 *
 *  \note Side-effect: Returns length of start-code in bytes. 
 *
 * \note
 *   GetAnnexbNALU expects start codes at byte aligned positions in the file
 *
 ************************************************************************
 */
 #ifdef CCU_IP_FIRMWARE
 int GetAnnexbNALU (NALU_t *nalu)
{
	u32 nalu_info;
	u32 read_data=0;
	u32 write_data;
	
	// Find next start code for byte aligned (0x000001) and get NALU info.
	write_data = 0x00000400; // SYS_BSM_NSC
	H264DEC_AVC_BDC= write_data;
	//DEBUG_H264("Addr 0x0300 = 0x%08x\n",H264DEC_AVC_BDC );
	
	// check parser_idle=1
    check_parser_idle();

	read_data = H264DEC_AVC_BDO;
	nalu_info = 0x000000FF & (read_data >> 24);
	//DEBUG_H264("Addr 0x0304 = 0x%08x\n",H264DEC_AVC_BDO );
    
    write_data = 0x00000108; // SYS_BSM_SFT
	H264DEC_AVC_BDC = write_data;
	//DEBUG_H264("Addr 0x0300 = 0x%08x\n",H264DEC_AVC_BDC );
	
	nalu->forbidden_bit     = (u32)((nalu_info>>7) & 0x01);
	nalu->nal_reference_idc = (u32)((nalu_info>>5) & 0x03);
	nalu->nal_unit_type     = (u32)((nalu_info   ) & 0x1f);
    //DEBUG_H264("<%08x, %08x, %08x,%08x>\n",nalu_info,nalu->forbidden_bit,nalu->nal_reference_idc,nalu->nal_unit_type);
	return 1;
}
#else // JM software

int GetAnnexbNALU (NALU_t *nalu)
{
  int info2, info3, pos = 0;
  int StartCodeFound, rewind;
  unsigned char *Buf;
  int LeadingZero8BitsCount=0, TrailingZero8Bits=0;
  int     ftell_position;
  static int loop=0;
#if 1	
  if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL) no_mem_exit("GetAnnexbNALU: Buf");

  while(!feof(bits) && (Buf[pos++]=fgetc(bits))==0);
  ftell_position = ftell(bits);
  if(feof(bits))
  {
    if(pos==0)
        return 0;
    else
    {
      printf( "GetAnnexbNALU can't read start code\n");
      free(Buf);
      return -1;
    }
  }
#endif
  if(Buf[pos-1]!=1)
  {
    printf ("GetAnnexbNALU: no Start Code at the begin of the NALU, return -1\n");
    free(Buf);
    return -1;
  }

  if(pos<3)
  {
    printf ("GetAnnexbNALU: no Start Code at the begin of the NALU, return -1\n");
    free(Buf);
    return -1;
  }
  else if(pos==3)
  {
    nalu->startcodeprefix_len = 3;
    LeadingZero8BitsCount = 0;
  }
  else
  {
    LeadingZero8BitsCount = pos-4;
    nalu->startcodeprefix_len = 4;
  }

  //the 1st byte stream NAL unit can has leading_zero_8bits, but subsequent ones are not
  //allowed to contain it since these zeros(if any) are considered trailing_zero_8bits
  //of the previous byte stream NAL unit.
  if(!IsFirstByteStreamNALU && LeadingZero8BitsCount>0)
  {
    printf ("GetAnnexbNALU: The leading_zero_8bits syntax can only be present in the first byte stream NAL unit, return -1\n");
    free(Buf);
    return -1;
  }
  IsFirstByteStreamNALU=0;

  StartCodeFound = 0;
  info2 = 0;
  info3 = 0;

  while (!StartCodeFound)
  {
    if (feof (bits))
    {
      ftell_position = ftell(bits);  
      //Count the trailing_zero_8bits
      while(Buf[pos-2-TrailingZero8Bits]==0)
        TrailingZero8Bits++;
      nalu->len = (pos-1)-nalu->startcodeprefix_len-LeadingZero8BitsCount-TrailingZero8Bits;
      memcpy (nalu->buf, &Buf[LeadingZero8BitsCount+nalu->startcodeprefix_len], nalu->len);     
      nalu->forbidden_bit = (nalu->buf[0]>>7) & 1;
      nalu->nal_reference_idc = (nalu->buf[0]>>5) & 3;
      nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;

// printf ("GetAnnexbNALU, eof case: pos %d nalu->len %d, nalu->reference_idc %d, nal_unit_type %d \n", pos, nalu->len, nalu->nal_reference_idc, nalu->nal_unit_type);

#if TRACE
  fprintf (p_trace, "\n\nLast NALU in File\n\n");
  fprintf (p_trace, "Annex B NALU w/ %s startcode, len %d, forbidden_bit %d, nal_reference_idc %d, nal_unit_type %d\n\n",
    nalu->startcodeprefix_len == 4?"long":"short", nalu->len, nalu->forbidden_bit, nalu->nal_reference_idc, nalu->nal_unit_type);
  fflush (p_trace);
#endif
      ftell_position = ftell(bits);

      free(Buf);
      return pos-1;
    }
    Buf[pos++] = fgetc (bits);
    info3 = FindStartCode(&Buf[pos-4], 3);
    if(info3 != 1)
      info2 = FindStartCode(&Buf[pos-3], 2);
    StartCodeFound = (info2 == 1 || info3 == 1);
	if(StartCodeFound  == 1)
		StartCodeFound = 1;
  }

  //Count the trailing_zero_8bits
  if(info3==1)	//if the detected start code is 00 00 01, trailing_zero_8bits is sure not to be present
  {
    while(Buf[pos-5-TrailingZero8Bits]==0)
      TrailingZero8Bits++;
  }
  // Here, we have found another start code (and read length of startcode bytes more than we should
  // have.  Hence, go back in the file
  rewind = 0;
  if(info3 == 1)
    rewind = -4;
  else if (info2 == 1)
    rewind = -3;
  else
    printf(" Panic: Error in next start code search \n");

  if (0 != fseek (bits, rewind, SEEK_CUR))
  {
    snprintf (errortext, ET_SIZE, "GetAnnexbNALU: Cannot fseek %d in the bit stream file", rewind);
    free(Buf);
    error(errortext, 600);
  }

  // Here the leading zeros(if any), Start code, the complete NALU, trailing zeros(if any)
  // and the next start code is in the Buf.
  // The size of Buf is pos, pos+rewind are the number of bytes excluding the next
  // start code, and (pos+rewind)-startcodeprefix_len-LeadingZero8BitsCount-TrailingZero8Bits
  // is the size of the NALU.

  nalu->len = (pos+rewind)-nalu->startcodeprefix_len-LeadingZero8BitsCount-TrailingZero8Bits;
  memcpy (nalu->buf, &Buf[LeadingZero8BitsCount+nalu->startcodeprefix_len], nalu->len);
  nalu->forbidden_bit = (nalu->buf[0]>>7) & 1;
  nalu->nal_reference_idc = (nalu->buf[0]>>5) & 3;
  nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;


//printf ("GetAnnexbNALU, regular case: pos %d nalu->len %d, nalu->reference_idc %d, nal_unit_type %d \n", pos, nalu->len, nalu->nal_reference_idc, nalu->nal_unit_type);
#if TRACE
  fprintf (p_trace, "\n\nAnnex B NALU w/ %s startcode, len %d, forbidden_bit %d, nal_reference_idc %d, nal_unit_type %d\n\n",
    nalu->startcodeprefix_len == 4?"long":"short", nalu->len, nalu->forbidden_bit, nalu->nal_reference_idc, nalu->nal_unit_type);
  fflush (p_trace);
#endif
  
  free(Buf);
 
  return (pos+rewind);
}
#endif
