///*****************************************
//  Copyright (C) 2009-2015
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <sha1.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2015/06/10
//   @fileversion: ITE_HDMITX_SAMPLE_3.20
//******************************************/
#include "general.h"
#include "board.h"

#if(HDMI_TXIC_SEL == HDMI_TX_IT66121)
#ifndef _SHA_1_H_
#define _SHA_1_H_

#ifdef HDMI_DEBUG_ENA
#include <stdio.h>
#endif
#include <string.h>
#include "IT66121_config.h"
#include "IT66121_typedef.h"

#ifndef HDCP_DEBUG_PRINTF
    #define HDCP_DEBUG_PRINTF(x)
#endif //HDCP_DEBUG_PRINTF

#ifndef HDCP_DEBUG_PRINTF1
    #define HDCP_DEBUG_PRINTF1(x)
#endif //HDCP_DEBUG_PRINTF1

#ifndef HDCP_DEBUG_PRINTF2
    #define HDCP_DEBUG_PRINTF2(x)
#endif //HDCP_DEBUG_PRINTF2


#ifndef DISABLE_HDCP
void SHA_Simple(void *p,WORD len,BYTE *output);
void SHATransform(ULONG * h);
#endif

#endif // _SHA_1_H_
#endif
