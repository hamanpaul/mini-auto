/*
Revision History:

    2018/07/31  Sean  Create

*/

#include "lwip/apps/MARS_tftp.h"
#include <string.h>
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include "General.h"
#include	".\fs\fs_api.h"
#include	"Dcfapi.h"

#define Save_Mode SD_mode

__packed typedef struct 
{
  u8   InitFlat;
  u8   W_R_Mode;   //Read/Write Mode: 1:Write 0: Read
  u8*  Flash_Addr ;
}Iap_FlashCont;

static  Iap_FlashCont   Iapflashcont ;

static void * OpenFile(const char* fname, const char* mode, u8_t write);
static void Close_File(void* handle);
static int Read_File(void* handle, void* buf, int bytes);
static int Write_File(void* handle, struct pbuf* p);

const struct tftp_context  tftpContext={       //TFTP SERVER interface
 OpenFile,
 Close_File,
 Read_File,    
 Write_File,
};

u8 tmp;
u32 writesize, codeSize; 
FS_FILE* pFile;	

extern u8* p2plocal_buffer; //Sean: Temp buff location(too small)

  /**
   * @param const char* fname   File Name
   * @param const char* mode
   * @param u8_t write   Mode  1:Write  0:Read
   * @returns Iapflashcont struct
  */
static void * OpenFile(const char* fname, const char* mode, u8_t write)
{   
    u8 file_name[64] ="\\"; //NAME lenth Only suppout 12 bytes, else will cut automatic
    
    printf("Open File  %s \n",fname);    
    printf("Open Mode  %s \n",mode);
    Iapflashcont.W_R_Mode = write ;
    Iapflashcont.W_R_Mode ? printf("Write File\n") : printf("Read File\n");
if (Save_Mode == Memory_mode)
{
  if(Iapflashcont.W_R_Mode == 1)  //Write mode
  {
    memset(p2plocal_buffer, 0 , 614400); //640*480*2
    Iapflashcont.Flash_Addr = p2plocal_buffer ;  //Memory Start address
    printf("Erasing memory...\n");  
    
    Iapflashcont.InitFlat =1 ;   //Mark init Ready
    printf("Write File Init Success\n");
    sysDeadLockMonitor_OFF();

  }
  else  //Read mode
  {
    Iapflashcont.InitFlat =1 ;   //Mark init Ready
    printf("Read File Init Success\n");
    Iapflashcont.Flash_Addr = p2plocal_buffer ;  //Memory Start address
    sysDeadLockMonitor_OFF();

  }
}
else if (Save_Mode == SD_mode)
{
  if(Iapflashcont.W_R_Mode == 1)  //Write mode
  {
    strcat(file_name, fname);
    pFile = dcfOpen(file_name, "w");
        
    Iapflashcont.Flash_Addr = p2plocal_buffer;  //Memory Start address
    
    Iapflashcont.InitFlat =1 ;   //Mark init Ready
    printf("Write File Init Succes \r\n");
    sysDeadLockMonitor_OFF();

  }
  else  //Read mode
  {
    strcat(file_name, fname);
    if((pFile = dcfOpen(file_name, "r")) == NULL)
            Iapflashcont.InitFlat =0 ;   //Mark init NOT Ready
    else
    {
        memset(p2plocal_buffer, 0 , 614400); //640*480*2
        dcfRead(pFile, p2plocal_buffer, pFile->size, &codeSize);
            
        Iapflashcont.Flash_Addr = p2plocal_buffer;  //Memory Start address

        Iapflashcont.InitFlat =1 ;   //Mark init Ready
        printf("Read File Init Succes\n");
        sysDeadLockMonitor_OFF();
    }
  }
}
  return (Iapflashcont.InitFlat) ? (&Iapflashcont) : NULL ;
}

  /**
   *  Close File Struct
   * @param None
   * @param None
   * @param None
   * @returns None
   */
static void Close_File(void* handle)
{
    Iap_FlashCont * Filehandle = (Iap_FlashCont *) handle ;

    Filehandle->InitFlat = 0 ;
    printf("Close File\n");  

    sysDeadLockMonitor_ON();

    if (Save_Mode == SD_mode)
        if(Filehandle->W_R_Mode)  //If it's Write mode before
            dcfClose(pFile, &tmp);
}

  /**
   *  Read File data
   * @param handle File struct
   * @param *buf    Save data's buffer
   * @param bytes   Read File size
   * @returns Return Read size, else ERROR <0
   */
static int Read_File(void* handle, void* buf, int bytes)
{  
  int Count;
  u8 zero_cnt = 0;
  Iap_FlashCont * Filehandle = (Iap_FlashCont *) handle ;

  //printf("Read Size: %ld \r\n",bytes);    
  printf(".");

  if(!Filehandle->InitFlat)  //Non-init
  {
    return ERR_MEM ;
  } 

  for(  Count = 0 ; (Count < bytes)&&(Filehandle->Flash_Addr<= (p2plocal_buffer+614400))  ;  Count++ ,Filehandle->Flash_Addr++ ) //640*480*2
  {
   ((u8 *)buf)[Count] = *((u8 *) Filehandle->Flash_Addr);     

   if(Filehandle->Flash_Addr[0] == 0x00)
   {
        zero_cnt++;
        if(zero_cnt > 3)     
        {
            Count -= 3;
            break;
        }
   }
   else
        zero_cnt = 0;
  }

  return Count;
}


  /**
   *  Write File data
   * @param handle           File Struct
   * @param struct pbuf* p  Data struct
   * @returns <0 if ERROR
   */
static int Write_File(void* handle, struct pbuf* p)
{
#define Memory_buffer_size  64*1024

  Iap_FlashCont * Filehandle = (Iap_FlashCont *) handle ;     
  //printf("Write Size %ld \r\n",p->len); 
  printf(".");
  if(!Filehandle->InitFlat)
  {
    printf("Write File, Non-init! ERROR.\n");
    return ERR_MEM ;
  }
  //printf("Start Write Address: 0X%08X \r\n",Filehandle->Flash_Addr);
if (Save_Mode == Memory_mode)
{
  memcpy((u32*)Filehandle->Flash_Addr, (u32 *)p->payload, p->len);
  Filehandle->Flash_Addr += p->len;
}
else if(Save_Mode == SD_mode) //Sean: First Buffer(128K) in m.m, and then write to SD card, about 6 times speedup.
{
    memcpy((u32*)Filehandle->Flash_Addr, (u32 *)p->payload, p->len);
    Filehandle->Flash_Addr += p->len;

    if(Filehandle->Flash_Addr >= p2plocal_buffer + Memory_buffer_size)
    {
        dcfWrite(pFile, p2plocal_buffer, Memory_buffer_size, &writesize);
        Filehandle->Flash_Addr = p2plocal_buffer;
    }
    else if((p->len) < 512) //Last packet
    {
        dcfWrite(pFile, p2plocal_buffer, (Filehandle->Flash_Addr-p2plocal_buffer), &writesize);
    }
}

  //printf("Write Success, Next Address: 0X%08X \r\n",(u32)Filehandle->Flash_Addr);
  return ERR_OK;
}

void Test_TFTP(void)
{
    #include "lwip/apps/Tftp_server.h"
    //#include "MARS_tftp.h"
    
    printf("Start TFTP Service.\n");
    tftp_init(&tftpContext);
}



