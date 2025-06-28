#ifndef __GPSAPI_H__
#define __GPSAPI_H__

#include "general.h"
#include "board.h"

//#define UART_GPS_COMMAND	1

#if (UART_GPS_COMMAND && UART_COMMAND)


#define MAXGPSPKERRORCOUNT		100

#define GPS_COMMAND_GGA		0x00
#define GPS_COMMAND_GLL		0x01
#define GPS_COMMAND_GSA		0x02
#define GPS_COMMAND_GSV		0x03
#define GPS_COMMAND_RMC		0x04
#define GPS_COMMAND_VTG		0x05

#define GPS_FIELD_TIME			0x01
#define GPS_FIELD_LATITUDE		0x02
#define GPS_FIELD_LONGITUDE		0x04
#define GPS_FIELD_DIRECTION		0x08
#define GPS_FIELD_SPEED			0x10
#define GPS_FIELD_DATE			0x20
#define GPS_FIELD_ALL			0x3F



/* _GPS_DATA Type */
typedef struct _GPS_DATA {
	u8	Year;			//year
	u8	Month;			//Month
	u8	Day;			//Day
	u8	Hour;			//UTC Hour

	u8	Min;			//UTC Minute
	u8	Sec;			//UTC Second
	u8  N_S;			//N: north, S=South
	u8  E_W;			//E:east, W=West

	u8  valid;			//if value, 1: valid, 0: invalid
	u8  Sec_pre;		//not GPS data, just for measure control
	u8  State;			//not GPS data, just for measure control
	u8	Field;			//
	
	int Lat_I;			//Latitude, integer part
	int Lat_F;			//Latitude, fraction part

	int Lon_I;			//Longitude, integer part
	int Lon_F;			//Longitude, fraction part

	int Dir_I;			//Direction, integer part
	int Dir_F;			//Direction, fraction part

	int Speed_I;		//Speed, integer part
	int Speed_F;		//Speed, fraction part
}GPS_DATA;

extern GPS_DATA	gGPS_data1, gGPS_data2;
extern OS_EVENT* GPSUpdateEvt;	
extern int gGPSSatellitesUsed;
extern int gGPSFixValid;

extern void GPS_Init(void);
#endif




#endif		//__GPSAPI_H__
