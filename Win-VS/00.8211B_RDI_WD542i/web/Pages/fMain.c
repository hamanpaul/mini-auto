/*===============================================
 *FileName : fMain.c
 *Purpose : Get Main Basic information
 *Html : start.html ,Hmain.html
 *Note :
 *Author : Vic Yu 2003-06/17
 *===============================================
 */


int l_language=0;   /*0: English, 1: T_Chinese*/

int fWeb_Main_DeviceModel_Get(void)
{
    return 0;
}

int	fWeb_Main_DeviceName_Get(char *chassisName)
{
	strcpy(chassisName, "XXXXXX");
	return 1;
}

int	fWeb_Language_Get(void)
{
	return l_language;
}

int	fWeb_Language_Set(int language)
{
	l_language=language;
	return 1;//setting success
}


