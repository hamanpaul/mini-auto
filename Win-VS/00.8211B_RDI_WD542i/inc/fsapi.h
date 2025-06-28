/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	uartapi.h

Abstract:

   	The application interface of the file system.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __FS_API_H__
#define __FS_API_H__

#include	".\fs\fs_api.h"
#include 	".\fs\fs_dev.h" 
#include 	".\fs\fs_lbl.h" 
#include ".\dcf\dcf.h"
#include 	"sysopt.h" 

// extern Valuable
extern OS_EVENT *FSSecIncSemEvt;


extern void fsTest(void);
extern unsigned int  OSTimeGet (void);

//FS error define
#define BPB_SETTING_ERROR  0xFFFF0110
#define FAT_SETTING_ERROR  0xFFFF0220
#define READ_SECTOR_ERROR 0xFFFF0330
#define BUFFER_ALLOC_ERROR 0xFFFF0440
#define GET_STATUS_ERROR 0xFFFF0550

#define FS_MEMORY_ALLOC_ERROR	0xFFFFFFEF	// -17
#define FS_FILE_ENT_FIND_ERROR	0xFFFFFFEE	// -18
#define FS_REAL_SEC_FIND_ERROR	0xFFFFFFED	// -19
#define FS_REAL_SEC_READ_ERROR	0xFFFFFFEC	// -20
#define FS_REAL_SEC_WRTIE_ERROR	0xFFFFFFEB	// -21

#define FS_FUNC_PTR_EXIST_ERROR	0xFFFFFFDF	// -33
#define FS_DEVICE_FIND_ERROR	0xFFFFFFDE	// -34
#define FS_FILE_PRT_EXIST_ERROR	0xFFFFFFDD	// -35
#define FS_DIR_FILE_FIND_ERROR	0xFFFFFFDC	// -36
#define FS_DIR_FILE_ENT_RO_ERROR	0xFFFFFFDB	// -37
#define FS_FILE_NOT_EXIST_ERROR	0xFFFFFFDA	// -38
#define FS_FILE_CREATE_ERROR	0xFFFFFFD9	// -39
#define FS_FAT_SEC_SIZE_ERROR	0xFFFFFFD8	// -40
#define FS_FAT_CLUS_SIZE_ERROR	0xFFFFFFD7	// -41
#define FS_DEVICE_ACCESS_ERROR	0xFFFFFFD6	// -42
#define FS_FAT_READ_LEN_ERROR	0xFFFFFFD5	// -43
#define FS_FAT_FIND_CLUS_ERROR	0xFFFFFFD4	// -44
#define FS_FAT_FIND_EOF_CLUS_ERROR	0xFFFFFFD3	// -45
#define FS_FAT_FIND_NEW_CLUS_ERROR	0xFFFFFFD2	// -46
#define FS_FAT_FILE_UPDATE_ERROR	0xFFFFFFD1	// -47
#define FS_FILE_BUF_EXIST_ERROR	0xFFFFFFCF	// -48
#define FS_DEVICE_ERASE_ERROR	0xFFFFFFCE	// -49
#define FS_DEVICE_READY_ERROR	0xFFFFFFCD	// -50
#define FS_DEVICE_FORMAT_ERROR	0xFFFFFFCC	// -51
#define FS_FAT_GET_SIZE_ERROR	0xFFFFFFCB	// -52
#define FS_FAT_SEC_ACCESS_ERROR	0xFFFFFFCA	// -53
#define FS_DIR_PRT_EXIST_ERROR	0xFFFFFFBF	// -54
#define FS_DIR_CLOSE_ERROR	0xFFFFFFBE	// -55
#define FS_FILE_NAME_LEN_ERROR	0xFFFFFFBD	// -56
#define FS_DIR_CREATE_ERROR	0xFFFFFFBC	// -57
#define FS_FAT_MKDIR_OP_ERROR	0xFFFFFFBB	// -58
#define FS_FAT_DIR_DELETE_ERROR	0xFFFFFFBA	// -59

#define FS_FILE_NAME_REPEAT_ERR	0xFFFFFF98


enum
{
	FS_E_DELETE_TYPE_AUTO = 0,
	FS_E_DELETE_TYPE_ORDER,
	FS_E_DELETE_TYPE_DIR,
};



#endif
