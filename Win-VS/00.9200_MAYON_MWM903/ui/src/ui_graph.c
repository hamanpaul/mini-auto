/*

  Copyright (c) 2008 Mars Semiconductor Corp.

   Module Name:

   ui_graph.c

   Abstract:

   The routines of user interface.

   Environment:

   ARM RealView Developer Suite

   Revision History:

   2010/12/27   Elsa Lee  Create

   */
#include "general.h"
#include "uiapi.h"
#include "jpegapi.h"
#include "spiapi.h"
#include "fsapi.h"
#include "dcfapi.h"
#include "ui.h"
/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */
#define GUI_MAXSIZE_PERJPG       40960        // 40KB


/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */

/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */ 


/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */  

u32 uiGraphAlphaColorRGB(u32 Color_Input_1,u32 Color_Input_2,u8 alpha) /* Alpha Parameter Range = 1~100 */
{
    u8 Input_1R,Input_1G, Input_1B;
    u8 Input_2R,Input_2G, Input_2B;
    u8 Output_R,Output_G, Output_B;
    u32 Color_Output;
    Input_1R = Color_Input_1 >> 16;  /* Color Separation */
    Input_1G = Color_Input_1 >> 8 & 0xFF;
    Input_1B = Color_Input_1 & 0xFF;
    Input_2R = Color_Input_2 >> 16;
    Input_2G = Color_Input_2 >> 8 & 0xFF;
    Input_2B = Color_Input_2 & 0xFF;
    if (alpha==0)
    {
        return Color_Input_1;
    }
    else if (alpha == 128 )
    {
        return Color_Input_2;
   
    }
    else
    {
        Output_R=((128-alpha)*Input_1R + (alpha)*Input_2R)>>7 ; /*color Mix */
 
        Output_G=((128-alpha)*Input_1G + (alpha)*Input_2G)>>7 ;

        Output_B=((128-alpha)*Input_1B + (alpha)*Input_2B)>>7 ;

        Color_Output = (Output_R << 16 | Output_G << 8 | Output_B);
        return Color_Output;
    }
}

u32 uiGraphAlphaColorYUV(u32 Color_Input_1,u32 Color_Input_2,u8 alpha) /* Alpha Parameter Range = 0~128 */
{

    u8 Input1_1Y,Input1_2Y, Input1_Cb,Input1_Cr;
    u8 Input2_1Y,Input2_2Y, Input2_Cb,Input2_Cr;
    u8 Output_1Y,Output_2Y, Output_Cb,Output_Cr;
    u32 Color_Output;
    if (alpha == 0)
    {
        return Color_Input_1;
    }
    else if (alpha == 128) 
    {
        return Color_Input_2;
    }
    else 
    {
        Input1_Cb = Color_Input_1 >> 24 & 0xFFFFFF;  
        Input1_Cr = Color_Input_1 >> 16 & 0xFFFF; 
        Input1_1Y = Color_Input_1 >> 8 & 0xFF;  
        Input1_2Y =  Color_Input_1 & 0xFF;      

        Input2_Cb = Color_Input_2 >> 24 & 0xFFFFFF;  
        Input2_Cr = Color_Input_2 >> 16 & 0xFFFF; 
        Input2_1Y = Color_Input_2 >> 8 & 0xFF;  
        Input2_2Y =  Color_Input_2& 0xFF;      

        Output_Cb = ((128+(~alpha+1))*Input1_Cb + (alpha*Input2_Cb)>>7) ;  /* ~alpha+1   = -alpha */
        Output_Cr = ((128+(~alpha+1))*Input1_Cr + (alpha*Input2_Cr)>>7) ; 
        Output_1Y = ((128+(~alpha+1))*Input1_1Y + (alpha*Input2_1Y)>>7) ;
        Output_2Y = ((128+(~alpha+1))*Input1_2Y + (alpha*Input2_2Y)>>7) ; 

        Color_Output = (Output_Cb <<24 |Output_Cr<<16 | Output_1Y<<8 | Output_2Y);
     
        return Color_Output;
    }
}

/*
 *********************************************************************************************************
 * Called by UI in other file
 *********************************************************************************************************
 */

void uiGraphColorFilter(u16 pannel_w , u8 *scaler_data , u16 scaler_w , u16 scaler_h , u16 x_pos , u16 y_pos,u32 color_range_start, u32 color_range_end,u8 alpha)
{
    u32 *addr;
    u32 *data;
    u32 index;
    u32 index2;
    u32 i,j;
    u32 sx=0,ex=0;

 /*過濾掉某些特定的顏色實現背景透明效果 */ 
    data = (u32 *)scaler_data;
    addr= (u32 *)PKBuf0;
    
//word align    
    pannel_w >>= 1; 
    scaler_w >>= 1;
    x_pos >>= 1;
    sx = x_pos;
    ex = scaler_w+x_pos;  
#if 1 /*paste pictrue */
    for(j=y_pos;j<(y_pos+scaler_h);j++)
    {
        for(i=sx;i<ex;i++)
        {  
             index =((j*pannel_w)+i);
             index2 =(((j-y_pos)*scaler_w)+(i-sx));
             if ((data[index2]>=color_range_start )&&(data[index2]<=color_range_end))  /*Filter Some color */
             {
                  /*Replace color*/
             } 
             else
             {   
                addr[index] =  uiGraphAlphaColorYUV(data[index2],  addr[index], alpha);  /* front Color & Background mix */
               
             }      
        } 
    }   
 
#endif   
}

/*
Return Value:

    0 - Failure. Decode error.    
    1 - Success.
    2 - Failure. Decode timeout.    

*/
u8 uiGraphDrawJpgGraph(s32 fb_index, u8* pResult, u16* pWidth, u16* pHeight)
{
    u32 sizeUsed;
    u8 *pJpgStartAddr;
    s32 jpeg_raw, ret;

    if(spiUI_OpenFB_ByIndex(fb_index) == 0)
        return 0;	
	
    jpeg_raw = exifFileParse(JPEG_IMAGE_PRIMARY, exifDecBuf, GUI_MAXSIZE_PERJPG, &sizeUsed, pWidth, pHeight);
    pJpgStartAddr = exifDecBuf+sizeUsed;

    
    #if ( (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1019A)||(CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
        if (sysTVOutOnFlag)
        {
            if(jpeg_raw == 2)
            {	
            	ret = jpegRAW420(pJpgStartAddr, pResult,pResult + PNBUF_SIZE_Y , gJPGValidWidth, gJPGValidHeight , TVOUT_X);
                if(ret == 0)
                {
                    DEBUG_UI("Primary JPEG RAW420 decode error!!!\n");
                    return ret;
                }
            }
            else
            {
            	ret = jpegDecompressionYUV420(pJpgStartAddr, pResult,pResult + PNBUF_SIZE_Y,1,TVOUT_X);
				if(ret == 0)	
                {
                    DEBUG_UI("Primary JPEG decode error!!!\n");
                    return ret;
                }
            }
        }
        else
        {
            if(jpeg_raw == 2)
            {
            	ret = jpegRAW420(pJpgStartAddr, pResult,pResult + PNBUF_SIZE_Y , gJPGValidWidth, gJPGValidHeight , UI_MENU_SIZE_X);
            	if(ret == 0)	                
                {
                    DEBUG_UI("Primary JPEG RAW420 decode error!!!\n");
                    return ret;
                }
            }
            else
            {
            	ret = jpegDecompressionYUV420(pJpgStartAddr, pResult,pResult + PNBUF_SIZE_Y,1,UI_MENU_SIZE_X);
                if(ret == 0)	                
                {
                    DEBUG_UI("Primary JPEG decode error!!!\n");
                    return ret;
                }
            }
        }
    #else
       	ret = jpegDecompression(pJpgStartAddr, pResult);
		if(ret == 0)	                	
        {
            DEBUG_UI("Primary JPEG decode error!!!\n");
            return ret;
        }
    #endif
    
    return ret;
}

void uiGraphDrawJPGImage(UI_NODE_PHOTO image, u16 x_pos, u16 y_pos)
{
    u16 uiJpgWidth, uiJpgHeight;
    u8  rteVal = 0, Cnt = 3;

    do
	{
        rteVal = uiGraphDrawJpgGraph(image.bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight);
		if(rteVal != 1)
			DEBUG_UI("File %s Open Fail\r\n",image.FileName);
		
        Cnt--;
        if (Cnt==0)
        {
        	if(rteVal==0)
        	{
	            DEBUG_UI("File %s Open Fail\r\n",image.FileName);
	            return;
        	}
			else
				break;
        }
    }while (rteVal != 1);
	
#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (MyHandler.MenuMode == SET_MASK_AREA)
	    IDU_TVLayer_Stride(1280 ,PKBuf2, gJPGValidWidth, gJPGValidHeight, x_pos, y_pos, UI_MENU_SIZE_X,PKBuf0);
    else 
        IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, x_pos, y_pos, UI_MENU_SIZE_X,PKBuf0);
#else
		IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, x_pos, y_pos, UI_MENU_SIZE_X,PKBuf0);
#endif
		    
}

void uiGraphDrawJPG(s32 index, u16 x_pos, u16 y_pos)
{
    u16 uiJpgWidth, uiJpgHeight;
    u8  rteVal = 0, Cnt = 3;

    do
	{
        rteVal = uiGraphDrawJpgGraph(index, PKBuf2, &uiJpgWidth, &uiJpgHeight);
		if(rteVal != 1)
			DEBUG_UI("File %d Open Fail\r\n",index);
		
        Cnt--;
        if (Cnt==0)
        {
        	if(rteVal==0)
        	{
	            DEBUG_UI("File %d Open Fail\r\n",index);
	            return;
        	}
			else
				break;
        }
    }while (rteVal != 1);
	
#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    if (MyHandler.MenuMode == SET_MASK_AREA)
	    IDU_TVLayer_Stride(1280 ,PKBuf2, gJPGValidWidth, gJPGValidHeight, x_pos, y_pos, UI_MENU_SIZE_X,PKBuf0);
    else 
        IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, x_pos, y_pos, UI_MENU_SIZE_X,PKBuf0);
#elif ((HW_BOARD_OPTION == MR9600_RX_OPCOM_CVI) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) ||\
       (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R))
        IDU_TVLayer_Stride(TVOUT_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, x_pos, y_pos, TVOUT_X,PKBuf0);
#else
		IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, x_pos, y_pos, UI_MENU_SIZE_X,PKBuf0);
#endif
		    
}

/*
 *********************************************************************************************************
 * Called by other moudle
 *********************************************************************************************************
 */
#if (JPEG_DEBUG_ENA_9200)
extern u32 jpeg_debug_mode;
extern u32 jpeg_start_resID;
extern u32 jpeg_end_resID;
extern u32 jpeg_test_count;


bool Random_Pic_fromJpgRes(void)
{
	s32 index;
    u16 uiJpgWidth, uiJpgHeight;	
	static int count_pass=0, count_fail=0;
	int retry=0;
	int result;
	static int count=0;

	sysSPI_Enable();
    sysJPEG_enable();

	index = jpeg_start_resID+(rand()%(jpeg_end_resID-jpeg_start_resID+1));

	do
	{
		result = uiGraphDrawJpgGraph(index, PKBuf1, &uiJpgWidth, &uiJpgHeight); 

		if(result!=1)
		{
			retry++;
		}
	}while(result!=1 && retry<1);

	if(retry==0)
		count_pass++;
	else
	{
		count_pass++;
		count_fail+=retry;
	}

	if(count_fail == 0)
		printf("#####$$$##### resID=%d, total:<%d, %d>\n", index, count_pass, count_fail);
	else
		printf("#####@@@##### resID=%d, total:<%d, %d>\n", index, count_pass, count_fail);

	count++;

	if(count==jpeg_test_count)
	{
		jpeg_debug_mode = 0;
		count = 0;
		count_pass = 0;
		count_fail = 0;
	}

	sysSPI_Disable();
    sysJPEG_disable();
}

bool Sequence_Pic_fromJpgRes(void)
{

	static s32 index=MAX_FRM_OBJ;
    u16 uiJpgWidth, uiJpgHeight;
	static int count_pass=0, count_fail=0;
	int retry=0;
	int result;
	static int count=0;

	sysSPI_Enable();
    sysJPEG_enable();
	
	if(index==MAX_FRM_OBJ)
		index = jpeg_start_resID;
	
	do
	{
		result = uiGraphDrawJpgGraph(index, PKBuf1, &uiJpgWidth, &uiJpgHeight); 
		if(result!=1)
		{
			retry++;
		}
	}while(result!=1 && retry<1);

	if(retry==0)
		count_pass++;
	else
	{
		count_pass++;
		count_fail+=retry;
	}

	count++;

	printf("resID=%d, total:<%d, %d>\n", index, count_pass, count_fail);
	if(count==jpeg_test_count)
	{

		if(count_fail == 0)
			printf("#####$$$##### resID=%d, total:<%d, %d>\n", index, count_pass, count_fail);
		else
			printf("#####@@@##### resID=%d, total:<%d, %d>\n", index, count_pass, count_fail);

		if(index == jpeg_end_resID)
		{
			jpeg_debug_mode = 0;
			index = MAX_FRM_OBJ;
			count = 0;
			count_pass = 0;
			count_fail = 0;
		}
		else
		{
			index++;
			count = 0;
			count_pass = 0;
			count_fail = 0;
		}
	}

	sysSPI_Disable();
    sysJPEG_disable();
}

bool FIX_Pic_fromJpgRes(void)
{

	s32 index=jpeg_start_resID;
    u16 uiJpgWidth, uiJpgHeight; //Lsk: 36~572
	static int count_pass=0, count_fail=0;
	int retry=0;
	int result;
	static int count=0;

	sysSPI_Enable();
    sysJPEG_enable();
	
	do
	{
		result = uiGraphDrawJpgGraph(index, PKBuf1, &uiJpgWidth, &uiJpgHeight); 
		if(result!=1)
		{
			retry++;
		}
	}while(result!=1 && retry<1);

	if(retry==0)
		count_pass++;
	else
	{
		count_pass++;
		count_fail+=retry;
	}

	if(count_fail == 0)
		printf("#####$$$##### resID=%d, total:<%d, %d>\n", index, count_pass, count_fail);
	else
		printf("#####@@@##### resID=%d, total:<%d, %d>\n", index, count_pass, count_fail);

	count++;

	if(count==jpeg_test_count)
	{
		jpeg_debug_mode = 0;
		count = 0;
		count_pass = 0;
		count_fail = 0;
	}

	sysSPI_Disable();
    sysJPEG_disable();
}
#endif

