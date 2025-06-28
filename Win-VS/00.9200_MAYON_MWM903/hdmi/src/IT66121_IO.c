///*****************************************
//  Copyright (C) 2009-2015
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IO.c>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2015/09/03
//   @fileversion: ITE_HDMITX_SAMPLE_3.21
//******************************************/
#include "general.h"
#include "board.h"

#if(HDMI_TXIC_SEL == HDMI_TX_IT66121)

#include <stdlib.h>
#include "IT66121_typedef.h"
#include <string.h>
#include "IT66121_Mcu.h"

//#include "i2capi.h"
extern s32 i2cWrite_Byte(u8 id, u8 addr, u8 data);
extern s32 i2cRead_Byte(u8 id, u8 addr, u8* pData);

// BYTE I2CADR=RXADR;
// BYTE I2CDEV=RXDEV;

///////////////////////////////////////////////////////////////////////////////
// Start: I2C for 8051
///////////////////////////////////////////////////////////////////////////////
#if 0
void set_8051_scl( BOOL bit_value )
{
     SCL_PORT=bit_value;
}

void set_8051_sda( BOOL bit_value,BYTE device )
{
     switch( device ) {
         case 0:
             TX0_SDA_PORT=bit_value;
            break;
     }
}

BOOL get_8051_sda( BYTE device )
{
     switch( device ) {
         case 0:
             TX0_SDA_PORT=1;
             return TX0_SDA_PORT;
            break;
     }
}

void i2c_8051_start( BYTE device )
{
    set_8051_sda( HIGH,device );
    set_8051_scl( HIGH );
    set_8051_sda( LOW,device );
    set_8051_scl( LOW );
}

void i2c_8051_write( BYTE byte_data,BYTE device )
{
 BYTE data bit_cnt,tmp;
 BOOL data bit_value;

     for(bit_cnt=0; bit_cnt<8; bit_cnt++) {
         tmp=(byte_data << bit_cnt) & 0x80;
         bit_value=tmp && 0x80;

         set_8051_sda( bit_value,device );
         set_8051_scl( HIGH );
         set_8051_scl( LOW );
     }
}

BOOL i2c_8051_wait_ack( BYTE device )
{
 BOOL data ack_bit_value;

    set_8051_sda( HIGH,device );
    set_8051_scl( HIGH );
    ack_bit_value=get_8051_sda( device );
    set_8051_scl( LOW );

    return ack_bit_value;
}

BYTE i2c_8051_read( BYTE device )
{
 BYTE data bit_cnt,byte_data;
 BOOL data bit_value;

     byte_data=0;
     for(bit_cnt=0; bit_cnt<8; bit_cnt++) {
         set_8051_scl( HIGH );

         bit_value=get_8051_sda( device );

         byte_data=(byte_data << 1) | bit_value;

         set_8051_scl( LOW );
     }

     return byte_data;
}

void i2c_8051_send_ack( BOOL bit_value,BYTE device )
{
     set_8051_sda( bit_value,device );
     set_8051_scl( HIGH );
     set_8051_scl( LOW );
     set_8051_sda( HIGH,device );
}

void i2c_8051_end( BYTE device )
{
     set_8051_sda( LOW,device );
     set_8051_scl( HIGH );
     set_8051_sda( HIGH,device );
}
#endif

BOOL i2c_write_byte( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data,BYTE device )
{
    if(byteno >1) 
        DEBUG_HDMI("HDMI i2c_write_byte byteno %d\n", byteno);
    return i2cWrite_Byte((u8)address, (u8)offset, (u8)*p_data);

#if 0
 BYTE data i;

     i2c_8051_start(device);                // S

     i2c_8051_write(address&0xFE,device);        // slave address (W)
     if( i2c_8051_wait_ack(device)==1 )    {        // As
         i2c_8051_end(device);
     return 0;
      }

     i2c_8051_write(offset,device);            // offset
     if( i2c_8051_wait_ack(device)==1 )    {        // As
         i2c_8051_end(device);
     return 0;
     }

     for(i=0; i<byteno-1; i++) {
          i2c_8051_write(*p_data,device);        // write d
          if( i2c_8051_wait_ack(device)==1 ) {        // As
              i2c_8051_end(device);
         return 0;
         }
         p_data++;
     }

     i2c_8051_write(*p_data,device);            // write last d
     if( i2c_8051_wait_ack(device)==1 )    {        // As
          i2c_8051_end(device);
     return 0;
     }
     else {
          i2c_8051_end(device);
     return 1;
     }
#endif
}

BOOL i2c_read_byte( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data,BYTE device )
{
    if(byteno >1) 
        DEBUG_HDMI("HDMI i2c_read_byte byteno %d\n", byteno);
    return i2cRead_Byte((u8)address, (u8)offset, (u8 *)p_data);

#if 0
 BYTE data i;

     i2c_8051_start(device);                // S

     i2c_8051_write(address&0xFE,device);        // slave address (W)
     if( i2c_8051_wait_ack(device)==1 ) {        // As
         i2c_8051_end(device);
         return 0;
     }

     i2c_8051_write(offset,device);            // offset
     if( i2c_8051_wait_ack(device)==1 ) {        // As
         i2c_8051_end(device);
         return 0;
     }

     i2c_8051_start(device);

     i2c_8051_write(address|0x01,device);        // slave address (R)
     if( i2c_8051_wait_ack(device)==1 ) {        // As
         i2c_8051_end(device);
         return 0;
     }

     for(i=0; i<byteno-1; i++) {
         *p_data=i2c_8051_read(device);        // read d
         i2c_8051_send_ack(LOW,device);        // Am

         p_data++;
     }

     *p_data=i2c_8051_read(device);            // read last d
     i2c_8051_send_ack(HIGH,device);            // NAm
     i2c_8051_end(device);

    return 1;
#endif
}


BYTE HDMITX_ReadI2C_Byte(BYTE RegAddr)
{
    BYTE p_data;

    i2c_read_byte(TX0ADR|0x01,RegAddr,1,&p_data,TX0DEV);

    return p_data;
}

SYS_STATUS HDMITX_WriteI2C_Byte(BYTE RegAddr,BYTE d)
{
    BOOL flag;

    flag=i2c_write_byte(TX0ADR,RegAddr,1,&d,TX0DEV);

    return !flag;
}

SYS_STATUS HDMITX_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
    BOOL flag;

    flag=i2c_read_byte(TX0ADR|0x01,RegAddr,N,pData,TX0DEV);

    return !flag;
}

// SYS_STATUS HDMITX_WriteI2C_ByteN(BYTE RegAddr,BYTE _CODE *pData,int N)
// {
//     //BOOL data flag;
//     //flag=i2c_write_byte(TX0ADR,RegAddr,N,pData,TX0DEV);
//     BOOL flag;
//     BYTE I2C_buf[18];
//     int     i;
//     for (i = 0; i < N; i++)
//     {
//         I2C_buf[i]=pData[i];
//         flag = i2c_write_byte(TX0ADR, RegAddr++, 1, &I2C_buf[i], TX0DEV);
//     }
//     //flag = i2c_write_byte(TX0ADR, RegAddr++, N, I2C_buf, TX0DEV);
//     return !flag;
// }

SYS_STATUS HDMITX_SetI2C_Byte(BYTE Reg,BYTE Mask,BYTE Value)
{
    BYTE Temp;
    if( Mask != 0xFF )
    {
        Temp=HDMITX_ReadI2C_Byte(Reg);
        Temp&=(~Mask);
        Temp|=Value&Mask;
    }
    else
    {
        Temp=Value;
    }
    return HDMITX_WriteI2C_Byte(Reg,Temp);
}

#if 0
SYS_STATUS HDMITX_ToggleBit(BYTE Reg,BYTE n)
{
    BYTE Temp;
    Temp=HDMITX_ReadI2C_Byte(Reg);
//    HDMITX_DEBUG_PRINTF(("INVERVIT  0x%bx[%bx]",Reg,n));
	printf("reg%02X = %02X -> toggle %dth bit ->",(int)Reg,(int)Temp,(int)n) ;
	Temp^=(1<<n) ;
	printf(" %02X\n",(int)Temp) ;

//    HDMITX_DEBUG_PRINTF(("0x%bx\n",Temp));
    return HDMITX_WriteI2C_Byte(Reg,Temp);
}
#endif

#ifdef SUPPORT_HDMIRX_I2C
BYTE HDMIRX_ReadI2C_Byte(BYTE RegAddr)
{
    BYTE p_data;

    i2c_read_byte(RXADR,RegAddr,1,&p_data,TX0DEV);

    return p_data;
}

SYS_STATUS HDMIRX_WriteI2C_Byte(BYTE RegAddr,BYTE d)
{
    BOOL flag;

    flag=i2c_write_byte(RXADR,RegAddr,1,&d,TX0DEV);

    return !flag;
}

SYS_STATUS HDMIRX_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
    BOOL flag;

    flag=i2c_read_byte(RXADR,RegAddr,N,pData,TX0DEV);

    return !flag;
}

SYS_STATUS HDMIRX_WriteI2C_ByteN(BYTE RegAddr,BYTE _CODE *pData,int N)
{
    //BOOL data flag;
    //flag=i2c_write_byte(RXADR,RegAddr,N,pData,TX0DEV);
    BOOL flag;
    BYTE I2C_buf[18];
    int     i;
    for (i = 0; i < N; i++)
    {
        I2C_buf[i]=pData[i];
        flag = i2c_write_byte(RXADR, RegAddr++, 1, &I2C_buf[i], TX0DEV);
    }
    //flag = i2c_write_byte(RXADR, RegAddr++, N, I2C_buf, TX0DEV);
    return !flag;
}

SYS_STATUS HDMIRX_SetI2C_Byte(BYTE Reg,BYTE Mask,BYTE Value)
{
    BYTE Temp;
    Temp=HDMIRX_ReadI2C_Byte(Reg);
    Temp&=(~Mask);
    Temp|=Value&Mask;
    return HDMIRX_WriteI2C_Byte(Reg,Temp);
}

SYS_STATUS HDMIRX_ToggleBit(BYTE Reg,BYTE n)
{
    BYTE Temp;
    Temp=HDMIRX_ReadI2C_Byte(Reg);
//    HDMIRX_DEBUG_PRINTF(("INVERVIT  0x%bx[%bx]",Reg,n));
	HDMITX_DEBUG_PRINTF("reg%02X = %02X -> toggle %dth bit ->",(int)Reg,(int)Temp,(int)n) ;
	Temp^=(1<<n) ;
	HDMITX_DEBUG_PRINTF(" %02X\n",(int)Temp) ;

//    HDMIRX_DEBUG_PRINTF(("0x%bx\n",Temp));
    return HDMIRX_WriteI2C_Byte(Reg,Temp);
}
#endif

BYTE CEC_ReadI2C_Byte(BYTE RegAddr)
{
    BYTE p_data;

    i2c_read_byte(TX0CECADR,RegAddr,1,&p_data,TX0DEV);

    return p_data;
}

SYS_STATUS CEC_WriteI2C_Byte(BYTE RegAddr,BYTE d)
{
    BOOL flag;

    flag=i2c_write_byte(TX0CECADR,RegAddr,1,&d,TX0DEV);

    return !flag;
}


SYS_STATUS CEC_SetI2C_Byte(BYTE Reg,BYTE Mask,BYTE Value)
{
    BYTE Temp;
    Temp=CEC_ReadI2C_Byte(Reg);
    Temp&=(~Mask);
    Temp|=Value&Mask;
    return CEC_WriteI2C_Byte(Reg,Temp);
}
#if 0

SYS_STATUS CEC_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
    BOOL data flag;

    flag=i2c_read_byte(TX0CECADR,RegAddr,N,pData,TX0DEV);

    return !flag;
}

SYS_STATUS CEC_WriteI2C_ByteN(BYTE RegAddr,BYTE _CODE *pData,int N)
{
    //BOOL data flag;
    //flag=i2c_write_byte(TX0CECADR,RegAddr,N,pData,TX0DEV);
    BOOL flag;
    BYTE I2C_buf[18];
    int     i;
    for (i = 0; i < N; i++)
    {
        I2C_buf[i]=pData[i];
        flag = i2c_write_byte(TX0CECADR, RegAddr++, 1, &I2C_buf[i], TX0DEV);
    }
    //flag = i2c_write_byte(TX0CECADR, RegAddr++, N, I2C_buf, TX0DEV);
    return !flag;
}


SYS_STATUS CEC_ToggleBit(BYTE Reg,BYTE n)
{
    BYTE Temp;
    Temp=CEC_ReadI2C_Byte(Reg);
//    CEC_DEBUG_PRINTF(("INVERVIT  0x%bx[%bx]",Reg,n));
	printf("reg%02X = %02X -> toggle %dth bit ->",(int)Reg,(int)Temp,(int)n) ;
	Temp^=(1<<n) ;
	printf(" %02X\n",(int)Temp) ;

//    CEC_DEBUG_PRINTF(("0x%bx\n",Temp));
    CEC_WriteI2C_Byte(Reg,Temp);
	return ER_SUCCESS ;
}

#endif
#endif

