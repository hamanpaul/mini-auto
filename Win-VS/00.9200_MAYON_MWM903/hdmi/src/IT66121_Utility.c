///*****************************************
//  Copyright (C) 2009-2015
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <Utility.c>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2015/09/03
//   @fileversion: ITE_HDMITX_SAMPLE_3.21
//******************************************/
#include "general.h"
#include "board.h"

#if(HDMI_TXIC_SEL == HDMI_TX_IT66121)
#include "IT66121_mcu.h"
#include "IT66121_hdmitx.h"
#include "IT66121_typedef.h"
//#include "TimerProcess.h"
#include "MemoryPool.h"

extern u32 OS_tickcounter;

void delay1ms(USHORT ms)
{
u32 count1ms = 26666;
count1ms = count1ms * ms;
while(count1ms--);
}


void HoldSystem(void)
{
/* Amy
    Hold_Pin=1;
    while(!Hold_Pin)
	{
		printf("Hold\\\r") ;
		printf("Hold-\r") ;
		printf("Hold/\r") ;
		printf("Hold|\r") ;
*/
}

#ifdef SUPPORT_UART_CMD
///////////////////////////////////////////////////////////////////////////////
// I2C for original function call
///////////////////////////////////////////////////////////////////////////////

void DumpReg(BYTE dumpAddress)
{
    int i,j ;
    BYTE BuffData[16] ;
    BYTE DataIn;
    DBG_printf("       ");
    for( j = 0 ; j < 16 ; j++ )
    {
        DBG_printf(" %02X",(int)j);
        if( (j == 3)||(j==7)||(j==11))
        {
            DBG_printf(("  "));
        }
    }
    DBG_printf("\n        -----------------------------------------------------\n");
///////////////////////////////////////////////
    DataIn=0;
    i2c_write_byte(dumpAddress, 0x0f, 1, &DataIn, 0);
///////////////////////////////////////////////
    for(i = 0 ; i < 0x100 ; i+=16 )
    {
        DBG_printf("[%3X]  ",(int)i);
        i2c_read_byte(dumpAddress, i, 16, BuffData, 0);
        for( j = 0 ; j < 16 ; j++ )
        {
            DBG_printf(" %02X",(int)BuffData[j]);
            if( (j == 3)||(j==7)||(j==11))
            {
                printf(" -");
            }
        }
        DBG_printf("\n");
        if( (i % 0x40) == 0x30)
        {
            DBG_printf("        -----------------------------------------------------\n");
        }
    }
    ///////////////////////////////////////////////
    DataIn=1;
    i2c_write_byte(dumpAddress, 0x0f, 1, &DataIn, 0);
    ///////////////////////////////////////////////
    for(i = 0x100; i < 0x200 ; i+=16 )
    {
        DBG_printf("[%3X]  ",(int)i);
        i2c_read_byte(dumpAddress, i&0xff, 16, BuffData, 0);
        for( j = 0 ; j < 16 ; j++ )
        {
            DBG_printf(" %02X",(int)BuffData[j]);
            if( (j == 3)||(j==7)||(j==11))
            {
                DBG_printf(" -");
            }
        }
        DBG_printf("\n");
        if( (i % 0x40) == 0x30)
        {
            DBG_printf("        -----------------------------------------------------\n");
        }

    }

    ///////////////////////////////////////////////
    DataIn=0;
    i2c_write_byte(dumpAddress, 0x0f, 1, &DataIn, 0);
    ///////////////////////////////////////////////
}

void ConfigfHdmiVendorSpecificInfoFrame(BYTE _3D_Stru);
void SetOutputColorDepthPhase(BYTE ColorDepth,BYTE bPhase);

void UartCommand()
{
    char STR[1];
    static int address=0x98;
    if(RI)
    {
        while(1)
        {
            DBG_printf("Operation[W/R/D]:");
            scanf("%01s",STR);DBG_printf("\n");
            if(0==strcmp(STR,"D")|| 0==strcmp(STR,"d"))
            {
                DumpReg(address);
            }
            else if(0==strcmp(STR,"c") || 0==strcmp(STR,"C"))
            {
                int ewdata=0;
                DBG_printf("Color depth [8/10/12]:");
                scanf("%d",&ewdata);printf("\n");
                if(ewdata==12)          ewdata=36;
                else if (ewdata==10)    ewdata=30;
                else                    ewdata=0;
                SetOutputColorDepthPhase((BYTE)ewdata,0);
            }
            else if(0==strcmp(STR,"3"))
            {
                int ewdata=0,__3D_Stru;
                DBG_printf("[1]:2D  [0]:Frame Packing  [6]:Top and Botton  [8]:Side by Side\n");
                scanf("%02x",&__3D_Stru);DBG_printf("\n");
                if(__3D_Stru==1)
                {
                    i2c_write_byte((BYTE)address, 0xd2, 1, ((BYTE *)&ewdata), 0);
                }
                else
                {
                    ConfigfHdmiVendorSpecificInfoFrame(__3D_Stru);
                }
            }
            else if(0==strcmp(STR,"W")|| 0==strcmp(STR,"w"))
            {
                int offset,ewdata;
                DBG_printf("[Address][Offset][Data]:");
                scanf("%02x %02x %02x",&address,&offset,&ewdata);DBG_printf("\n");
                i2c_write_byte((BYTE)address, (BYTE)offset, 1, ((BYTE *)&ewdata)+1, 0);
                i2c_read_byte((BYTE)address,(BYTE)offset, 1, ((BYTE *)&ewdata)+1, 0);
                printf("%02x, %02x, %02x\n",address,offset,ewdata);
            }
            else if( 0==strcmp(STR,"R")|| 0==strcmp(STR,"r"))
            {
                int offset,count,j;
                BYTE DataBf[16];
                DBG_printf("[Address][Offset][count]:");
                scanf("%02x %02x %02d",&address,&offset,&count);DBG_printf("\n");
                DBG_printf("Address = %02x, Offset = %02x,\n",address,offset);
                if(count>26)count=26;
                while(RI==0)
                {
                    DBG_printf("[%3X]  ",(int)offset);
                    if(i2c_read_byte(address, offset, count, DataBf, 0))
                    {
                        for( j = 0 ; j < count ; j++ )
                        {
                            if(0==(j%4) && j)
                            {
                                DBG_printf(" -");
                            }
                            DBG_printf(" %02X",(int)DataBf[j]);
                        }
                        DBG_printf("\r");
                    }
                    else
                    {
                        DBG_printf("\r*****I2C Read Fail******\n");
                        break;
                    }
                }
                DBG_printf("\n");
            }
            else if( 0==strcmp(STR,"P")|| 0==strcmp(STR,"p"))
            {
                //show_vid_info();
            }
            else if( 0==strcmp(STR,"S")|| 0==strcmp(STR,"s"))
            {
                int setVIC;
                extern _IDATA BYTE bOutputColorMode;
                #include "IT66121_hdmitx_sys.h"
                DBG_printf("HDMITX_ChangeDisplayOption==>VIC = ");
                scanf(" %d",&setVIC);
                DBG_printf("\n");
                HDMITX_ChangeDisplayOption(setVIC,bOutputColorMode) ;
                HDMITX_SetOutput();
            }
            else
            {
                DBG_printf("Press not 'Z/z'  key continue\n");
                if(0==strcmp(STR,"Z")|| 0==strcmp(STR,"z"))
                {
                    DBG_printf("====Continue Normal Operation====\n");
                    break;
                }
                else
                {
                    DBG_printf("Press 'Z/z' key to break setup mode\n");
                    continue;
                }
            }
        }
    }
}
#endif

#endif
