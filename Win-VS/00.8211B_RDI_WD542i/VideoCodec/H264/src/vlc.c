/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	Vlc.c

Abstract:

   	(CA)VLC coding functions.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/09/02	Lsk	Create	

*/

#include <stdlib.h>
#include <math.h>
#include "general.h"
#include "board.h"
#include "H264api.h"
#include "vlc.h"

#define absm(A) ((A)<(0) ? (-(A)):(A)) //!< abs macro, faster than procedure

//////////////////////////////////////////////////////////
//
// H264 encoder functions
//
//////////////////////////////////////////////////////////
void encode_ue_linfo(int ue, int dummy, int *len,int *info);
void encode_se_linfo(int se, int dummy, int *len,int *info);
void  encode_writeUVLC2buffer(SyntaxElement *se, Bitstream *currStream);
int encode_symbol2vlc(SyntaxElement *sym);
int encode_symbol2uvlc(SyntaxElement *sym);
/*! 
 *************************************************************************************
 * \brief
 *    ue_v, writes an ue(v) syntax element, returns the length in bits
 *
 * \param value
 *    the value to be coded
 *  \param bitstream
 *    the target bitstream the value should be coded into
 *
 * \return
 *    Number of bits used by the coded syntax element
 *
 *************************************************************************************
 */
 
int encode_ue_v (int value, Bitstream *bitstream)
{
  SyntaxElement symbol, *sym=&symbol;
  sym->value1 = value;
  sym->value2 = 0;



  encode_ue_linfo(sym->value1,sym->value2,&(sym->len),&(sym->inf));
  encode_symbol2uvlc(sym);

  encode_writeUVLC2buffer (sym, bitstream);


  return (sym->len);
}


/*! 
 *************************************************************************************
 * \brief
 *    se_v, writes an se(v) syntax element, returns the length in bits
 *
 * \param value
 *    the value to be coded
 *  \param bitstream
 *    the target bitstream the value should be coded into
 *
 * \return
 *    Number of bits used by the coded syntax element
 *
 * \ note
 *    This function writes always the bit buffer for the progressive scan flag, and
 *    should not be used (or should be modified appropriately) for the interlace crap
 *    When used in the context of the Parameter Sets, this is obviously not a
 *    problem.
 *
 *************************************************************************************
 */
int encode_se_v (int value, Bitstream *bitstream)
{
  SyntaxElement symbol, *sym=&symbol;
  sym->value1 = value;
  sym->value2 = 0;
 
  encode_se_linfo(sym->value1,sym->value2,&(sym->len),&(sym->inf));
  encode_symbol2uvlc(sym);
  encode_writeUVLC2buffer (sym, bitstream);
  return (sym->len);
}

/*! 
 *************************************************************************************
 * \brief
 *    u_1, writes a flag (u(1) syntax element, returns the length in bits, 
 *    always 1
 *
 * \param value
 *    the value to be coded
 *  \param bitstream
 *    the target bitstream the value should be coded into
 *
 * \return
 *    Number of bits used by the coded syntax element (always 1)
 *
 *************************************************************************************
 */
int encode_u_1 (int value, Bitstream *bitstream)
{
  SyntaxElement symbol, *sym=&symbol;

  sym->bitpattern = value;
  sym->len = 1;
  encode_writeUVLC2buffer(sym, bitstream);
  return (sym->len);
}


/*! 
 *************************************************************************************
 * \brief
 *    u_v, writes a n bit fixed length syntax element, returns the length in bits, 
 *
 * \param n
 *    length in bits
 * \param value
 *    the value to be coded
 *  \param bitstream
 *    the target bitstream the value should be coded into
 *
 * \return
 *    Number of bits used by the coded syntax element 
 *
 *************************************************************************************
 */

int encode_u_v (int n, int value, Bitstream *bitstream)
{
  SyntaxElement symbol, *sym=&symbol;

  sym->bitpattern = value;
  sym->len = n;
  encode_writeUVLC2buffer(sym, bitstream);  
  return (sym->len);
}


/*!
 ************************************************************************
 * \brief
 *    mapping for ue(v) syntax elements
 * \param ue
 *    value to be mapped
 * \param dummy
 *    dummy parameter
 * \param info
 *    returns mapped value
 * \param len
 *    returns mapped value length
 ************************************************************************
 */
void encode_ue_linfo(int ue, int dummy, int *len,int *info)
{
  int i,nn;

  nn=(ue+1)/2;

  for (i=0; i < 16 && nn != 0; i++)
  {
    nn /= 2;
  }
  *len= 2*i + 1;
  *info=ue+1-(int)pow(2,i);
}


/*!
 ************************************************************************
 * \brief
 *    mapping for se(v) syntax elements
 * \param se
 *    value to be mapped
 * \param dummy
 *    dummy parameter
 * \param len
 *    returns mapped value length
 * \param info
 *    returns mapped value
 ************************************************************************
 */
void encode_se_linfo(int se, int dummy, int *len,int *info)
{

  int i,n,sign,nn;

  sign=0;

  if (se <= 0)
  {
    sign=1;
  }
  n=absm(se) << 1;

  /*
  n+1 is the number in the code table.  Based on this we find length and info
  */

  nn=n/2;
  for (i=0; i < 16 && nn != 0; i++)
  {
    nn /= 2;
  }
  *len=i*2 + 1;
  *info=n - (int)pow(2,i) + sign;
}

/*!
 ************************************************************************
 * \brief
 *    writes UVLC code to the appropriate buffer
 ************************************************************************
 */
void  encode_writeUVLC2buffer(SyntaxElement *se, Bitstream *currStream)
{

  int i;
  unsigned int mask = 1 << (se->len-1);

  // Add the new bits to the bitstream.
  // Write out a byte if it is full
  for (i=0; i<se->len; i++)
  {
    currStream->byte_buf <<= 1;
    if (se->bitpattern & mask)
      currStream->byte_buf |= 1;
    currStream->bits_to_go--;
    mask >>= 1;
    if (currStream->bits_to_go==0)
    {
      currStream->bits_to_go = 8;
      currStream->streamBuffer[currStream->byte_pos++]=currStream->byte_buf;
      currStream->byte_buf = 0;
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Makes code word and passes it back
 *
 * \par Input:
 *    Info   : Xn..X2 X1 X0                                             \n
 *    Length : Total number of bits in the codeword
 ************************************************************************
 */

int encode_symbol2vlc(SyntaxElement *sym)
{
  int info_len = sym->len;

  // Convert info into a bitpattern int
  sym->bitpattern = 0;

  // vlc coding
  while(--info_len >= 0)
  {
    sym->bitpattern <<= 1;
    sym->bitpattern |= (0x01 & (sym->inf >> info_len));
  }
  return 0;
}

/*!
 ************************************************************************
 * \brief
 *    Makes code word and passes it back
 *    A code word has the following format: 0 0 0 ... 1 Xn ...X2 X1 X0.
 *
 * \par Input:
 *    Info   : Xn..X2 X1 X0                                             \n
 *    Length : Total number of bits in the codeword
 ************************************************************************
 */
 // NOTE this function is called with sym->inf > (1<<(sym->len/2)).  The upper bits of inf are junk
int encode_symbol2uvlc(SyntaxElement *sym)
{
  int suffix_len=sym->len/2;  
  sym->bitpattern = (1<<suffix_len)|(sym->inf&((1<<suffix_len)-1));
  return 0;
}
//////////////////////////////////////////////////////////
//
// H264 decoder functions
//
//////////////////////////////////////////////////////////
int more_rbsp_data()
{
	u32 read_data;
	read_data = H264DEC_AVC_BDO;  
	read_data = read_data >> 24;
  
	//if(more_rbsp_data(s->streamBuffer, s->frame_bitoffset,s->bitstream_length))
	if(read_data != 0x80)
		return 1;
	else
		return 0;
}

int rbsp_trailing_bits()
{
	u32 write_data;

	// `define SYS_BSM_RBSPTB 3'd5
	check_parser_idle();
    write_data = 5<<8;	
	H264DEC_AVC_BDC= write_data;
	//DEBUG_H264("Addr 0x0300 = 0x%08x\n", H264DEC_AVC_BDC );
    //DEBUG_H264("Addr 0x0300 = 0x%08x\n", write_data );
	return 0;
}
void check_parser_idle()
{
    u32 status;
    u32 i=0;

	for(i=0; i<10000; i++)
    {
      status = H264DEC_AVC_DIS0 & 0x00000001;
	  if(status==1)
	  	break;
	  
    }
	
	if(status==0)
		DEBUG_H264("H264 decoder parser can't enter idle state\n");
}
/*! 
 *************************************************************************************
 * \brief
 *    ue_v, reads an ue(v) syntax element, the length in bits is stored in 
 *    the global UsedBits variable
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
#ifdef CCU_IP_FIRMWARE
int decode_ue_v(char *tracestring, Bitstream *bitstream)
{
    u32   result;
    u32   result_tmp1;
    u32   result_tmp2;
    u32   length;
    u32   write_data;
       
    check_parser_idle();
	write_data = 0x00000300;
	H264DEC_AVC_BDC = write_data;
	//DEBUG_H264("Addr 0x0300 = 0x%08x\n",H264DEC_AVC_BDC );
	//DEBUG_H264("1.Addr 0x0300 = 0x%08x\n",write_data );

	length      = H264DEC_AVC_UVL;
	result_tmp1 = H264DEC_AVC_BDO;
    
    if(length == 0x00000000){
    	  result = 0;
    }
    else 
    {
        result_tmp2 = result_tmp1 >>(32-length);
        result = result_tmp2 + (0x0000FFFF >> (16-length));
           
        check_parser_idle();  
        write_data = 0x00000100 | length;     
		H264DEC_AVC_BDC = write_data;			
		//DEBUG_H264("Addr 0x0300 = 0x%08x\n",H264DEC_AVC_BDC );	
		//DEBUG_H264("2.Addr 0x0300 = 0x%08x\n",write_data );
    }   
    return result;
}
#else
int decode_ue_v (char *tracestring, Bitstream *bitstream)
{
  SyntaxElement symbol, *sym=&symbol;

  assert (bitstream->streamBuffer != NULL);
  sym->type = SE_HEADER;
  sym->mapping = linfo_ue;   // Mapping rule
  SYMTRACESTRING(tracestring);
  readSyntaxElement_VLC (sym, bitstream);
  UsedBits+=sym->len;
  return sym->value1;
}
#endif

/*! 
 *************************************************************************************
 * \brief
 *    ue_v, reads an se(v) syntax element, the length in bits is stored in 
 *    the global UsedBits variable
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
#ifdef CCU_IP_FIRMWARE
int decode_se_v(char *tracestring, Bitstream *bitstream)
{
    u32 result;
    u32 temp_data;	

    temp_data = decode_ue_v(NULL, NULL);
    if((temp_data & 0x00000001) == 0x00000001)
        result = temp_data / 2 + 1 ;
    else
        result = -(temp_data / 2);
        
    return result;
}
#else
int decode_se_v (char *tracestring, Bitstream *bitstream)
{
  SyntaxElement symbol, *sym=&symbol;

  assert (bitstream->streamBuffer != NULL);
  sym->type = SE_HEADER;
  sym->mapping = linfo_se;   // Mapping rule: signed integer
  SYMTRACESTRING(tracestring);
  readSyntaxElement_VLC (sym, bitstream);
  UsedBits+=sym->len;
  return sym->value1;
}
#endif
/*! 
 *************************************************************************************
 * \brief
 *    ue_v, reads an u(v) syntax element, the length in bits is stored in 
 *    the global UsedBits variable
 *
 * \param LenInBits
 *    length of the syntax element
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
#ifdef CCU_IP_FIRMWARE
int decode_u_v(int size, char*tracestring, Bitstream *bitstream)
{
	u32 result ;
	u32 write_data;

	if(size < 32)
	{		
		result = H264DEC_AVC_BDO;
		
		check_parser_idle();
		write_data = 0x00000100 | size;
		H264DEC_AVC_BDC = write_data;
		//DEBUG_H264("Addr 0x0300 = 0x%08x\n",H264DEC_AVC_BDC );	
		//DEBUG_H264("3.Addr 0x0300 = 0x%08x\n",write_data );	
		result = result >> (32-size); 
	}
	else if(size == 32)
	{ // 32 bits
	
		result = H264DEC_AVC_BDO;// bdc??		 
		write_data = 0x00000100 | 16;		
		check_parser_idle();
		H264DEC_AVC_BDC = write_data; // 1st 16 bit		
		//DEBUG_H264("Addr 0x0300 = 0x%08x\n",H264DEC_AVC_BDC );	
		//DEBUG_H264("4.Addr 0x0300 = 0x%08x\n",write_data );	
		check_parser_idle();
		H264DEC_AVC_BDC = write_data; // 2nd 16 bit
		//DEBUG_H264("Addr 0x0300 = 0x%08x\n",H264DEC_AVC_BDC );	
		//DEBUG_H264("5.Addr 0x0300 = 0x%08x\n",write_data );	
	}
	else
	{
		//printf("more than 32 bits shift operation is not supported.\n");
	}
	return result;
}
#else
int decode_u_v (int LenInBits, char*tracestring, Bitstream *bitstream)
{
  SyntaxElement symbol, *sym=&symbol;

  assert (bitstream->streamBuffer != NULL);
  sym->type = SE_HEADER;
  sym->mapping = linfo_ue;   // Mapping rule
  sym->len = LenInBits;
  SYMTRACESTRING(tracestring);
  readSyntaxElement_FLC (sym, bitstream);
  UsedBits+=sym->len;
  return sym->inf;
}
#endif
                
/*! 
 *************************************************************************************
 * \brief
 *    ue_v, reads an u(1) syntax element, the length in bits is stored in 
 *    the global UsedBits variable
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
int decode_u_1 (char *tracestring, Bitstream *bitstream)
{
  return decode_u_v (1, tracestring, bitstream);
}

