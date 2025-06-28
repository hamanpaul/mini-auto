/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    sSetting.c  

Abstract:

    The routines of web page function

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2011/07/05  Elsa Lee    Create  

*/

#include "AsEngine.h"
#include "web_util.h"
#include "sSetting.h"
#include "wb_Menu.h"
#include "sMain.h"

/*---------------------------------------------------------------------------
 * RecSetting.html
 *---------------------------------------------------------------------------
 */
static sWeb_Set_RecSetting Web_RecSet;
static int  RecSet_Channel = 0;

char *sWeb_RecSet_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    RecSet_Channel =*(theIndexValuesPtr)+1;
    fWeb_RecSet_Get(RecSet_Channel, &Web_RecSet);
    return  NULL;
}

void sWeb_RecSet_Apply(void *theServerDataPtr, Signed16Ptr theIndexValuesPtr)
{
    fWeb_RecSet_Set(RecSet_Channel, &Web_RecSet);
    return;
}

char *sWeb_RecSet_Channel_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    HPF(NULL);
    return HPF("0");
}

void sWeb_RecSet_Channel_Set(void *theTaskDataPtr, char *theValuePtr,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{
    //RecSet_Channel=VDSLPortName2Num(theValuePtr);
}

char *sWeb_RecSet_ChannelSel_Select_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    int language = fWeb_Language_Get();
    int conut = 0;

    HPF(NULL);
    HPF("<SELECT name=\"sWeb_RecSet_ChannelSel\" size=1 onChange=\"ReloadByChannel()\">\n");

    if (language == 0)
    {
        while(conut < WEB_MAX_CHANNEL_NUM)
        {
            if(RecSet_Channel == conut)
                HPF("<OPTION VALUE=\"%d\" selected>Channel %d</OPTION>",conut, (conut+1));
            else
                HPF("<OPTION VALUE=\"%d\" >Channel %d</OPTION>",conut, (conut+1));
            conut++;
        }
    }
    else if(language == 1)
    {
        while(conut < WEB_MAX_CHANNEL_NUM)
        {
            if(RecSet_Channel == conut)
                HPF("<OPTION VALUE=\"%d\" selected>頻道 %d</OPTION>",conut, (conut+1));
            else
                HPF("<OPTION VALUE=\"%d\" >頻道 %d</OPTION>",conut, (conut+1));
            conut++;
        }
    }
    HPF("</SELECT>");
    return HPF("\n");
}

rpOneOfSeveral sWeb_STP_GlobalSel_Get(void *theServerDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    return (rpOneOfSeveral)0;
}

void sWeb_STP_GlobalSel_Set(void *theServerDataPtr, rpOneOfSeveral theValue,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{

}
rpOneOfSeveral sWeb_RecSet_Manual_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    return (rpOneOfSeveral)Web_RecSet.Manual;
}

void sWeb_RecSet_Manual_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{
    Web_RecSet.Manual=(int)theValue;
}

rpOneOfSeveral sWeb_RecSet_Schedule_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    return (rpOneOfSeveral)Web_RecSet.Schedule;
}

void sWeb_RecSet_Schedule_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{
    Web_RecSet.Schedule=(int)theValue;
}

rpOneOfSeveral sWeb_RecSet_Motion_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    return (rpOneOfSeveral)Web_RecSet.MotionDet;
}

void sWeb_RecSet_Motion_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{
    Web_RecSet.MotionDet=(int)theValue;
}

rpOneOfSeveral sWeb_RecSet_Alarm_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    return (rpOneOfSeveral)Web_RecSet.AlarmDet;
}

void sWeb_RecSet_Alarm_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{
    Web_RecSet.AlarmDet=(int)theValue;
}

rpOneOfSeveral sWeb_RecSet_Section_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    return (rpOneOfSeveral)Web_RecSet.Section;
}

void sWeb_RecSet_Section_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{
    Web_RecSet.Section=(int)theValue;
}

rpOneOfSeveral sWeb_RecSet_Sensitivity_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    return (rpOneOfSeveral)Web_RecSet.Sensitivity;
}

void sWeb_RecSet_Sensitivity_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{
    Web_RecSet.Sensitivity=(int)theValue;
}

char *sWeb_RecSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    int pageString = (int)atoi(theNamePtr);
    int language = fWeb_Language_Get();
    char szTemp[256];

    szTemp[0]=0;

    switch (pageString)
    {
        case 1:
            if(language == 0)
                strcpy(szTemp, "Rec. Setting");
            else if(language == 1)
                strcpy(szTemp, "錄影設定");
            break;

        case 2:
            if(language == 0)
                strcpy(szTemp, "Setting Item");
            else if(language == 1)
                strcpy(szTemp, "設定項目");
            break;

        case 3:
            if(language == 0)
                strcpy(szTemp, "Contents");
            else if(language == 1)
                strcpy(szTemp, "內容");
            break;

        case 4:
            if(language == 0)
                strcpy(szTemp, "Manual");
            else if(language == 1)
                strcpy(szTemp, "手動錄影");
            break;

        case 5:
            if(language == 0)
                strcpy(szTemp, "Schedule");
            else if(language == 1)
                strcpy(szTemp, "預約錄影");
            break;

        case 6:
            if(language == 0)
                strcpy(szTemp, "Motion Detection");
            else if(language == 1)
                strcpy(szTemp, "移動偵測");
            break;

        case 7:
            if(language == 0)
                strcpy(szTemp, "Alarm Detection");
            else if(language == 1)
                strcpy(szTemp, "警報偵測");
            break;

        case 8:
            if(language == 0)
                strcpy(szTemp, "Section");
            else if(language == 1)
                strcpy(szTemp, "檔案長度");
            break;

        case 9:
            if(language == 0)
                strcpy(szTemp, "Disable");
            else if(language == 1)
                strcpy(szTemp, "關閉");
            break;

        case 10:
            if(language == 0)
                strcpy(szTemp, "Enable");
            else if(language == 1)
                strcpy(szTemp, "開啟");
            break;

        case 11:
            if(language == 0)
                strcpy(szTemp, "15 min");
            else if(language == 1)
                strcpy(szTemp, "15分鐘");
            break;

        case 12:
            if(language == 0)
                strcpy(szTemp, "30 min");
            else if(language == 1)
                strcpy(szTemp, "30分鐘");
            break;

        case 13:
            if(language == 0)
                strcpy(szTemp, "60 min");
            else if(language == 1)
                strcpy(szTemp, "60分鐘");
            break;

        case 14:
            if(language == 0)
                strcpy(szTemp, "Motion Sensitivity");
            else if(language == 1)
                strcpy(szTemp, "移動偵測靈敏度");
            break;

        case 15:
            if(language == 0)
                strcpy(szTemp, "H");
            else if(language == 1)
                strcpy(szTemp, "高");
            break;

        case 16:
            if(language == 0)
                strcpy(szTemp, "M");
            else if(language == 1)
                strcpy(szTemp, "中");
            break;

        case 17:
            if(language == 0)
                strcpy(szTemp, "L");
            else if(language == 1)
                strcpy(szTemp, "低");
            break;

        case 18:
            if(language == 0)
                strcpy(szTemp, "Motion Mask Area");
            else if(language == 1)
                strcpy(szTemp, "移動偵測遮蔽區");
            break;

        case 19:
            if(language == 0)
                strcpy(szTemp, "Detail Setting");
            else if(language == 1)
                strcpy(szTemp, "詳細設定");
            break;

        case 20:
            if(language == 0)
                strcpy(szTemp, "Rec. Mode");
            else if(language == 1)
                strcpy(szTemp, "錄影模式");
            break;

        case 21:
            if(language == 0)
                strcpy(szTemp, "Channel Select");
            else if(language == 1)
                strcpy(szTemp, "頻道選擇");
            break;

        case 22:
            if(language == 0)
                strcpy(szTemp, "Channel 2");
            else if(language == 1)
                strcpy(szTemp, "頻道2");
            break;

        case 23:
            if(language == 0)
                strcpy(szTemp, "Channel 3");
            else if(language == 1)
                strcpy(szTemp, "頻道3");
            break;
    }

    HPF(NULL);
    return HPF("%s", szTemp);
}

/*---------------------------------------------------------------------------
 * DisplaySetting.html
 *---------------------------------------------------------------------------
 */
static sWeb_Set_DisplaySetting Web_DisSet;

char *sWeb_DisplaySet_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    fWeb_DisplaySet_Get(&Web_DisSet);
    return  NULL;

}

void sWeb_DisplaySet_Apply(void *theTaskDataPtr, Signed16Ptr theIndexValuesPtr)

{
    fWeb_DisplaySet_Set(&Web_DisSet);
    return;
}

char *sWeb_DisplaySet_AutoChannel_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    HPF(NULL);
    return HPF("%d", Web_DisSet.AutoChannel);
}

void sWeb_DisplaySet_AutoChannel_Set(void *theTaskDataPtr, char *theValuePtr,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{
    Web_DisSet.AutoChannel = (int)atoi(theValuePtr);
}

rpOneOfSeveral sWeb_DisplaySet_OSDTime_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    return (rpOneOfSeveral)Web_DisSet.OSDTime;
}

void sWeb_DisplaySet_OSDTime_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{
    Web_DisSet.OSDTime=(int)theValue;
}

rpOneOfSeveral sWeb_DisplaySet_OSDChel_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    return (rpOneOfSeveral)Web_DisSet.OSDChannel;
}

void sWeb_DisplaySet_OSDChel_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{
    Web_DisSet.OSDChannel=(int)theValue;
}

rpOneOfSeveral sWeb_DisplaySet_OSDCard_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    return (rpOneOfSeveral)Web_DisSet.OSDCard;
}

void sWeb_DisplaySet_OSDCard_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{
    Web_DisSet.OSDCard=(int)theValue;
}

char *sWeb_DisplaySet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    int pageString = (int)atoi(theNamePtr);
    int language = fWeb_Language_Get();
    char szTemp[256];

    szTemp[0]=0;
    switch (pageString)
    {
        case 1:
            if(language == 0)
                strcpy(szTemp, "Display Setting");
            else if(language == 1)
                strcpy(szTemp, "顯示設定");
            break;

        case 2:
            if(language == 0)
                strcpy(szTemp, "Setting Item");
            else if(language == 1)
                strcpy(szTemp, "設定項目");
            break;

        case 3:
            if(language == 0)
                strcpy(szTemp, "Contents");
            else if(language == 1)
                strcpy(szTemp, "內容");
            break;

        case 4:
            if(language == 0)
                strcpy(szTemp, "Auto Switch Channels");
            else if(language == 1)
                strcpy(szTemp, "頻道自動切換");
            break;

        case 5:
            if(language == 0)
                strcpy(szTemp, "(0~15) Seconds");
            else if(language == 1)
                strcpy(szTemp, "(0~15) 秒");
            break;

        case 6:
            if(language == 0)
                strcpy(szTemp, "OSD Display");
            else if(language == 1)
                strcpy(szTemp, "OSD顯示");
            break;

        case 7:
            if(language == 0)
                strcpy(szTemp, "Time");
            else if(language == 1)
                strcpy(szTemp, "時間");
            break;

        case 8:
            if(language == 0)
                strcpy(szTemp, "Disable");
            else if(language == 1)
                strcpy(szTemp, "關閉");
            break;

        case 9:
            if(language == 0)
                strcpy(szTemp, "Enable");
            else if(language == 1)
                strcpy(szTemp, "開啟");
            break;

        case 10:
            if(language == 0)
                strcpy(szTemp, "Channel");
            else if(language == 1)
                strcpy(szTemp, "頻道");
            break;

        case 11:
            if(language == 0)
                strcpy(szTemp, "SD Card Free Space");
            else if(language == 1)
                strcpy(szTemp, "SD卡剩餘空間");
            break;
    }
    HPF(NULL);
    return HPF("%s", szTemp);
}

/*---------------------------------------------------------------------------
 * ImageSetting.html
 *---------------------------------------------------------------------------
 */
static sWeb_Set_ImageSetting Web_ImgSet;

char *sWeb_ImgSet_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    fWeb_ImageSet_Get(&Web_ImgSet);
    return  NULL;
}

void sWeb_ImgSet_Apply(void *theTaskDataPtr, Signed16Ptr theIndexValuesPtr)
{
    fWeb_ImageSet_Set(&Web_ImgSet);
    return;
}

rpOneOfSeveral sWeb_ImgSet_Quality_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    return (rpOneOfSeveral)Web_ImgSet.Quality;
}

void sWeb_ImgSet_Quality_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{
    Web_ImgSet.Quality=(int)theValue;
}

rpOneOfSeveral sWeb_ImgSet_Resolution_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    return (rpOneOfSeveral)Web_ImgSet.Resolution;
}

void sWeb_ImgSet_Resolution_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{
    Web_ImgSet.Resolution=(int)theValue;
}

rpOneOfSeveral sWeb_ImgSet_FrameRate_Radio_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    return (rpOneOfSeveral)Web_ImgSet.FrameRate;
}

void sWeb_ImgSet_FrameRate_Radio_Set(void *theTaskDataPtr, rpOneOfSeveral theValue,
        char *theNamePtr, Signed16Ptr theIndexValuesPtr)
{
    Web_ImgSet.FrameRate=(int)theValue;
}

char *sWeb_ImgSet_Language_Get(void *theTaskDataPtr, char *theNamePtr,
        Signed16Ptr theIndexValuesPtr)
{
    int pageString = (int)atoi(theNamePtr);
    int language = fWeb_Language_Get();
    char szTemp[256];

    szTemp[0]=0;

    switch (pageString)
    {
        case 1:
            if(language == 0)
                strcpy(szTemp, "Image Setting");
            else if(language == 1)
                strcpy(szTemp, "影像設定");
            break;

        case 2:
            if(language == 0)
                strcpy(szTemp, "Setting Item");
            else if(language == 1)
                strcpy(szTemp, "設定項目");
            break;

        case 3:
            if(language == 0)
                strcpy(szTemp, "Contents");
            else if(language == 1)
                strcpy(szTemp, "內容");
            break;

        case 4:
            if(language == 0)
                strcpy(szTemp, "Quality");
            else if(language == 1)
                strcpy(szTemp, "畫質");
            break;

        case 5:
            if(language == 0)
                strcpy(szTemp, "H");
            else if(language == 1)
                strcpy(szTemp, "高");
            break;

        case 6:
            if(language == 0)
                strcpy(szTemp, "M");
            else if(language == 1)
                strcpy(szTemp, "中");
            break;

        case 7:
            if(language == 0)
                strcpy(szTemp, "L");
            else if(language == 1)
                strcpy(szTemp, "低");
            break;

        case 8:
            if(language == 0)
                strcpy(szTemp, "Resolution");
            else if(language == 1)
                strcpy(szTemp, "解析度");
            break;

        case 9:
            if(language == 0)
                strcpy(szTemp, "D1");
            else if(language == 1)
                strcpy(szTemp, "D1");
            break;

        case 10:
            if(language == 0)
                strcpy(szTemp, "VGA");
            else if(language == 1)
                strcpy(szTemp, "VGA");
            break;

        case 11:
            if(language == 0)
                strcpy(szTemp, "QVGA");
            else if(language == 1)
                strcpy(szTemp, "QVGA");
            break;

        case 12:
            if(language == 0)
                strcpy(szTemp, "Frame Rate");
            else if(language == 1)
                strcpy(szTemp, "禎率");
            break;

        case 13:
            if(language == 0)
                strcpy(szTemp, "30fps");
            else if(language == 1)
                strcpy(szTemp, "30fps");
            break;

        case 14:
            if(language == 0)
                strcpy(szTemp, "15fps");
            else if(language == 1)
                strcpy(szTemp, "15fps");
            break;

        case 15:
            if(language == 0)
                strcpy(szTemp, "5fps");
            else if(language == 1)
                strcpy(szTemp, "5fps");
            break;

    }
    HPF(NULL);
    return HPF("%s", szTemp);
}
