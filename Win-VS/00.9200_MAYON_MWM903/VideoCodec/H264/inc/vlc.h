/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	Vlc.h

Abstract:

   	Prototypes for VLC coding funtions.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/09/02	Lsk	Create	

*/

#ifndef _VLC_H_
#define _VLC_H_

typedef struct syntaxelement
{
    //int                 type;           //!< type of syntax element for data part.
    int                 value1;         //!< numerical value of syntax element
    int                 value2;         //!< for blocked symbols, e.g. run/level
    int                 len;            //!< length of code
    int                 inf;            //!< info part of UVLC code
    unsigned int        bitpattern;     //!< UVLC bitpattern 
} SyntaxElement;

//! Bitstream
typedef struct
{
    u32 byte_pos;           //!< current position in bitstream;
    u32 bits_to_go;         //!< current bitcounter
    u8  byte_buf;           //!< current buffer for last written byte
    //u32 stored_byte_pos;    //!< storage for position in bitstream;
    //u32 stored_bits_to_go;  //!< storage for bitcounter
    //u8  stored_byte_buf;    //!< storage for buffer of last written byte
    //u8  byte_buf_skip;      //!< current buffer for last written byte
    //u32 byte_pos_skip;      //!< storage for position in bitstream;
    //u32 bits_to_go_skip;    //!< storage for bitcounter

    u8  *streamBuffer;      //!< actual buffer for written bytes
    //u32 write_flag;         //!< Bitstream contains data and needs to be written
} Bitstream;

//////////////////////////////////////////////////////////
//
// H264 encoder functions
//
//////////////////////////////////////////////////////////
extern int encode_u_1  (int value, Bitstream *bitstream);
extern int encode_se_v (int value, Bitstream *bitstream);
extern int encode_ue_v (int value, Bitstream *bitstream);
extern int encode_u_v  (int n, int value, Bitstream *bitstream);
//extern void encode_ue_linfo(int n, int dummy, int *len,int *info);
//extern int encode_symbol2uvlc(SyntaxElement *se);
//extern void encode_se_linfo(int mvd, int dummy, int *len,int *info);
//extern void encode_writeUVLC2buffer(SyntaxElement *se, Bitstream *currStream);
//////////////////////////////////////////////////////////
//
// H264 decoder functions
//
//////////////////////////////////////////////////////////
extern int more_rbsp_data(void);
extern int rbsp_trailing_bits(void);
extern void check_parser_idle(void);
extern int decode_ue_v(char *tracestring, Bitstream *bitstream);
extern int decode_se_v(char *tracestring, Bitstream *bitstream);
extern int decode_u_v(int size, char*tracestring, Bitstream *bitstream);
extern int decode_u_1 (char *tracestring, Bitstream *bitstream);

#endif

