/*

  Copyright (c) 2008 Mars Semiconductor Corp.

   Module Name:

   ui_flow_project.c

   Abstract:

   The routines of user interface.

   Environment:

   ARM RealView Developer Suite

   Revision History:

   2010/12/27   Elsa Lee  Create

   */
#include "general.h"
#include "uiapi.h"

/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */

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

s32 uiSetFileInitFinish(void)
{
    return 0;
}

s32 uiSetFileDoCommand(u8* pucAddr, u32* bufferlen)
{    
    
    u32 value;
    u8  i;
    switch(*pucAddr)
	{
	#if(UI_VERSION == UI_VERSION_MUXCOM)
	    case 'G':
            if(!strncmp((const char *)pucAddr,"GMT=",strlen("GMT=")))
            {
                pucAddr +=strlen("GMT=");
                
                while ( (*pucAddr==' ') || (*pucAddr==0x09))
                    pucAddr++;
                uiGMTparse(pucAddr);
            }
            else
                DEBUG_UART("IS Command Error [GMT]\r\n");
            break;
	    case 'T':
            if(!strncmp((const char *)pucAddr,"TIME=",strlen("TIME=")))
            {
                pucAddr +=strlen("TIME=");
                
                while ( (*pucAddr==' ') || (*pucAddr==0x09))
                    pucAddr++;
                uiTIMEparse(pucAddr);
            }
            else
                DEBUG_UART("IS Command Error [TIME]\r\n");
            break;
	    case 'R':
            if(!strncmp((const char *)pucAddr,"REC=",strlen("REC=")))
            {
                pucAddr +=strlen("REC=");
                
                while ( (*pucAddr==' ') || (*pucAddr==0x09))
                    pucAddr++;
                uiRECModeparse(pucAddr);
            }
            else
                DEBUG_UART("IS Command Error [REC]\r\n");
            break;
	    case 'S':
            if(!strncmp((const char *)pucAddr,"SENS=",strlen("SENS=")))
            {
                pucAddr +=strlen("SENS=");
                
                while ( (*pucAddr==' ') || (*pucAddr==0x09))
                    pucAddr++;
                uiMotionSensparse(pucAddr);
            }
            else
                DEBUG_UART("IS Command Error [SENS]\r\n");
            break;
	    case 'E':
            if(!strncmp((const char *)pucAddr,"END",strlen("END")))
            {
                pucAddr +=strlen("END");
                
                while ( (*pucAddr==' ') || (*pucAddr==0x09))
                    pucAddr++;
                uiDelSetup();
            }
            else
                DEBUG_UART("IS Command Error [END]\r\n");
            break;
    #endif
	#if(TUTK_SUPPORT)
	    case 'G':
            if(!strncmp((const char *)pucAddr,"GUID=",strlen("GUID=")))
            {
                pucAddr +=strlen("GUID=");
                
                while ( (*pucAddr==' ') || (*pucAddr==0x09))
                    pucAddr++;

                sscanf((const char *)pucAddr, "%s", uiP2PID); 
                uiMenuAction(UI_MENU_SETIDX_P2PID);
            }
            else
                DEBUG_UART("IS Command Error [GUID]\r\n");
            break;
    #endif
    #if(RFIU_SUPPORT)
        case 'R':
            if(!strncmp((const char *)pucAddr,"RFID=",strlen("RFID=")))
            {
                pucAddr +=strlen("RFID=");
                
                while ( (*pucAddr==' ') || (*pucAddr==0x09))
                    pucAddr++;

                for(i=0;i<RFID_MAX_WORD;i++)
                {
                    sscanf((const char *)pucAddr, "%x",&value);
                    while ( (*pucAddr==' ') || (*pucAddr==0x09))
                        pucAddr++;
                    pucAddr+=9;
                    uiRFID[i]=value;     
                }
                uiMenuAction(UI_MENU_SETIDX_RFID);
            }
            else
                DEBUG_UART("IS Command Error [RFID]\r\n");

            if(!strncmp((const char *)pucAddr,"RFCODE=",strlen("RFCODE=")))
            {
                pucAddr +=strlen("RFCODE=");
                
                while ( (*pucAddr==' ') || (*pucAddr==0x09))
                    pucAddr++;

                for(i=0;i<RFID_MAX_WORD;i++)
                {
                    sscanf((const char *)pucAddr, "%x",&value);
                    while ( (*pucAddr==' ') || (*pucAddr==0x09))
                        pucAddr++;
                    pucAddr+=9;
                    uiRFCODE[i]=value;     
                }                
            }
            else
                DEBUG_UART("IS Command Error [RFCODE]\r\n");
            
            break;
    #endif
    #if(NIC_SUPPORT)
        case 'M':
            if(!strncmp((const char *)pucAddr,"MAC=",strlen("MAC=")))
            {
                pucAddr +=strlen("MAC=");
                 
                while ( (*pucAddr==' ') || (*pucAddr==0x09))
                    pucAddr++;

                for(i=0;i<MAC_LENGTH-1;i++)
                {
                    sscanf((const char *)pucAddr, "%x-",&value);
                    pucAddr+=3;
                    uiMACAddr[i]=value;
                }
                sscanf((const char *)pucAddr, "%x",&value);
                uiMACAddr[MAC_LENGTH-1]=value;
                uiMenuAction(UI_MENU_SETIDX_MAC);
            }
            else
                DEBUG_UART("IS Command Error [MAC]\r\n");
            break;
    #endif
        default:
            break;
    }
    return 0;
}

