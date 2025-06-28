/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	gpioapi.h

Abstract:

   	The application interface of general purpose I/O.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2016/09/19	Amy Pan	Create

*/

#ifndef __HDMI_API_H__
#define __HDMI_API_H__


enum TV_Video_Type {
    TV_Unkown = 0 ,
    TV_640x480p60 = 1 ,
    TV_480p60,
    TV_480p60_16x9,
    TV_720p60,
    TV_1080i60,
    TV_480i60,
    TV_480i60_16x9,
    TV_1080p60 = 16,
    TV_576p50,
    TV_576p50_16x9,
    TV_720p50,
    TV_1080i50,
    TV_576i50,
    TV_576i50_16x9,
    TV_1080p50 = 31,
    TV_1080p24,
    TV_1080p25,
    TV_1080p30,
    TV_720p30 = 61,
};

#if(HDMI_TXIC_SEL == HDMI_TX_EP952)
extern void HDMITX_VideoChange(unsigned char VdoSize);
extern void EP_HDMI_952_Init(unsigned char  Audfreq,unsigned char  VdoSize);
extern void EP952Controller_Task(void);
#endif

#if(HDMI_TXIC_SEL == HDMI_TX_IT66121)
extern void HDMITX_VideoChange(unsigned char VdoSize);
extern void HDMITX_AudioEnable(unsigned char audioEnable);
#endif

extern u8 isHDMIpluged(void);
extern u8 isCap1280x720P(void);
extern u8 isCap1920x1080I(void);
extern u8 getCurrentVDOSize(void);
#endif
