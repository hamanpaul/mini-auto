/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    BLE.c

Abstract:

    The routines of BT 4.0 Interface Unit and Uart Commands.
    
    
Environment:

        ARM RealView Developer Suite

Revision History:
    
    2016/09/30    Amy Create  
*/





#include "general.h"
#include "board.h"
#include "ble.h"
#include "rfiuapi.h"
#include "gpioapi.h"
#include "uartapi.h"
#include "Uiapi.h"
#include "uiKey.h"
#include "spiapi.h"
#include "sysapi.h"




#if(BLE_SUPPORT)
/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */
#define BLE_TIMEOUT 120 //20 ticks (1 sec) * 6 sec
#define MacAddr TRUE
#define ICON_BAND_DISCONNECT_DLYTIME 25 //meet band's 30 secs icon disappear/vibrate(late 2 secs than disappear)
#define VOX_CMD_SAMECH_INTERVAL 180 //3 mins
#define VOX_CMD_DIFFCH_INTERVAL 5  //5 secs

/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
OS_EVENT* bleSemReq;
OS_EVENT* bleSemFin;
OS_EVENT* bleSemScan;
BLE_SCAN_LIST mBLEScanList = {0};
static u8 BLEVoxDuration[4] = {255};
u8 u8BLEVoxCmdDiff = 0;
u8 g_BLEmoduleDetCount = 255;
u8 g_BLEsyncTimeError = 0;
u8 u8BLEDisconnectCount = 0;
u8 u8Total = 0;
u8 u8ConnectedIdx = 10;
char auConnectError[] = "CONNECT ERROR";
char auNoConnect[] = "NO CONNECT";
char auAlreadyConnect[] = "ALREADY CONNECT";
/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern u8 u8BLEexist;
extern u8 u8BLEStatus;

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
u8 BLEGetStatus(void);
u8 BLE_Switch(BLE_WORKING_STATE ble_status);
u8 BLE_ScanList(BLE_SCAN_LIST *sScanList);
u8 BLE_ChooseToPair(BLE_SCAN_LIST *sScanList, u8 num);
u8 BLE_VoxTriggerSetting(u8 num, u8 value);
u8 BLEInit(void);
u8 BLEUnInit(void);
u8 BLEGetUartCmd(u8 *pcString);
u8 BLECmdParse(u8 * cmdString);
void BLESendCommand(u8 cmd, u8 data);
 
/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */

/*
*********************************************************************************************************
*                                         BLE Status
*
* Description : UI can get BLE status anytime
*
* Arguments   : void
*
* Returns     : BLE_WORKING_STATE (BLE_OFF/ BLE_ADVERTISING/ BLE_CONNECTED)
*********************************************************************************************************
*/
u8 BLEGetStatus(void)
{

    if(gpioGetBLENTP() && gpioGetBLEPWR())
    {
        return BLE_CONNECTED;
    }
    else
    {
        return BLE_ADVERTISING;
    }

}

/*
*********************************************************************************************************
*                                         BLE Switch
*
* Description : BLE off->on, BLE on->off
*
* Arguments   : ble_status : BLE_OFF, BLE_ON, and TBD.
*
* Returns     : 0: success
*               1: fail
*********************************************************************************************************
*/
u8 BLE_Switch(BLE_WORKING_STATE ble_status)
{
    u8 err = 0;
    GPIO_CFG c;

    if(ble_status)
    {
        c.ena = GPIO_DISA;
        c.dir = GPIO_DIR_OUT;
        c.level = GPIO_LEVEL_LO;
        c.inPullUp = GPIO_IN_PULLUP_ENA;

        gpioConfig(1,17,&c);
        //OSTimeDly(5);
    }

    OSSemPend(bleSemReq, BLE_TIMEOUT, &err);
    gpioSetBLEPWR((u8)ble_status);
    OSSemPost(bleSemReq);

    if(!ble_status)
    {
        //OSTimeDly(5);
        c.ena = GPIO_ENA;
        c.dir = GPIO_DIR_IN;
        c.level = GPIO_LEVEL_LO;
        c.inPullUp = GPIO_IN_PULLUP_ENA;

        gpioConfig(1,17,&c);
    }
    else
    {   //keep enough time to prepare start scanlist
        OSTimeDly(5);
    }

    u8Total = 0;
    u8ConnectedIdx = 10;
    memset((void *)&mBLEScanList, 0, sizeof(BLE_SCAN_LIST));

    u8BLEStatus = ble_status;
    return 0;
}

/*
*********************************************************************************************************
*                                         BLE ScanList
*
* Description : Switch BLE status to BLE_SCANNING, and scanned list
*
* Arguments   : *sScanList : empty structure to fill data.
*
* Returns     : 0: success
*               1: fail
*********************************************************************************************************
*/
u8 BLE_ScanList(BLE_SCAN_LIST *sScanList)
{

    u8 i = 0;
    u8 j = 0;
    printf("BLE_ScanList\n");
    
    i = 0;
    while(i < 100)
    {
        OSTimeDly(2);// 2 sec = 40 ticks = 2 * 20
        i++;
        if(u8BLEStatus == BLE_CONNECTED) break;
        if(mBLEScanList.sDev[0].devName[0] != 0) break;

    }

    i = 0;
    while(mBLEScanList.sDev[i].devName[0] != 0)
    {
        printf("ID = %d \t", mBLEScanList.sDev[i].devID);
    #if(MacAddr == TRUE)
        printf("DevName = ");
        for(j = 0; j < BLE_MAC_ADDR_MAX-1 ; j++)
            printf("%02x ", mBLEScanList.sDev[i].devName[j]);
        printf("\n");
    #endif
        printf("Status = %d \n", mBLEScanList.sDev[i].status);
        i++;
    }
    if(i == 0) //band not in range
    {
        return 1;
    }
    else
    {
        memset((void *)sScanList, 0, sizeof(BLE_SCAN_LIST));
        memcpy(sScanList, &mBLEScanList, sizeof(BLE_SCAN_LIST));
    }
    printf("BLE_ScanList End\n");

    return 0;
}

/*
*********************************************************************************************************
*                                         BLE ChooseToPair
*
* Description : Pairing, before unpair anyone. 
*               Unpairing, if you assign a connected device. TBD (no an interact screen)
*
* Arguments   : *sScanList : structure with data.
*             : num : which to pair(the ith in order, or Device ID maybe...TBD)
*
* Returns     : 0: success
*               1: fail
*********************************************************************************************************
*/
u8 BLE_ChooseToPair(BLE_SCAN_LIST *sScanList, u8 num)
{

    u8 err = 0;
    u8 i = 0;

    printf("BLE_ChooseToPair\n");

    //might need to check MAC addr in sScanList/mBLEScanList is the same, then, action.
    if(sScanList->sDev[num].status == BLE_DEV_UNPAIRED) //what if detect gpio?
    {
        OSSemPend(bleSemReq, BLE_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            printf("Error: BLE_ChooseToPair bleSemReq is %d.\n", err);
            return 1;
        }
        BLESendCommand(BLE_CMD_CONNECT_DEV, sScanList->sDev[num].devID); //cmd index

        OSSemPend(bleSemFin, BLE_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            printf("Error: BLE_ChooseToPair BLE_CMD_CONNECT_DEV bleSemFin is %d.\n", err);
            OSSemPost(bleSemReq);
            return 1;
        }
        OSSemPost(bleSemReq);

        i = 0;
        while(u8BLEStatus != BLE_CONNECTED) //check this out pairing time is good or not
        {
            OSTimeDly(2);
            i++;
            if(i == 80) break; //need about OSTimeDly(26) //30
        }

        if(i < 80)
        {
            sScanList->sDev[num].status = BLE_DEV_PAIRED;
            DEBUG_BLUE("BLE paired with %d\n", sScanList->sDev[num].devID);
        }
        else
        {
            printf("BLE Pair fail(BLEGetStatus = BLE_ADVERTISING)");
            return 1;
        }
    }
    else if(sScanList->sDev[num].status == BLE_DEV_PAIRED)
    {
        OSSemPend(bleSemReq, BLE_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            printf("Error: BLE_ChooseToPair bleSemReq is %d.\n", err);
            return 1;
        }
        BLESendCommand(BLE_CMD_UNPAIR_DEV, sScanList->sDev[num].devID); //cmd index

        OSSemPend(bleSemFin, BLE_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            printf("Error: BLE_ChooseToPair BLE_CMD_UNPAIR_DEV bleSemFin is %d.\n", err);
            OSSemPost(bleSemReq);
            return 1;
        }
        OSSemPost(bleSemReq);

        i = 0;
        while(BLEGetStatus() != BLE_ADVERTISING)
        {
            OSTimeDly(2);
            i++;
            if(i == 60) break; // equal OSTimeDly(6);
        }

        if(i < 60)
        {
            sScanList->sDev[num].status = BLE_DEV_UNPAIRED;
            memset((void *)&mBLEScanList, 0, sizeof(BLE_SCAN_LIST));
            DEBUG_BLUE("BLE Unpaired with %d\n", sScanList->sDev[num].devID);
            u8BLEStatus = BLE_ADVERTISING;
        }
        else
        {
            printf("BLE_ChooseToPair BLE UnPair fail(BLEGetStatus = BLE_CONNECTED)");
            return 1;
        }

    }

    return 0;
}

u8 BLE_UnPair(u8 devID)
{
    u8 err;
    u8 timeout = 10;

    printf("BLE_UnPair \n");

    OSSemPend(bleSemReq, timeout, &err);
    if (err != OS_NO_ERR)
    {
        printf("Error: BLE_UnPair bleSemReq is %d.\n", err);
        return 1;
    }
    BLESendCommand(BLE_CMD_UNPAIR_DEV, devID); //cmd index
    OSSemPend(bleSemFin, timeout, &err);
    if (err != OS_NO_ERR)
    {
        printf("Error: BLE_UnPair BLE_CMD_UNPAIR_DEV bleSemFin is %d.\n", err);
        OSSemPost(bleSemReq);
        return 1;
    }
    OSSemPost(bleSemReq);

#if(1)

    //DEBUG_BLUE("BLE Unpaired with %d\n", devID);

#else
    i = 0;
    while(BLEGetStatus() != BLE_ADVERTISING)
    {
        OSTimeDly(2);
        i++;
        if(i == 40) break; // equal OSTimeDly(6);
    }

    if(i < 40)
    {
        DEBUG_BLUE("BLE Unpaired with %d\n", devID);
//        memset((void *)&mBLEScanList, 0, sizeof(BLE_SCAN_LIST));
        u8BLEStatus = BLE_ADVERTISING;
    }
    else
    {
        printf("BLE_UnPair BLE UnPair fail(BLEGetStatus = BLE_CONNECTED)\n");
        return 1;
    }
#endif
    return 0;
}

u8 BLE_syncTime(void)
{
    u8 err;
    if(u8BLEStatus == BLE_CONNECTED)
    {
        OSSemPend(bleSemReq, 20, &err);
        if (err != OS_NO_ERR)
        {
            printf("Error: BLE_syncTime bleSemReq is %d.\n", err);
            g_BLEsyncTimeError = 1;
            return 1;
        }
        BLESendCommand(BLE_CMD_TIME_SYNC, 0);

        OSSemPend(bleSemFin, 20, &err);
        if (err != OS_NO_ERR)
        {
            printf("Error: BLE_syncTime bleSemFin is %d.\n", err);
            g_BLEsyncTimeError = 1;
            OSSemPost(bleSemReq);
            return 1;
        }
        OSSemPost(bleSemReq);
        g_BLEsyncTimeError = 0;
        return 0;
    }
    else
    {
        printf("BLE not connect\n");
        return 1;
    }
}

u8 sysBack_BLE_SyncTime(u8 FuncType)
{
    u8 i = 0;
    GPIO_CFG c;

    switch(FuncType)
    {
        case 0:
            BLE_syncTime();
            break;
        case 1: //for master default
            OSTimeDly(5);
            for(i = 0; i < 3 ; i++)
            {
                if(BLE_UnPair(i)){
                    OSTimeDly(10);
                    BLE_UnPair(i);
                }else{
                    OSTimeDly(10);
                }
            }
            BLE_Switch(BLE_OFF);
            break;
        case 2: //reset BT when disconnect 255 secs (4 mins)
            printf("BLE reset for scanning\n");
            BLE_Switch(BLE_OFF);
            OSTimeDly(10);
            BLE_Switch(BLE_ON);
            break;
        case 3:
            if(g_BLEmoduleDetCount == 0)
            {
                g_BLEmoduleDetCount++;
            }
            else
            {
                g_BLEmoduleDetCount++;
                if(gpioGetBLENTP())
                {
                    if(u8BLEStatus == BLE_OFF)
                    {
                        u8BLEexist = FALSE;
                        printf("No BLE module\n");
                    }
                    else
                    {
                        u8BLEexist = TRUE;
                        printf("Detect BLE module\n");
                    }
                }
                else
                {
                    u8BLEexist = TRUE;
                    printf("Detect BLE module\n");
                }

                if(u8BLEexist)
                {
                    g_BLEmoduleDetCount = 255;

                    c.ena = GPIO_ENA;
                    c.dir = GPIO_DIR_IN;
                    c.level = GPIO_LEVEL_LO;
                    c.inPullUp = GPIO_IN_PULLUP_ENA;

                    gpioConfig(GPIO_GROUP_NTF, GPIO_BIT_NTF,&c);

                    if(iconflag[UI_MENU_SETIDX_BLUETOOTH] == BLE_ON)
                        BLE_Switch(BLE_ON);
                    else
                        BLE_Switch(BLE_OFF);
                }
            }
            break;
    }

    return 0;
}

void BLE_VoxDurationCountdown(void)
{
    u8 i = 0;
    for(i = 0 ; i < 4; i++)
    {
        if(BLEVoxDuration[i] != BLE_VOX_OFF)
            BLEVoxDuration[i] = BLEVoxDuration[i] < BLE_VOX_ON ? BLEVoxDuration[i] + 1 : BLE_VOX_ON;
    }
    u8BLEVoxCmdDiff = u8BLEVoxCmdDiff < BLE_VOX_ON ? u8BLEVoxCmdDiff + 1 : BLE_VOX_ON;

    if(u8BLEStatus == BLE_ADVERTISING)
    {
        if(u8BLEDisconnectCount < ICON_BAND_DISCONNECT_DLYTIME) //update BT indicator (fit band 30 sec icon disappear)
        {
            u8BLEDisconnectCount++;
            if(u8BLEDisconnectCount == ICON_BAND_DISCONNECT_DLYTIME)
                uiSentKeyToUi(UI_KEY_UPDATE_BLE_ICON);
        }
        else if(u8BLEDisconnectCount < 255) //wait for 4 mins to reset BT
        {
            u8BLEDisconnectCount++;
            if(u8BLEDisconnectCount == 255)
                sysbackSetEvt(SYS_BACK_BLE_SYNCTIME, 2);
        }
    }
}

void BLE_VoxDurationCountReset(u8 num, u8 u8OnOff)
{
    //printf("num %d u8OnOff %d\n", num, u8OnOff);
    if(u8OnOff == BLE_VOX_ON) //ON
        BLEVoxDuration[num] = BLE_VOX_ON;
    else //Off
        BLEVoxDuration[num] = BLE_VOX_OFF;

}

u8 BLE_VoxTriggerSetting(u8 num, u8 value)
{
    u8 err;

    //printf("num %d BLEVoxDuration %d \n", num, BLEVoxDuration[num]);

    if(u8BLEStatus == BLE_CONNECTED && BLEVoxDuration[num] != BLE_VOX_OFF)
    {
        if(BLEVoxDuration[num] < VOX_CMD_SAMECH_INTERVAL || u8BLEVoxCmdDiff < VOX_CMD_DIFFCH_INTERVAL)
            return 0;

        OSSemPend(bleSemReq, BLE_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            printf("Error: BLE_VoxTriggerSetting bleSemReq is %d.\n", err);
            return 1;
        }

        BLESendCommand(BLE_CMD_SET_ALARM, num);
        OSSemPend(bleSemFin, BLE_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            printf("Error: BLE_VoxTriggerSetting bleSemFin is %d.\n", err);
            OSSemPost(bleSemReq);
            return 1;
        }
        OSSemPost(bleSemReq);

        BLEVoxDuration[num] = 0;
        u8BLEVoxCmdDiff = 0;

    }
    else
    {
        //printf("Wrist band is not connected / VOX OFF, can't sent command\n");
    }

    return 0;
}

/*
*********************************************************************************************************
*                                         BLE Init
*
* Description : Create a BLE_TASK to recevie command(char to string) and parse command
*
* Arguments   : void
*
* Returns     : 0: success
*               1: fail
*********************************************************************************************************
*/
u8 BLEInit(void)
{
    u8 i = 0;
    //uart TX & NTF pull high default disable for leak power
    GPIO_CFG c;
    c.ena = GPIO_ENA;
    c.dir = GPIO_DIR_IN;
    c.level = GPIO_LEVEL_LO;
    c.inPullUp = GPIO_IN_PULLUP_DISA;

    gpioConfig(GPIO_GROUP_NTF, GPIO_BIT_NTF,&c);
    
    gpioSetBLEPWR(BLE_ON);
    
    for(i = 0; i < 5; i++)
    {
        OSTimeDly(1);
        if(gpioGetBLENTP())
        {
            u8BLEexist = FALSE;
            printf("No BLE module\n");
        }
        else
        {
            u8BLEexist = TRUE;
            printf("Detect BLE module\n");
            break;
        } 
    }

    if(u8BLEexist)//off pull high for leak pwr
    {
        c.ena = GPIO_ENA;
        c.dir = GPIO_DIR_IN;
        c.level = GPIO_LEVEL_LO;
        c.inPullUp = GPIO_IN_PULLUP_ENA;

        gpioConfig(GPIO_GROUP_NTF, GPIO_BIT_NTF,&c);
    }
    else
    {
        //keep trying
        g_BLEmoduleDetCount = 0;
    }

    /* Create the semaphore */
    bleSemReq = OSSemCreate(1); //init for a share resource
    bleSemFin = OSSemCreate(0); //mean one or more event happen
    bleSemScan = OSSemCreate(1); //might need it for scanning list /getting list at the same time

    return 1;
}

u8 BLEUninit(void)
{
    u8 err;

    /* Delete the semaphore */
    OSSemDel(bleSemReq, OS_DEL_ALWAYS, &err);
    OSSemDel(bleSemFin, OS_DEL_ALWAYS, &err);
    OSSemDel(bleSemScan, OS_DEL_ALWAYS, &err);

    return 1;
}

/*
*********************************************************************************************************
*                                   BLE Feedback cmd from module
*
* Description : Receive char by char and End up with a correct length, 
*               depends on each command's format (OK+XXXX-xxxxxx)
*
* Arguments   : *pcString : emtpy to fill up cmdString
*
* Returns     : 0: success
*               1: fail
*********************************************************************************************************
*/
#if(1)
u8 BLEGetUartCmd(u8 *pcString)
{
    u8 idx = 0;
    u8 i;
    u8 macAddr[BLE_MAC_ADDR_MAX] = {0};
    u8 temp = 0, checksum = 0;
    while(1)
    {
        idx=0;

        /*preamble*/
        pcString[idx++]=receive_char(BLE_UART_ID); 

        if(pcString[BLE_CMD_BIT_PREAMBLE] == (u8)BLE_MASTER_PREAMBLE) //88
        {
            pcString[idx++]=receive_char(BLE_UART_ID);
            if(pcString[BLE_CMD_BIT_PREAMBLE2] == (u8)BLE_CMD_ACK_PREAMBLE)
            {
                pcString[idx]=receive_char(BLE_UART_ID);
                if(pcString[idx] == receive_char(BLE_UART_ID))
                {
                    if(pcString[idx] == (u8)BLE_CMD_ACK_BYTE)
                    {
                        printf("Get cmd : 58 57 96 96\n");
                        OSSemPost(bleSemFin);
                    }
                    else if(pcString[idx] == (u8)BLE_CMD_ACK_ERR_BYTE)
                    {
                        printf("Get cmd : 58 57 B5 B5\n");
                        OSSemPost(bleSemFin);
                    }
                }
            }
            else if(pcString[BLE_CMD_BIT_PREAMBLE2] == (u8)BLE_CMD_SCAN_DEV_PREAMBLE)//81
            {

                pcString[idx++]=receive_char(BLE_UART_ID); //dev number
                checksum = pcString[BLE_CMD_BIT_CMD1];
                
                memset(macAddr, 0, BLE_MAC_ADDR_MAX);
                //data
                
                for(i=0; i < BLE_MAC_ADDR_MAX -1; i++)
                {
                    //pcString[idx]=receive_char(BLE_UART_ID);
                    macAddr[i] = receive_char(BLE_UART_ID);
                    printf("%02x_",macAddr[i]);
                    checksum += macAddr[i];
                    //idx++;
                }

                if(checksum == receive_char(BLE_UART_ID))
                {
                    BLESendCommand(BLE_CMD_ACK, pcString[BLE_CMD_BIT_CMD1]);
                    mBLEScanList.sDev[u8Total].devID = pcString[BLE_CMD_BIT_CMD1];
                    memcpy(mBLEScanList.sDev[u8Total].devName, &macAddr[0], sizeof(char) * BLE_MAC_ADDR_MAX);
                    u8Total++;
                }
                else
                {
                    printf("BLE get not complete cmd, SKIP\n");
                }
            }
            else if(pcString[BLE_CMD_BIT_PREAMBLE2] == (u8)BLE_CMD_TIMEREADY_DEV_PREAMBLE)
            {

                pcString[idx++]=receive_char(BLE_UART_ID);
                //mBLEScanList.sDev[pcString[BLE_CMD_BIT_CMD1]].devID = pcString[BLE_CMD_BIT_CMD1];
                // need not MAC addr

                printf("58 55 %02x ", pcString[BLE_CMD_BIT_CMD1]);
                /*
                temp = 0;
                for(i=0;i<u8Total;i++)
                {
                    if(pcString[BLE_CMD_BIT_CMD1] == mBLEScanList.sDev[i].devID)
                    {
                        mBLEScanList.sDev[i].status = 1;
                        temp = 1;
                        break;
                    }
                }
                if(temp != 1)
                {
                    mBLEScanList.sDev[u8Total].devID = pcString[BLE_CMD_BIT_CMD1];
                    mBLEScanList.sDev[u8Total].devName[0] = 0x41;
                    mBLEScanList.sDev[u8Total].status = 1;
                    u8Total++;
                }
                */
                mBLEScanList.sDev[0].devID = pcString[BLE_CMD_BIT_CMD1];
                mBLEScanList.sDev[0].devName[0] = 0x41;
                mBLEScanList.sDev[0].status = 1;

                u8BLEStatus = BLE_CONNECTED;
                u8BLEDisconnectCount = 0;
                u8ConnectedIdx = pcString[BLE_CMD_BIT_CMD1];
                
                if(receive_char(BLE_UART_ID) !=  (u8)BLE_CMD_TIMEREADY_DEV_PREAMBLE) continue;
                if(receive_char(BLE_UART_ID) !=  (u8)BLE_MASTER_PREAMBLE) continue;
                printf("55 58\n");
                uiSentKeyToUi(UI_KEY_UPDATE_BLE_ICON);
                g_BLEsyncTimeError = 1; //BLE_syncTime();
            }
            else if(pcString[BLE_CMD_BIT_PREAMBLE2] == (u8)BLE_CMD_DISCONNECT_DEV_PREAMBLE)
            {

                pcString[idx++]=receive_char(BLE_UART_ID);

                u8Total = 0;
                memset((void *)&mBLEScanList, 0, sizeof(BLE_SCAN_LIST));
                printf("58 56 %02x ", pcString[BLE_CMD_BIT_CMD1]);

                u8BLEStatus = BLE_ADVERTISING;
                u8BLEDisconnectCount = 0; //disconnect count to 30 sec
                u8ConnectedIdx = 10;

                if(receive_char(BLE_UART_ID) !=  (u8)BLE_CMD_DISCONNECT_DEV_PREAMBLE) continue;
                if(receive_char(BLE_UART_ID) !=  (u8)BLE_MASTER_PREAMBLE) continue;
                printf("56 58\n");
                printf("BLE DisConnect %02x \n", pcString[BLE_CMD_BIT_CMD1]);

            }
        }
        else
        {
            printf("UARTB %02x broken data \n",pcString[BLE_CMD_BIT_PREAMBLE]);
            continue;
        }
        
        printf("[BLEGetUartCmd] Get a command Done\n");
    }
}

#else

u8 BLEGetUartCmd(u8 *pcString)
{
    u8 idx = 0;
    u8 i;
    u8 macAddr[BLE_MAC_ADDR_MAX] = {0};
    u8 temp = 0, checksum = 0;
    while(1)
    {
        idx=0;

        /*preamble*/
        pcString[idx++]=receive_char(BLE_UART_ID); 

        if(pcString[BLE_CMD_BIT_PREAMBLE] == (u8)BLE_RSV_PREAMBLE_O)
        {
            pcString[idx++]=receive_char(BLE_UART_ID);
            if(pcString[BLE_CMD_BIT_PREAMBLE2] == (u8)BLE_RSV_PREAMBLE_K)
            {
                printf("Recevie OK\n");
                OSSemPost(bleSemFin);
                return 0;
            }
        }
        else if(pcString[BLE_CMD_BIT_PREAMBLE] == (u8)BLE_RSV_PREAMBLE_C)
        {
            for(i=0; i < 13-1 ; i++) // CONNECT ERROR //13 chars
            {
                pcString[idx++]=receive_char(BLE_UART_ID);
                if(auConnectError[i] != pcString[idx-1]) break;
            }
            printf("Get cmd : %s\n", pcString);
            OSSemPost(bleSemFin);
            return 0;
        }
        else if(pcString[BLE_CMD_BIT_PREAMBLE] == (u8)BLE_RSV_PREAMBLE_N)
        {
            for(i=0; i < 10-1 ; i++) // NO CONNECT //10 chars
            {
                pcString[idx++]=receive_char(BLE_UART_ID);
                if(auNoConnect[i] != pcString[idx-1]) break;
        }
            printf("Get cmd : %s\n", pcString);
            OSSemPost(bleSemFin);
            return 0;
        }
        else if(pcString[BLE_CMD_BIT_PREAMBLE] == (u8)BLE_RSV_PREAMBLE_A)
        {
            for(i=0; i < 15-1 ; i++) // Already CONNECT //15 chars
            {
                pcString[idx++]=receive_char(BLE_UART_ID);
                if(auAlreadyConnect[i]!= pcString[idx-1]) break;
            }
            printf("Get cmd : %s\n", pcString);
            OSSemPost(bleSemFin);
            return 0;
        }
        else if(pcString[BLE_CMD_BIT_PREAMBLE] == (u8)BLE_MASTER_PREAMBLE) //88
        {
            pcString[idx++]=receive_char(BLE_UART_ID);
            if(pcString[BLE_CMD_BIT_PREAMBLE2] == (u8)BLE_MASTER_PREAMBLE) //avoid 58 58 51 XX when collect last cmd
            {
                pcString[BLE_CMD_BIT_PREAMBLE2]=receive_char(BLE_UART_ID);
            }
            if(pcString[BLE_CMD_BIT_PREAMBLE2] == (u8)BLE_CMD_SCAN_DEV_PREAMBLE)//81
            {
                pcString[idx++]=receive_char(BLE_UART_ID);

                temp = 0;
                for(i=0;i<u8Total;i++)
                {
                    if(pcString[BLE_CMD_BIT_CMD1] == mBLEScanList.sDev[i].devID)
                    {
                        temp = 1;
                        break;
                    }
                }
                if(temp != 1)
                {
                    mBLEScanList.sDev[u8Total].devID = pcString[BLE_CMD_BIT_CMD1];
                    mBLEScanList.sDev[u8Total].devName[0] = 0x41;
                    u8Total++;
                }
                // need not MAC addr

                printf("58 51 %02x ", pcString[BLE_CMD_BIT_CMD1]);
                
            #if(MacAddr == TRUE)
                memset(macAddr, 0, BLE_MAC_ADDR_MAX);
                //data
                
                for(i=0; i < BLE_MAC_ADDR_MAX -1; i++)
                {
                    //pcString[idx]=receive_char(BLE_UART_ID);
                    macAddr[i] = receive_char(BLE_UART_ID);
                    printf("%02x_",macAddr[i]);
                    //idx++;
                }

                //memcpy(mBLEScanList.sDev[u8Total].devName, &macAddr[0], sizeof(char) * BLE_MAC_ADDR_MAX);
            #endif

                if(receive_char(BLE_UART_ID) !=  (u8)BLE_CMD_SCAN_DEV_PREAMBLE) continue;
                if(receive_char(BLE_UART_ID) !=  (u8)BLE_MASTER_PREAMBLE) continue;

                printf("51 58\n");
            }
            else if(pcString[BLE_CMD_BIT_PREAMBLE2] == (u8)BLE_CMD_TIMEREADY_DEV_PREAMBLE)
            {
                pcString[idx++]=receive_char(BLE_UART_ID);
                //mBLEScanList.sDev[pcString[BLE_CMD_BIT_CMD1]].devID = pcString[BLE_CMD_BIT_CMD1];
                // need not MAC addr

                printf("58 55 %02x ", pcString[BLE_CMD_BIT_CMD1]);
            #if(MacAddr == TRUE)
                memset(macAddr, 0, BLE_MAC_ADDR_MAX);
                //data
                for(i=0; i < BLE_MAC_ADDR_MAX - 1; i++)
                {
                    //pcString[idx]=receive_char(BLE_UART_ID);
                    macAddr[i] = receive_char(BLE_UART_ID);
                    printf("%02x_",macAddr[i]);
                    //idx++;
                }

                //memcpy(mBLEScanList.sDev[i].devName, &macAddr[0], sizeof(char) * BLE_MAC_ADDR_MAX);
            #endif

                temp = 0;
                for(i=0;i<u8Total;i++)
                {
                    if(pcString[BLE_CMD_BIT_CMD1] == mBLEScanList.sDev[i].devID)
                    {
                        mBLEScanList.sDev[i].status = 1;
                        temp = 1;
                        break;
                    }
                }
                if(temp != 1)
                {
                    mBLEScanList.sDev[u8Total].devID = pcString[BLE_CMD_BIT_CMD1];
                    mBLEScanList.sDev[u8Total].devName[0] = 0x41;
                    mBLEScanList.sDev[u8Total].status = 1;
                    u8Total++;
                }
                u8BLEStatus = BLE_CONNECTED;

                if(receive_char(BLE_UART_ID) !=  (u8)BLE_CMD_TIMEREADY_DEV_PREAMBLE) continue;
                if(receive_char(BLE_UART_ID) !=  (u8)BLE_MASTER_PREAMBLE) continue;
                printf("55 58\n");
                BLE_syncTime();
            }
            else if(pcString[BLE_CMD_BIT_PREAMBLE2] == (u8)BLE_CMD_DISCONNECT_DEV_PREAMBLE)
            {

                pcString[idx++]=receive_char(BLE_UART_ID);

                u8Total = 0;
                memset((void *)&mBLEScanList, 0, sizeof(BLE_SCAN_LIST));
                printf("58 56 %02x ", pcString[BLE_CMD_BIT_CMD1]);

            #if(MacAddr == TRUE)
                memset(macAddr, 0, BLE_MAC_ADDR_MAX);
                //data
                for(i=0; i < BLE_MAC_ADDR_MAX - 1; i++)
                {
                    //pcString[idx]=receive_char(BLE_UART_ID);
                    macAddr[i] = receive_char(BLE_UART_ID);
                    printf("%02x_",macAddr[i]);
                    //idx++;
                }
            #endif
                u8BLEStatus = BLE_ADVERTISING;
                if(receive_char(BLE_UART_ID) !=  (u8)BLE_CMD_DISCONNECT_DEV_PREAMBLE) continue;
                if(receive_char(BLE_UART_ID) !=  (u8)BLE_MASTER_PREAMBLE) continue;
                printf("56 58\n");
                printf("BLE DisConnect %02x \n", pcString[BLE_CMD_BIT_CMD1]);

            }
        }
        else if(pcString[BLE_CMD_BIT_PREAMBLE] == (u8)BLE_CMD_SCAN_DEV_PREAMBLE)//81
        {
            pcString[idx++]=receive_char(BLE_UART_ID);
            if(pcString[idx-1] == (u8)BLE_MASTER_PREAMBLE) continue;
            //UI can't recognize which device is the target
            //if the order is changed and connect to another, it would be fine
            //if(pcString[BLE_CMD_BIT_CMD1] == 0) //broadcast status, reset data, if data is the first item
            //{
            //    memset(&mBLEScanList, 0, sizeof(BLE_SCAN_LIST));
            //}

            temp = 0;
            for(i=0;i<u8Total;i++)
            {
                if(pcString[BLE_CMD_BIT_CMD1-1] == mBLEScanList.sDev[i].devID)
                {
                    temp = 1;
                    break;
                }
            }
            if(temp != 1)
            {
                mBLEScanList.sDev[u8Total].devID = pcString[BLE_CMD_BIT_CMD1-1];
                mBLEScanList.sDev[u8Total].devName[0] = 0x41;
                u8Total++;
            }

            // need not MAC addr
            printf("51 %02x ", pcString[BLE_CMD_BIT_CMD1-1]);

        #if(MacAddr == TRUE)
            memset(macAddr, 0, BLE_MAC_ADDR_MAX);
            //data

            for(i=0; i < BLE_MAC_ADDR_MAX -1; i++)
            {
                //pcString[idx]=receive_char(BLE_UART_ID);
                macAddr[i] = receive_char(BLE_UART_ID);
                printf("%02x_",macAddr[i]);
                //idx++;
            }

            //memcpy(mBLEScanList.sDev[u8Total].devName, &macAddr[0], sizeof(char) * BLE_MAC_ADDR_MAX);
       #endif

            if(receive_char(BLE_UART_ID) !=  (u8)BLE_CMD_SCAN_DEV_PREAMBLE) continue;
            if(receive_char(BLE_UART_ID) !=  (u8)BLE_MASTER_PREAMBLE) continue;

            printf("51\n");

        }
        else
        {
            printf("UARTB %02x broken data \n",pcString[BLE_CMD_BIT_PREAMBLE]);
            continue;
        }
        
        printf("[BLEGetUartCmd] Get a command Done\n");
    }
}

#endif
/*
*********************************************************************************************************
*                                    BLE Send Command to module
*
* Description : transfer cmdID to cmdString and send to module by UartB
*
* Arguments   : *pcString : feedback-cmd
*
* Returns     : 0: success
*               1: fail
*********************************************************************************************************
*/
void BLESendCommand(u8 cmd, u8 data)
{
    unsigned char cmdStr[32];
    u8 idx = 0, i;

    printf("BLESendCommand cmd = %d data =%d \n", cmd, data);
    cmdStr[idx++] = BLE_MASTER_PREAMBLE;
    switch(cmd)
    {
        case BLE_CMD_SET_ALARM:
            cmdStr[idx++] = BLE_CMD_SET_ALARM_PREAMBLE;
            switch(data)
            {
                case 0:
                    cmdStr[idx++] = 0x00; //channel information 0x0001
                    cmdStr[idx++] = 0x01; //channel information                    
                    break;
                case 1:
                    cmdStr[idx++] = 0x00; //channel information 0x0010
                    cmdStr[idx++] = 0x10; //channel information
                    break;
                case 2:
                    cmdStr[idx++] = 0x00; //channel information 0x0011
                    cmdStr[idx++] = 0x11; //channel information
                    break;
                case 3:
                    cmdStr[idx++] = 0x01; //channel information 0x0100
                    cmdStr[idx++] = 0x00; //channel information
                    break;
            }
            cmdStr[idx++] = 0xff; //00 normal FF urgent (do we need normal status?)
            cmdStr[idx++] = 0xff; //00 normal FF urgent
            cmdStr[idx++] = BLE_CMD_SET_ALARM_PREAMBLE;
            cmdStr[idx++] = BLE_MASTER_PREAMBLE;
            break;
        case BLE_CMD_SCAN_DEV:
            break;
        case BLE_CMD_TIME_SYNC:
            cmdStr[idx++] = BLE_CMD_TIME_SYNC_PREAMBLE;
            cmdStr[idx++] = 20; //year 20XX
            cmdStr[idx++] = g_LocalTime.year;
            cmdStr[idx++] = g_LocalTime.month;
            cmdStr[idx++] = g_LocalTime.day;
            cmdStr[idx++] = g_LocalTime.hour;
            cmdStr[idx++] = g_LocalTime.min;
            cmdStr[idx++] = g_LocalTime.sec;
            cmdStr[idx++] = BLE_CMD_TIME_SYNC_PREAMBLE;
            cmdStr[idx++] = BLE_MASTER_PREAMBLE;
            
            printf("Sync time  20%02d/%02d/%02d %02d:%02d:%02d\n", g_LocalTime.year, g_LocalTime.month, g_LocalTime.day, g_LocalTime.hour, g_LocalTime.min, g_LocalTime.sec);
            break;
        case BLE_CMD_CONNECT_DEV:
            cmdStr[idx++] = BLE_CMD_CONNECT_DEV_PREAMBLE;
            cmdStr[idx++] = data;
            cmdStr[idx++] = BLE_CMD_CONNECT_DEV_PREAMBLE;
            cmdStr[idx++] = BLE_MASTER_PREAMBLE;
            break;
        case BLE_CMD_UNPAIR_DEV: //clear(erase) auto-connect information(unpair)
            cmdStr[idx++] = BLE_CMD_UNPAIR_DEV_PREAMBLE;
            cmdStr[idx++] = data;
            cmdStr[idx++] = 0xff;
            cmdStr[idx++] = 0xff;
            cmdStr[idx++] = BLE_CMD_UNPAIR_DEV_PREAMBLE;
            cmdStr[idx++] = BLE_MASTER_PREAMBLE;
            break;
        case BLE_CMD_ACK:
            cmdStr[idx++] = BLE_CMD_ACK_PREAMBLE;
            cmdStr[idx++] = data;
            cmdStr[idx++] = BLE_CMD_ACK_BYTE;
            cmdStr[idx++] = data + BLE_CMD_ACK_BYTE;
            break;
    }


    for( i = 0; i < idx; i++)
    {
        //sendburst(BLE_UART_ID, cmdStr, len)
        sendData(BLE_UART_ID, &cmdStr[i]);
        printf("%02x", cmdStr[i]);
    }
    printf("\n");
}

#endif  /* end #if (BLE_SUPPORT )  */
