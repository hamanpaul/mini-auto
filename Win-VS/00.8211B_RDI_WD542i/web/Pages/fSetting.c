/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	sSetting.c  

Abstract:

   	The routines of web page function

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/07/05	Elsa Lee	Create	

*/

#include <string.h>
#include "sSetting.h"
#include "sMain.h"

/*---------------------------------------------------------------------------
 * RecSetting.html
 *---------------------------------------------------------------------------
 */
static sWeb_Set_RecSetting RecSet[WEB_MAX_CHANNEL_NUM];
int fWeb_RecSet_Get(int channel, sWeb_Set_RecSetting *data)
{
	static int first_run = 1;

    if(first_run == 1)
    {
        RecSet[0].Manual = 1;
        RecSet[0].Schedule = 0;
        RecSet[0].MotionDet = 0;
        RecSet[0].AlarmDet = 0;
        RecSet[0].Section = 0;
        RecSet[0].Sensitivity = 0;
        RecSet[1].Manual = 0;
        RecSet[1].Schedule = 1;
        RecSet[1].MotionDet = 0;
        RecSet[1].AlarmDet = 0;
        RecSet[1].Section = 1;
        RecSet[1].Sensitivity = 0;
        RecSet[2].Manual = 1;
        RecSet[2].Schedule = 1;
        RecSet[2].MotionDet = 0;
        RecSet[2].AlarmDet = 0;
        RecSet[2].Section = 2;
        RecSet[2].Sensitivity = 1;
        first_run = 0;
    }
    memcpy(data, &RecSet[channel], sizeof(sWeb_Set_RecSetting));
	return 1;
}

int fWeb_RecSet_Set(int channel, sWeb_Set_RecSetting *data)
{
    memcpy(&RecSet[channel], data, sizeof(sWeb_Set_RecSetting));
    return 1;
}

/*---------------------------------------------------------------------------
 * DisplaySetting.html
 *---------------------------------------------------------------------------
 */
static sWeb_Set_DisplaySetting DisSet;
int fWeb_DisplaySet_Get(sWeb_Set_DisplaySetting *data)
{
	static int first_run = 1;

    if(first_run == 1)
    {
        DisSet.AutoChannel = 5;
        DisSet.OSDTime = 1;
        DisSet.OSDChannel = 1;
        DisSet.OSDCard = 1;
        first_run = 0;
    }
    memcpy(data, &DisSet, sizeof(sWeb_Set_DisplaySetting));
	return 1;
}

int fWeb_DisplaySet_Set(sWeb_Set_DisplaySetting *data)
{
    memcpy(&DisSet, data, sizeof(sWeb_Set_DisplaySetting));
    return 1;
}

/*---------------------------------------------------------------------------
 * ImageSetting.html
 *---------------------------------------------------------------------------
 */
static sWeb_Set_ImageSetting ImgSet;
int fWeb_ImageSet_Get(sWeb_Set_ImageSetting *data)
{
	static int first_run = 1;

    if(first_run == 1)
    {
        ImgSet.Quality = 0;
        ImgSet.Resolution = 1;
        ImgSet.FrameRate = 0;
        first_run = 0;
    }
    memcpy(data, &ImgSet, sizeof(sWeb_Set_ImageSetting));
	return 1;

}

int fWeb_ImageSet_Set(sWeb_Set_ImageSetting *data)
{
    memcpy(&ImgSet, data, sizeof(sWeb_Set_ImageSetting));
    return 1;
}


