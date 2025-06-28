#include "AsExtern.h"
#include "AsEngine.h"
#include "wb_Menu.h"
#include "web_util.h"


char *sWeb_Menu_Language_Get(void *theServerDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr)
{
	int pageString = (int)atoi(theNamePtr);
	int language = fWeb_Language_Get();
	char szTemp[256];

	szTemp[0]=0;

	switch (pageString)
	{
		case 1:
			if (language==0)
			    strcpy(szTemp, "<FRAME src=\"./menu.htm?0\" name=\"m\">");
			else if (language==1)
			    strcpy(szTemp, "<FRAME src=\"./menu.htm?1\" name=\"m\">");
			break;
		case 2:
			if (language==0)
				strcpy(szTemp, "<A href=\"menu.htm?1\">Traditional</A>");
			else if (language==1)
				strcpy(szTemp, "<A href=\"menu.htm?0\">English</A>");
			break;
		case 3:
			if (language==0)
				strcpy(szTemp, "Menu");
			else if (language==1)
				strcpy(szTemp, "選單");
			break;
		case 4:
			if (language==0)
				strcpy(szTemp, "Preview");
			else if (language==1)
				strcpy(szTemp, "預覽");
			break;
		case 5:
			if (language==0)
				strcpy(szTemp, "Quick Setting");
			else if (language==1)
				strcpy(szTemp, "快速設定");
			break;
		case 6:
			if (language==0)
				strcpy(szTemp, "Channel Control");
			else if (language==1)
				strcpy(szTemp, "頻道控制");
			break;
		case 7:
			if (language==0)
				strcpy(szTemp, "Volume Control");
			else if (language==1)
				strcpy(szTemp, "音量控制");
			break;
		case 8:
			if (language==0)
				strcpy(szTemp, "Image Adjustment");
			else if (language==1)
				strcpy(szTemp, "影像調整");
			break;
		case 20:
			if (language==0)
				strcpy(szTemp, "General Setting");
			else if (language==1)
				strcpy(szTemp, "一般設定");
			break;
		case 21:
			if (language==0)
				strcpy(szTemp, "REC. Setting");
			else if (language==1)
				strcpy(szTemp, "錄影設定");
			break;
		case 22:
			if (language==0)
				strcpy(szTemp, "Display Setting");
			else if (language==1)
				strcpy(szTemp, "顯示設定");
			break;
		case 23:
			if (language==0)
				strcpy(szTemp, "Image Setting");
			else if (language==1)
				strcpy(szTemp, "影像設定");
			break;
		case 24:
			if (language==0)
				strcpy(szTemp, "System Setting");
			else if (language==1)
				strcpy(szTemp, "系統設定");
			break;
		case 25:
			if (language==0)
				strcpy(szTemp, "Network Setting");
			else if (language==1)
				strcpy(szTemp, "網路設定");
			break;
		case 26:
			if (language==0)
				strcpy(szTemp, "Schedule Time");
			else if (language==1)
				strcpy(szTemp, "預約時間");
			break;
		case 27:
			if (language==0)
				strcpy(szTemp, "System Info.");
			else if (language==1)
				strcpy(szTemp, "系統資訊");
			break;
		case 28:
			if (language==0)
				strcpy(szTemp, "Event");
			else if (language==1)
				strcpy(szTemp, "事件");
			break;
		case 29:
			if (language==0)
				strcpy(szTemp, "Playback");
			else if (language==1)
				strcpy(szTemp, "回放");
			break;
		case 30:
			if (language==0)
				strcpy(szTemp, "Playback List");
			else if (language==1)
				strcpy(szTemp, "回放清單");
			break;
        case 69:
			if (language==0)
				strcpy(szTemp, "charset=iso-8859-1");
			else if (language==1)
				strcpy(szTemp, "charset=Traditional");
			break;
	}

	HPF(NULL);
	return HPF("%s", szTemp);
}


char *sWeb_Menu_Language_Set(void *theServerDataPtr, char *theNamePtr,
		Signed16Ptr theIndexValuesPtr)
{
	int language=(int)(*(theIndexValuesPtr+0)+1);
	int current_language=fWeb_Language_Get();

	if (current_language!=language)
		fWeb_Language_Set(language);
	return NULL;
}

