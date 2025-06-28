/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    HomRF.c

Abstract:

    The routines of Home Sensor Interface Unit.
    
    
Environment:

        ARM RealView Developer Suite

Revision History:
    
    2014/09/16    Roy Create  
*/





#include "general.h"
#include "rfiuapi.h"
#include "uartapi.h"
#include "homerf.h"
#include "Uiapi.h"
#include "uiKey.h"
#include "spiapi.h"
#include "sysapi.h"
#include "P2pserver_api.h"
#include "HomeRF_project.h"
//#include "Dcfapi.h"
#if LWIP2_SUPPORT
#include "../LwIP_2.0/include/tutk_P2P/MR8200def_homeautomation.h"
#else
#include "../LwIP/include/tutk_P2P/MR8200def_homeautomation.h"
#endif

#if HOME_RF_SUPPORT
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

u32 gHomeRFHostID;
u32 gHomeRFVersion;
u32 gHomeRFSensorID;
//HOMERF_SENS getDeviceData[HOME_SENSOR_MAX];
OS_FLAG_GRP  *gHomeRFEventFlagGrp;
OS_FLAG_GRP  *gHomeRFStatusFlagGrp;
OS_FLAG_GRP  *gHomeRFFWStatusFlagGrp;
u8  gHomeRFOpMode=0;
u8  gHomeRFSensorCnt=0;
u32 gHomeRFPairCnt=0;
u8  gHomeRFPairStat=HOMERF_PAIR_NORMAL;
u64 homeRFPID;
u32 homeRFFieldID;
u32 homeRFChecksum=0;
u8  gHomeRFCheckAlert[16];   /*用來給UI判斷哪些Sensor有Trigger ,Total 128bit*/
u8* homeRFUpdateSrcAddr=0;
u32 homeRFCurFWPos=0;
u8  homeRFLogData[HOMERF_LOG_DATA_SIZE];
u16 DefaultSensorCnt=0; /*用於DefaultName後面之編號0-999*/
u8  homeRFSceneAppMode=0; /*0: non-App Mode ; 1: Arm Mode ; 2: DisArm Mode ; 3: Home Mode ; 4...: App-Mode */
u8  homeRFLowBattery=0; 
u8  homeSwitchControl=0;
unsigned int Siren_trigger_time=0;

HOMERF_SENSOR_LIST  *gHomeRFSensorList;
HOMERF_ROOM_LIST    *gHomeRFRoomList;
HOMERF_SCENE_LIST   *gHomeRFSceneList;
OS_STK homeRFTaskStack[HOMERF_TASK_STACK_SIZE];
HOMERF_CMD_QUEUE     homeRFCmdQueue[HOMERF_QUEUE_SIZE];
/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
//extern u8 UI_HA_Sensor_Beep_Flag;
#if MARSS_SUPPORT
extern BYTE HostCmd[48];// __attribute__ ((aligned (4)));
extern BYTE HostLen;               // host command length
extern void procHostCmd(void);
#endif
#if 1//HOME_RF_SUPPORT
u32 UI_HA_OutDoorCountdown;
u32 UI_HA_InDoorCountdown;
u8 UI_HA_Sensor_Beep_Flag;
#endif
extern void P2pHAcmdHashInit(void);
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

void homeRFAddCmdQueue(u32 cmdType, u32 sIdx);
void homeRFWriteLog(u32 idx, u8 type, u8 subType, u8 event , u16 eventStatus, u8 battery);
u8 homeRFSetSirenStatus(u8 act);
u8 homeRFGetDeviceType(u16 type);

/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */


void homeRFSentKeyToUI(u8 key)
{
    bool isFree=TRUE;   
    isFree=uiSentKeyToUi(key);

    while(isFree== FALSE)
    {
        OSTimeDly(2);  /*delay 100ms*/
        isFree=uiSentKeyToUi(key);
    }
}

void homeRFReset(void)
{
	printf("HOME RF CHECKSUM ERROR RESET\n"); 
    memset(gHomeRFSensorList->sSensor, 0, sizeof(HOMERF_SENSOR_DATA)*HOMERF_SENSOR_MAX);
    memset(gHomeRFRoomList->sRoom,     0, sizeof(HOMERF_ROOM_DATA)*HOMERF_ROOM_MAX);
    memset(gHomeRFSceneList->sScene,   0, sizeof(HOMERF_SCENE_DATA)*HOMERF_SCENE_MAX);
    homeRFChecksum=0;
    spiWriteHomeRF(SPI_HOMERF_RODATA);
    spiWriteHomeRF(SPI_HOMERF_CONFIG);
    spiWriteHomeRF(SPI_HOMERF_SENSOR);
    spiWriteHomeRF(SPI_HOMERF_ROOM);
    spiWriteHomeRF(SPI_HOMERF_SCENE); 
}

u8 homeRFInit(void)
{
    u8 err;
    u8 i;
    u8 ret;
    u32 value;
    
    gHomeRFEventFlagGrp  = OSFlagCreate(0x00000000, &err);    
    gHomeRFStatusFlagGrp = OSFlagCreate(0x00000000, &err);  
    gHomeRFFWStatusFlagGrp = OSFlagCreate(0x00000000, &err);  
    OSTaskCreate(HOMERF_TASK, HOMERF_TASK_PARAMETER, HOME_RF_TASK_STACK, HOMERF_TASK_PRIORITY);
    spiReadHomeRF(SPI_HOMERF_RODATA);
    if(homeRFChecksum == 0xFFFFFFFF)
    {
		printf("HOME RF CHECKSUM ERROR RESET\n"); 
        memset(gHomeRFSensorList->sSensor, 0, sizeof(HOMERF_SENSOR_DATA)*HOMERF_SENSOR_MAX);
        memset(gHomeRFRoomList->sRoom,     0, sizeof(HOMERF_ROOM_DATA)*HOMERF_ROOM_MAX);
        memset(gHomeRFSceneList->sScene,   0, sizeof(HOMERF_SCENE_DATA)*HOMERF_SCENE_MAX);
        homeRFChecksum=0;
        spiWriteHomeRF(SPI_HOMERF_RODATA);
        spiWriteHomeRF(SPI_HOMERF_CONFIG);
        spiWriteHomeRF(SPI_HOMERF_SENSOR);
        spiWriteHomeRF(SPI_HOMERF_ROOM);
        spiWriteHomeRF(SPI_HOMERF_SCENE);         
    }
    else
        printf("HOME RF DATA VALID\n"); 
    memset_hw(homeRFCmdQueue, 0, sizeof(HOMERF_CMD_QUEUE)*HOMERF_QUEUE_SIZE);
    //homeRFSendToSensor(HOMERF_SEND_RF_VERSON,0);
#if((HOME_RF_OPTION == HOME_SENSOR_MARS) || (HOME_RF_OPTION == HOME_SENSOR_TRANWO))
    homeRFSendCommand(HOMERF_SEND_SET_RFID, 0);
//    homeRFSendCommand(HOMERF_SEND_RF_VERSON, 0);
    /*homeRFSendCommand(HOMERF_SEND_SET_RFID, 0);
    spiReadHomeRF(SPI_HOMERF_READ_RODATA);

    if((homeRFFieldID==0x00) || (homeRFFieldID == 0xFFFFFFFF))
    {
        homeRFFieldID=(uiMACAddr[5]) | (uiMACAddr[4]<<8) |  (uiMACAddr[3] << 16) | (uiMACAddr[2] << 24);

        spiWriteHomeRF(SPI_HOMERF_WRITE_RODATA);
        homeRFSendCommand(HOMERF_SEND_SET_RFID, 0);
    }*/
#endif
    spiReadHomeRF(SPI_HOMERF_CONFIG);
    spiReadHomeRF(SPI_HOMERF_SENSOR);
    spiReadHomeRF(SPI_HOMERF_ROOM);
    spiReadHomeRF(SPI_HOMERF_SCENE); 

    for(i=0; i<HOMERF_SENSOR_MAX; i++)
    {

        if((gHomeRFSensorList->sSensor[i].sID !=0) && (gHomeRFSensorList->sSensor[i].sID !=0xffffffff))
        {       
            gHomeRFSensorCnt++; 
        }
        gHomeRFSensorList->sSensor[i].lifeCount = 0;
        gHomeRFSensorList->sSensor[i].alarmTimer = 1;/* default alarm is right now, Add by Paul@20190401*/
    }
#if(HOME_RF_OPTION == HOME_SENSOR_TRANWO)   
    if((gHomeRFSceneList->sScene[0].sceneID != DEFAULT_SCENEID_ALARM) || (gHomeRFSceneList->sScene[1].sceneID != DEFAULT_SCENEID_DISALARM) || (gHomeRFSceneList->sScene[2].sceneID != DEFAULT_SCENEID_INHOMEMODE))
    {
        homeRFSetDefaultScene(); 
    }
#else
    if((gHomeRFSceneList->sScene[0].sceneID != DEFAULT_SCENEID_ALARM) || (gHomeRFSceneList->sScene[1].sceneID != DEFAULT_SCENEID_DISALARM))
    {
        homeRFSetDefaultScene(); 
    }
#endif     

	//homeRFSendCommand(HOMERF_SEND_QUERY_ALL, 0);
	//P2pHAcmdHashInit();

}


s32 homeRFgetDecemail(u8 idx)
{
    switch(idx)
    {
        case 0:
           return 50;  /* 2^-1 , 0.5*/
        case 1:
           return 25;  /* 2^-2, 0.25*/
        case 2:
           return 12;  /* 2^-3 0.125 */  
        case 3:
           return 6;   /* 2^-3 0.0625 */
        case 4:
        case 5:
        case 6:
           return 1;  
    } 
    
    return 0;   
}





/*
    return value:
       0 - Fail
       1 - Success
*/
u8 homeRFSendToSensor(u8 cmd_type, u8 sensor)
{
    u8 err;
    u32 waitFlag=0;
    
    
    if(cmd_type == HOMERF_SEND_PAIR)
    {
        gHomeRFOpMode=HOMERF_OP_PAIR;
        homeRFSendCommand(cmd_type, sensor);
    }
    else if(cmd_type == HOMERF_SEND_APP_PAIR)
    {
        gHomeRFOpMode=HOMERF_OP_APP_PAIR;
        homeRFSendCommand(cmd_type, sensor);
    }
    else
    {
        homeRFSendCommand(cmd_type, sensor);
        if((cmd_type == HOMERF_SEND_PLUG_ON) || (cmd_type == HOMERF_SEND_PLUG_OFF) || 
           (cmd_type == HOMERF_SEND_SIREN_ON) || (cmd_type == HOMERF_SEND_SIREN_OFF) ||
           (cmd_type == HOMERF_SEND_SWITCH_ON) || (cmd_type == HOMERF_SEND_SWITCH_OFF))
            homeRFAddCmdQueue(cmd_type, sensor);
    }
    return HOMERF_SEND_CMD_OK;
}


void homeRFRunPerSec(void)
{
    u8 i, err;
    u32 value;

    for(i=0; i< HOMERF_SENSOR_MAX; i++)
    {
        if((gHomeRFSensorList->sSensor[i].sID == 0) || (gHomeRFSensorList->sSensor[i].sID > 65535))
            continue;

        if(gHomeRFSensorList->sSensor[i].alarmTimer !=0)
        {
            gHomeRFSensorList->sSensor[i].alarmTimer--;
        }        
        else
        {
            if(gHomeRFSensorList->sSensor[i].status & HOMERF_SENSOR_STATUS_ALARM )
            {
                //printf("\x1B[91m ID:%d, TYPE:%d\x1B[0m\n", gHomeRFSensorList->sSensor[i].sID, gHomeRFSensorList->sSensor[i].type);
                gHomeRFSensorList->sSensor[i].status &= ~HOMERF_SENSOR_STATUS_ALARM;    
                OSFlagPost(gHomeRFEventFlagGrp, i+1, OS_FLAG_SET, &err);
				if((gHomeRFSensorList->sSensor[i].type == HOMERF_DEVICE_SIREN) || (gHomeRFSensorList->sSensor[i].type == HOMERF_DEVICE_IR))
				{
					//Do nothing.
				}
				else
				{
	                UpdateAPPSensorListStatus();
	                UpdateAPPSensorStatus(gHomeRFSensorList->sSensor[i].sID);
                }
                homeRFSentKeyToUI(UI_KEY_HOMERF_EVENT);
            }

        }
		if(gHomeRFSensorList->sSensor[i].type == HOMERF_DEVICE_IR)
			continue;
			
		if(gHomeRFSensorList->sSensor[i].lifeCount >= 0)
		{
        	gHomeRFSensorList->sSensor[i].lifeCount++;	
        	if((gHomeRFSensorList->sSensor[i].type == HOMERF_DEVICE_SIREN) || (gHomeRFSensorList->sSensor[i].type == HOMERF_DEVICE_TEMP_HYG))
        	{
        		if(gHomeRFSensorList->sSensor[i].lifeCount > HOMERF_SIREN_LIFE_CYCLE) /* 1min 30sec*/
		        {      
					printf("[HOEMRF]sID:%d,%d Battery Sensor Lost\n", gHomeRFSensorList->sSensor[i].sID,gHomeRFSensorList->sSensor[i].type);

					if(gHomeRFSensorList->sSensor[i].pushOnOff == HOMERF_SENSOR_ON)
					{
						homeRFSetSirenStatus(TRUE);
	                    gHomeRFCheckAlert[i>>3] |= (0x01 << (i%8));
						gHomeRFSensorList->sSensor[i].status |= HOMERF_SENSOR_STATUS_ALARM;
						homeRFSentEvent2UI(i);
						sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, gHomeRFSensorList->sSensor[i].sID,EVENT_SIREN | HOMERF_PUSHMSG_PREFIX);   
					}
					gHomeRFSensorList->sSensor[i].battery=0;
					gHomeRFSensorList->sSensor[i].lifeCount = 0;
					homeRFWriteLog(i, homeRFGetDeviceType(gHomeRFSensorList->sSensor[i].type), 0, HOMERF_EVENT_NONE, EventID_Unlink, 0);  //Lost Link Log			
				}
        	}
        	else
        	{
		        if(gHomeRFSensorList->sSensor[i].lifeCount > HOMERF_SENSOR_LIFE_CYCLE) /* 1hr 15min*/
		        {      
					printf("[HOEMRF]sID:%d Lost Link Log\n", gHomeRFSensorList->sSensor[i].sID);			
					if(gHomeRFSensorList->sSensor[i].pushOnOff == HOMERF_SENSOR_ON)
					{
	                    homeRFSetSirenStatus(TRUE);
						gHomeRFCheckAlert[i>>3] |= (0x01 << (i%8));
						gHomeRFSensorList->sSensor[i].status |= HOMERF_SENSOR_STATUS_ALARM;
						homeRFSentEvent2UI(i);
						sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, gHomeRFSensorList->sSensor[i].sID,EVENT_SIREN | HOMERF_PUSHMSG_PREFIX);   
					}
					gHomeRFSensorList->sSensor[i].lifeCount = 0;
		            gHomeRFSensorList->sSensor[i].battery=0;
					homeRFWriteLog(i, homeRFGetDeviceType(gHomeRFSensorList->sSensor[i].type), 0, HOMERF_EVENT_NONE, EventID_Unlink, 0);  //Lost Link Log			
				}
			}
		}
		else
			gHomeRFSensorList->sSensor[i].lifeCount = 0;
    }

    homeRFCheckAppPairSensor();
}


        
void homeRFReadFromFlash(u8* bufAddr, u8 type)
{
    HOMERF_ROOM_DATA* tempRoom;
    HOMERF_SCENE_DATA* tempScene;
    HOMERF_SENSOR_DATA* tempSensor;
    
    if(type == SPI_HOMERF_RODATA)
    {
        memcpy((void *)&homeRFChecksum, (void *)bufAddr,  sizeof(homeRFChecksum));
    }
    else if(type == SPI_HOMERF_CONFIG)
    {
        memcpy((void *)&gHomeRFPairCnt, (void *)bufAddr,  sizeof(gHomeRFPairCnt));
        memcpy((void *)&DefaultSensorCnt, (void *)(bufAddr+sizeof(gHomeRFPairCnt)),  sizeof(DefaultSensorCnt));
    }
    else if(type == SPI_HOMERF_ROOM)
    {
        tempRoom=(HOMERF_ROOM_DATA *)bufAddr;
        memcpy((void *)&gHomeRFRoomList->sRoom,(void *) bufAddr, sizeof(gHomeRFRoomList->sRoom)); 
    }
    else if(type == SPI_HOMERF_SCENE)
    {
        tempScene=(HOMERF_SCENE_DATA *)bufAddr;
        memcpy((void *)&gHomeRFSceneList->sScene,(void *)bufAddr, sizeof(gHomeRFSceneList->sScene));  
        
    }
    else if(type == SPI_HOMERF_SENSOR)
    {
        tempSensor=(HOMERF_SENSOR_DATA *)bufAddr;
        memset_hw((void *)&gHomeRFSensorList->sSensor, 0, sizeof(gHomeRFSensorList->sSensor));
        memcpy((void *)&gHomeRFSensorList->sSensor,(void *)bufAddr, sizeof(gHomeRFSensorList->sSensor));        
    }


}


u8 *homeRFWriteToFlash(u8* bufAddr, u8 type)
{
    HOMERF_ROOM_DATA* tempRoom;

    if(type == SPI_HOMERF_RODATA)
    {
//        memcpy((void *)bufAddr,(void *)&homeRFChecksum,  sizeof(homeRFChecksum));     
        return (u8 *)&homeRFChecksum;
    }
    else if(type == SPI_HOMERF_CONFIG)
    {
        memcpy((void *)bufAddr,(void *)&gHomeRFPairCnt,  sizeof(gHomeRFPairCnt)); 
        memcpy((void *)(bufAddr+sizeof(gHomeRFPairCnt)),(void *)&DefaultSensorCnt,  sizeof(DefaultSensorCnt));
        return bufAddr;
    }
    else if(type == SPI_HOMERF_ROOM)
    {
//        memcpy((void *)bufAddr,(void *)&gHomeRFRoomList->sRoom,  sizeof(gHomeRFRoomList->sRoom));  
        bufAddr = (u8 *)&gHomeRFRoomList->sRoom;
        tempRoom=(HOMERF_ROOM_DATA *)bufAddr;
        return (u8 *)bufAddr;
    }
    else if(type == SPI_HOMERF_SCENE)
    {
//        memcpy((void *)bufAddr,(void *)&gHomeRFSceneList->sScene,  sizeof(gHomeRFSceneList->sScene));  
        return (u8 *)&gHomeRFSceneList->sScene;
    }
    else if(type == SPI_HOMERF_SENSOR)
    {
//        DEBUG_GREEN("[%s] sSensor size = %d @ gList 0x%p to 0x%p\n",__func__,sizeof(gHomeRFSensorList->sSensor), gHomeRFSensorList, bufAddr);
//        DEBUG_RED("[%s] Checksum = 0x%X\n",__func__,csum(&gHomeRFSensorList->sSensor,SPI_HOMERF_SENSOR_SIZE));
        return (u8 *)&gHomeRFSensorList->sSensor;
//		DEBUG_GREEN("[%s] Checksum = 0x%X\n",__func__,csum((void *)&gHomeRFSensorList->sSensor,sizeof(gHomeRFSensorList->sSensor)));
//        memcpy((void *)bufAddr,(void *)&gHomeRFSensorList->sSensor,  sizeof(gHomeRFSensorList->sSensor));
//		DEBUG_GREEN("[%s] Checksum = 0x%X\n",__func__,csum((void *)&gHomeRFSensorList->sSensor,sizeof(gHomeRFSensorList->sSensor)));
    }
    
}


void homeRFAddCmdQueue(u32 cmdType, u32 sIdx)
{
    u8 i;

    for(i=0; i< HOMERF_QUEUE_SIZE; i++)
    {
        if(homeRFCmdQueue[i].sID == 0)
        {
            homeRFCmdQueue[i].sID = gHomeRFSensorList->sSensor[sIdx].sID ;
            homeRFCmdQueue[i].sType = gHomeRFSensorList->sSensor[sIdx].type;
            homeRFCmdQueue[i].cmdType = cmdType; 
            break;
        }
    }
}

void homeRFClearCmdQueue(u32 sID)
{
    u8 i;
    u8 idx;

    for(i=0; i<HOMERF_QUEUE_SIZE; i++)
    {
        if(homeRFCmdQueue[i].sID == sID)
        {
            memset(homeRFCmdQueue+i,0, sizeof(HOMERF_CMD_QUEUE));
            break;
        }
    }
}


void homeRFWriteLog(u32 idx, u8 type, u8 subType, u8 event , u16 eventStatus, u8 battery)
{
    RTC_DATE_TIME   localTime;
    u32 second;
    u32 ID;
    u8 i;
    u8 *ptr= homeRFLogData;

    memset(homeRFLogData, 0, HOMERF_LOG_DATA_SIZE);
    RTC_Get_Time(&localTime);
    second=RTC_Time_To_Second(&localTime);
    ID = gHomeRFSensorList->sSensor[idx].sID;

    memcpy(ptr, &second, sizeof(second));
    ptr += sizeof(second);

    memcpy(ptr, &gHomeRFSensorList->sSensor[idx].name, HOMERF_NAME_MAX );
    ptr += HOMERF_NAME_MAX;

    memcpy(ptr, &type, sizeof(type));
    ptr += sizeof(type);

    memcpy(ptr , &subType, sizeof(subType));
    ptr += sizeof(subType);

    memcpy(ptr, &ID, sizeof(ID));
    ptr +=sizeof(ID);

    memcpy(ptr, &event, sizeof(event));
    ptr +=sizeof(event);

    memcpy(ptr, &eventStatus, sizeof(eventStatus));
    ptr +=sizeof(eventStatus);

    memcpy(ptr, &battery, sizeof(battery));
    ptr +=sizeof(battery);
    
#if CDVR_iHome_LOG_SUPPORT
    dcfWriteLogFile(homeRFLogData, HOMERF_LOG_DATA_SIZE);
#endif
}

#if 0
u8 homeRFCheckSensorData(void)
{
	u8 i;
	
	for(i=0;i<HOMERF_SENSOR_MAX;i++)
	{
		if(gHomeRFSensorList->sSensor[i].sID != 0)
		{
			memset(gHomeRFSensorList->sSensor, 0, sizeof(gHomeRFSensorList->sSensor));
	        memset(gHomeRFRoomList->sRoom, 0, sizeof(gHomeRFRoomList->sRoom));
	        memset(gHomeRFSceneList->sScene, 0, sizeof(gHomeRFSceneList->sScene));
	        gHomeRFSensorCnt=0;
	        DefaultSensorCnt=0;
	        homeRFSendToSensor(HOMERF_SEND_DELETE_ALL, 0);
	        homeRFSendToSensor(HOMERF_SEND_SET_RFID, 0);
			spiWriteHomeRF(SPI_HOMERF_CONFIG);
	        spiWriteHomeRF(SPI_HOMERF_SENSOR);
	        spiWriteHomeRF(SPI_HOMERF_ROOM);
	        spiWriteHomeRF(SPI_HOMERF_SCENE);
			printf("Sync Complete.(Del All)\n");
			return 0;
		}
	}
	printf("Sync Complete.\n");
	return 1;
}
#endif
#if((HOME_RF_OPTION == HOME_SENSOR_MARS) || (HOME_RF_OPTION == HOME_SENSOR_BARVOTECH) || (HOME_RF_OPTION == HOME_SENSOR_SWANN) || (HOME_RF_OPTION == HOME_SENSOR_TRANWO))
void homeRFCheckSensorLive(void)
{
            

}





#if((HOME_RF_OPTION == HOME_SENSOR_MARS ) || (HOME_RF_OPTION == HOME_SENSOR_TRANWO))

/* 0: pair time out 
   1: pair ID is exist, refresh sensor ID
   2: pair ID isn't exist, insert sensor ID and pair ID
   3: the amount of pairing is exceeded;
*/

u8  homeRFCheckPID(u16 sensorID, u64 pairID, u8 *idx)
{
    u8  i;

	if((gHomeRFSensorList->sSensor[i].byteSameOldID == 0) ||(gHomeRFSensorList->sSensor[i].byteSameOldID == 0xff))
		gHomeRFSensorList->sSensor[i].byteSameOldID = gHomeRFSensorList->sSensor[i].sID; //Sean: 20170612 Add.
	printf("PAIR UID ret: 0x%X 0x%X\n", pairID.hi, pairID.lo);

    if(sensorID == 0xffff)
        return HOMERF_PAIR_TIMEOUT;

    for(i=0; i< HOMERF_SENSOR_MAX; i++)
    {
        if((gHomeRFSensorList->sSensor[i].sUID.hi == pairID.hi) && (gHomeRFSensorList->sSensor[i].sUID.lo == pairID.lo))
        {   
            gHomeRFSensorList->sSensor[i].sID=sensorID;
            gHomeRFSensorID=sensorID;
            *idx=i;
            return HOMERF_PAIR_SAME;
        }
    }

    for(i=0; i< HOMERF_SENSOR_MAX; i++)
    {
        if((gHomeRFSensorList->sSensor[i].sUID.hi == 0) && (gHomeRFSensorList->sSensor[i].sUID.lo == 0))
        {   
            gHomeRFSensorList->sSensor[i].sID=sensorID;
            gHomeRFSensorID=sensorID;
            *idx=i;
            memcpy(&gHomeRFSensorList->sSensor[i].sUID, &pairID, sizeof(u64));
		
            break;
        }
    }
	printf("[%s]ret = %s[idx = %d]\n", __func__, (i== HOMERF_SENSOR_MAX)?"FAIL":"SUCCESS", *idx);
    if(i== HOMERF_SENSOR_MAX)
        return HOMERF_PAIR_FAIL;
       
    return HOMERF_PAIR_SUCCESS;
}
#elif((HOME_RF_OPTION == HOME_SENSOR_BARVOTECH) || (HOME_RF_OPTION == HOME_SENSOR_SWANN))
/* 0: pair time out 
   1: pair ID is exist, refresh sensor ID
   2: pair ID isn't exist, insert sensor ID and pair ID
   3: the amount of pairing is exceeded;
*/

u8  homeRFCheckPID(u16 sensorID, u64 pairID, u8 *idx)
{
    u8  i;

    if(sensorID == 0xffff)
        return HOMERF_PAIR_TIMEOUT;

    for(i=0; i< HOMERF_SENSOR_MAX; i++)
    {
        if((gHomeRFSensorList->sSensor[i].sUID.hi == pairID.hi) && (gHomeRFSensorList->sSensor[i].sUID.lo == pairID.lo))
        { 
            gHomeRFSensorID=sensorID;
            *idx=i;
            return HOMERF_PAIR_SAME;
        }
    }

    for(i=0; i< HOMERF_SENSOR_MAX; i++)
    {
        if((gHomeRFSensorList->sSensor[i].sUID.hi == 0) && (gHomeRFSensorList->sSensor[i].sUID.lo == 0))
        {   
            gHomeRFSensorList->sSensor[i].sID=sensorID;
            gHomeRFSensorID=sensorID;
            *idx=i;
            memcpy(&gHomeRFSensorList->sSensor[i].sUID, &pairID, sizeof(u64));
            break;
        }
    }

    if(i== HOMERF_SENSOR_MAX)
        return HOMERF_PAIR_FAIL;
       
    return HOMERF_PAIR_SUCCESS;
}
#endif

void homeRFsendAckToRF(u8 cmd_type, u8 value)
{
    u8 cmdStr[16];
    u8 idx=0, i;
    u8 cmdID, checksum, len;

#if(HOME_RF_OPTION ==HOME_SENSOR_BARVOTECH)
    OSTimeDly(10);
#endif

    /* preamble*/
    cmdStr[idx++] = HOMERF_PREAMBLE_BYTE1;
    cmdStr[idx++] = HOMERF_PREAMBLE_BYTE2;
    
    switch(cmd_type)
    {  
        case HOMERF_RX_CMDID_PAIR_MODE:
//#if(HOME_RF_OPTION ==HOME_SENSOR_BARVOTECH)
        case HOMERF_RX_CMDID_TRIG_EVENET:
        case HOMERF_RX_CMDID_PERIOD_EVENT:
//#endif
            len=0x01;
            cmdID=(cmd_type & 0x0f) | 0x40;
            checksum = len+cmdID;

            cmdStr[idx++]=len;
            cmdStr[idx++]=cmdID;
            cmdStr[idx++]=checksum;
            break;
            
    }
#if MARSS_SUPPORT
	memcpy(HostCmd, cmdStr+3, 16-3);
	HostLen = len+1;
	procHostCmd();
//	myHostCmdIdx++;
//	if( myHostCmdIdx>=16)
//		myHostCmdIdx=0;
//	memcpy(&myHostCmdQueue[myHostCmdIdx][0], cmdStr, 16);
//	OSQPost(OSQHost_MarssEvent, &myHostCmdQueue[myHostCmdIdx][0] );
//	OSFlagPost (gMarssFlagGrp, 0x800,OS_FLAG_SET,0);
#else
    for(i=0; i<len+4; i++)
    {
        sendData(HOMERF_UART_ID,&cmdStr[i]);    
    }
#endif
    printf("\n");
	printf("[HOMERF]Send ACK cmd 0x%X \n",cmdID);
#if 0
	for(i=0; i<len+4; i++)
/* preamble(2 byte) + len(1 byte) + cmdID(1 byte) + checksum (1 byte) */
	{
		printf("%02x ",cmdStr[i]);		
	}
#endif
}


u8 homeRFGetDevice(u8 type, u8 subType)
{
    u8 type_idx;
    
    switch(type)
    {
        case HOMERF_MARS_TYPE_DOOR:
            type_idx= HOMERF_DEVICE_DOOR;
            break;
            
        case HOMERF_MARS_TYPE_PIR:
            type_idx= HOMERF_DEVICE_PIR;
            break;
            
        case HOMERF_MARS_TYPE_SIREN:
            type_idx= HOMERF_DEVICE_SIREN;
            break;
            
        case HOMERF_MARS_TYPE_REMOTE:
            if(subType == HOMERF_SUB_TYPE_PANIC)
                type_idx= HOMERF_DEVICE_PANIC;
            else
                type_idx= HOMERF_DEVICE_IR;
            break;
            
        case HOMERF_MARS_TYPE_PLUG:
            type_idx= HOMERF_DEVICE_PLUG;
            break;
            
        case HOMERF_MARS_TYPE_TEMP_HUMIDITY:
            type_idx= HOMERF_DEVICE_TEMP_HYG;
            break;
            
        case HOMERF_MARS_TYPE_TEMP:
            type_idx= HOMERF_DEVICE_TEMP;
            break;
            
        case HOMERF_MARS_TYPE_HUMIDITY:
            type_idx= HOMERF_DEVICE_HUM;
            break;
            
        case HOMERF_MARS_TYPE_ROUTER:
            type_idx= HOMERF_DEVICE_ROUTER;
            break;
            
        case HOMERF_MARS_TYPE_LEAK:
            type_idx= HOMERF_DEVICE_LEAK;
            break;
        
        case HOMERF_MARS_TYPE_VIBRATE:
            type_idx= HOMERF_DEVICE_VIBRATE;
            break;

        case HOMERF_MARS_TYPE_SMOKE:
            type_idx= HOMERF_DEVICE_SMOKE;
            break;

        case HOMERF_MARS_TYPE_GAS:
            type_idx= HOMERF_DEVICE_GAS;
            break;
            
        case HOMERF_MARS_TYPE_MULTI_SENSOR:
            break;

        case HOMERF_MARS_TYPE_IAQ:
            type_idx= HOMERF_DEVICE_IAQ;
            break;

        case HOMERF_MARS_TYPE_ADE:
            type_idx= HOMERF_DEVICE_ADE;
            break;

        case HOMERF_MARS_TYPE_FDS:
            type_idx= HOMERF_DEVICE_FDS;
            break;

        case HOMERF_MARS_TYPE_SWITCH:
            type_idx= HOMERF_DEVICE_SWITCH;
            break;			
            
        default:
            type_idx= HOMERF_MARS_TYPE_UNKNOWN;
            break;
           
        
    }
#if 0
    DEBUG_RED("[HomeRF] MARS TYPE:%d, UI_TYPE:%d\n", type, type_idx);
#endif
    return type_idx;
}


u8 homeRFGetDeviceType(u16 type) //Use for Log record transform.
{
    u8 type_idx;
    
    switch(type)
    {
        case HOMERF_DEVICE_DOOR:
            type_idx= HOMERF_MARS_TYPE_DOOR;
            break;
            
        case HOMERF_DEVICE_PIR:
            type_idx= HOMERF_MARS_TYPE_PIR;
            break;
            
        case HOMERF_DEVICE_SIREN:
            type_idx= HOMERF_MARS_TYPE_SIREN;
            break;
            
        case HOMERF_DEVICE_PANIC:
        case HOMERF_DEVICE_IR:
            type_idx= HOMERF_MARS_TYPE_REMOTE;
            break;
            
        case HOMERF_DEVICE_PLUG:
            type_idx= HOMERF_MARS_TYPE_PLUG;
            break;
            
        case HOMERF_DEVICE_TEMP_HYG:
            type_idx= HOMERF_MARS_TYPE_TEMP_HUMIDITY;
            break;
            
        case HOMERF_DEVICE_TEMP:
            type_idx= HOMERF_MARS_TYPE_TEMP;
            break;
            
        case HOMERF_DEVICE_HUM:
            type_idx= HOMERF_MARS_TYPE_HUMIDITY;
            break;
            
        case HOMERF_DEVICE_ROUTER:
            type_idx= HOMERF_MARS_TYPE_ROUTER;
            break;
            
        case HOMERF_DEVICE_LEAK:
            type_idx= HOMERF_MARS_TYPE_LEAK;
            break;
        
        case HOMERF_DEVICE_VIBRATE:
            type_idx= HOMERF_MARS_TYPE_VIBRATE;
            break;

        case HOMERF_DEVICE_SMOKE:
            type_idx= HOMERF_MARS_TYPE_SMOKE;
            break;

        case HOMERF_DEVICE_GAS:
            type_idx= HOMERF_MARS_TYPE_GAS;
            break;
            
        case HOMERF_DEVICE_IAQ:
            type_idx= HOMERF_MARS_TYPE_IAQ;
            break;

        case HOMERF_DEVICE_ADE:
            type_idx= HOMERF_MARS_TYPE_ADE;
            break;

        case HOMERF_DEVICE_FDS:
            type_idx= HOMERF_MARS_TYPE_FDS;
            break;

        case HOMERF_DEVICE_SWITCH:
            type_idx= HOMERF_MARS_TYPE_SWITCH;
            break;			
            
        default:
            type_idx= HOMERF_MARS_TYPE_UNKNOWN;
            break;
           
        
    }

    return type_idx;
}


/* 
   Return value:
   0: Set Siren Fail, (Timeout, no siren sensor) 
   1: Set Siren Success, 
*/
u8 homeRFSetSirenStatus(u8 act)
{
    u8 result, idx;
    
    #if(HOME_RF_OPTION == HOME_SENSOR_BARVOTECH)
        OSTimeDly(15);
    #endif

    for(idx =0; idx < HOMERF_SENSOR_MAX; idx++)
    {

        if(gHomeRFSensorList->sSensor[idx].type != HOMERF_DEVICE_SIREN)
            continue;
                    
        if(act == HOMERF_SIREN_ON)
        {
        	if(gHomeRFSensorList->sSensor[idx].sirenOnOff == HOMERF_SENSOR_ON)
        	{
	            homeRFSendToSensor(HOMERF_SEND_SIREN_ON,idx);
	            //homeRFSendToSensor(HOMERF_SEND_SIREN_ON,idx);
	            //homeRFSendToSensor(HOMERF_SEND_SIREN_ON,idx);
	            gHomeRFSensorList->sSensor[idx].data.siren.isRinging=TRUE;
            }
        }
        else if(act == HOMERF_SIREN_BEEP) //PANIC
        {
            //homeRFSendToSensor(HOMERF_SEND_SIREN_BEEP,idx);
			homeRFSendToSensor(HOMERF_SEND_SIREN_ON,idx);
			gHomeRFSensorList->sSensor[idx].data.siren.isRinging=TRUE;
        }
        else
        {
            homeRFSendToSensor(HOMERF_SEND_SIREN_OFF,idx);    
            gHomeRFSensorList->sSensor[idx].data.siren.isRinging=FALSE;
        }
        
    }
}
u8 homeRFSaveSensor(u8 * cmdStr)
{
    u16 sID=0;
    u64 sUID;
    u8  type=0, subType=0;
    u8  idx=0, i;
    u8  err,pairStatus;
    u32 waitFlag;
    u8  majorVer=0, minorVer=0;

	printf("\n[%s] CMD 0x%X - ", __func__, cmdStr[HOMERF_CMD_BIT_CMD]);
    for(i=0;i<cmdStr[HOMERF_CMD_BIT_CMD-1];i++)
    {
		printf("0x%X ", cmdStr[HOMERF_CMD_BIT_CMD+i]);
    }
	printf("\n");

    sID=((cmdStr[HOMERF_CMD_BIT_CMD+1] << 8) & 0xFF00) | (cmdStr[HOMERF_CMD_BIT_CMD+2] & 0xFF);
    memcpy(&sUID,cmdStr+HOMERF_CMD_BIT_CMD+3, sizeof(u64));
    type=cmdStr[HOMERF_CMD_BIT_CMD+11 ];
    subType=cmdStr[HOMERF_CMD_BIT_CMD+12 ];
    majorVer=cmdStr[HOMERF_CMD_BIT_CMD+13];
    minorVer=cmdStr[HOMERF_CMD_BIT_CMD+14];
    pairStatus=homeRFCheckPID(sID, sUID, &idx);
	//printf("[%s] Pair status [0x%X]0x%X:%d:T-0x%X:0x%X\n", __func__,idx, sID, pairStatus, type,subType);
    if( (pairStatus == HOMERF_PAIR_TIMEOUT) || (pairStatus == HOMERF_PAIR_FAIL)) /* pair Fail */
    {
        gHomeRFPairStat=HOMERF_PAIR_NORMAL;
        gHomeRFOpMode=HOMERF_OP_NOMAL;
        gAppPairFlag=APP_PAIR_FAIL;
        gHomeRFSensorID=0;
		printf("[HomeRF] HOMERF_PAIR_FAIL/TIMEOUT\n");
        OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_PAIR_FAIL, OS_FLAG_SET, &err);                    
    }
    else
    {
        if(pairStatus ==HOMERF_PAIR_SUCCESS)
      	 {
            gHomeRFSensorCnt++;
	    	DefaultSensorCnt++;
			if(DefaultSensorCnt > 999)
				DefaultSensorCnt = 1;
			if(gHomeRFSensorCnt == 1) /* When sensor number became 0, index start from 1 */
				DefaultSensorCnt = 1;
            //spiWriteHomeRF(SPI_HOMERF_CONFIG);
			printf("[HomeRF] HOMERF_PAIR_SUCCESS\n");
      	 }
        gHomeRFSensorList->sSensor[idx].sirenOnOff = HOMERF_SENSOR_ON;
        gHomeRFSensorList->sSensor[idx].pushOnOff = HOMERF_SENSOR_ON;
		gHomeRFSensorList->sSensor[idx].battery = 100;

        gHomeRFSensorList->sSensor[idx].type = homeRFGetDevice(type, subType);

        //homeRFSetDefaultScene();
        if(pairStatus != HOMERF_PAIR_SAME){
        	homeRFSetDefaultSensorName(gHomeRFSensorList->sSensor[idx].type, gHomeRFSensorList->sSensor[idx].name);
        }
        else
            DEBUG_RED("[HOMERF] Same Sensor paired\n");
		//Paul Test for sensor add, should removed
//		spiWriteHomeRF(SPI_HOMERF_SENSOR);
		//End paul test
        if(gHomeRFOpMode == HOMERF_OP_APP_PAIR)
        {
        printf("[HomeRF] HOMERF_OP_APP_PAIR\n");
		printf("sID_1=%d\n",gHomeRFSensorList->sSensor[0].sID);
        spiWriteHomeRF(SPI_HOMERF_SENSOR);  /* It wiil image open fail, when flash writing data simultaneously*/ 
		spiReadHomeRF(SPI_HOMERF_SENSOR);
		printf("---sID_2=%d\n",gHomeRFSensorList->sSensor[0].sID);
            gAppPairFlag=APP_PAIR_SUCCESS;
        }
        else
            OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_PAIR_SUC, OS_FLAG_SET, &err);  
        gHomeRFOpMode=HOMERF_OP_NOMAL;
        gHomeRFPairStat=HOMERF_PAIR_NORMAL;  
        gHomeRFPairCnt++;
#if CLOUD_SUPPORT
		SendCloud_ADD_DEL_Message(sID, HOMERF_ADD_SENSOR, 0);
#endif 		

    }
    
}

void homeRFSentEvent2UI(u8 idx)
{
    u32 waitFlag=0;
    u8  err;
    
    waitFlag=OSFlagAccept(gHomeRFEventFlagGrp, 1<<(idx), OS_FLAG_WAIT_SET_ANY , &err); 
    if(waitFlag!=0)
    {
        printf("homeRF busy %x\n",waitFlag);
        return;
    } 

    OSFlagPost(gHomeRFEventFlagGrp, idx+1, OS_FLAG_SET, &err);
    
    homeRFSentKeyToUI(UI_KEY_HOMERF_EVENT);
}

void homeRFHandleEvent(u8 *cmdStr)
{
    u16 sID=0;
    u16 updateTick=0;
    u16 adc_val=0;
    u8  type=0, subType=0, keyTrigger=0, keyStatus=HOMERF_KEY_NONE, modeleStatus=0;
    u16 status=0;
    u16 idx=0, i,siren_idx;
    u8 err, tLen;
    u32 waitFlag;
    static u8 count=0;
    u8 battery_value=0; /* <=20:Low Battery ; 0:Lost Link*/
    unsigned int current_time;

    tLen = cmdStr[HOMERF_CMD_BIT_LEN];
    #if 0
    printf("[%s]",__func__);
	for(i=0;i<tLen+4;i++)
		printf("%x ",cmdStr[i]);
	printf("\n");
    #endif
    sID=(cmdStr[HOMERF_CMD_BIT_CMD+1] << 8) | cmdStr[HOMERF_CMD_BIT_CMD+2];
	
    updateTick=(cmdStr[HOMERF_CMD_BIT_CMD+3] << 8) | cmdStr[HOMERF_CMD_BIT_CMD+4];
    adc_val=(cmdStr[HOMERF_CMD_BIT_CMD+5] << 8) | cmdStr[HOMERF_CMD_BIT_CMD+6];
    //printf("\x1B[96 !sID=%x\x1B[0m\n",sID);
    type=cmdStr[HOMERF_CMD_BIT_CMD+7 ];   
    if(type == HOMERF_MARS_TYPE_TEMP_HUMIDITY)
    {
        status = (cmdStr[HOMERF_CMD_BIT_CMD+8] << 8) | cmdStr[HOMERF_CMD_BIT_CMD+9]; 
    }
    else if(type == HOMERF_MARS_TYPE_REMOTE)
    {
        subType= cmdStr[HOMERF_CMD_BIT_CMD+8];
        keyTrigger= cmdStr[HOMERF_CMD_BIT_CMD+9];   
    }
    else if(type == HOMERF_MARS_TYPE_IAQ)
    {
        status = (cmdStr[HOMERF_CMD_BIT_CMD+8] << 8) | cmdStr[HOMERF_CMD_BIT_CMD+9]; 
		modeleStatus = cmdStr[HOMERF_CMD_BIT_CMD+10];
    }
    else
    {
        status = cmdStr[HOMERF_CMD_BIT_CMD+8]; 
    }
    

    for(i=0; i < HOMERF_SENSOR_MAX; i++)
    {
        idx=i;
		//printf("[%s]Sensor[%d].sID = %d,targetsID=%d\n",__func__, idx,gHomeRFSensorList->sSensor[i].sID, sID);
        if(gHomeRFSensorList->sSensor[i].sID == sID)
        {
            break;
        }
    }
    if(idx == HOMERF_SENSOR_MAX-1)
    {
    	//printf("[%s]Unsearch sensor, idx = %d\n", idx);
        gHomeRFPairStat=HOMERF_PAIR_NORMAL;
        gHomeRFOpMode=HOMERF_OP_NOMAL;
        //gAppPairFlag=APP_PAIR_FAIL;
        gHomeRFSensorID=0;
        homeRFsendAckToRF(cmdStr[HOMERF_CMD_BIT_CMD],0); 
        return;
    }

    /* If battery Level is 30~20, needs to count 3 times for sure Low Battery or not */   
    if (HOME_RF_OPTION == HOME_SENSOR_TRANWO)
    {
	    if (type == HOMERF_MARS_TYPE_PIR)
	        battery_value = homeRFGetBattery_TRANWO_PIR(adc_val);
	    else if (type == HOMERF_MARS_TYPE_TEMP_HUMIDITY)
	        battery_value = homeRFGetBattery_TRANWO_PIR(adc_val);
	    else if(type == HOMERF_MARS_TYPE_DOOR)
			battery_value = homeRFGetBattery_TRANWO_DOOR(adc_val);
		else
        battery_value = 100;//homeRFGetBattery(adc_val);
	}
	else
        battery_value = 100;//homeRFGetBattery(adc_val);
	
    if((battery_value == 10) ||(battery_value == 20))
    {
        gHomeRFSensorList->sSensor[idx].lowbatterycnt ++;

        if(gHomeRFSensorList->sSensor[idx].lowbatterycnt > 2)
        {
            gHomeRFSensorList->sSensor[idx].battery = 20;
			printf("[HOEMRF]Low Battery Log[%d, %d]\n",idx,  type);			
            homeRFWriteLog(idx, type, 0, HOMERF_EVENT_NONE, EventID_LowBattery, battery_value);  //Low Battery Log
        }
        else
            gHomeRFSensorList->sSensor[idx].battery = 30;
    }
    else if(battery_value >= 40)
    {
        gHomeRFSensorList->sSensor[idx].battery = battery_value;
        gHomeRFSensorList->sSensor[idx].lowbatterycnt = 0;
    }
    else
        gHomeRFSensorList->sSensor[idx].battery = battery_value;
        
    gHomeRFSensorList->sSensor[idx].lifeCount =0;
    if(type == HOMERF_MARS_TYPE_DOOR)
    {
        if(status & 0x80 ) /* trigger bit*/
        {
            if(status & 0x01) /* high */
            {
                printf("\n[HOEMRF]Sensor%d Door Open[%d]\n",idx, type);
                gHomeRFSensorList->sSensor[idx].data.door.isOpen=TRUE;
                gHomeRFSensorList->sSensor[idx].status |= HOMERF_SENSOR_STATUS_ALARM;
                gHomeRFSensorList->sSensor[idx].alarmTimer=HOMERF_ALARM_PERIOD;								
                if(gHomeRFSensorList->sSensor[idx].sirenOnOff == HOMERF_SENSOR_ON)
                    homeRFSetSirenStatus(TRUE);  
                //if(gHomeRFSensorList->sSensor[idx].pushOnOff == HOMERF_SENSOR_ON)
				{
					sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, gHomeRFSensorList->sSensor[idx].sID,EVENT_DOOR | HOMERF_PUSHMSG_PREFIX); 
#if CLOUD_SUPPORT			
					//sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_EVENT);
#endif	
				}
#if(HOME_RF_OPTION == HOME_SENSOR_TRANWO)
                if(gHomeRFSensorList->sSensor[idx].sirenOnOff == HOMERF_SENSOR_ON) 
#endif   
                {
                    gHomeRFCheckAlert[idx>>3] |= (0x01 << (idx%8));
                    homeRFSentEvent2UI(idx);
                }
                homeRFWriteLog(idx, type, 0, HOMERF_EVENT_TRIGGER, EventID_DoorWindowOpen, battery_value);
#if CLOUD_SUPPORT		
				sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_DATA);
#endif	

            }
            else 
            {
                printf("\n[HOEMRF]Sensor%d Door Close[%d]\n",idx, type);
                gHomeRFSensorList->sSensor[idx].data.door.isOpen=FALSE;
                
#if(HOME_RF_OPTION != HOME_SENSOR_TRANWO)
                homeRFWriteLog(idx, type, subType, HOMERF_EVENT_TRIGGER, EventID_DoorWindowClose, battery_value);
#endif                
#if CLOUD_SUPPORT			
				sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_DATA | HOMERF_PUSHMSG_PREFIX);
				//sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_EVENT);
#endif

                //gHomeRFSensorList->sSensor[idx].status &= ~HOMERF_SENSOR_STATUS_ALARM;

            }

            if(status & 0x02) /* Tamper Switch Open*/
            {
                printf("[HOEMRF]Sensor%d Door Tamper Switch Open\n",idx);
            }          
        }
    }
    else if(type == HOMERF_MARS_TYPE_PIR)
    {
        if(status & 0x80 ) /* trigger bit*/
        {
            printf("\n[HOEMRF]Sensor%d PIR Trigger\n",idx);
            gHomeRFSensorList->sSensor[idx].status |= HOMERF_SENSOR_STATUS_ALARM;
            gHomeRFSensorList->sSensor[idx].alarmTimer=HOMERF_ALARM_PERIOD;           
            if(gHomeRFSensorList->sSensor[idx].sirenOnOff == HOMERF_SENSOR_ON)
                homeRFSetSirenStatus(TRUE);
            if(gHomeRFSensorList->sSensor[idx].pushOnOff == HOMERF_SENSOR_ON)
			{
				sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, gHomeRFSensorList->sSensor[idx].sID,EVENT_PIR | HOMERF_PUSHMSG_PREFIX);
#if CLOUD_SUPPORT	
				//sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_EVENT);
#endif					
			}
#if(HOME_RF_OPTION == HOME_SENSOR_TRANWO)                
            if(gHomeRFSensorList->sSensor[idx].sirenOnOff == HOMERF_SENSOR_ON)
#endif  
            {
                gHomeRFCheckAlert[idx>>3] |= (0x01 << (idx%8));
                homeRFSentEvent2UI(idx);
            }
                
            homeRFWriteLog(idx, type, 0, HOMERF_EVENT_TRIGGER, EventID_PIRDetected, battery_value);
#if CLOUD_SUPPORT			
			sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_DATA);
#endif	

            if(status & 0x02) /* Tamper Switch Open*/
            {
                printf("[HOEMRF]Sensor%d PIR Tamper Switch Open\n",idx);
            }
        }
    }

/*********** 20160929 Sean: Not Use Yet, save code size.
LEAK
VIBRATE
GAS
SMOKE
***********/
#if 0 	
    else if(type == HOMERF_MARS_TYPE_LEAK)
    {
        if(status & 0x80 ) /* trigger bit*/
        {
            printf("\n[HOEMRF]Sensor%d LEAK Trigger\n",idx);
            gHomeRFSensorList->sSensor[idx].status |= HOMERF_SENSOR_STATUS_ALARM;
            gHomeRFSensorList->sSensor[idx].alarmTimer=HOMERF_ALARM_PERIOD;
            if(gHomeRFSensorList->sSensor[idx].sirenOnOff == HOMERF_SENSOR_ON)
                homeRFSetSirenStatus(TRUE);
            if(gHomeRFSensorList->sSensor[idx].pushOnOff == HOMERF_SENSOR_ON)
                sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, gHomeRFSensorList->sSensor[idx].sID,EVENT_MOTIONDECT);     //EVENT TBD
#if(HOME_RF_OPTION == HOME_SENSOR_TRANWO)                
            if(gHomeRFSensorList->sSensor[idx].sirenOnOff == HOMERF_SENSOR_ON)
#endif  
            {
                gHomeRFCheckAlert[idx>>3] |= (0x01 << (idx%8));
                homeRFSentEvent2UI(idx);
            }
            homeRFWriteLog(idx, type, 0, HOMERF_EVENT_TRIGGER, EventID_PIRDetected, battery_value);

            if(status & 0x02) /* Tamper Switch Open*/
            {
                printf("[HOEMRF]Sensor%d LEAK Tamper Switch Open\n",idx);
            }
        }
    }
    else if(type == HOMERF_MARS_TYPE_VIBRATE)
    {
        if(status & 0x80 ) /* trigger bit*/
        {
            printf("\n[HOEMRF]Sensor%d VIBRATE Trigger\n",idx);
            gHomeRFSensorList->sSensor[idx].status |= HOMERF_SENSOR_STATUS_ALARM;
            gHomeRFSensorList->sSensor[idx].alarmTimer=HOMERF_ALARM_PERIOD;
            if(gHomeRFSensorList->sSensor[idx].sirenOnOff == HOMERF_SENSOR_ON)
                homeRFSetSirenStatus(TRUE);
            if(gHomeRFSensorList->sSensor[idx].pushOnOff == HOMERF_SENSOR_ON)
                sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, gHomeRFSensorList->sSensor[idx].sID,EVENT_MOTIONDECT);     //EVENT TBD
#if(HOME_RF_OPTION == HOME_SENSOR_TRANWO)                
            if(gHomeRFSensorList->sSensor[idx].sirenOnOff == HOMERF_SENSOR_ON)
#endif  
            {
                gHomeRFCheckAlert[idx>>3] |= (0x01 << (idx%8));
                homeRFSentEvent2UI(idx);
            }
            homeRFWriteLog(idx, type, 0, HOMERF_EVENT_TRIGGER, EventID_PIRDetected, battery_value);

            if(status & 0x02) /* Tamper Switch Open*/
            {
                printf("[HOEMRF]Sensor%d VIBRATE Tamper Switch Open\n",idx);
            }
        }
    }    
    else if(type == HOMERF_MARS_TYPE_GAS)/*Siren, Push Message always ON*/
    {
        if(status & 0x80 ) /* trigger bit*/
        {
            printf("\n[HOEMRF]Sensor%d GAS Trigger\n",idx);
            gHomeRFSensorList->sSensor[idx].status |= HOMERF_SENSOR_STATUS_ALARM;
            gHomeRFSensorList->sSensor[idx].alarmTimer=HOMERF_ALARM_PERIOD;
            homeRFSetSirenStatus(TRUE);
            sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, gHomeRFSensorList->sSensor[idx].sID,EVENT_MOTIONDECT);     //EVENT TBD
#if(HOME_RF_OPTION == HOME_SENSOR_TRANWO)                
            if(gHomeRFSensorList->sSensor[idx].sirenOnOff == HOMERF_SENSOR_ON)
#endif  
            {
                gHomeRFCheckAlert[idx>>3] |= (0x01 << (idx%8));
                homeRFSentEvent2UI(idx);
            }            
            homeRFWriteLog(idx, type, 0, HOMERF_EVENT_TRIGGER, EventID_PIRDetected, battery_value);

            if(status & 0x02) /* Tamper Switch Open*/
            {
                printf("[HOEMRF]Sensor%d PIR Tamper Switch Open\n",idx);
            }
        }
    }
    else if(type == HOMERF_MARS_TYPE_SMOKE)/*Siren, Push Message always ON*/
    {
        if(status & 0x80 ) /* trigger bit*/
        {
            printf("\n[HOEMRF]Sensor%d SMOKE Trigger\n",idx);
            gHomeRFSensorList->sSensor[idx].status |= HOMERF_SENSOR_STATUS_ALARM;
            gHomeRFSensorList->sSensor[idx].alarmTimer=HOMERF_ALARM_PERIOD;
            gHomeRFCheckAlert[idx>>3] |= (0x01 << (idx%8));
            homeRFSetSirenStatus(TRUE);
            sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, gHomeRFSensorList->sSensor[idx].sID,EVENT_MOTIONDECT);     //EVENT TBD
            homeRFSentEvent2UI(idx);
            homeRFWriteLog(idx, type, 0, HOMERF_EVENT_TRIGGER, EventID_PIRDetected, battery_value);
#if CLOUD_SUPPORT				
			sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_DATA);
			//sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_EVENT);
#endif	

            if(status & 0x02) /* Tamper Switch Open*/
            {
                printf("[HOEMRF]Sensor%d SMOKE Tamper Switch Open\n",idx);
            }
        }
        else
        {
#if CLOUD_SUPPORT				
			sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_DATA);
#endif	
        }
        
    }
/**************************************************************************/
/*******************************Panasonic**********************************/
/*******************************************************20161103 Sean Add**/
    else if(type == HOMERF_MARS_TYPE_IAQ)	//Indoor Air Quility
    {
		if(modeleStatus == 0x00)	// OK(data valid)
		{
			gHomeRFSensorList->sSensor[idx].data.IAQ.value = status;
#if CLOUD_SUPPORT				
			sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_DATA);
			//SendCloudMessage(gHomeRFSensorList->sSensor[idx].sID, "data");
#endif		
			printf("\n[HOEMRF]Sensor%d FDS %d\n", idx, gHomeRFSensorList->sSensor[idx].data.IAQ.value);
		}
		else if(modeleStatus == 0x10)
		{
			printf("\n[HOEMRF]Sensor%d FDS RUNIN\n", idx);
		}
		else if(modeleStatus == 0x01)
		{
			printf("\n[HOEMRF]Sensor%d FDS BUSY\n", idx);
		}
		else if(modeleStatus == 0x80)
		{
			printf("\n[HOEMRF]Sensor%d FDS ERROR\n", idx);
		}
    }	
    else if(type == HOMERF_MARS_TYPE_ADE)	//Analog Device
    {

    }	
    else if(type == HOMERF_MARS_TYPE_FDS)	//Fall Detection Sensor 
    {
        //if(status & 0x08 ) /* trigger bit*/
        {
            //printf("\n[HOEMRF]Sensor%d FDS Trigger\n",idx);
			if(status >> 4 == 1)
			{
				gHomeRFSensorList->sSensor[idx].data.FDS.RSSI = cmdStr[HOMERF_CMD_BIT_CMD+9]; 
			}
			else if(status >> 4 == 2)
			{
				gHomeRFSensorList->sSensor[idx].data.FDS.Accel_X_H = cmdStr[HOMERF_CMD_BIT_CMD+9]; 
				gHomeRFSensorList->sSensor[idx].data.FDS.Accel_X_L = cmdStr[HOMERF_CMD_BIT_CMD+10]; 
				gHomeRFSensorList->sSensor[idx].data.FDS.Accel_Y_H = cmdStr[HOMERF_CMD_BIT_CMD+11]; 
				gHomeRFSensorList->sSensor[idx].data.FDS.Accel_Y_L = cmdStr[HOMERF_CMD_BIT_CMD+12]; 
				gHomeRFSensorList->sSensor[idx].data.FDS.Accel_Z_H = cmdStr[HOMERF_CMD_BIT_CMD+13]; 
				gHomeRFSensorList->sSensor[idx].data.FDS.Accel_Z_L = cmdStr[HOMERF_CMD_BIT_CMD+14]; 
			}
			else if(status >> 4 == 3)
			{
				gHomeRFSensorList->sSensor[idx].data.FDS.GYRO_X_H = cmdStr[HOMERF_CMD_BIT_CMD+9]; 
				gHomeRFSensorList->sSensor[idx].data.FDS.GYRO_X_L = cmdStr[HOMERF_CMD_BIT_CMD+10]; 
				gHomeRFSensorList->sSensor[idx].data.FDS.GYRO_Y_H = cmdStr[HOMERF_CMD_BIT_CMD+11]; 
				gHomeRFSensorList->sSensor[idx].data.FDS.GYRO_Y_L = cmdStr[HOMERF_CMD_BIT_CMD+12]; 
				gHomeRFSensorList->sSensor[idx].data.FDS.GYRO_Z_H = cmdStr[HOMERF_CMD_BIT_CMD+13]; 
				gHomeRFSensorList->sSensor[idx].data.FDS.GYRO_Z_L = cmdStr[HOMERF_CMD_BIT_CMD+14]; 
#if CLOUD_SUPPORT	
				sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_DATA);
				//sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_EVENT);
#endif	
			}			
			
            //gHomeRFSensorList->sSensor[idx].status |= HOMERF_SENSOR_STATUS_ALARM;
            //gHomeRFSensorList->sSensor[idx].alarmTimer=HOMERF_ALARM_PERIOD;
            //gHomeRFCheckAlert[idx>>3] |= (0x01 << (idx%8));
            //homeRFSetSirenStatus(TRUE);
            //sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, gHomeRFSensorList->sSensor[idx].sID,EVENT_MOTIONDECT);     //EVENT TBD
            //homeRFSentEvent2UI(idx);
            //homeRFWriteLog(idx, type, 0, HOMERF_EVENT_TRIGGER, EventID_PIRDetected, battery_value);

            //if(status & 0x02) /* Tamper Switch Open*/
            //{
            //    printf("[HOEMRF]Sensor%d FDS Tamper Switch Open\n",idx);
            //}
        }
    }		
    else if(type == HOMERF_MARS_TYPE_SWITCH) 
    {
    #if 0 //old setting
    	if(status & 0x04)
    	{
            printf("\n[HOEMRF]Sensor%d SWITCH ON\n",idx);
			gHomeRFSensorList->sSensor[idx].data.SWITCH.isPowerOn = TRUE;
#if CLOUD_SUPPORT	
			sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_DATA);
#endif	
    	}
		else
		{
            printf("\n[HOEMRF]Sensor%d SWITCH OFF\n",idx);		
			gHomeRFSensorList->sSensor[idx].data.SWITCH.isPowerOn = FALSE;
#if CLOUD_SUPPORT		
			sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_DATA);
#endif				
		}
	#else  //20170112 Sean : change switch control bit setting.
		homeSwitchControl = (status & 0x0f);
		gHomeRFSensorList->sSensor[idx].data.SWITCH.isPowerOn = homeSwitchControl;
		printf("\n[HOEMRF]Sensor%d SWITCH Setting: %d\n", idx, homeSwitchControl);	
#if CLOUD_SUPPORT	
		sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_DATA);
#endif			
	#endif
	
		spiWriteHomeRF(SPI_HOMERF_SENSOR);
    }	
#endif    
/*******************************Panasonic****************************End***/

    else if(type == HOMERF_MARS_TYPE_REMOTE)
    {
        if(subType == HOMERF_SUB_TYPE_TRANWO_REMOTE)
        {
            if(keyTrigger & 0x01) 
            {
                keyStatus=HOMERF_KEY_DISALARM;
				UI_HA_Sensor_Beep_Flag = 0;
				homeRFSceneAppMode = 2;
				Beep_function(1,200,60,60,200,0);
                printf("\n[HOEMRF]Remote DISALARM\n");
            }
            else if(keyTrigger & 0x02)
            {
                keyStatus=HOMERF_KEY_ALARM;  
                homeRFSceneAppMode = 1;
				Beep_function(2,200,60,60,200,0);
                printf("\n[HOEMRF]Remote ALARM\n");
            }
            else if(keyTrigger & 0x04)
            {
                keyStatus=HOMERF_KEY_HOME; 
                homeRFSceneAppMode = 3;
				Beep_function(3,200,60,60,200,0);
                printf("\n[HOEMRF]Remote HOME\n");
            }
            else if(keyTrigger & 0x08)
            {
                keyStatus=HOMERF_KEY_PANIC;
                printf("\n[HOEMRF]Remote PANIC %d\n", type);
                homeRFSceneAppMode = 1;
                gHomeRFSensorList->sSensor[idx].status |= HOMERF_SENSOR_STATUS_ALARM;             
                gHomeRFSensorList->sSensor[idx].alarmTimer=1;								
				gHomeRFCheckAlert[idx>>3] |= (0x01 << (idx%8));
				homeRFWriteLog(idx, type, 0, HOMERF_EVENT_TRIGGER, EventID_MotionDetected, battery_value);
				homeRFSentEvent2UI(idx);
                sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, gHomeRFSensorList->sSensor[idx].sID,EVENT_SOS | HOMERF_PUSHMSG_PREFIX); //2017525 Sean
            }
			UpdateAPPSceneListStatus();
        }
        #if 0
        else if(subType == HOMERF_SUB_TYPE_PANIC)
        {
            keyStatus=HOMERF_KEY_PANIC; 
            sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, gHomeRFSensorList->sSensor[idx].sID,EVENT_MOTIONDECT);   //EVENT TBD
        }
        else if(subType == HOMERF_SUB_TYPE_MEIAN_REMOTE)
        {
            if(keyTrigger & 0x04) 
            {
                keyStatus=HOMERF_KEY_DISALARM;
            }
            else if(keyTrigger & 0x02)
            {
                keyStatus=HOMERF_KEY_ALARM;    
            }
            else if(keyTrigger & 0x01)
            {
                keyStatus=HOMERF_KEY_PANIC;    
                sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, gHomeRFSensorList->sSensor[idx].sID,EVENT_MOTIONDECT | HOMERF_PUSHMSG_PREFIX); //EVENT TBD
            }
            else if(keyTrigger & 0x08)
            {
                keyStatus=HOMERF_KEY_HOME;    
            }
        }				
        #endif
        homeRFsendAckToRF(cmdStr[HOMERF_CMD_BIT_CMD], 0);
        homeRFKeyParse(keyStatus);
    }
    else if((type == HOMERF_MARS_TYPE_TEMP) || (type == HOMERF_MARS_TYPE_TEMP_HUMIDITY))	//sync per 30 sec.
    {
        u8 tempH, tempL, hygH, hygL;
        u8 tempLmtH, tempLmtL, hygLmtH, hygLmtL;
		gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Thermometer_H = cmdStr[HOMERF_CMD_BIT_CMD+ 8];
		gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Thermometer_L = cmdStr[HOMERF_CMD_BIT_CMD+ 9];
		gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Hygrometer_H  = cmdStr[HOMERF_CMD_BIT_CMD+10];
		gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Hygrometer_L  = cmdStr[HOMERF_CMD_BIT_CMD+11];
        tempH = gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Thermometer_H;
        tempL = gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Thermometer_L;
        hygH = gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Hygrometer_H;
        hygL = gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Hygrometer_L;
        tempLmtH = gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Temp_alarm_H;
        tempLmtL = gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Temp_alarm_L;
        hygLmtH = gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Humi_alarm_H;
        hygLmtL = gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Humi_alarm_L;
#if 0
        DEBUG_RED("Curr: %d-%d; AlermH: %d-%d; AlermL: %d-%d\n",tempH,hygH,
                                                    tempLmtH,hygLmtH,
                                                    tempLmtL,hygLmtL);
#endif
        if(((tempH > tempLmtH ) || (tempH < tempLmtL) || (hygH > hygLmtH ) || (hygH < hygLmtL ))
            && (gHomeRFSensorList->sSensor[idx].alarmTimer <= 0))

		{
            gHomeRFSensorList->sSensor[idx].status |= HOMERF_SENSOR_STATUS_ALARM;
    		gHomeRFSensorList->sSensor[idx].alarmTimer=HOMERF_TEMP_HUM_ALARM_PERIOD; 
            printf("\n[HOEMRF]Sensor%d TEMP Trigger\n",idx);
    		if(gHomeRFSensorList->sSensor[idx].sirenOnOff == HOMERF_SENSOR_ON)
    			homeRFSetSirenStatus(TRUE);
    		if(gHomeRFSensorList->sSensor[idx].pushOnOff == HOMERF_SENSOR_ON)
    		{
    			sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, gHomeRFSensorList->sSensor[idx].sID,0 | HOMERF_PUSHMSG_PREFIX);
    		}
#if(HOME_RF_OPTION == HOME_SENSOR_TRANWO)                
    		if(gHomeRFSensorList->sSensor[idx].sirenOnOff == HOMERF_SENSOR_ON)
#endif  
    		{
    			gHomeRFCheckAlert[idx>>3] |= (0x01 << (idx%8));
    			homeRFSentEvent2UI(idx);
    		}
    		if(gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Thermometer_H < gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Temp_alarm_L)
    			homeRFWriteLog(idx, type, 0, HOMERF_EVENT_TRIGGER, EventID_TemperatureLow, battery_value);
    		else
    			homeRFWriteLog(idx, type, 0, HOMERF_EVENT_TRIGGER, EventID_TemperatureHigh, battery_value);
		}
#if CLOUD_SUPPORT		
		sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_CLOUD,gHomeRFSensorList->sSensor[idx].sID, TARGET_DATA);
#endif		
    }
    else if(type == HOMERF_MARS_TYPE_SIREN)
    {
		//if((status & 0x80) && (status & 0x02)) /* Tamper Switch Open*/
		if(status & 0x02) /* Tamper Switch Open*/
		{
			current_time=OSTimeGet();
			if(((current_time - Siren_trigger_time) > 1200) || (Siren_trigger_time==0) || (status & 0x80)) // 1min
			{
				Siren_trigger_time = current_time;
				printf("[HOEMRF]Sensor%d Siren Tamper Switch Open\n",idx);
				if(gHomeRFSensorList->sSensor[idx].sirenOnOff == HOMERF_SENSOR_ON) 
				{
					gHomeRFSensorList->sSensor[idx].status |= HOMERF_SENSOR_STATUS_ALARM;	
					gHomeRFSensorList->sSensor[idx].alarmTimer=1;	
					gHomeRFCheckAlert[idx>>3] |= (0x01 << (idx%8));
					homeRFSentEvent2UI(idx);
				}
				homeRFSetSirenStatus(HOMERF_SIREN_BEEP);
				homeRFWriteLog(idx, type, 0, HOMERF_EVENT_TRIGGER, EventID_SirenSwitchOn, battery_value);			
				sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, gHomeRFSensorList->sSensor[idx].sID,EVENT_SIREN | HOMERF_PUSHMSG_PREFIX);   
			}
		}  
		else if((status & 0x80) && (status & 0x01))/* Tamper Switch Open*/
		{
			homeRFSetSirenStatus(HOMERF_SIREN_OFF);
		}
		else
		{
			goto End;
		}

        homeRFClearCmdQueue(sID);    
    }
    else if(type == HOMERF_MARS_TYPE_PLUG)
    {
        homeRFClearCmdQueue(sID);    
    }

//#if(HOME_RF_OPTION ==HOME_SENSOR_BARVOTECH)
    //homeRFsendAckToRF(cmdStr[HOMERF_CMD_BIT_CMD], 0);
//#endif    
    UpdateAPPSensorListStatus();
    UpdateAPPSensorStatus(sID);
    return ;
End:
}



u8 homeRFGetUartCmd(u8 *pcString)
{
    u8 idx=0, len=0, checksum=0;
    u8 i;
    while(1)
    {
        idx=0;
        len=0;
        checksum=0;
        /*preamble*/
        pcString[idx++]=receive_char(HOMERF_UART_ID);
		DEBUG_UART(".");
        if(pcString[0] != HOMERF_PREAMBLE_BYTE1)
            continue;
        pcString[idx++]=receive_char(HOMERF_UART_ID);   
		DEBUG_UART(".");
        if(pcString[1] !=HOMERF_PREAMBLE_BYTE2)
            continue;
        
        /*len*/
        len=receive_char(HOMERF_UART_ID);
		DEBUG_UART(".");
		if(len>33) //20160823 Sean: Avoid invalid lenth.
			continue;
        checksum+=len;
        pcString[idx++]=len;
        DEBUG_UART("|");
        /*data*/
        for(i=0; i<len; i++)
        {
            pcString[idx]=receive_char(HOMERF_UART_ID);
            checksum+=pcString[idx];
            idx++;
        }
        DEBUG_UART("@");
        /*checksum*/
        pcString[idx]=receive_char(HOMERF_UART_ID);
        if(checksum != pcString[idx])
        {
        	printf("Cmd Not Allow!\n");
			for(i=0; i<len+4; i++)
			{
				printf("%02x ",pcString[i]);
			}
			printf("\n");        
            continue;
        }
        else 
            break;
    }
	DEBUG_UART("\%");
	if((pcString[3] == 0x11) || (pcString[3] == 0x12))
	{
		homeRFsendAckToRF(pcString[3], 0);
	}


	printf("\n");
    
    #if 1
    for(i=0; i<len+4; i++)
    {
        printf("%02x ",pcString[i]);
    }
    printf("\n");

    #endif

}
void homeRFSendCommand(u8 cmd,u8 sensorIdx)
{
    u8 cmdStr[32];
    u8 len=0, cmdID=0, value=0, checksum=0;
    u8 idx=0, i, addr1, addr2, control, intface, port=0,range;
    u8 RFID[4];
    u32 timerCnt1=0, timerCnt2=0;
    u64 pairID;
    u32 homeRFFieldID;
    const u8 PROTECT=0x5E;


    /* preamble*/
    cmdStr[idx++] = HOMERF_PREAMBLE_BYTE1;
    cmdStr[idx++] = HOMERF_PREAMBLE_BYTE2;

    switch(cmd)
    {           
        case HOMERF_SEND_RF_VERSON:
            len=0x01;
            cmdID=HOMERF_TX_CMDID_VERSION;
            checksum=len+cmdID;

            cmdStr[idx++]=len;
            cmdStr[idx++]=cmdID;
            cmdStr[idx++]=checksum;
            break;
            
        case HOMERF_SEND_DELETE_ALL:   /*MARS1.0: delete all 結束後 會自動產生新的MAC ID */
                                       /*必須重新下HOMERF_SEND_SET_RFID 設定回原本的 否則下次開機會配不上*/
            len=0x02;
            cmdID=HOMERF_TX_CMDID_DELETE_ALL;
            checksum = len+cmdID+PROTECT;

            cmdStr[idx++]=len;
            cmdStr[idx++]=cmdID;
            cmdStr[idx++]=PROTECT;
            cmdStr[idx++]=checksum;
            break;

        case HOMERF_SEND_DELETE:
            len=0x03;
            addr1= (gHomeRFSensorList->sSensor[sensorIdx].sID & 0xff00) >> 8;
            addr2= gHomeRFSensorList->sSensor[sensorIdx].sID & 0xff;            
            cmdID=HOMERF_TX_CMDID_DELETE_ONE;
            checksum = len+ cmdID+addr1+addr2;

            cmdStr[idx++]=len;
            cmdStr[idx++]=cmdID;
            cmdStr[idx++]=addr1;
            cmdStr[idx++]=addr2;
            cmdStr[idx++]=checksum;

            break;

        case HOMERF_SEND_APP_PAIR:
        case HOMERF_SEND_PAIR:
            len=0x09;
            cmdID=HOMERF_TX_CMDID_PAIR_MODE;
            timerCountRead(guiRFTimerID, &timerCnt1);
            timerCountRead(guiSysTimerId, &timerCnt2);
            pairID.hi=(timerCnt1<<3)|(timerCnt2 &0x03);
            pairID.lo=gHomeRFPairCnt;

            if(pairID.hi == 0)
                pairID.hi++;    //Can Not Be 0; MCU will drop
                
            if(pairID.lo == 0)
                pairID.lo++;    //Can Not Be 0; MCU will drop
          
            cmdStr[idx++]=len;
            cmdStr[idx++]=cmdID;
            memcpy(cmdStr+idx, &pairID, sizeof(u64));
            idx+=8;
            for(i=2;i<12; i++)
            {
                checksum+=cmdStr[i];

            }
            printf("**** PAIR ID %x %x checksum %x\n",pairID.hi, pairID.lo,checksum);
            cmdStr[idx++]=checksum;
            break;

        case HOMERF_SEND_PLUG_OFF:
        case HOMERF_SEND_PLUG_ON:
            len=0x06;
            cmdID=HOMERF_TX_CMDID_SET_SENSOR;
            addr1= (gHomeRFSensorList->sSensor[sensorIdx].sID & 0xff00) >> 8;
            addr2= gHomeRFSensorList->sSensor[sensorIdx].sID & 0xff;           
            if(cmd == HOMERF_SEND_PLUG_OFF)
                control =0x03;
            else
                control =0;
            
            port=0x03;
            intface=0;
            checksum = len + cmdID + addr1+ addr2 +port+ control + intface;

            cmdStr[idx++]=len;
            cmdStr[idx++]=cmdID;
            cmdStr[idx++]=addr1;
            cmdStr[idx++]=addr2;
            cmdStr[idx++]=intface;
            cmdStr[idx++]=port;
            cmdStr[idx++]=control;
            cmdStr[idx++]=checksum;
           
            break;
        case HOMERF_SEND_SIREN_OFF:
        case HOMERF_SEND_SIREN_ON:
        case HOMERF_SEND_SIREN_BEEP:
            len=0x05;
            cmdID=HOMERF_TX_CMDID_SET_SENSOR;
            addr1= (gHomeRFSensorList->sSensor[sensorIdx].sID & 0xff00) >> 8;
            addr2= gHomeRFSensorList->sSensor[sensorIdx].sID & 0xff;
            if(HOME_RF_OPTION == HOME_SENSOR_BARVOTECH || HOME_RF_OPTION == HOME_SENSOR_SWANN)
            {
                if(cmd == HOMERF_SEND_SIREN_OFF)
                    control =1;
                else if(cmd == HOMERF_SEND_SIREN_BEEP)
                    control =2;
                else
                    control =0;
            }
            else
            {
                if(cmd == HOMERF_SEND_SIREN_OFF)
                    control =0;
                else if(cmd == HOMERF_SEND_SIREN_BEEP)
                    control =2;
                else
                    control =1;
            }
            intface=0;
            checksum = len + cmdID + addr1+ addr2 + control + intface;

            cmdStr[idx++]=len;
            cmdStr[idx++]=cmdID;
            cmdStr[idx++]=addr1;
            cmdStr[idx++]=addr2;
            cmdStr[idx++]=intface;
            cmdStr[idx++]=control;
            cmdStr[idx++]=checksum;
            break;

        case HOMERF_SEND_QUERY_STATUS:
            len=0x03;
            cmdID=HOMERF_TX_CMDID_QUERY_ONE_STATUS;
            addr1= (gHomeRFSensorList->sSensor[sensorIdx].sID & 0xff00) >> 8;
            addr2= gHomeRFSensorList->sSensor[sensorIdx].sID & 0xff;   
            checksum= len+cmdID+addr1+addr2;

            cmdStr[idx++]=len;
            cmdStr[idx++]=cmdID;
            cmdStr[idx++]=addr1;
            cmdStr[idx++]=addr2;
            cmdStr[idx++]=checksum;
            
            break;


        case HOMERF_SEND_QUERY_ALL:
            len=0x02;
            cmdID=HOMERF_TX_CMDID_QUERY_ALL_PAIR;
            range=0;
            checksum=len+cmdID+range;

            cmdStr[idx++]=len;
            cmdStr[idx++]=cmdID;
            cmdStr[idx++]=range;
            cmdStr[idx++]=checksum;
            break;


        case HOMERF_SEND_SET_RFID:
            homeRFFieldID=(uiMACAddr[5]) | (uiMACAddr[4]<<8) |  (uiMACAddr[3] << 16) | (uiMACAddr[2] << 24);
            len=0x05;
            cmdID=HOMERF_TX_CMDID_SET_RFID;
            RFID[0]=homeRFFieldID &0xff;
            RFID[1]=(homeRFFieldID >> 8) &0xff;
            RFID[2]=(homeRFFieldID >>16) &0xff;
            RFID[3]=(homeRFFieldID >>24) &0xff;
            checksum = len+cmdID+RFID[0]+RFID[1]+RFID[2]+RFID[3];
            
            cmdStr[idx++]=len;
            cmdStr[idx++]=cmdID;
            cmdStr[idx++]=RFID[0];
            cmdStr[idx++]=RFID[1];
            cmdStr[idx++]=RFID[2];
            cmdStr[idx++]=RFID[3];
            cmdStr[idx++]=checksum;
            
            break;   
        case HOMERF_SEND_SWITCH_ON:
        case HOMERF_SEND_SWITCH_OFF:
            len=0x05;
            cmdID=HOMERF_TX_CMDID_SET_SENSOR;
            addr1= (gHomeRFSensorList->sSensor[sensorIdx].sID & 0xff00) >> 8;
            addr2= gHomeRFSensorList->sSensor[sensorIdx].sID & 0xff;

            //if(cmd == HOMERF_SEND_SWITCH_ON)
            //    control = 4;
            //else
            //    control = 0;
            control = homeSwitchControl;
            intface=0;
            checksum = len + cmdID + addr1+ addr2 + control + intface;

            cmdStr[idx++]=len;
            cmdStr[idx++]=cmdID;
            cmdStr[idx++]=addr1;
            cmdStr[idx++]=addr2;
            cmdStr[idx++]=intface;
            cmdStr[idx++]=control;
            cmdStr[idx++]=checksum;
            break;

			
        	break;
        
    }

    printf("[HOMERF]Send command  %d, Length = %d \n", cmdID, len);
    
#if MARSS_SUPPORT
//	myHostCmdIdx++;
//	if( myHostCmdIdx>=16)
//		myHostCmdIdx=0;
//	memcpy(&myHostCmdQueue[myHostCmdIdx][0], cmdStr, 32);
	memcpy(HostCmd, cmdStr+3, 32-3);//Bypass length & preambale
#if 1
	HostLen = len+1;
    for( i=0; i<len+4; i++ )   /* len(1 byte) + message(len byte) + preambel(2 byte) + checksum(1 byte) */
    {
        printf("0x%02x ", cmdStr[i]);
    }
#endif
	procHostCmd();
#else
    for( i=0; i<len+4; i++ )   /* len(1 byte) + message(len byte) + preambel(2 byte) + checksum(1 byte) */
    {

        sendData(HOMERF_UART_ID, &cmdStr[i]);
        printf("0x%02x ", cmdStr[i]);
    }
#endif
    printf(" \n");
}


u8 homeRFCmdParse(u8 * cmd)
{
    bool result=FALSE;   /* TRUE: send cmd success, FALSE: send cmd fail */
    u8  ret, err, i;
    u32 value;
    //u8  homeRFCheckSensorDataflg=0;

    if(cmd[HOMERF_CMD_BIT_CMD] & 0x80)
    {
        switch(cmd[HOMERF_CMD_BIT_CMD] &(~0x80))
        {
            case HOMERF_TX_CMDID_VERSION:
                gHomeRFVersion=(cmd[HOMERF_CMD_BIT_CMD+1] <<8) | (cmd[HOMERF_CMD_BIT_CMD+2]);

                printf("GET Version %x %x %x\n", cmd[HOMERF_CMD_BIT_CMD+1], cmd[HOMERF_CMD_BIT_CMD+2], gHomeRFVersion);
                
                result=TRUE;
                break;

            case HOMERF_TX_CMDID_SET_SENSOR:
                if(cmd[HOMERF_CMD_BIT_CMD +1 ] == 0x00)
                {
                    printf("****  FAIL \n");  
                    OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_SEND_CMD_FAIL, OS_FLAG_SET, &err);
                }
                else
                {
                    printf("****  SUCCESS \n");      
                    OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_SEND_CMD_SUC, OS_FLAG_SET, &err);
                }
                
                break;

            case HOMERF_TX_CMDID_QUERY_ALL_PAIR:

                //for(i=0; i<32; i++)
                //{
                    //printf("[HOMERF]PAIR SENSOR%02d %x \n",i, cmd[HOMERF_CMD_BIT_CMD+1+i]);
					//if(cmd[HOMERF_CMD_BIT_CMD+1+i] == 0)
						//homeRFCheckSensorDataflg++;
                //}
				//if(homeRFCheckSensorDataflg == 32)
					//homeRFCheckSensorData();

                OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_SEND_CMD_SUC, OS_FLAG_SET, &err);
                break;

            case HOMERF_TX_CMDID_DELETE_ONE:
                if(cmd[HOMERF_CMD_BIT_CMD +1] == 0x00)
                {
                    printf("[HOMERF]DELTE FAIL \n");
                    OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_SEND_CMD_FAIL, OS_FLAG_SET, &err);    
                }
                else
                {
                    printf("[HOMERF]DELTE SUC \n");
                    OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_SEND_CMD_SUC, OS_FLAG_SET, &err);
                }
                break;

            case HOMERF_TX_CMDID_PAIR_MODE:
                break;

            case HOMERF_TX_CMDID_NEW_FW_VERSION:
                if(cmd[HOMERF_CMD_BIT_CMD +1] == 0xFF)
                {
                    OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_OLD_FW, OS_FLAG_SET, &err);            
                }
                else
                {
                    OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_NEW_FW, OS_FLAG_SET, &err);        
                }
                break;
            
        }
        
    }
    else if(cmd[HOMERF_CMD_BIT_CMD] == HOMERF_RX_CMDID_PAIR_MODE)
    {
        homeRFSaveSensor(cmd);
        homeRFsendAckToRF(HOMERF_RX_CMDID_PAIR_MODE,0);
    }
    else if(cmd[HOMERF_CMD_BIT_CMD] == HOMERF_RX_CMDID_PERIOD_EVENT)
    {
        homeRFHandleEvent(cmd);
    }
    else if(cmd[HOMERF_CMD_BIT_CMD] == HOMERF_RX_CMDID_TRIG_EVENET)
    {
        homeRFHandleEvent(cmd);    
    }
    else if(cmd[HOMERF_CMD_BIT_CMD] == HOMERF_RX_CMDID_SENSOR_STATUS)
    {
        if(cmd[HOMERF_CMD_BIT_CMD+1]== 0) /* No pair sensor*/
        {
            OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_SEND_CMD_FAIL, OS_FLAG_SET, &err); 
            return ;
        }
    
        homeRFHandleEvent(cmd);   
        //OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_SEND_CMD_SUC, OS_FLAG_SET, &err); 
    }
    else if((cmd[HOMERF_CMD_BIT_CMD] == HOMERF_RX_CMDID_SET_SENSOR) ||\
            (cmd[HOMERF_CMD_BIT_CMD] == HOMERF_RX_CMDID_DELETE) ||\
            (cmd[HOMERF_CMD_BIT_CMD] == HOMERF_RX_CMDID_DELETE_ALL))
    {
        if(cmd[HOMERF_CMD_BIT_CMD+1] == 0) /*Fail*/
        {
            OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_SEND_CMD_FAIL, OS_FLAG_SET, &err); 
        }
        else
        {
            OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_SEND_CMD_SUC, OS_FLAG_SET, &err); 
        }
    }

    else if(cmd[HOMERF_CMD_BIT_CMD] == HOMERF_RX_CMDID_GET_FW_DATA)
    {
        /*start Pos*/
        value =(cmd[HOMERF_CMD_BIT_CMD+1]<<24) |
               (cmd[HOMERF_CMD_BIT_CMD+2]<<16) |
               (cmd[HOMERF_CMD_BIT_CMD+3]<< 8) |
               (cmd[HOMERF_CMD_BIT_CMD+4]);
               
        homeRFStartUpdate(value,cmd[HOMERF_CMD_BIT_CMD + 5]);        
    }
    else if(cmd[HOMERF_CMD_BIT_CMD] == HOMERF_RX_CMDID_GET_FW_STATUS)
    {
        value = cmd[HOMERF_CMD_BIT_CMD+1];
        //printf("UPDATE STATUS %x\n",value);
        if((value == 0xFF) || (value == 0xFE) || (value == 0xFD) || (value == 0xFC))
        {
            OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_UPDATE_FAIL, OS_FLAG_SET, &err);         
        }
        else if(value == 0x01)
        {
            OSFlagPost(gHomeRFStatusFlagGrp, HOMERF_FLAG_UPDATE_SUC, OS_FLAG_SET, &err);        
        }
    }
    
}

u8 homeRFGetNewFWAvailable(u8 *srcAddr, u32 fileSize)
{
    u8 err, i;
    u32 waitFlag=0;
    u8 cmdStr[32];
    u8 idx=0;
    u8 checksum=0;
    
    cmdStr[idx++] = HOMERF_PREAMBLE_BYTE1;
    cmdStr[idx++] = HOMERF_PREAMBLE_BYTE2;
    cmdStr[idx++] = 0x08; /*len*/
    cmdStr[idx++] = HOMERF_TX_CMDID_NEW_FW_VERSION;
    cmdStr[idx++] = 0x00; /*Ctrl ->TBD*/
    cmdStr[idx++] = srcAddr[0x1E];//srcAddr[0x1E];
    cmdStr[idx++] = srcAddr[0x1F];//srcAddr[0x1F];
    cmdStr[idx++] = (fileSize & 0xff000000)>>24; 
    cmdStr[idx++] = (fileSize & 0xff0000)>>16; 
    cmdStr[idx++] = (fileSize & 0xff00)>>8; 
    cmdStr[idx++] = (fileSize & 0xff);  /*File Size */
    
    
    

    for(i=HOMERF_CMD_BIT_LEN; i<(cmdStr[HOMERF_CMD_BIT_LEN]+HOMERF_CMD_BIT_LEN); i++)
        checksum+=cmdStr[i];
        
   
    cmdStr[idx] = checksum;

    
#if 0
    printf("[HOMERF]Send command  %d \n", HOMERF_TX_CMDID_NEW_FW_VERSION);
    for( i=0; i<cmdStr[HOMERF_CMD_BIT_LEN]+4; i++ )   /* len(1 byte) + message(len byte) + preambel(2 byte) + checksum(1 byte) */
    {

        sendData(HOMERF_UART_ID, &cmdStr[i]);
        printf("0x%02x ", cmdStr[i]);
    }
    printf(" \n");

#endif

    
    waitFlag=OSFlagPend(gHomeRFStatusFlagGrp, HOMERF_FLAG_OLD_FW | HOMERF_FLAG_NEW_FW, OS_FLAG_WAIT_SET_ANY | OS_FLAG_CONSUME, 400, &err); 
    if(err != OS_NO_ERR)
    {
        printf("[HOMERF]%d TIMER OUT\n",err);
        return HOMERF_FLAG_UPDATE_FAIL;
    }
    else if(waitFlag & HOMERF_FLAG_OLD_FW)
    {
        printf("[HOMERF]Old Version \n");
        return HOMERF_FLAG_OLD_FW;
    }
    else if(waitFlag & HOMERF_FLAG_NEW_FW)
    {
        printf("[HOMERF]New Version \n");
        homeRFUpdateSrcAddr=srcAddr;
        
        return HOMERF_FLAG_NEW_FW;
    }
            
}


void homeRFStartUpdate(u32 starPos, u8 codeLen)
{
    u8 cmdStr[48];
    u8 idx=0, i;
    u8 checksum=0;
    u32 j;
    static u32 totalCheck=0;
    
    
    if(homeRFUpdateSrcAddr != 0)
    {
        cmdStr[idx++]=HOMERF_PREAMBLE_BYTE1;
        cmdStr[idx++]=HOMERF_PREAMBLE_BYTE2;
        cmdStr[idx++]=1+codeLen;
        cmdStr[idx++]=(HOMERF_RX_CMDID_GET_FW_DATA & 0x0f) | 0x40;
        checksum=cmdStr[HOMERF_CMD_BIT_LEN] + cmdStr[HOMERF_CMD_BIT_CMD];
        for(i=0; i<codeLen; i++)
        {
            cmdStr[idx++]=homeRFUpdateSrcAddr[starPos+i];
            checksum+=cmdStr[idx-1];

        }
        cmdStr[idx]=checksum;        

        for( i=0; i<cmdStr[HOMERF_CMD_BIT_LEN]+4; i++ )   /* len(1 byte) + message(len byte) + preambel(2 byte) + checksum(1 byte) */
        {
            sendData(HOMERF_UART_ID, &cmdStr[i]);
            if((starPos >0x10) && ((i >HOMERF_CMD_BIT_CMD)&& (i< (cmdStr[HOMERF_CMD_BIT_LEN]+3))))
            {
                totalCheck += cmdStr[i];
            }
        }

        for(j=0; j<0xffff ;j++);
        memset(cmdStr, 0, sizeof(cmdStr));

       // printf("*** Total Check %x \n", totalCheck);
    }

    

}


void homeRFKeyParse(u8 key)
{
    u8 i, idx;
    switch(key)
    {
        case HOMERF_KEY_ALARM:
        case HOMERF_KEY_DISALARM:    
            for(i=0; i<HOMERF_SENSOR_MAX; i++)
            {
                if(key == HOMERF_KEY_ALARM)    
                {
                    gHomeRFSensorList->sSensor[i].sirenOnOff=HOMERF_SENSOR_ON;
                    gHomeRFSensorList->sSensor[i].pushOnOff=HOMERF_SENSOR_ON;
                }
                else
                {
                    gHomeRFSensorList->sSensor[i].pushOnOff=HOMERF_SENSOR_OFF;                    
                    gHomeRFSensorList->sSensor[i].sirenOnOff=HOMERF_SENSOR_OFF;                    
                }
            }
            if(key == HOMERF_KEY_DISALARM)
            {
                homeRFSetSirenStatus(HOMERF_SIREN_OFF);
            }
            else if(key == HOMERF_KEY_ALARM)
            {
                //homeRFSetSirenStatus(HOMERF_SIREN_BEEP);
                //homeRFSetSirenStatus(HOMERF_SIREN_OFF);                
            }
            else
            {
                //homeRFSetSirenStatus(HOMERF_SIREN_BEEP);
            }
        
            break;

        case HOMERF_KEY_PANIC:
            homeRFSetSirenStatus(HOMERF_SIREN_BEEP);
            for(i=0; i<HOMERF_SENSOR_MAX; i++)
            {
                gHomeRFSensorList->sSensor[i].sirenOnOff=HOMERF_SENSOR_ON;
                gHomeRFSensorList->sSensor[i].pushOnOff=HOMERF_SENSOR_ON;
            }

            break;

        case HOMERF_KEY_HOME:
        	homeRFSceneAppMode = 3;
            break;
    }


}




#else


u8 homeRFCmdParse(u8 * cmd)
{
}

u8 homeRFGetUartCmd(u8 *pcString)
{
}
void homeRFSendCommand(u8 cmd,u8 sensor)
{
}


#endif

#if  (HOME_RF_OPTION == HOME_SENSOR_TRANWO)
void homeRFSetDefaultSensorName(u8 type, u8 *sName)
{
    memset(sName, 0, HOMERF_NAME_MAX);

    switch(type)
    {
        case HOMERF_DEVICE_DOOR:
            //strcpy(sName, "DOOR");
            sprintf(sName, "DOOR %03d", DefaultSensorCnt);
            break;
            
        case HOMERF_DEVICE_PIR:
            //strcpy(sName, "PIR");
            sprintf(sName, "PIR %03d", DefaultSensorCnt);            
            break;
            
        case HOMERF_DEVICE_SIREN:
            //strcpy(sName, "SIREN");
            sprintf(sName, "SIREN %03d", DefaultSensorCnt);            
            break;

        case HOMERF_DEVICE_IR:
            //strcpy(sName, "IR");
            sprintf(sName, "REMOTE %03d", DefaultSensorCnt);
            break;

        case HOMERF_DEVICE_PLUG:
            //strcpy(sName, "SMART PLUG");
            sprintf(sName, "SMART PLUG %03d", DefaultSensorCnt);
            break;

        case HOMERF_DEVICE_ROUTER:
            //strcpy(sName, "ROUTER");
            sprintf(sName, "ROUTER %03d", DefaultSensorCnt);
            break;

        case HOMERF_DEVICE_TEMP_HUM:
            //strcpy(sName, "TEMP");
            sprintf(sName, "TEMP %03d", DefaultSensorCnt);
            break;

        case HOMERF_DEVICE_VIBRATE:
            //strcpy(sName, "VIBRATE");
            sprintf(sName, "VIBRATE %03d", DefaultSensorCnt);
            break;

        case HOMERF_DEVICE_SMOKE:
            //strcpy(sName, "SMOKE");
            sprintf(sName, "SMOKE %03d", DefaultSensorCnt);
            break;

        case HOMERF_DEVICE_LEAK:
            //strcpy(sName, "LEAK");
            sprintf(sName, "LEAK %03d", DefaultSensorCnt);
            break;

        case HOMERF_DEVICE_GAS:
            //strcpy(sName, "GAS");
            sprintf(sName, "GAS %03d", DefaultSensorCnt);
            break;

        case HOMERF_DEVICE_PANIC:
            //strcpy(sName, "PANIC");
            sprintf(sName, "PANIC %03d", DefaultSensorCnt);
            break;
            
        case HOMERF_DEVICE_TEMP_HYG:
	        sprintf(sName, "THERMO %03d", DefaultSensorCnt);
	        break;
            
        default:
            strcpy(sName, "UNKNOWN");
            sName="UNKNOWN";
            break;
        
    }
}

void homeRFSetDefaultScene(void)
{
    u8 i, j;
	BOOLEAN isEmpty = FALSE;

    gHomeRFSceneList->sScene[0].sceneID = DEFAULT_SCENEID_ALARM;
    gHomeRFSceneList->sScene[1].sceneID = DEFAULT_SCENEID_DISALARM;
    gHomeRFSceneList->sScene[2].sceneID = DEFAULT_SCENEID_INHOMEMODE;
    strcpy(gHomeRFSceneList->sScene[0].sceneName, "Alarm");
    strcpy(gHomeRFSceneList->sScene[1].sceneName, "Disarm");
    strcpy(gHomeRFSceneList->sScene[2].sceneName, "HomeMode");    
    for(j=0; j< HOMERF_SCENE_MAX; j++)
    {
		// Add by paul for assign default alarm status of custom scene.
    	if(gHomeRFSceneList->sScene[0].sceneID == 0)
			continue;
		// Paul add end
        for(i=0; i<gHomeRFSensorCnt;i++)
        {
			// Add by paul for assign default alarm status of custom scene.
			if(gHomeRFSceneList->sScene[j].sID[i] !=0)
			{//not sync, show waring, sync them
				isEmpty = FALSE;
			} else {
				isEmpty = TRUE;
//				printf("[HOMERF]Add scene sensor %d\n", gHomeRFSensorList->sSensor[i].sID);
			}
			// Paul add end

            gHomeRFSceneList->sScene[j].sID[i] = gHomeRFSensorList->sSensor[i].sID;
            if(j==0)
                gHomeRFSceneList->sScene[j].isAlarm[i] = HOMERF_SENSOR_ON;
            else if(j==1)
                gHomeRFSceneList->sScene[j].isAlarm[i] = HOMERF_SENSOR_OFF;
            else if(j==2)
            {
                if((gHomeRFSensorList->sSensor[i].type == HOMERF_DEVICE_DOOR) || (gHomeRFSensorList->sSensor[i].type == HOMERF_DEVICE_SIREN) || (gHomeRFSensorList->sSensor[i].type == HOMERF_DEVICE_IR))
                    gHomeRFSceneList->sScene[j].isAlarm[i] = HOMERF_SENSOR_ON;
                else
                    gHomeRFSceneList->sScene[j].isAlarm[i] = HOMERF_SENSOR_OFF;
            }
			else if(isEmpty) // Add by paul for assign default alarm status of custom scene.
				gHomeRFSceneList->sScene[j].isAlarm[i] = HOMERF_SENSOR_ON;
			// Paul add end
        }
        gHomeRFSceneList->sScene[j].totalCnt = gHomeRFSensorCnt;
    }
    spiWriteHomeRF(SPI_HOMERF_SCENE);
}

#else
void homeRFSetDefaultSensorName(u8 type, u8 *sName)
{
    memset(sName, 0, HOMERF_NAME_MAX);

    switch(type)
    {
        case HOMERF_DEVICE_DOOR:
            strcpy(sName, "DOOR");
            break;

        case HOMERF_DEVICE_PIR:
            strcpy(sName, "PIR");
            break;

        case HOMERF_DEVICE_SIREN:
            strcpy(sName, "SIREN");
            break;

        case HOMERF_DEVICE_IR:
            strcpy(sName, "IR");
            break;

        case HOMERF_DEVICE_PLUG:
            strcpy(sName, "SMART PLUG");
            break;

        case HOMERF_DEVICE_ROUTER:
            strcpy(sName, "ROUTER");
            break;

        case HOMERF_DEVICE_TEMP_HUM:
            strcpy(sName, "TEMP");
            break;

        case HOMERF_DEVICE_VIBRATE:
            strcpy(sName, "VIBRATE");
            break;

        case HOMERF_DEVICE_SMOKE:
            strcpy(sName, "SMOKE");
            break;

        case HOMERF_DEVICE_LEAK:
            strcpy(sName, "LEAK");
            break;

        case HOMERF_DEVICE_GAS:
            strcpy(sName, "GAS");
            break;

        case HOMERF_DEVICE_PANIC:
            strcpy(sName, "PANIC");
            break;
            
        default:
            strcpy(sName, "UNKNOWN");
            sName="UNKNOWN";
            break;
        
    }
}

void homeRFSetDefaultScene(void)
{
    u8 i, j;

    gHomeRFSceneList->sScene[0].sceneID = DEFAULT_SCENEID_ALARM;
    gHomeRFSceneList->sScene[1].sceneID = DEFAULT_SCENEID_DISALARM;
    strcpy(gHomeRFSceneList->sScene[0].sceneName, "Alarm");
    strcpy(gHomeRFSceneList->sScene[1].sceneName, "Disarm");    
    for(j=0; j< 2; j++)
    {
        for(i=0; i<gHomeRFSensorCnt;i++)
        {
            gHomeRFSceneList->sScene[j].sID[i] = gHomeRFSensorList->sSensor[i].sID;
            if(j==0)
                gHomeRFSceneList->sScene[j].isAlarm[i] = HOMERF_SENSOR_ON;
            else if(j==1)
                gHomeRFSceneList->sScene[j].isAlarm[i] = HOMERF_SENSOR_OFF;
        }
        gHomeRFSceneList->sScene[j].totalCnt = gHomeRFSensorCnt;
    }
    spiWriteHomeRF(SPI_HOMERF_SCENE);
}

#endif


void homeRFSortRoomList(HOMERF_ROOM_DATA * sList)
{
    HOMERF_ROOM_DATA tempList[HOMERF_ROOM_MAX];
    u32  tempSID[HOMERF_SENSOR_MAX];
    u8 i, j, k, l;

    j=0;
    memset(tempList, 0, sizeof(HOMERF_ROOM_DATA)*HOMERF_ROOM_MAX);
    for(i=0; i<HOMERF_ROOM_MAX; i++)
    {
        if(sList[i].roomID !=0)
        {
            memcpy(&tempList[j],&sList[i],sizeof(sList[i]));           
            j++;
        }
        
        l=0;
        memset(tempSID, 0, sizeof(tempSID));
        for(k=0; k<HOMERF_SENSOR_MAX; k++)
        {
            if(sList[i].sID[k] !=0)
            {
                memcpy(&tempSID[l],&sList[i].sID[k],sizeof(u32));            
                l++;
            }
        }
        memcpy(sList[i].sID, tempSID,sizeof(u32)*HOMERF_SENSOR_MAX);      
    }
    memcpy(sList, tempList,sizeof(HOMERF_ROOM_DATA)*HOMERF_ROOM_MAX);
    
}

//Add by paul for deletesensor
void homeRFDeleteSceneSensor(u8 idx, HOMERF_SCENE_DATA *sList)
{
    u8 i, j;
	printf("[HOMERF]delete scene sensor, SID=%d\n",idx);
	for(i=0; i<HOMERF_SCENE_MAX; i++)
    {//sList[i] == current scene
        if(sList[i].sceneID==0)
        	continue;

	    for(j=idx; j<HOMERF_SENSOR_MAX-1; j++)
        { //Start from next of delete index. 
        	if(sList[i].sID[j]==0)
        		continue;
			sList[i].sID[j] = sList[i].sID[j+1];
        	sList[i].isAlarm[j] = sList[i].isAlarm[j+1];
	    }
		if(sList[i].sID[j] != 0)
		{
			sList[i].sID[j-1] = 0;
			sList[i].isAlarm[j-1] = HOMERF_SENSOR_OFF;
		}
		sList[i].totalCnt--;

		if(gHomeRFSensorCnt != sList[i].totalCnt)
			printf("[HOMERF]WARING!! Scene count %d difference then sensor count %d\n",
					sList[i].totalCnt, gHomeRFSensorCnt);
	}
	spiWriteHomeRF(SPI_HOMERF_SCENE);
}

void homeRFSortSceneList(HOMERF_SCENE_DATA *sList)
{
    HOMERF_SCENE_DATA tempList[HOMERF_SCENE_MAX];
    u8 i, j;

    j=0;
    memset(tempList, 0, sizeof(HOMERF_SCENE_DATA)*HOMERF_SCENE_MAX);
    for(i=0; i<HOMERF_SCENE_MAX; i++)
    {
        if(sList[i].sceneID !=0)
        {
            memcpy(&tempList[j],&sList[i],sizeof(HOMERF_SCENE_DATA));
            j++;
        }
    }

    memcpy(sList, tempList, sizeof(HOMERF_SCENE_DATA)*HOMERF_SCENE_MAX);
    
}



u8 homeRFDeleteSensor(u8 idx)
{
    u8 ret;
    u8 i, j;
    
    ret=homeRFSendToSensor(HOMERF_SEND_DELETE, idx);

    if(ret == 0)
        return ret;
	
#if CLOUD_SUPPORT
	SendCloud_ADD_DEL_Message(gHomeRFSensorList->sSensor[idx].sID, HOMERF_DEL_SENSOR, 0);
#endif	

    for(i=0; i<HOMERF_ROOM_MAX; i++)
    {
        for(j=0; j< HOMERF_SENSOR_MAX; j++)
        {
            if(gHomeRFRoomList->sRoom[i].sID[j] == gHomeRFSensorList->sSensor[idx].sID)
            {
                gHomeRFRoomList->sRoom[i].sID[j]=0;  
            }
        }
    }
    gHomeRFCheckAlert[idx>>3]&= ~(0x01 <<(idx%8));//Clear alert status for UI, Paul
    gHomeRFSensorCnt--;
    memset(gHomeRFSensorList->sSensor+idx, 0, sizeof(HOMERF_SENSOR_DATA));
    memcpy(gHomeRFSensorList->sSensor+idx, gHomeRFSensorList->sSensor+idx+1,sizeof(HOMERF_SENSOR_DATA)*(HOMERF_SENSOR_MAX-idx-1));
    memset(gHomeRFSensorList->sSensor+gHomeRFSensorCnt, 0, sizeof(HOMERF_SENSOR_DATA));
    homeRFSortRoomList(gHomeRFRoomList->sRoom);
    homeRFDeleteSceneSensor(idx, gHomeRFSceneList->sScene);
    spiWriteHomeRF(SPI_HOMERF_SENSOR);
    spiWriteHomeRF(SPI_HOMERF_ROOM);
    return ret;
}


void homeRFDeleteRoom(u8 idx)
{
    u8 ret;
    u8 i;

    gHomeRFRoomList->sRoom[idx].roomID=0;
    strcpy(gHomeRFRoomList->sRoom[idx].roomNmae,"");

    memset(gHomeRFRoomList->sRoom[idx].sID,0, sizeof(u32)*HOMERF_SENSOR_MAX);
    
    gHomeRFRoomList->sRoom[idx].scount=0;
    homeRFSortRoomList(gHomeRFRoomList->sRoom);
    spiWriteHomeRF(SPI_HOMERF_ROOM);
}

void homeRFDeleteScene(u8 idx)
{
    u8 ret;
    u8 i;

    gHomeRFSceneList->sScene[idx].sceneID=0;
    strcpy(gHomeRFSceneList->sScene[idx].sceneName,"");
    homeRFSortSceneList(gHomeRFSceneList->sScene);
    spiWriteHomeRF(SPI_HOMERF_SCENE);

}




void homeRFCheckAppPairSensor(void)
{
    static u8 count=30;
    
    if(gHomeRFOpMode == HOMERF_OP_APP_PAIR)
    {
        count--;
        if(count==0)
        {
            gHomeRFOpMode == HOMERF_OP_NOMAL;
            gAppPairFlag=APP_PAIR_FAIL;
        }
    }
    else
    {
        count=30;
    }
}


u8 homeRFGetSensorCnt(u8 type)
{
    u8 cnt=0, i=0;
    
    for(i=0; i<HOMERF_SENSOR_MAX; i++)
    {
        if(gHomeRFSensorList->sSensor[i].sID == 0)
            continue;
            
        if(gHomeRFSensorList->sSensor[i].type == type)
            cnt++;
    }
    
    return cnt;
}

u8 homeRFGetSensorIndex(u32 ID)
{
    u8 idx;
    for(idx=0; idx<HOMERF_SENSOR_MAX; idx++)
    {
        if(gHomeRFSensorList->sSensor[idx].sID== ID)
        {
            break;
        }
    }

    return idx;    
}



void homeRFTask(void* pData)
{
    u32 waitFlag=0;
    u32 setFlag=0;
    u8  err;
    u8  i;

    while(1)
    {
       //waitFlag=OSFlagPend(gHomeRFStatusFlagGrp, HOMERF_SENSOR_STATUS_RESEND, (OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME), 100, &err);  
		waitFlag=OSFlagPend(gHomeRFStatusFlagGrp, HOMERF_SENSOR_STATUS_RESEND, (OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME), OS_IPC_WAIT_FOREVER, &err);  
        for(i=0; i< HOMERF_QUEUE_SIZE; i++)
        {
            if(homeRFCmdQueue[i].sID == 0)
                continue;
            if(homeRFCmdQueue[i].timeCount > 2)  /* command exceed send 2 times , system regard the sensor as unlink sensor*/
            {
                memset(homeRFCmdQueue+i,0, sizeof(HOMERF_CMD_QUEUE));
                continue;
            }
            
            homeRFCmdQueue[i].timeCount++;
            homeRFSendCommand(homeRFCmdQueue[i].cmdType, homeRFGetSensorIndex(homeRFCmdQueue[i].sID));

        }
        
        
        
        
    }
}
#endif  /* end #if (HOME_RF_SUPPORT )  */
