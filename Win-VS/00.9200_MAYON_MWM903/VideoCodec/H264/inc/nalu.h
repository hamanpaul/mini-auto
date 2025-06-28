/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	nalu.h
	
Abstract:

   	RPSB to NALU.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/09/05	Lsk	Create	

*/



#ifndef _NALU_H_
#define _NALU_H_

#include "nalucommon.h"

#define MAX_CODED_FRAME_SIZE 8000000         //!< bytes for one frame


extern int GetAnnexbNALU (NALU_t *nalu);

extern int RBSPtoNALU (unsigned char *rbsp, NALU_t *nalu, int rbsp_size, int nal_unit_type, int nal_reference_idc);

#endif
