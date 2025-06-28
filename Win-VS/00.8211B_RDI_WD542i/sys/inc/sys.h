/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	sys.h

Abstract:

   	The declaration of system control.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __SYS_H__
#define __SYS_H__

/* Constant */
#define SYS_PREVIEW_INIT_RESET           1
#define SYS_PREVIEW_INIT_OSDDRAWICON     2

#define SYS_PREVIEW_RESET_PWRMAG         1
#define SYS_PREVIEW_RESET_TV_VIDEOOFF    2

#define SYS_SELF_TIMER_DRAW_ICON        1

enum
{
    SYS_CAPTURE_VIDEO_OVERWRITE_DELETE_PASS = 1,
    SYS_CAPTURE_VIDEO_RISE_FREQUENCY,
    SYS_CAPTURE_VIDEO_CHANGE_CHANNEL,
    SYS_CAPTURE_VIDEO_CHECK_SET_60FPS,
    SYS_CAPTURE_VIDEO_WRITE_ASF_FILE,
    SYS_CAPTURE_VIDEO_SET_SIU_MODE,
    SYS_CAPTURE_VIDEO_FALL_FREQUENCY,
    SYS_CAPTURE_VIDEO_SET_LED,
    SYS_CAPTURE_VIDEO_POWER_OFF,
    SYS_CAPTURE_VIDEO_CLEAR_BUFFER

};

extern u8 sysProjectSysInit(u8 Step);
extern u8 sysProjectExifWrite(u8 Step);
extern u8 sysProjectPreviewInit(u8 Step);
extern u8 sysProjectPreviewReset(u8 Step);
extern u8 sysProjectPreviewStop(u8 Step);
extern u8 sysProjectSnapshot(u8 Step);
extern u8 sysProjectSnapshotOnPreview(u8 Step, s32 ScalingFactor);
extern u8 sysProjectVideoCaptureRoot(u8 Step);
extern u8 sysProjectPowerOff(u8 Step);
extern u8 sysProjectMacro(u8 Step);
extern u8 sysProjectSDCD_IN(u8 Step);
extern u8 sysProjectWhiteLight(u8 Step, u8 On);
extern u8 sysProjectSDCD_OFF(u8 Step);
extern u8 sysProjectSelfTimer(u8 Step);
extern u32 sysProjectCaptureImage(u8 Step);
extern u32 sysProjectCaptureImage_Burst_OnPreview(u8 Step, u8 BurstNum, u32 i);
extern u8 sysProjectCaptureVideo(u8 Step, s32 ZoomFactor);
extern s32 sysForceWDTtoReboot(void);
#endif

