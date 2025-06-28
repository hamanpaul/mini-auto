/*===============================================
 *FileName : sMain.c
 *Purpose : Get Main Basic information
 *Html : start.html ,Hmain.html
 *Note :
 *Author : Vic Yu 2003-06/17
 *===============================================
 */
#include "AsEngine.h"
#include "web_util.h"
#include "Web_flag.h"
#include "sMain.h"
#include "wb_Menu.h"


/*---------------------------------------------------------------------------
 * Function :WEB_StartPage_TitleDisp_Get()
 * Purpose: Get Device Type name
 * Notes:
 *  Modified by Vic Yu 2003-06/17
 *---------------------------------------------------------------------------
 */
char *sWEB_Main_TitleDisplay_Get(void)
{
	char *sp;
	char deviceType[128];
	
	sp=html_SetBuffer(NULL);
	memset(deviceType, 0, 128);
	
	switch(fWeb_Main_DeviceModel_Get())
	{
		case 0:
			strcpy(deviceType, "A1013");
			break;
		default:
		case 1:
			strcpy(deviceType, "UNKNOW");
			break;
	}
	sprintf(sp, "%s",deviceType);
	
	return sp; 
}

Unsigned8 sWeb_Main_DeviceModel_Get(void *theTaskDataPtr, Signed16Ptr theIndexValuesPtr)
{
	if (fWeb_Main_DeviceModel_Get()==0)
		return 0;
	else
		return 1;
}

char *sWeb_Main_DeviceName_Get(void *theServerDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr)
{
	char chassisName[41];
	char szTemp[256];

	HPF(NULL);

	fWeb_Main_DeviceName_Get((char *)&chassisName);

	//return HPF("%s", chassisName);
	sWeb_DisplayString_Reword(chassisName, (char *)&szTemp);
	return HPF("%s", szTemp);
}

Unsigned8 sWeb_Language_DynamicDisplay(void *theServerDataPtr, Signed16Ptr theIndexValuesPtr)
{
	int language=fWeb_Language_Get();

	return (Unsigned8)language;
}

