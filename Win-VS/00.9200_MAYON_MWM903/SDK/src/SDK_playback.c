/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

    SDK_playback.c

Abstract:

    The routines of Playback SDK.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2015/12/14  Lsk  Create  

*/

#include "General.h"
#include "SDK_playbackapi.h"
#include "sysapi.h"


#define PLAYBACK_SPEED_LEVEL     10

static u8 SdkAllowStartPlay;

extern u8  videoPlayNext;

#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
extern u32 CopyDone;
#endif

#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
BOOLEAN SDK_PLAYBACK_PlayAllFile(u32 time, u8 flag)
{
    INT8U err;
    u32   waitFlag;
    u32   timeInSec;

    waitFlag = OSFlagAccept(gSysReadyFlagGrp, (FLAGSYS_RDYSTAT_SET_PLAY|FLAGSYS_RDYSTAT_PLAY_START), OS_FLAG_WAIT_SET_ALL,&err);	
    if(err == OS_NO_ERR)
    {
        if(OSFlagAccept(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, &err)>0)
        {
        	DEBUG_SHOW_START();
        	DEBUG_UI("UI select playback time=%d, flage=%d\r\n", time, flag);
			DEBUG_SHOW_END();
            
            timeInSec = (time/10000)*3600 + ((time%10000)/100)*60 + time%100; // transfer to Second unit      
            SdkAllowStartPlay = sysIsTimeExistFile(timeInSec, timeInSec, SYS_PLAYBACK_MODE);
            if(!SdkAllowStartPlay){
                return FALSE;
            }
			
        	if(flag == play_direct)
				videoPlayNext = 1;
			sysSetSeekTime(time);
			sysEnableThumb(flag);
            sysSetEvt(SYS_EVT_ContinuousReadFile, 0);
            OSTimeDly(2); // make sure sysPlayback status is set
            DEBUG_UI("uiReadVideoFile, thumbnail:%d \r\n", sysPlaybackThumbnail);
            return TRUE;
        }
    }
    DEBUG_UI("Can not Read File %d\r\n",waitFlag);
    return FALSE;
}

BOOLEAN SDK_PLAYBACK_PlayOneFile(void)
{
    INT8U err;
    u32   waitFlag;

    waitFlag = OSFlagAccept(gSysReadyFlagGrp, (FLAGSYS_RDYSTAT_SET_PLAY|FLAGSYS_RDYSTAT_PLAY_START), OS_FLAG_WAIT_SET_ALL,&err);
    if(err == OS_NO_ERR)
    {
        if(OSFlagAccept(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, &err)>0)
        {
            sysSetEvt(SYS_EVT_ReadFile, 0);
            DEBUG_UI("uiReadVideoFile\r\n");
            return TRUE;
        }
    }
    DEBUG_UI("Can not Read File %d\r\n",waitFlag);
    return FALSE;
}

BOOLEAN SDK_CopyFile(u32 BeginSec, u32 EndSec)
{
	INT8U err;
    u32   waitFlag;

	DEBUG_UI("[SDK]Copy File \r\n");
    waitFlag = OSFlagAccept(gSysReadyFlagGrp, (FLAGSYS_RDYSTAT_COPY_FILE), OS_FLAG_WAIT_SET_ALL,&err);
	if(err == OS_NO_ERR)
    {
        if(OSFlagAccept(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_COPY_FILE, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, &err)>0)
        {
            if(!sysIsTimeExistFile(BeginSec, EndSec, SYS_FILE_BACKUP_MODE)){
                CopyDone = 1;
             	DEBUG_UI("[SDK] Can not Copy File %d\r\n", waitFlag);                
                return FALSE;
            }
        	//sysSetSeekTime(time);
        	sysSetCopyDuration(BeginSec, EndSec);
            sysSetEvt(SYS_EVT_CopyFile, 0);
            DEBUG_UI("[SDK] uiCopyVideoFile\r\n");
            return TRUE;
        }
    }
	DEBUG_UI("[SDK] Can not Copy File %d\r\n", waitFlag);
	return FALSE;
}

u32 SDK_CopyFile_GetState(void)
{
	if(CopyDone == 1)
	{
		CopyDone = 0;
		return TRUE;
	}
	else
		return FALSE;		
}
#endif

BOOLEAN SDK_PLAYBACK_StopAllFile(void)
{
    u8 err;
	videoPlayNext = 0;
    if (sysThumnailPtr->type == 0)
        return FALSE;
    if((OSFlagAccept(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_SET_PLAY|FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_WAIT_CLR_ANY, &err)> 0) && (err == OS_NO_ERR))
    {
        DEBUG_UI("uiPlaybackStop \r\n");
        OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_WAIT_CLR_ANY, 200, &err);
        #if(MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_ASF)
        while (asfPlaybackVideoStop() == 0)
        #elif(MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_AVI)
        while (aviPlaybackVideoStop() == 0)
        #endif
        {
            OSTimeDly(4);
        }		
        OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_START, OS_FLAG_WAIT_SET_ANY, 200, &err);
		if (curr_playback_speed != (PLAYBACK_SPEED_LEVEL>>1))
        {
            curr_playback_speed = (PLAYBACK_SPEED_LEVEL>>1);                
        }   
        OSTimeDly(5); // Delay for protect system        
        return TRUE;
    }
    else
        DEBUG_UI("Not in playing \r\n");

    return FALSE;
}

BOOLEAN SDK_PLAYBACK_PlayPause(void)
{
    if(sysPlaybackThumbnail == 1)   /*it is thumbnail, don't do anything*/
        return FALSE;
        
	if(sysPlaybackVideoPause == 1)    /*pause -> play*/
    {
        curr_playback_speed = (PLAYBACK_SPEED_LEVEL>>1);
        curr_slow_speed = 0;
        sysPlaybackVideoPause=0;
		return TRUE;
    }

	if(sysPlaybackVideoStart == 1)  /*play -> pause*/
    {
        if((VideoDuration-3) != (VideoNextPresentTime/1000000))
        {
            curr_playback_speed = (PLAYBACK_SPEED_LEVEL>>1);                
            curr_slow_speed = 0;
            sysPlaybackVideoPause = 1;
            OSTimeDly(3); // Delay for protect system            
        }
        else
        {    
        	//TODO
        }
		return TRUE;
    }
	return FALSE;
}

BOOLEAN SDK_PLAYBACK_FF(void)
{
    if(!SdkAllowStartPlay)
        return FALSE;
        
	if(curr_playback_speed == 3)
		curr_playback_speed = 5; // 2x backward -> 1x resume, speed 4 is not support for asf, speed 5 is normal play
	else if (curr_playback_speed < PLAYBACK_SPEED_LEVEL-1)
		curr_playback_speed++;
    
	curr_slow_speed = 0;
	printf("%s_%d\n", __FUNCTION__, curr_playback_speed);
	return TRUE;
}

BOOLEAN SDK_PLAYBACK_RF(void)
{
    if(!SdkAllowStartPlay)
        return FALSE;
    
	if(curr_playback_speed == 5)
		curr_playback_speed = 3;  // 1x->2x backward, speed 4 is not support for asf    
	else if (curr_playback_speed > 0)
		curr_playback_speed--;

	curr_slow_speed = 0;    
	printf("%s_%d\n", __FUNCTION__, curr_playback_speed);
	return TRUE;
}

BOOLEAN SDK_PLAYBACK_SlowMotion(void)
{
    if(!SdkAllowStartPlay)
        return FALSE;
    
	if(curr_slow_speed < 5){
        curr_slow_speed++;
        curr_playback_speed = (PLAYBACK_SPEED_LEVEL>>1);
	}
    printf("%s_%d\n", __FUNCTION__, curr_slow_speed);
	return TRUE;
}

u8  PlaybackFileFind(void)
{
    u8  CamId;

    CamId = dcfPlaybackCurFile->pDirEnt->d_name[7]-'1';
    if (sysPlaybackCamList & (0x01 << CamId))
        return TRUE;
    else
        return FALSE;
}


BOOLEAN SDK_PLAYBACK_PrevFile(void)
{
    u8  retVal = 0;
    DCF_LIST_DIRENT* PlaytmpDir;
    DCF_LIST_FILEENT* playTmpFile;

    if (dcfPlaybackCurFile == NULL)
        return FALSE;

    playTmpFile = dcfPlaybackCurFile;
    do
    {
        if (dcfPlaybackCurFile  == dcfGetPlaybackFileListHead())
        {
            break;
        }
        dcfPlaybackFilePrev();
        retVal = PlaybackFileFind();
        DEBUG_UI("FilePrev %s %d\n",dcfPlaybackCurFile->pDirEnt->d_name, retVal);
    }
    while (retVal == 0);
    if (retVal == 1)
    {
        return TRUE;
    }
    else
    {
    	#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
        PlaytmpDir = dcfPlaybackCurDir;
        dcfPlaybackCurDir = dcfPlaybackCurDir->playbackPrev;
        while (dcfPlaybackCurDir != PlaytmpDir)
        {
            if(dcfScanFileOnPlaybackDir() == 0)
                break;
            while(retVal == 0)
            {
                retVal = PlaybackFileFind();
                DEBUG_UI("Prev Dir %s file %s %d\n",dcfPlaybackCurDir->pDirEnt->d_name, dcfPlaybackCurFile->pDirEnt->d_name+7, retVal);
                if (retVal == 1)
                {
                    return TRUE;
                }
                if (dcfPlaybackCurFile  == dcfGetPlaybackFileListHead())
                    break;
                dcfPlaybackFilePrev();
            }
            dcfPlaybackCurDir = dcfPlaybackCurDir->playbackPrev;
        }
        if (retVal == 0)
        {
            if(dcfScanFileOnPlaybackDir() == 0)
                return FALSE;
            while(dcfPlaybackCurFile != playTmpFile)
            {
                retVal = PlaybackFileFind();
                DEBUG_UI("Prev orDir %s file %s %d\n",dcfPlaybackCurDir->pDirEnt->d_name, dcfPlaybackCurFile->pDirEnt->d_name+7, retVal);
                if (retVal == 1)
                    return TRUE;
                dcfPlaybackFilePrev();
            }
        }
		#endif
    }
    return FALSE;
}

BOOLEAN SDK_PLAYBACK_NextFile(void)
{
    u8  retVal = 0;
    DCF_LIST_DIRENT* PlaytmpDir;
    DCF_LIST_FILEENT* playTmpFile;

    if (dcfPlaybackCurFile == NULL)
        return FALSE;
    playTmpFile = dcfPlaybackCurFile;
    do
    {
        if (dcfPlaybackCurFile  == dcfGetPlaybackFileListTail())
            break;
        dcfPlaybackFileNext();
        retVal = PlaybackFileFind();
        DEBUG_UI("FileNext %s %d\n",dcfPlaybackCurFile->pDirEnt->d_name, retVal);
    }while (retVal == 0);

    if (retVal == 1)
    {
        return TRUE;
    }
    else
    {
    	#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
        PlaytmpDir = dcfPlaybackCurDir;
        dcfPlaybackCurDir = dcfPlaybackCurDir->playbackNext;
        while (dcfPlaybackCurDir != PlaytmpDir)
        {
            if(dcfScanFileOnPlaybackDir() == 0)
                break;
            dcfPlaybackCurFile = dcfGetPlaybackFileListHead();
            while(retVal == 0)
            {
                retVal = PlaybackFileFind();
                DEBUG_UI("next Dir %s file %s %d\n",dcfPlaybackCurDir->pDirEnt->d_name, dcfPlaybackCurFile->pDirEnt->d_name+7, retVal);
                if (retVal == 1)
                {
                    return TRUE;
                }
                if (dcfPlaybackCurFile  == dcfGetPlaybackFileListTail())
                    break;
                dcfPlaybackFileNext();
            }
            dcfPlaybackCurDir = dcfPlaybackCurDir->playbackNext;
        }
        if (retVal == 0)
        {
            if(dcfScanFileOnPlaybackDir() == 0)
                return FALSE;
            dcfPlaybackCurFile = dcfGetPlaybackFileListHead();
            while(dcfPlaybackCurFile != playTmpFile)
            {
                retVal = PlaybackFileFind();
                DEBUG_UI("next orDir %s file %s %d\n",dcfPlaybackCurDir->pDirEnt->d_name, dcfPlaybackCurFile->pDirEnt->d_name+7, retVal);
                if (retVal == 1)
                    return TRUE;
                dcfPlaybackFileNext();
            }
        }
		#endif
    }
    return FALSE;
}
BOOLEAN SDK_PLAYBACK_DeleteDir(void)
{
	IduVideo_ClearPKBuf(0);
    if(Write_protet() && sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
    {
        DEBUG_UI("Write_protet.....\r\n");
        //osdDrawProtect(1);
    }
    else
    {
        DEBUG_UI("Delete Folder\r\n");
        if(dcfPlaybackDelDir() == 0)
            DEBUG_UI("Delete Folder Fail\r\n");
    }
    return TRUE;
}

BOOLEAN SDK_PLAYBACK_DeleteFile(void)
{
	IduVideo_ClearPKBuf(0);
    if(Write_protet() && sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
    {
        DEBUG_UI("Write_protet.....\r\n");
        //osdDrawProtect(1);
    }
    else
    {
        DEBUG_UI("Delete File\r\n");
        dcfPlaybackDel();        
    }
	return TRUE;
}

BOOLEAN SDK_PLAYBACK_AudioVolume(s8 setting)
{
    DEBUG_UI("uiMenuSet_Audio_Vol %d\r\n",setting);

    if (Main_Init_Ready == 0)
        sysVolumnControl = setting;

    adcSetDAC_OutputGain((u32) setting);

    return TRUE;
}

u32 SDK_PLAYBACK_GetFileDuration(DCF_LIST_FILEENT* _pTempCurFile)
{
	u32 duration=0;
   	duration = GetVideoDuration( (s8*)(_pTempCurFile->pDirEnt->d_name));
    return duration;
	
	//return VideoDuration;
}

u8 SDK_PLAYBACK_GetVideoPresentTIme(u32* cur_time)
{
    //File creat time add video VideoNextPresentTime
	u8 h1, m1, s1;
	u8 h2, m2, s2;
	u8 h3, m3, s3;	
	u8	add=0;
	u32 tmp;

    if(sysPlaybackVideoStop || IsuIndex <= 1)
        return FALSE;

	h1 = (u8)((((u32)(VideoNextPresentTime)/1000000)/3600));
    m1 = (u8)(((u32)(VideoNextPresentTime)/1000000 - h1*3600) / 60);
    s1 = (u8)((u32)(VideoNextPresentTime)/1000000 - h1*3600 - m1*60);
	
	sscanf((char*)(signed char*)dcfPlaybackCurFile->pDirEnt->d_name, "%d", &tmp);	
	h2=tmp/10000;
	m2=(tmp-h2*10000)/100;
	s2 = tmp%100;	

	if(s1+s2>60)
	{
		s3=s1+s2-60;
		add=1;
	}
	else
	{
		s3=s1+s2;
		add=0;
	}

	if(m1+m2+add>60)
	{
		m3=m1+m2+add-60;
		add=1;
	}
	else
	{
		m3=m1+m2+add;
		add=0;
	}

	#if 0	
	if(h1+h2+add>60)
	{
		h3=h1+h2+add-60;
		add=1;
	}
	else
	{
		h3=h1+h2+add;	
		if(h3>=24)
			h3=h3-24;
		add=0;
	}
	#else
	h3=h1+h2+add;	
	if(h3>=24)
	{
		printf("Playback hour >= 24, <%d, %s>, <%d, %d, %d>, <%d, %d, %d>, <%d, %d, %d>\n", (u32)VideoNextPresentTime, dcfPlaybackCurFile->pDirEnt->d_name, h1, m1, s1, h2, m2, s2, h3, m3, s3);
	}
	#endif

    *cur_time = (u32)(h3*10000+m3*100+s3);
    
	return TRUE;
}

u32 SDK_PLAYBACK_GetState(void)
{
	//printf("SDK_PLAYBACK_GetState = <%d, %d, %d>\n", sysPlaybackVideoPause, sysPlaybackVideoStop, videoPlayNext);
	if(sysPlaybackVideoPause==1)
		return state_pause;
	else if(sysPlaybackVideoStop==1 && videoPlayNext==0)
		return state_stop;
	else
		return state_play;
}

