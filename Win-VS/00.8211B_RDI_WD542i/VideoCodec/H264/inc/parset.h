/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	parset.h

Abstract:

   	Picture and Sequence Parameter Sets, encoder operations.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/09/02	Lsk	Create	

*/


#ifndef _PARSET_H_
#define _PARSET_H_

#include "H264.h"
#include "Parsetcommon.h"
#include "NALUcommon.h"
#include "H264api.h"


//////////////////////////////////////////////////////////
//
// H264 encoder functions
//
//////////////////////////////////////////////////////////
    
extern int GenerateParameterSets_SW (H264_ENC_CFG*, u8*);
//////////////////////////////////////////////////////////
//
// H264 decoder functions
//
//////////////////////////////////////////////////////////
extern void ProcessSPS (NALU_t *nalu);
extern void ProcessPPS (NALU_t *nalu);

#endif
