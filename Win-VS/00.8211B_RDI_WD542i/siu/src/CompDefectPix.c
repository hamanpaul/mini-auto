/*
Module Name:

	CompDefectPix.c

Abstract:

	1. Capture sensor raw image and check the defect pixels
	2. sorting the defect pixel to defect pixel's table
	3. It also can get the OB value

	SDV_1 	// capture and avi mode use the identical defect compenation table.
	
		1.	-0x00050012-	[0] X-12, Y-5
		2.	------------	[1]
		3.	------------	[2]
		4.	------------	[3]
		5.	------------	[4]
		6.	------------	[5]
		7.	------------	[6]
		8.	-0xffffffff-		[7] final coordinate for HW to read
		9.	-0x00000000-	[8] OB value 
		10.	-0x12345678-	[9] confirm word

	SDV_2	// capture and avi(3 zoom) defect compensation tables differ from each other.
			// capture - 5 / avi_1 - 5 / avi_2 - 5 / avi_3 - 5
			
		1.	------------	[0] cap/START.
		2.	------------	[1]
		3.	-0xffffffff-		[2] cap/END. final coordinate for HW to read (cap mode)
		4.	-0x00000000-	[3] OB value
		5.	-0x12345678-	[4] confirm word
		6.	------------	[5] avi1/START.
		7.	------------	[6]
		8.	------------	[7]
		9.	------------	[8]
		10.	-0xffffffff-		[9] avi1/END. final coordinate for HW to read (avi1 mode)
		11.	------------	[10] avi2/START.
		12.	------------	[11]
		13.	------------	[12]
		14.	------------	[13]
		15.	-0xffffffff-		[14] avi2/END. final coordinate for HW to read (avi2 mode)
		16.	------------	[15] avi3/START.
		17.	------------	[16]
		18.	------------	[17] 
		19.	------------	[18]  
		20.	-0xffffffff-		[19] avi3/END. final coordinate for HW to read (avi3 mode)
	
Revision History:
	
	2008/08/28	Achi		Create	
*/


#include <stdio.h>
#include "general.h"
#include "siuapi.h"
#include "siu.h"
#include "smcapi.h"
#include "siuapi.h"

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
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
 
extern unsigned char smcWriteCache[SMC_MAX_PAGE_PER_BLOCK][SMC_MAX_PAGE_SIZE];
extern s32 smcSectorsWrite(u32, u32, u8*);
extern s32 smcSectorsRead(u32, u32, u8*);
extern u8 ucNANDInit;
#endif






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







