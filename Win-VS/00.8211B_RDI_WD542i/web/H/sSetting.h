

/*---------------------------------------------------------------------------
 * RecSetting.html
 *---------------------------------------------------------------------------
 */
typedef struct _sWeb_Set_RecSetting {
	int		Manual;     /*0:Disable  1:Enable*/
	int		Schedule;   /*0:Disable  1:Enable*/
	int		MotionDet;  /*0:Disable  1:Enable*/
	int		AlarmDet;   /*0:Disable  1:Enable*/
    int     Section;    /*0:15min, 1:30min, 2:60min*/
	int		Sensitivity;/*0:H, 1:M, 2:L*/
} sWeb_Set_RecSetting;

int fWeb_RecSet_Get(int channel, sWeb_Set_RecSetting *data);
int fWeb_RecSet_Set(int channel, sWeb_Set_RecSetting *data);


/*---------------------------------------------------------------------------
 * DisplaySetting.html
 *---------------------------------------------------------------------------
 */
typedef struct _sWeb_Set_DisplaySetting {
	int		AutoChannel;    /*0~15Sec*/
	int		OSDTime;        /*0:Disable  1:Enable*/
	int		OSDChannel;     /*0:Disable  1:Enable*/
	int		OSDCard;        /*0:Disable  1:Enable*/
} sWeb_Set_DisplaySetting;

int fWeb_DisplaySet_Get(sWeb_Set_DisplaySetting *data);
int fWeb_DisplaySet_Set(sWeb_Set_DisplaySetting *data);

/*---------------------------------------------------------------------------
 * ImageSetting.html
 *---------------------------------------------------------------------------
 */
typedef struct _sWeb_Set_ImageSetting {
	int		Quality;        /*0:H, 1:M, 2:L*/
	int		Resolution;     /*0:D1  1:VGA 2:QVGA*/
	int		FrameRate;      /*0:30fps  1:15fps 2:5fps*/
} sWeb_Set_ImageSetting;

int fWeb_ImageSet_Get(sWeb_Set_ImageSetting *data);
int fWeb_ImageSet_Set(sWeb_Set_ImageSetting *data);

