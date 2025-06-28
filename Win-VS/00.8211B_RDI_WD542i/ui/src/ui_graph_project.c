/*

  Copyright (c) 2008 Mars Semiconductor Corp.

   Module Name:

   ui_graph_project.c

   Abstract:

   The routines of user interface.

   Environment:

   ARM RealView Developer Suite

   Revision History:

   2010/12/27   Elsa Lee  Create

   */
#include "general.h"
#include "board.h"
#include "uiapi.h"
#include "ui.h"
#include "dcfapi.h"
#include "jpegapi.h"
#include "ui_project.h"
#include "spiapi.h"
#include "sysapi.h"
#include "gpioapi.h"
/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */
#define UI_MENU_SIZE_X           640

/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */


UI_NODE_PHOTO TimeNum[20] =
{
    {"time00.jpg",0},
    {"time01.jpg",0},
    {"time02.jpg",0},
    {"time03.jpg",0},
    {"time04.jpg",0},
    {"time05.jpg",0},
    {"time06.jpg",0},
    {"time07.jpg",0},
    {"time08.jpg",0},
    {"time09.jpg",0},
    {"time10.jpg",0},
    {"time11.jpg",0},
    {"time12.jpg",0},
    {"time13.jpg",0},
    {"time14.jpg",0},
    {"time15.jpg",0},
    {"time16.jpg",0},
    {"time17.jpg",0},
    {"time18.jpg",0},
    {"time19.jpg",0}
};

UI_NODE_PHOTO TimeFram[4] =
{
    {"yf1.jpg",0},
    {"yf2.jpg",0},
    {"df1.jpg",0},
    {"df2.jpg",0},
};


UI_NODE_PHOTO PairNum[10]=
{
    {"time00.jpg",0},
    {"time01.jpg",0},
    {"time02.jpg",0},
    {"time03.jpg",0},
    {"time04.jpg",0},
    {"time05.jpg",0},
    {"time06.jpg",0},
    {"time07.jpg",0},
    {"time08.jpg",0},
    {"time09.jpg",0},
};

UI_NODE_PHOTO Colon = {"colon.jpg",0};
UI_NODE_PHOTO Slash = {"slash.jpg",0};

UI_NODE_PHOTO OK_1              = {"ok_m1.jpg",0};
UI_NODE_PHOTO OK_2              = {"ok_m2.jpg",0};
UI_NODE_PHOTO ON_1              = {"on_m1.jpg",0};
UI_NODE_PHOTO ON_2              = {"on_m2.jpg",0};
UI_NODE_PHOTO OFF_1             = {"off_m1.jpg",0};
UI_NODE_PHOTO OFF_2             = {"off_m2.jpg",0};
UI_NODE_PHOTO IP_Address        = {"ip_addr.jpg",0};
UI_NODE_PHOTO Default_Gateway   = {"dft_gate.jpg",0};
UI_NODE_PHOTO Subnet_mask       = {"sub_mask.jpg",0};
UI_NODE_PHOTO IP_SET_BLOCK      = {"ip_set.jpg",0};
UI_NODE_PHOTO Static_IP_1       = {"staip_m1.jpg",0};
UI_NODE_PHOTO Static_IP_2       = {"staip_m2.jpg",0};
UI_NODE_PHOTO Dynamic_IP_1      = {"dynip_m1.jpg",0};
UI_NODE_PHOTO Dynamic_IP_2      = {"dynip_m2.jpg",0};    
UI_NODE_PHOTO Baseline          = {"baseline.jpg",0};
UI_NODE_PHOTO NoSignal          = {"nosignal.jpg",0};
UI_NODE_PHOTO PairText          = {"pair_txt.jpg",0};  
UI_NODE_PHOTO PairBG            = {"pair_bg.jpg",0};      
UI_NODE_PHOTO number[10]=
{
    {"a_10.jpg",0},     /* 0 */
    {"a_01.jpg",0},     /* 1 */
    {"a_02.jpg",0},     /* 2 */
    {"a_03.jpg",0},     /* 3 */
    {"a_04.jpg",0},     /* 4 */
    {"a_05.jpg",0},     /* 5 */
    {"a_06.jpg",0},     /* 6 */
    {"a_07.jpg",0},     /* 7 */
    {"a_08.jpg",0},     /* 8 */
    {"a_09.jpg",0},     /* 9 */
        
};

UI_NODE_PHOTO number_select[10]=
{
    {"as_10.jpg",0},    /* 0 */
    {"as_01.jpg",0},    /* 1 */
    {"as_02.jpg",0},    /* 2 */
    {"as_03.jpg",0},    /* 3 */
    {"as_04.jpg",0},    /* 4 */
    {"as_05.jpg",0},    /* 5 */
    {"as_06.jpg",0},    /* 6 */
    {"as_07.jpg",0},    /* 7 */
    {"as_08.jpg",0},    /* 8 */
    {"as_09.jpg",0},    /* 9 */        
};



u8 uiIPAddr[4]        ={192,168,1,118};
u8 uiSubnetMask[4]    ={255,255,255,0};
u8 uiDefaultGateway[4]={192,168,1,1};
u8 uiISStatic=0; 
u8 ipAddrSetting[12]         ={1,9,2,1,6,8,0,0,1,1,1,8};
u8 subMaskSetting[12]        ={2,5,5,2,5,5,2,5,5,0,0,0};
u8 defaultGatewaySetting[12] ={1,9,2,1,6,8,0,0,1,0,0,1};
u8 uiIpAddrInfo[4]={0};
u8 uiSubMaskInfo[4]={0};
u8 uiDefaultGatewayInfo[4]={0};



enum
{
    UI_NET_S_BACKGROUND=0,         /* 0 */
    UI_NET_S_STATICIP_1,
    UI_NET_S_DYNAMICIP_2,
    UI_NET_S_IP_ADDRESS,
    UI_NET_S_SUBNET_MASK,
    UI_NET_S_DEFAULT_GATEWAY,      /* 5 */
    UI_NET_S_IP_BLOCK_1,
    UI_NET_S_IP_BLOCK_2,
    UI_NET_S_IP_BLOCK_3,
    UI_NET_S_OK_2,
    UI_NET_S_OK_1,                 /* 10 */

    UI_NET_STATIC_ICON_COUNT,  /*keep last */
    
};

enum
{
    UI_NET_D_BACKGROUND=0,         /* 0 */
    UI_NET_D_STATICIP_1,
    UI_NET_D_DYNAMICIP_2,
    UI_NET_D_OK_2,
    UI_NET_D_OK_1,
    
    UI_NET_DYNAMIC_ICON_COUNT    /*keep last */
};

UI_NODE_SUB_FILE uistaticIPSetting[UI_NET_STATIC_ICON_COUNT];
UI_NODE_SUB_FILE uidynamicIPSetting[UI_NET_DYNAMIC_ICON_COUNT];

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
 extern UI_NODE_PHOTO Background[UI_MULT_LANU_END];
 extern u8 uiIsRFParing[RFID_MAX_WORD];   // 1:Pairing 0:Not Pairing 
/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */
extern u8 uiGraphDrawJpgGraph(s32 fb_index, u8* pResult, u16* pWidth, u16* pHeight);
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


void uiGraphGetMenuData(void)
{
    u32 i;
    UI_NODE_DATA *draw_data;
    static UI_NODE_DATA  last_data;
    u16 uiJpgWidth, uiJpgHeight;

#if ((FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))
    draw_data = uiCurrNode->item.NodeData;
    sysSD_Disable();
    sysSPI_Enable();
    sysJPEG_enable();

    for (i = 0; i < draw_data->FileNum; i++)
    {
        if((MyHandler.WhichKey == UI_KEY_UP) || (MyHandler.WhichKey == UI_KEY_DOWN)||(MyHandler.WhichKey == UI_KEY_LEFT)||(MyHandler.WhichKey == UI_KEY_RIGHT))
        {
            if(draw_data->FileData[i].FileInfo[CurrLanguage].bufIndex == last_data.FileData[i].FileInfo[CurrLanguage].bufIndex)
                continue;
        }
        if(uiGraphDrawJpgGraph(draw_data->FileData[i].FileInfo[CurrLanguage].bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 0)
        {
            DEBUG_UI("File %s Open Fail\r\n",draw_data->FileData[i].FileInfo[CurrLanguage].FileName);
            continue;
        }
        IDU_TVLayer(UI_MENU_SIZE_X ,PKBuf2, uiJpgWidth, uiJpgHeight,
                draw_data->FileData[i].Location_x,
                draw_data->FileData[i].Location_y);
    }
    sysJPEG_disable();
    sysSPI_Disable();
    sysSD_Enable();
    memcpy(&last_data, draw_data, sizeof(UI_NODE_DATA));
#endif
}


void uiGraphDrawTimeGraph(s8 setCursor, RTC_DATE_TIME* drawTime, u8 Act, u16 x_pos, u16 y_pos)
{
    u8  numIndex[2], i, Num;
    u16 uiJpgWidth, uiJpgHeight;

    switch((setCursor%5))
    {
        case 0: /*year*/
            Num = drawTime->year;
            break;

        case 1: /*month*/
            Num = drawTime->month;
            break;

        case 2: /*day*/
            Num = drawTime->day;
            break;

        case 3: /*hour*/
            Num = drawTime->hour;
            break;

        case 4: /*min*/
            Num = drawTime->min;
            break;

    }
    numIndex[0] = (Num/10)+(Act*10);
    numIndex[1] = (Num%10)+(Act*10);
    sysSD_Disable();
    sysSPI_Enable();
    sysJPEG_enable();
    for (i = 0; i < 2; i++)
    {
        if(uiGraphDrawJpgGraph(TimeNum[numIndex[i]].bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 0)
        {
            DEBUG_UI("File %s Open Fail\r\n",TimeNum[numIndex[i]].FileName);
            continue;
        }
        IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, (x_pos+i*24), y_pos, uiJpgWidth,PKBuf0);
    }
    sysJPEG_disable();
    sysSPI_Disable();
    sysSD_Enable();
}

void uiGraphDrawTimeFrameGraph(s8 setCursor, u8 Act, u16 x_pos, u16 y_pos)
{
    u8 index = 0;
    u16 uiJpgWidth, uiJpgHeight;

    if((setCursor %5) == 0)
        index = Act;
    else
        index = 2+Act;

    sysSD_Disable();
    sysSPI_Enable();
    sysJPEG_enable();
    if(uiGraphDrawJpgGraph(TimeFram[index].bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 0)
    {
        DEBUG_UI("File %s Open Fail\r\n",TimeFram[index].FileName);
    }
    else
        IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, x_pos, y_pos, uiJpgWidth,PKBuf0);
    sysJPEG_disable();
    sysSPI_Disable();
    sysSD_Enable();
}

void uiGraphDrawSelectDateTime(RTC_DATE_TIME* date, u8 index, u8 opt)
{
    u8 leap_year;
    switch (index)
    {
        case 0: /*year*/
            if (opt == 1)   /*+*/
            {
                if (date->year >= 63)
                    date->year = 0;
                else
                    date->year++;
            }
            else if (opt == 2)  /*-*/
            {
                if (date->year == 0)
                    date->year = 63;
                else
                    date->year--;
            }
            leap_year = date->year%4;
            if (date->month == 2 && leap_year != 0 && date->day == 29)
                date->day = 28;
            break;

        case 1: /*month*/
            if (opt == 1)   /*+*/
            {
                if (date->month >= 12)
                    date->month = 1;
                else
                    date->month++;
            }
            else if (opt == 2)  /*-*/
            {
                if (date->month <= 1)
                    date->month = 12;
                else
                    date->month--;
            }
            switch(date->month)
            {
                case 2:
                    leap_year = date->year%4;
                    if (leap_year == 0 && date->day > 29)
                        date->day = 29;
                    else if (leap_year != 0 && date->day > 28)
                        date->day = 28;
                    break;

                case 4:
                case 6:
                case 9:
                case 11:
                    if (date->day > 30)
                        date->day = 30;
                    break;
                default:
                    break;

            }
            break;

        case 2: /*day*/
            if (opt == 1)   /*+*/
                date->day++;
            else if (opt == 2)  /*-*/
                date->day--;

            switch(date->month)
            {
                case 2:
                    leap_year = date->year%4;
                    if (leap_year == 0)
                    {
                        if (date->day > 29)
                            date->day = 1;
                        else if (date->day == 0)
                            date->day = 29;
                    }
                    else
                    {
                        if (date->day > 28)
                            date->day = 1;
                        else if (date->day == 0)
                            date->day = 28;
                    }
                    break;

                case 4:
                case 6:
                case 9:
                case 11:
                    if (date->day > 30)
                        date->day = 1;
                    else if (date->day == 0)
                        date->day = 30;
                    break;

                default:
                    if (date->day >31)
                        date->day = 1;
                    else if (date->day == 0)
                        date->day = 31;
                    break;
            }
            break;

        case 3: /*hour*/
            if (opt == 1)   /*+*/
            {
                if (date->hour >= 23)
                    date->hour = 0;
                else
                    date->hour++;
            }
            else if (opt == 2)  /*-*/
            {
                if (date->hour == 0)
                    date->hour = 23;
                else
                    date->hour--;
            }
            break;

        case 4: /*min*/
            if (opt == 1)   /*+*/
            {
                if (date->min >= 59)
                    date->min = 0;
                else
                    date->min++;
            }
            else if (opt == 2)  /*-*/
            {
                if (date->min == 0)
                    date->min = 59;
                else
                    date->min--;
            }
            break;

        default:
            DEBUG_UI("uiGraphDrawSelectDateTime error index %d\r\n",index);
            return;
    }
}

void uiGraphSetStaticInitialPosition(void)
{
    u16 ip_xpos=28;            /* ip_addr.jpg x position */ 
    u16 ip_set_xpos=188;        /* ip_set.jpg  x position */
    u8  ip_y_spacing=108;        /* IP block的y間距 */
    u8 i=0,idx=0;;

    // 0
    uistaticIPSetting[idx].FileInfo=&Background[0];
    uistaticIPSetting[idx].Location_x=0;
    uistaticIPSetting[idx].Location_y=0;
    idx++;

    // 1
    uistaticIPSetting[idx].FileInfo=&Static_IP_1;
    uistaticIPSetting[idx].Location_x=152;
    uistaticIPSetting[idx].Location_y=30;
    idx++;

    // 2
    uistaticIPSetting[idx].FileInfo=&Dynamic_IP_2;
    uistaticIPSetting[idx].Location_x=364;
    uistaticIPSetting[idx].Location_y=30;
    idx++;

    // 3
    uistaticIPSetting[idx].FileInfo=&IP_Address;
    uistaticIPSetting[idx].Location_x=ip_xpos;
    uistaticIPSetting[idx].Location_y=132;
    idx++;

    // 4
    uistaticIPSetting[idx].FileInfo=&Subnet_mask;
    uistaticIPSetting[idx].Location_x=ip_xpos;
    uistaticIPSetting[idx].Location_y=240;
    idx++;

    // 5
    uistaticIPSetting[idx].FileInfo=&Default_Gateway;
    uistaticIPSetting[idx].Location_x=ip_xpos;
    uistaticIPSetting[idx].Location_y=348;
    idx++;

    // 6~8
    for(i=0;i<3;i++)
    {
        uistaticIPSetting[idx].FileInfo=&IP_SET_BLOCK;
        uistaticIPSetting[idx].Location_x=ip_set_xpos;
        uistaticIPSetting[idx].Location_y=132+i*ip_y_spacing;
        idx++;
    }

    // 9
    uistaticIPSetting[idx].FileInfo=&OK_2;
    uistaticIPSetting[idx].Location_x=500;
    uistaticIPSetting[idx].Location_y=410;
    idx++;

    // 11
    uistaticIPSetting[idx].FileInfo=&OK_1;
    uistaticIPSetting[idx].Location_x=500;
    uistaticIPSetting[idx].Location_y=410;
    
}

void uiGraphSetDynamicInitialPosition(void)
{
    u8 idx=0;
    
    // 0
    uidynamicIPSetting[idx].FileInfo=&Background[0];
    uidynamicIPSetting[idx].Location_x=0;
    uidynamicIPSetting[idx].Location_y=0;
    idx++;

    // 1
    uidynamicIPSetting[idx].FileInfo=&Static_IP_2;
    uidynamicIPSetting[idx].Location_x=152;
    uidynamicIPSetting[idx].Location_y=30;
    idx++;

    // 2
    uidynamicIPSetting[idx].FileInfo=&Dynamic_IP_1;
    uidynamicIPSetting[idx].Location_x=364;
    uidynamicIPSetting[idx].Location_y=30;
    idx++;

    // 3
    uidynamicIPSetting[idx].FileInfo=&OK_2;
    uidynamicIPSetting[idx].Location_x=500;
    uidynamicIPSetting[idx].Location_y=410;
    idx++;

    // 4
    uidynamicIPSetting[idx].FileInfo=&OK_1;
    uidynamicIPSetting[idx].Location_x=500;
    uidynamicIPSetting[idx].Location_y=410;
    idx++;
    
}

void uiGraphDrawStaticIPBackground(u8 mode)
{
    u16 uiJpgWidth, uiJpgHeight;
    u8 i;

    switch(mode)
    {
        case 0xFF:
            for(i=0;i<(UI_NET_S_OK_2+1);i++)
            {
                if(uiGraphDrawJpgGraph(uistaticIPSetting[i].FileInfo->bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 1)
                    IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, uistaticIPSetting[i].Location_x, uistaticIPSetting[i].Location_y, uiJpgWidth,PKBuf0);        

                else
                    DEBUG_UI("File %s Open Fail\r\n",uistaticIPSetting[i].FileInfo->FileName);
            }  
        break;

        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            if(uiGraphDrawJpgGraph(uistaticIPSetting[mode].FileInfo->bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 1)
                IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, uistaticIPSetting[mode].Location_x, uistaticIPSetting[mode].Location_y, uiJpgWidth,PKBuf0);        

            else
                DEBUG_UI("File %s Open Fail\r\n",uistaticIPSetting[mode].FileInfo->FileName);
        break;
            
         
    }
      
}

void uiGraphDrawDynamicIPBackground(u8 mode)
{
    u16 uiJpgWidth, uiJpgHeight;
    u8 i;
    switch(mode)
    {
        case 0xFF:
            for(i=0;i<(UI_NET_D_OK_2+1);i++)
            {
                if(uiGraphDrawJpgGraph(uidynamicIPSetting[i].FileInfo->bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 1)
                    IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, uidynamicIPSetting[i].Location_x, uidynamicIPSetting[i].Location_y, uiJpgWidth,PKBuf0);        

                else
                    DEBUG_UI("File %s Open Fail\r\n",uidynamicIPSetting[i].FileInfo->FileName);
            } 
            break;
            
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            if(uiGraphDrawJpgGraph(uidynamicIPSetting[mode].FileInfo->bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 1)
                IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, uidynamicIPSetting[mode].Location_x, uidynamicIPSetting[mode].Location_y, uiJpgWidth,PKBuf0);        

            else
                DEBUG_UI("File %s Open Fail\r\n",uidynamicIPSetting[mode].FileInfo->FileName);
       
            break;
        
    }
    
}
void uiGraphSaveStaticIP(u8 select)
{
    u16 num;
    u8 j,idx=0;    
    u8 ip_s[12]={0};
    u8 ip_u[4]={0};

    switch(select)
    {
        case 0:
            memcpy((void *)ip_s,(void *)ipAddrSetting,12);
            break;
        case 1:
            memcpy((void *)ip_s,(void *)subMaskSetting,12);
            break;
        case 2:
            memcpy((void *)ip_s,(void *)defaultGatewaySetting,12);
            break;           
    }
    
    for(j=0;j<4;j++)
    {
        num=0;          
        num+=(ip_s[idx]*100);
        idx++;
        num+=(ip_s[idx]*10);
        idx++;
        num+=ip_s[idx];        
        idx++;
        ip_u[j]=num;        
            
    }

     
    switch(select)
    {
        case 0:
            memcpy((void *)uiIPAddr,(void *)ip_u,4);
            break;
        case 1:
            memcpy((void *)uiSubnetMask,(void *)ip_u,4);
            break;
        case 2:
            memcpy((void *)uiDefaultGateway,(void *)ip_u,4);
            break;           
    }
    
}

void uiGraphRestorStaticIP(void)
{
    u8 i,j,num,value;
    u8 *ip_set;
    u8 *ip;
    for(i=0;i<3;i++)
    {
        switch(i)
        {
            case 0:
                ip_set=ipAddrSetting;
                ip=uiIPAddr;
                break;
            case 1:
                ip_set=subMaskSetting;
                ip=uiSubnetMask;
                break;
            case 2:
                ip_set=defaultGatewaySetting;
                ip=uiDefaultGateway;
                break;           
        }

        for(j=0;j<4;j++)
        {
            value=ip[j];
            num=0;
            
            num=value/100;
            ip_set[j*3]=num;
            value=value-num*100;

            num=value/10;
            ip_set[j*3+1]=num;
            value=value-num*10;

            num=value;
            ip_set[j*3+2]=num;
            
        }

        
    }
}
void uiGraphCheckStaticIPOverflow(void)
{
    u8 i,j;
    u16 sub_ip;
    u8 *ip;
    for(i=0;i<3;i++)
    {
        switch(i)
        {
            case 0:
                ip=ipAddrSetting;
                
                break;
            case 1:
                ip=subMaskSetting;
                
                break;
            case 2:
                ip=defaultGatewaySetting;
                
                break;           
        }
        for(j=0;j<4;j++)
        {
            sub_ip=0;
            sub_ip+=(ip[j*3]*100);
            sub_ip+=(ip[j*3+1]*10);
            sub_ip+=ip[j*3+2];

            if(sub_ip>255)
            {                
                ip[j*3+1]=5;
                ip[j*3+2]=5;
            }
        }
    }
}

void uiGraphDrawStaticIPNumberBlockSelect(u8 select)
{
    u16 uiJpgWidth, uiJpgHeight;
    u16 ip_num_start_xpos=216;  /* ip 數字的起始 x position */
    u16 ip_num_start_ypos=152;  /* ip 數字的起始 y position */
    u8  ip_num_x_spacing=16;      /* IP數字的x間距 */
    u8  ip_y_spacing=108;        /* IP的y間距 */
    u8  ip_x_spacing=63;        /* IP的x間距 */   
    u8  single_digit,ten_digit,hundred_digit; /* 個位數,十位數,百位數 */
    u8  i,j,idx;
    for(i=0;i<4;i++)
    {

        switch(select)
        {
            case 0:  /* IP Address */
                hundred_digit=ipAddrSetting[i*3];
                ten_digit=ipAddrSetting[i*3+1];
                single_digit=ipAddrSetting[i*3+2];
                break;
            case 1:  /* Subnet Mask */
                hundred_digit=subMaskSetting[i*3];
                ten_digit=subMaskSetting[i*3+1];
                single_digit=subMaskSetting[i*3+2];                
                break;
            case 2:  /* Default Gateway */
                hundred_digit=defaultGatewaySetting[i*3];
                ten_digit=defaultGatewaySetting[i*3+1];
                single_digit=defaultGatewaySetting[i*3+2];
                break;
        }

        
        
        /* 個位數 */
        if((hundred_digit==0)&&(ten_digit==0))
        {
            idx=single_digit;
            if(uiGraphDrawJpgGraph(number[idx].bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 1)
                IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, ip_num_start_xpos+ip_x_spacing*i+ip_num_x_spacing*2, ip_num_start_ypos+ip_y_spacing*select, uiJpgWidth,PKBuf0);        
            else
                DEBUG_UI("File %s Open Fail\r\n",number[idx].FileName); 
        }
        /* 兩位數 */
        else if(hundred_digit==0)
        {
            for(j=0;j<2;j++)
            {
                switch (j)
                {                            
                    case 0:
                        idx=ten_digit;
                        break;
                    case 1:
                        idx=single_digit;
                        break;
                }
                if(uiGraphDrawJpgGraph(number[idx].bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 1)
                    IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, ip_num_start_xpos+ip_x_spacing*i+ip_num_x_spacing*(j+1), ip_num_start_ypos+ip_y_spacing*select, uiJpgWidth,PKBuf0);        
                else
                    DEBUG_UI("File %s Open Fail\r\n",number[idx].FileName); 
                
            }  
        }
        /* 三位數 */
        else
        {
            for(j=0;j<3;j++)
            {
                switch (j)
                {
                    case 0:
                        idx=hundred_digit;
                        break;
                    case 1:
                        idx=ten_digit;
                        break;
                    case 2:
                        idx=single_digit;
                        break;
                }
                if(uiGraphDrawJpgGraph(number[idx].bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 1)
                    IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, ip_num_start_xpos+ip_x_spacing*i+ip_num_x_spacing*j, ip_num_start_ypos+ip_y_spacing*select, uiJpgWidth,PKBuf0);        
                else
                    DEBUG_UI("File %s Open Fail\r\n",number[idx].FileName); 
                
            }  
        }
                        

    }
}

void uiGraphDrawStaticIPNumberSelect(u8 location)
{   
    u16 uiJpgWidth, uiJpgHeight;
    u16 ip_num_start_xpos=216;  /* ip 數字的起始 x position */
    u16 ip_num_start_ypos=152;  /* ip 數字的起始 y position */
    u8  ip_num_x_spacing=16;      /* IP數字的x間距 */
    u8  ip_y_spacing=108;        /* IP的y間距 */
    u8  ip_x_spacing=63;        /* IP的x間距 */  
    u8  idx,i,j,k;
    
    switch((location/12))
    {
        case 0:
            k=0;
            idx=ipAddrSetting[location%12];
        break;
        case 1:
            k=1;
            idx=subMaskSetting[location%12];
        break;
        case 2:
            k=2;
            idx=defaultGatewaySetting[location%12];
        break;
    }
    
    i=(location%12)/3;
    j=(location%12)%3;
    if(uiGraphDrawJpgGraph(number_select[idx].bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 1)
        IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, ip_num_start_xpos+ip_x_spacing*i+ip_num_x_spacing*j, ip_num_start_ypos+ip_y_spacing*k, uiJpgWidth,PKBuf0);        
    else
        DEBUG_UI("File %s Open Fail\r\n",number_select[idx].FileName); 
    
}
void uiGraphDrawStaticIPNumber(u8 mode)
{
    u8 i;

    switch(mode)
    {
        /* 3個block 都畫 */
        case 0xFF:
            
            for(i=0;i<3;i++)
            {   
                uiGraphDrawStaticIPNumberBlockSelect(i);
            } 
            break;

        /* 只畫特定的block */
        case 0:
        case 1:
        case 2:
            uiGraphDrawStaticIPNumberBlockSelect(mode);
            break;
        
    }
   
    
}
u8 uiGraphDrawStaticIP(u8 key)
{
    u16 uiJpgWidth, uiJpgHeight;
    u16 baseline_start_xpos = 216;
    u16 baseline_start_ypos = 177;
    u8  ip_num_x_spacing=16;      /* IP數字的x間距 */
    u8  ip_y_spacing=108;        /* IP的y間距 */
    u8  ip_x_spacing=63;        /* IP的x間距 */    
    u8  *ip_set;
    u8  i,j,k;
    s8  temp;
    
    static s8 setCursor=0;
    sysSD_Disable();
    sysSPI_Enable();
    sysJPEG_enable();

    switch (key)
    {
        case 0:
            setCursor=0;   
            uiGraphDrawStaticIPBackground(0xFF);
            uiGraphDrawStaticIPNumber(0xFF);              
        break;
        case 0xFF:
            uiGraphDrawStaticIPNumberSelect(0);
            if(uiGraphDrawJpgGraph(Baseline.bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 1)
                IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, baseline_start_xpos, baseline_start_ypos, uiJpgWidth,PKBuf0);        
            else
                DEBUG_UI("File %s Open Fail\r\n",Baseline.FileName);     
        break;

        case UI_KEY_RIGHT:            
        case UI_KEY_LEFT:
            if(key==UI_KEY_RIGHT)
            {
                setCursor++;
                if(setCursor>=37)
                    setCursor=0;
            }
            else
            {
                setCursor--;
                if(setCursor<0)
                    setCursor=36;
            }

            /* 判斷是否有超過255*/
            if(setCursor%3==0)
                uiGraphCheckStaticIPOverflow();
            
            if(setCursor!=36)
            {
                                
                i=(setCursor%12)/3;
                j=(setCursor%12)%3;
                k=setCursor/12;
                
                /* clear */
                if((setCursor==0)||(setCursor==35))
                {
                    uiGraphDrawStaticIPBackground(UI_NET_S_OK_2); 
                }
                if(((setCursor%12)==0)&&(key==UI_KEY_RIGHT))
                {
                    
                    temp=(setCursor/12)-1;
  
                    if(temp<0)
                        temp=2;
                    uiGraphDrawStaticIPBackground(temp+UI_NET_S_IP_BLOCK_1);
                    uiGraphDrawStaticIPNumber(temp); 
                }
                else if(((setCursor%12)==11)&&(key==UI_KEY_LEFT))
                {
                    temp=(setCursor/12)+1;
  
                    if(temp>2)
                        temp=0;
                    uiGraphDrawStaticIPBackground(temp+UI_NET_S_IP_BLOCK_1);
                    uiGraphDrawStaticIPNumber(temp); 
                }
                else
                {
                    uiGraphDrawStaticIPBackground(k+UI_NET_S_IP_BLOCK_1);
                    uiGraphDrawStaticIPNumber(k);
                }
                
                /*draw baseline */
                if(uiGraphDrawJpgGraph(Baseline.bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 1)
                    IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, baseline_start_xpos+ip_x_spacing*i+ip_num_x_spacing*j, baseline_start_ypos+ip_y_spacing*k, uiJpgWidth,PKBuf0);        
                else
                    DEBUG_UI("File %s Open Fail\r\n",Baseline.FileName);
                
                uiGraphDrawStaticIPNumberSelect(setCursor);
            }
            else
            {
                if(key==UI_KEY_RIGHT)
                {
                    uiGraphDrawStaticIPBackground(UI_NET_S_IP_BLOCK_3);
                    uiGraphDrawStaticIPNumber(2);     
                }
                else
                {
                    uiGraphDrawStaticIPBackground(UI_NET_S_IP_BLOCK_1);
                    uiGraphDrawStaticIPNumber(0);   
                }
                uiGraphDrawStaticIPBackground(UI_NET_S_OK_1); 
            }
            
                
            break;
        case UI_KEY_UP:
            if(setCursor!=36)
            {
                
                switch(setCursor/12)
                {
                    case 0:
                        ip_set=ipAddrSetting;
                        break;
                    case 1:
                        ip_set=subMaskSetting;
                        break;
                    case 2:
                        ip_set=defaultGatewaySetting;
                        break;
                }
                /* 避免百位數超過2 */
                if(((setCursor%12)%3) == 0)
                {
                    ip_set[setCursor%12]++;
                    if(ip_set[setCursor%12]>2)
                        ip_set[setCursor%12]=0;
                }
                else
                {
                    ip_set[setCursor%12]++;                    
                    if(ip_set[setCursor%12] >9)
                        ip_set[setCursor%12]=0;
                }
                
                uiGraphDrawStaticIPNumberSelect(setCursor);
               
                
            }
            
            break;
        case UI_KEY_DOWN:
            if(setCursor!=36)
            {
                
                switch(setCursor/12)
                {
                    case 0:
                        ip_set=ipAddrSetting;
                        break;
                    case 1:
                        ip_set=subMaskSetting;
                        break;
                    case 2:
                        ip_set=defaultGatewaySetting;
                        break;
                }
                /* 避免百位數超過2 */
                if(((setCursor%12)%3) == 0)
                {
                    ip_set[setCursor%12]--;
                    if((s8)(ip_set[setCursor%12])<0)
                        ip_set[setCursor%12]=2;
                }
                else
                {
                    ip_set[setCursor%12]--;
                    if((s8)(ip_set[setCursor%12]) <0)
                        ip_set[setCursor%12]=9;
                }

                uiGraphDrawStaticIPNumberSelect(setCursor);
                
                
            }
            
            
            break;
        case UI_KEY_ENTER:
            if(setCursor==36)
            {
                uiISStatic=1;
                uiGraphDrawStaticIPBackground(0xFF);
                uiGraphDrawStaticIPNumber(0xFF);
                for(i=0;i<3;i++)
                    uiGraphSaveStaticIP(i);
            }
            break;
            
        case UI_KEY_MODE:
        case UI_KEY_MENU:
            uiGraphRestorStaticIP();
            uiGraphDrawStaticIPBackground(0xFF);
            uiGraphDrawStaticIPNumber(0xFF);
            
            break;
            
        default:
            DEBUG_UI("uiGraphDrawStaticIP error key %d \r\n",key);
            sysSPI_Disable();
            return 0;
            
            
    }
    

    
    
    sysJPEG_disable();
    sysSPI_Disable();
    sysSD_Enable();

    return 1;

}

u8 uiGraphDrawDynamicIP(u8 key)
{
    static s8 setCursor=0;
    
    sysSD_Disable();
    sysSPI_Enable();
    sysJPEG_enable();

    switch(key)
    {
        case 0:
            setCursor=0;
            uiGraphDrawDynamicIPBackground(0xFF);
            break;
            
        case 0xFF:
            uiGraphDrawDynamicIPBackground(UI_NET_D_OK_1);
            setCursor=1;
            break;
        case UI_KEY_ENTER:
           
            if(setCursor == 1)
            {
                uiGraphDrawDynamicIPBackground(0xFF);
                setCursor=0;
                uiISStatic=0;
            }            
                      
            break;
        case UI_KEY_MODE:
            setCursor=0;
            uiGraphDrawDynamicIPBackground(0xFF);
            break;
        
          
    }
    

    
    sysJPEG_disable();
    sysSPI_Disable();
    sysSD_Enable();

    return 1;
}

void uiGraphGetNumPhotoID(void)
{
}


/*
 *********************************************************************************************************
 * Called by UI in other file
 *********************************************************************************************************
 */

void uiGraphDrawDateTime(u8 key)
{
    u16 uiJpgWidth, uiJpgHeight;
    static s8 setCursor;
    u16 x_pot[5] = {282, 384, 492, 384, 492};
    u16 y_pot[5] = {144, 144, 144, 200, 200};
    u16 x_frame[5] = {224, 368, 480, 368, 480};
    u16 y_frame[5] = {136, 136, 136, 192, 192};
    u8  i;

    switch(key)
    {
        case 0:
            sysSD_Disable();
            sysSPI_Enable();
            sysJPEG_enable();
            if(uiGraphDrawJpgGraph(Colon.bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 1)
                IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, 452, 204, uiJpgWidth,PKBuf0);
            if(uiGraphDrawJpgGraph(Slash.bufIndex, PKBuf2, &uiJpgWidth, &uiJpgHeight) == 1)
            {
                IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, 352, 148, uiJpgWidth,PKBuf0);
                IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf2, gJPGValidWidth, gJPGValidHeight, 452, 148, uiJpgWidth,PKBuf0);
            }

            /*get current time*/
            RTC_Get_Time(&SetTime);
            SetTime.sec = 0;
            setCursor = 0;
            for (i = 0; i < 5; i++)
            {
                if(i == 0)
                {
                    uiGraphDrawTimeFrameGraph(i, 0, x_frame[i], y_frame[i]);
                    uiGraphDrawTimeGraph(i, &SetTime, 0, x_pot[i], y_pot[i]);
                }
                else
                {
                    uiGraphDrawTimeFrameGraph(i, 1, x_frame[i], y_frame[i]);
                    uiGraphDrawTimeGraph(i, &SetTime, 1, x_pot[i], y_pot[i]);
                }
            }
            sysSPI_Disable();
            break;

        case UI_KEY_RIGHT:
        case UI_KEY_LEFT:
            /*clean old Frame*/
            uiGraphDrawTimeFrameGraph(setCursor, 1, x_frame[setCursor], y_frame[setCursor]);
            uiGraphDrawTimeGraph(setCursor, &SetTime, 1, x_pot[setCursor], y_pot[setCursor]);
            if(key == UI_KEY_RIGHT)
            {
                setCursor++;
                if (setCursor >= 5)
                    setCursor = 0;
            }
            else
            {
                setCursor--;
                if (setCursor < 0)
                    setCursor = 4;
            }
            /*draw new Frame*/
            uiGraphDrawTimeFrameGraph(setCursor, 0, x_frame[setCursor], y_frame[setCursor]);
            uiGraphDrawTimeGraph(setCursor, &SetTime, 0, x_pot[setCursor], y_pot[setCursor]);
            break;

        case UI_KEY_UP:
            uiGraphDrawSelectDateTime(&SetTime, setCursor, 1);
            uiGraphDrawTimeGraph(setCursor, &SetTime, 0, x_pot[setCursor], y_pot[setCursor]);
            break;

        case UI_KEY_DOWN:
            uiGraphDrawSelectDateTime(&SetTime, setCursor, 2);
            uiGraphDrawTimeGraph(setCursor, &SetTime, 0, x_pot[setCursor], y_pot[setCursor]);
            break;

        case UI_KEY_ENTER:
            uiCurrNode = uiCurrNode->parent;
            iconflag[uiCurrNode->item.NodeData->Action_no] = 1;
            uiMenuAction(uiCurrNode->item.NodeData->Action_no);
            MyHandler.MenuMode = SETUP_MODE;
            iduTVOSDDisable(OSD_Blk0);
            uiGraphDrawMenu();
            break;

        case UI_KEY_MODE:   /*leave*/
            uiCurrNode = uiCurrNode->parent;
            MyHandler.MenuMode = SETUP_MODE;
            iduTVOSDDisable(OSD_Blk0);
            uiGraphDrawMenu();
            break;

        default:
            DEBUG_UI("osdDrawDateTime error key %d\r\n",key);
            return;
    }
}

void uiGraphDrawMenu(void)
{
    UI_MENU_NODE *paretn;
#if(SHOW_UI_PROCESS_TIME == 1)
    u32 time1;
#endif

    if(uiCurrNode->parent)
        paretn = uiCurrNode->parent;
    else
        paretn = uiCurrNode;
    DEBUG_UI("current node %s action %d\r\n",uiCurrNode->item.NodeData->Node_Name, uiCurrNode->item.NodeData->Action_no);
    DEBUG_UI("paretn node %s action %d\r\n",paretn->item.NodeData->Node_Name, paretn->item.NodeData->Action_no);
    if (uiCurrNode->item.NodeData->Action_no == UI_MENU_SETIDX_CARDINFO)
        iduTVOSDDisable(OSD_Blk0);

    switch(paretn->item.NodeData->Action_no)
    {
        case UI_MENU_SETIDX_MOTION_MASK:
            DEBUG_UI("Enter Mask Area Setting\r\n");
            MyHandler.MenuMode = SET_MASK_AREA;
            uiOsdDrawMaskArea(0);
            break;

        case UI_MENU_SETIDX_DATE_TIME:
            sysISU_enable();
            uiGraphGetMenuData();
            uiGraphDrawDateTime(0);
            IduVidBuf0Addr = (u32)PKBuf0;

        #if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
        #endif
            MyHandler.MenuMode = SET_NUMBER_MODE;
            break;

        case UI_MENU_SETIDX_CARDINFO:
            sysISU_enable();
            uiGraphGetMenuData();
            IduVidBuf0Addr = (u32)PKBuf0;

        #if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
        #endif
            uiOsdDrawCardInfo();
            break;


        case UI_MENU_SETIDX_NETWORK:
            sysISU_enable();
            uiGraphGetMenuData();
            uiGraphDrawNetwork(0);
            IduVidBuf0Addr = (u32)PKBuf0;

        #if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
        #endif
            MyHandler.MenuMode = SET_NUMBER_MODE;
            break;

        default:
            sysISU_enable();
            uiGraphGetMenuData();
            IduVidBuf0Addr = (u32)PKBuf0;

        #if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
        #endif
            break;
    }

#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("UI Show Menu Time 1 =%d (x50ms)\n",time1);
#endif
}

void uiGraphDrawNetwork(u8 key)
{
    u8 retval=1;
    static s8 setCursor=0 ,SetLevel = 0;    /*SetLevel = 0: Active, SetLevel=1: Set Detail*/


    if(SetLevel==1)
    {
        if(setCursor==UI_MENU_STATIC_IP)
            retval=uiGraphDrawStaticIP(key);
        else
            retval=uiGraphDrawDynamicIP(key);

        if((key==UI_KEY_MODE)||((key==UI_KEY_ENTER)&&(retval==1))||(key==UI_KEY_MENU))
        {
            SetLevel=0;
        }
        return ;
    }
    
    switch(key)
    {
        case 0:
            if(uiISStatic)
            {
                uiGraphDrawStaticIP(0);
                setCursor=UI_MENU_STATIC_IP;
            }
                
            else
            {
                uiGraphDrawDynamicIP(0);
                setCursor=UI_MENU_DYNAMIC_IP;
            }
                
            break;
        case UI_KEY_MODE:   /*leave*/
            if(uiISStatic)
            {
                setCursor=UI_MENU_STATIC_IP;
            }
            else
            {
                setCursor=UI_MENU_DYNAMIC_IP;
            }
            uiCurrNode = uiCurrNode->parent;
            MyHandler.MenuMode = SETUP_MODE;
            iduTVOSDDisable(OSD_Blk0);
            uiGraphDrawMenu();
            break;

        case UI_KEY_RIGHT:
        case UI_KEY_LEFT:
            if(key == UI_KEY_RIGHT)
            {
                setCursor++;
                if(setCursor>=2)
                    setCursor=0;
            }
                           
            else
            {
                setCursor--;
                if(setCursor < 0)
                    setCursor=1;
            }
                
            if(setCursor==UI_MENU_STATIC_IP)
                uiGraphDrawStaticIP(0);
            else
                uiGraphDrawDynamicIP(0);
                
            break;
        case UI_KEY_ENTER:
            SetLevel=1;
            if(setCursor==UI_MENU_STATIC_IP)
                uiGraphDrawStaticIP(0xFF);
            else
                uiGraphDrawDynamicIP(0xFF);
            break;

 
        default:
            DEBUG_UI("uiGraphDrawNetwork error key %d\r\n",key);
            return;
    }
}

#if((HW_BOARD_OPTION == MR8600_RX_RDI) || (HW_BOARD_OPTION == MR8120_RX_RDI) )

void uiGraphDrawNoSignal(u8 Camera)
{
    u8  *targetBuf;
    u8  *sourceBuf;
    u8  err;
    u16 uiJpgWidth, uiJpgHeight;
    u16 x_pos;
    u16 y_pos;
    #if(HW_BOARD_OPTION == MR8600_RX_RDI)
    sourceBuf=OSD_buf;    // Roy:因為8600沒有OSD故使用OSD_buf來解JPG圖
    #else    
    sourceBuf=PKBuf2;
    #endif
    sysSPI_Enable();
    sysJPEG_enable();
    uiIsRFParing[Camera]=2;
    switch(Camera)
    {
        case 0:
            targetBuf=MainVideodisplaybuf[0];           
            
            break;
        case 1:
            targetBuf=Sub1Videodisplaybuf[0];
            
            break;
    }
    OSSemPend(uiOSDSemEvt, OS_IPC_WAIT_FOREVER, &err);
    if(uiGraphDrawJpgGraph(PairBG.bufIndex, sourceBuf, &uiJpgWidth, &uiJpgHeight) == 1)
        IDU_TVLayer_Stride(UI_MENU_SIZE_X ,sourceBuf, gJPGValidWidth, gJPGValidHeight, 0, 0, uiJpgWidth,targetBuf);
    else
        DEBUG_UI("File %s Open Fail\r\n",PairBG.FileName);
    OSTimeDly(20);
    if(uiGraphDrawJpgGraph(NoSignal.bufIndex, sourceBuf, &uiJpgWidth, &uiJpgHeight) == 1)
    {
        x_pos=(640-uiJpgWidth)/2;
        y_pos=(480-uiJpgHeight)/2;
        IDU_TVLayer_Stride(UI_MENU_SIZE_X ,sourceBuf, gJPGValidWidth, gJPGValidHeight, x_pos, y_pos, uiJpgWidth,targetBuf);
    }
       
    else
        DEBUG_UI("File %s Open Fail\r\n",NoSignal.FileName);

    OSSemPost(uiOSDSemEvt);
    sysJPEG_disable();
    sysSPI_Disable();
}

void uiGraphDrawPairNum(u8 num, u8 Camera)
{
    u8  numIndex[2], i;
    u8  err;
    u16 uiJpgWidth, uiJpgHeight;
    u16 x_pos=440;
    u16 y_pos=230;
    u8  *targetBuf;
    u8  *sourceBuf;
    sysSPI_Enable();
    sysJPEG_enable();
    numIndex[0] = (num/10);
    numIndex[1] = (num%10);
    #if(HW_BOARD_OPTION == MR8600_RX_RDI)
    sourceBuf=OSD_buf;    // Roy:因為8600沒有OSD故使用OSD_buf來解JPG圖
    #else
    sourceBuf=PKBuf2;       
    #endif
    switch(Camera)
    {
        case 0:
            targetBuf=MainVideodisplaybuf[0];
            break;
        case 1:
            targetBuf=Sub1Videodisplaybuf[0];
            break;
    }
    
    OSSemPend(uiOSDSemEvt, OS_IPC_WAIT_FOREVER, &err);
    for (i = 0; i < 2; i++)
    {
        if(uiGraphDrawJpgGraph(PairNum[numIndex[i]].bufIndex, sourceBuf, &uiJpgWidth, &uiJpgHeight) == 0)
        {
            DEBUG_UI("File %s Open Fail\r\n",PairNum[numIndex[i]].FileName);
            continue;
        }
        IDU_TVLayer_Stride(UI_MENU_SIZE_X ,sourceBuf, gJPGValidWidth, gJPGValidHeight, (x_pos+i*24), y_pos, uiJpgWidth,targetBuf);
    }
    OSSemPost(uiOSDSemEvt);
    sysJPEG_disable();
    sysSPI_Disable();
}



u8 uiGraphDrawPair(u8 Camera )
{
    u8  err;
    u16 uiJpgWidth, uiJpgHeight;
    u32 waitFlag = 0, cnt = 30;
    u16 x_pos;
    u16 y_pos;
    u8 *targetBuf;
    u8 *sourceBuf;

    DEBUG_UI("enter graph draw pair %x\n",Camera);
    #if(HW_BOARD_OPTION == MR8600_RX_RDI)
    sourceBuf=OSD_buf;    // Roy:因為8600沒有OSD故使用OSD_buf來解JPG圖
    #else
    sourceBuf=PKBuf2;    
    #endif
    switch(Camera)
    {
        case 0:
            targetBuf=MainVideodisplaybuf[0];
            break;
        case 1:
            targetBuf=Sub1Videodisplaybuf[0];
            break;
    }
    
    sysSPI_Enable();
    sysJPEG_enable();
    
    if(Camera==0)
    {
        gpioTimerCtrLed(LED_L_LONG_FLASH);
        gpioTimerCtrLed(LED_R_OFF);
    }
    else
    {
        gpioTimerCtrLed(LED_R_LONG_FLASH);
        gpioTimerCtrLed(LED_L_OFF);
    }
    OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_PAIR_SUCCESS, OS_FLAG_CLR, &err);
    rfiu_PAIR_Linit(Camera); 
    OSTimeDly(10);
    OSSemPend(uiOSDSemEvt, OS_IPC_WAIT_FOREVER, &err);
    if(uiGraphDrawJpgGraph(PairBG.bufIndex, sourceBuf, &uiJpgWidth, &uiJpgHeight) == 1)
        IDU_TVLayer_Stride(UI_MENU_SIZE_X ,sourceBuf, gJPGValidWidth, gJPGValidHeight, 0, 0, uiJpgWidth,targetBuf);
    else
        DEBUG_UI("File %s Open Fail\r\n",PairBG.FileName);
    
    if(uiGraphDrawJpgGraph(PairText.bufIndex, sourceBuf, &uiJpgWidth, &uiJpgHeight) == 1)
    {
        x_pos=(640-uiJpgWidth)/2;
        y_pos=(480-uiJpgHeight)/2;

        DEBUG_UI("FILE WIDTH: %d, Height: %d (%d,%d)\n",uiJpgWidth,uiJpgHeight,gJPGValidWidth,gJPGValidHeight);
        IDU_TVLayer_Stride(UI_MENU_SIZE_X ,sourceBuf, gJPGValidWidth, gJPGValidHeight, x_pos, y_pos, uiJpgWidth,targetBuf);
    }
    else
    {
        DEBUG_UI("File %s Open Fail\r\n",PairText.FileName);
    }
        
    OSSemPost(uiOSDSemEvt);
    uiGraphDrawPairNum(cnt,Camera);    
    

    
    while (waitFlag == 0)
    {
        waitFlag = OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_PAIR_SUCCESS,OS_FLAG_WAIT_SET_ANY, 20, &err);
        if (waitFlag & FLAGUI_RF_PAIR_SUCCESS)
        {
            
            if(Camera == 0)
            {
                gpioTimerCtrLed(LED_L_ON);
            }
            else
            {
                gpioTimerCtrLed(LED_R_ON);
            }
            uiIsRFParing[Camera]=0;
        }
        else if (cnt != 0)
        {
           
            if(Camera==0)
            {
                gpioTimerCtrLed(LED_L_LONG_FLASH);
            }
            else
            {
                gpioTimerCtrLed(LED_R_LONG_FLASH);
            }
            uiGraphDrawPairNum(cnt,Camera);
            
            
            
            cnt --;
            
        }
        else
        {
            uiIsRFParing[Camera]=0;            
            uiGraphDrawPairNum(cnt,Camera);            
            sysJPEG_disable();
            sysSPI_Disable();
            rfiu_PAIR_Stop(Camera);            

            return 0;
        }
    }


    sysJPEG_disable();
    sysSPI_Disable();
    return 1;

}

#endif
void uiGraphGetTimePhotoID(void)
{
    u8  i;

    for ( i = 0; i < 20; i++)
        TimeNum[i].bufIndex = spiGet_UI_FB_Index(TimeNum[i].FileName);
    for ( i = 0; i < 4; i++ )
        TimeFram[i].bufIndex = spiGet_UI_FB_Index(TimeFram[i].FileName);
    Colon.bufIndex = spiGet_UI_FB_Index(Colon.FileName);
    Slash.bufIndex = spiGet_UI_FB_Index(Slash.FileName);
}

void uiGraphGetNetwokPhotoID(void)
{
    u8 i;
    
    OK_1.bufIndex              = spiGet_UI_FB_Index(OK_1.FileName);
    OK_2.bufIndex              = spiGet_UI_FB_Index(OK_2.FileName);
    ON_1.bufIndex              = spiGet_UI_FB_Index(ON_1.FileName);
    ON_2.bufIndex              = spiGet_UI_FB_Index(ON_2.FileName);
    OFF_1.bufIndex             = spiGet_UI_FB_Index(OFF_1.FileName);
    OFF_2.bufIndex             = spiGet_UI_FB_Index(OFF_2.FileName);
    IP_Address.bufIndex        = spiGet_UI_FB_Index(IP_Address.FileName);
    Default_Gateway.bufIndex   = spiGet_UI_FB_Index(Default_Gateway.FileName);
    Subnet_mask.bufIndex       = spiGet_UI_FB_Index(Subnet_mask.FileName);
    IP_SET_BLOCK.bufIndex      = spiGet_UI_FB_Index(IP_SET_BLOCK.FileName);
    Static_IP_1.bufIndex       = spiGet_UI_FB_Index(Static_IP_1.FileName);
    Static_IP_2.bufIndex       = spiGet_UI_FB_Index(Static_IP_2.FileName);
    Dynamic_IP_1.bufIndex      = spiGet_UI_FB_Index(Dynamic_IP_1.FileName);
    Dynamic_IP_2.bufIndex      = spiGet_UI_FB_Index(Dynamic_IP_2.FileName);
    Baseline.bufIndex          = spiGet_UI_FB_Index(Baseline.FileName);
    for(i=0;i<10;i++)
        number[i].bufIndex=spiGet_UI_FB_Index(number[i].FileName);

    for(i=0;i<10;i++)
        number_select[i].bufIndex=spiGet_UI_FB_Index(number_select[i].FileName);
    
    
    uiGraphSetStaticInitialPosition();
    uiGraphSetDynamicInitialPosition();
}

void uiGraphGetPairPhotoID(void)
{
    u8 i;
    NoSignal.bufIndex           = spiGet_UI_FB_Index(NoSignal.FileName); 
    PairText.bufIndex           = spiGet_UI_FB_Index(PairText.FileName); 
    PairBG.bufIndex             = spiGet_UI_FB_Index(PairBG.FileName); 
    for ( i = 0; i < 10; i++)
        PairNum[i].bufIndex     = spiGet_UI_FB_Index(PairNum[i].FileName);
    
}



/*
 *********************************************************************************************************
 * Called by other moudle
 *********************************************************************************************************
 */


