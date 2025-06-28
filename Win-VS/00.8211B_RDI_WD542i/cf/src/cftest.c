#include "general.h"
#include "board.h"
#include "osapi.h"
#include "cfapi.h"
#include "cf.h"
#include "cfreg.h"
#include "cferr.h"
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
#if VERIFYCF

extern u8 cfReadBuf[CF_BLK_LEN * CF_MAX_SECTOR]; 
extern u8 cfWriteBuf[CF_BLK_LEN * CF_MAX_SECTOR]; 
extern HD_INFORMATION cfSHDInformation;
/*

Routine Description:

    Register Reset Test

Arguments:

    None.
¡b
Return Value:

    None

*/

u32 cfTestRegisterReset(void)
{
    bool bTestPass = TRUE;
    u32 unCFCRegResetValue[CF_REGNUM];
    u32 unIndex;
    u32 unRegContent;

    DEBUG_CF("(1) Register Reset Test: \n");

    /* Assign the reset value to the array. */
    for (unIndex = 0; unIndex < CF_REGNUM; unIndex++)
        unCFCRegResetValue[unIndex] = 0x00000000;   

    unCFCRegResetValue[0x14 >> 2] = 0x00000002;
    unCFCRegResetValue[0x20 >> 2] = 0x00000001;
    unCFCRegResetValue[0x24 >> 2] = 0x00245024;
    unCFCRegResetValue[0x28 >> 2] = 0x00245024;
    unCFCRegResetValue[0x2C >> 2] = 0x00245024;
    unCFCRegResetValue[0x34 >> 2] = 0xffffffff;
    unCFCRegResetValue[0x38 >> 2] = 0x0000000b;
    unCFCRegResetValue[0x3c >> 2] = 0x00000037;    
    unCFCRegResetValue[0x40 >> 2] = 0x00000001;        
    unCFCRegResetValue[0x48 >> 2] = 0x00000001;            
    unCFCRegResetValue[0x4c >> 2] = 0x00000400;               
    unCFCRegResetValue[0x58 >> 2] = 0x00245024;
    unCFCRegResetValue[0x5C >> 2] = 0x00245024;
    unCFCRegResetValue[0x60 >> 2] = 0x00245024;

    /* Read the values of all CF Controller registers
       and compare them with the Reset Values of all registers. */
    for (unIndex = 0; unIndex < CF_REGNUM; unIndex++)
    {
        u32 unOffset;

        unOffset = unIndex << 2;
#if 0

        if (((unOffset > 0x10) & (unOffset < 0x20)) | (unOffset == 0x08))
            continue;

#endif

        unRegContent = INPW(IdeCtrlBase + unOffset);

        if (unRegContent != unCFCRegResetValue[unIndex])
        {
            DEBUG_CF("    RegunOffset 0x%03x: The read register content(0x%08x) != the default value(0x%08x)\n",
                   unOffset,
                   unRegContent,
                   unCFCRegResetValue[unIndex]);
            bTestPass = FALSE;
        }
    }

    if (bTestPass)
    {
        DEBUG_CF("\n Test Result => Pass!\n");
        return TRUE;
    }
    else
    {
        DEBUG_CF("\n Test Result => False!\n");
        return FALSE;
    }
}

/*

Routine Description:

    Read back and check the content of each register after wrote them

Arguments:

    None.

Return Value:

    None

*/
static s32 checkRegs(u32 unRegOffset, u32 unCheckBitsofReg, u32 unExpectedValue)
{
    u32 unRegContent;

    unRegContent = INPW(IdeCtrlBase + unRegOffset);

    if ((unRegContent &= unCheckBitsofReg) != unExpectedValue)
    {
        DEBUG_CF("    unRegOffset 0x%03X: The read register content(0x%08X) != the expected value(0x%08X)\n",
               unRegOffset,
               unRegContent,
               unExpectedValue);
        return FALSE;
    }

    return CF_OK;
}

/*

Routine Description:

    Register Read/Write Test

Arguments:

    None.

Return Value:

    None

*/
u32 cfTestRWTest(void)
{
    u32 unJ;                        /* offset */
    u32 testPass = TRUE;
    u32 unCheckBitsofReg[CF_REGNUM];
    u32 uReadOnlyReg[CF_REGNUM]; /* This isn't required. */

    DEBUG_CF("(2) Register Read/Write Test: \n");

    memset((void *)IdeCtrlBase, 0x0, sizeof(u32) * CF_REGNUM);

    /* Set the check bits of each register. */
#if 0

    for (i = 0; i < CF_REGNUM; i++)
        unCheckBitsofReg[i] = 0x0;

#endif
    unCheckBitsofReg[0x00 >> 2] = 0x0000003F;
    unCheckBitsofReg[0x04 >> 2] = 0x0000003F;
    unCheckBitsofReg[0x08 >> 2] = 0x00000003;
    unCheckBitsofReg[0x0C >> 2] = 0x00000001;
    unCheckBitsofReg[0x10 >> 2] = 0x00000000; /* Don't check this register */
    unCheckBitsofReg[0x14 >> 2] = 0x00000007;
    unCheckBitsofReg[0x18 >> 2] = 0x000007FF;
    unCheckBitsofReg[0x1C >> 2] = 0x000000FF;
    unCheckBitsofReg[0x20 >> 2] = 0x000000FF;
    unCheckBitsofReg[0x24 >> 2] = 0xFFFFFFFF;
    unCheckBitsofReg[0x28 >> 2] = 0xFFFFFFFF;
    unCheckBitsofReg[0x2C >> 2] = 0xFFFFFFFF; 
    unCheckBitsofReg[0x30 >> 2] = 0x000000FF;
    unCheckBitsofReg[0x34 >> 2] = 0xFFFFFFFF;    
    unCheckBitsofReg[0x38 >> 2] = 0x0000000F;        
    unCheckBitsofReg[0x3C >> 2] = 0x0000003F;            
    unCheckBitsofReg[0x40 >> 2] = 0x00000001;                
    unCheckBitsofReg[0x44 >> 2] = 0x000000FF;                    
    unCheckBitsofReg[0x48 >> 2] = 0x00000001;                    
    unCheckBitsofReg[0x4C >> 2] = 0x0000FFFF;                    
    unCheckBitsofReg[0x50 >> 2] = 0x00000001;	
	unCheckBitsofReg[0x54 >> 2] = 0x000000FF;
    unCheckBitsofReg[0x58 >> 2] = 0xFFFFFFFF;
    unCheckBitsofReg[0x5C >> 2] = 0xFFFFFFFF;
    unCheckBitsofReg[0x60 >> 2] = 0xFFFFFFFF; 
    /* Set the reserved or read only bits of each register. 
       1 means this bit is reserved or read only 
       0 means this bit is not reserved or read only */
#if 0
    for (i = 0; i < CF_REGNUM; i++)
        uReadOnlyReg[i] = 0x0;

#endif
    uReadOnlyReg[0x00 >> 2] = 0xFFFFFF00;
    uReadOnlyReg[0x04 >> 2] = 0xFFFFFFFF;
    uReadOnlyReg[0x08 >> 2] = 0xFFFFFFFC;
    uReadOnlyReg[0x0C >> 2] = 0xFFFFFFFE;
    uReadOnlyReg[0x10 >> 2] = 0xFFFFFF02;
    uReadOnlyReg[0x14 >> 2] = 0xFFFFFFF8;
    uReadOnlyReg[0x18 >> 2] = 0xFFFFF800;
    uReadOnlyReg[0x1C >> 2] = 0xFFFFFFFF;
    uReadOnlyReg[0x20 >> 2] = 0xFFFFFF00;
    uReadOnlyReg[0x24 >> 2] = 0x00000000;
    uReadOnlyReg[0x28 >> 2] = 0x00000000;
    uReadOnlyReg[0x2C >> 2] = 0x00000000;
    uReadOnlyReg[0x30 >> 2] = 0xFFFFFFFF;
    uReadOnlyReg[0x34 >> 2] = 0x00000000;
    uReadOnlyReg[0x38 >> 2] = 0xFFFFFFFF;
    uReadOnlyReg[0x3C >> 2] = 0xFFFFFFC0;
    uReadOnlyReg[0x40 >> 2] = 0xFFFFFFFE;
    uReadOnlyReg[0x44 >> 2] = 0xFFFFFF00;
    uReadOnlyReg[0x48 >> 2] = 0xFFFFFFFF;    
    uReadOnlyReg[0x4C >> 2] = 0xFFFF0000;    
    uReadOnlyReg[0x50 >> 2] = 0xFFFFFFFE;
    uReadOnlyReg[0x54 >> 2] = 0xFFFFFF00;
	uReadOnlyReg[0x58 >> 2] = 0x00000000;
    uReadOnlyReg[0x5C >> 2] = 0x00000000;
    uReadOnlyReg[0x60 >> 2] = 0x00000000;

    /*
       Write 1's Test
    */
    DEBUG_CF("    Writing 1's test\n");

    /* Write 1 to all bits of the R/W registers 0x00 ~ 0x4C */
    for (unJ = 0x00; unJ < sizeof(u32) * CF_REGNUM; unJ += 4)
        OUTPW(IdeCtrlBase + unJ, unCheckBitsofReg[unJ >> 2]);

    /* The following bits are write clear */
#if 0

    unCheckBitsofReg[0x00 >> 2] &= ~0x00000001;
    unCheckBitsofReg[0x04 >> 2] &= ~0x00000F00;
    unCheckBitsofReg[0x08 >> 2] &= ~0x00000F00;
    unCheckBitsofReg[0x0C >> 2] &= ~0x00000010;

#endif

    /* Read the values of control registers 0x00 ~ 0x4C
       , and compare them with the expected results 
       of all registers.*/

    for (unJ = 0x00; unJ < sizeof(u32) * CF_REGNUM; unJ += 4)
        if (!checkRegs(unJ, unCheckBitsofReg[unJ >> 2], unCheckBitsofReg[unJ >> 2] & ~uReadOnlyReg[unJ >> 2]))
            testPass = FALSE;

    /*
       Write 0's Test
    */
    DEBUG_CF("    Writing 0's test\n");

    /* The following bits are write clear */
#if 0

    unCheckBitsofReg[0x0C >> 2] |= 0x00000010;

#endif

    /* Write 0 to all bits of the R/W registers 0x00 ~ 0x2C
       except the write one clear bits, 
       and then read back and check if the values equal to the ones we write.*/

    for (unJ = 0x00; unJ < sizeof(u32) * CF_REGNUM; unJ += 4)
        OUTPW(IdeCtrlBase + unJ, 0x0);

    /* Read the values of control registers 0x00 ~ 0x4C,
       and compare them with the expected results of all registers.*/
    for (unJ = 0x00; unJ < sizeof(u32) * CF_REGNUM; unJ += 4)
    {
        if (!checkRegs(unJ, unCheckBitsofReg[unJ >> 2], 0x0))
            testPass = FALSE;
    }

    /* Clear all registers */
    memset((void *)IdeCtrlBase, 0x00, sizeof(u32) * CF_REGNUM);

    if (testPass)
    {
        DEBUG_CF("    Result => Pass!\n");
        return TRUE;
    }
    else
    {
        DEBUG_CF("    Result => False!\n");
        return FALSE;
    }
}

/*

Routine Description:

    Identify Drive Command Test

Arguments:

    None.

Return Value:

    None

*/
u32 cfTestIdentifyDriveCommands(void)
{
    
    u8 ucTempStr[41];

    DEBUG_CF("(6) Identify Drive Command Test: \n");

    DEBUG_CF("Number of logical cylinders = %d\n", cfSHDInformation.cylinders);
    DEBUG_CF("Number of logical heads = %d\n", cfSHDInformation.heads);
    DEBUG_CF("Number of logical sectors per logical track = %d\n", cfSHDInformation.sectors);    
    memcpy(ucTempStr, cfSHDInformation.firware_rev, 8);
    ucTempStr[8] = '\0';
    DEBUG_CF("Firmware revision = %s\n", ucTempStr);        
    memcpy(ucTempStr, cfSHDInformation.model_num, 40);
    ucTempStr[40] = '\0';
    DEBUG_CF("Model number = %s\n", ucTempStr);            
    DEBUG_CF("Maximum number of sectors = %d\n", cfSHDInformation.support_max_sector_no);                

    DEBUG_CF("PIO data transfer mode number = %d\n", cfSHDInformation.pio_mode);                
    DEBUG_CF("PIO data transfer mode number = %d\n", cfSHDInformation.pio_mode);                    

    DEBUG_CF("PIO data transfer mode number = %d\n", cfSHDInformation.pio_mode);                
    DEBUG_CF("Number of current logical cylinders = %d\n", cfSHDInformation.cur_cylinders);                
    DEBUG_CF("Number of current logical heads = %d\n", cfSHDInformation.cur_heads);                
    DEBUG_CF("Number of current logical sectors per track = %d\n", cfSHDInformation.cur_sectors);                
    DEBUG_CF("Total number of user addressable sectors (LBA mode only) = %d\n", *(u32 *)(&(cfSHDInformation.lba_capacity)));                    
    DEBUG_CF("Multiword DMA mode 0-2 = 0x%x\n", cfSHDInformation.multiword_DMA_mode);                        
	
    DEBUG_CF("Advanced PIO modes supported = %d\n", cfSHDInformation.eide_pio_mode);      
    DEBUG_CF("Minimum Multiword DMA transfer cycle time per word = %d\n", cfSHDInformation.min_multiword_dma_tran_cycle_time_per_word);          
    DEBUG_CF("Manufacturer\'s recommended Multiword DMA transfer cycle time = %d\n", cfSHDInformation.recommended_multiword_dma_tran_cycle_time_per_word);              
    DEBUG_CF("Minimum PIO transfer cycle time without flow control = %d\n", cfSHDInformation.min_PIO_tran_cycle_time_without_flow_control);              
    DEBUG_CF("Minimum PIO transfer cycle time with IORDY flow control = %d\n", cfSHDInformation.min_PIO_tran_cycle_time_with_IORDY_flow_control);              
    DEBUG_CF("Queue depth = %d\n", cfSHDInformation.queue_depth);              
    DEBUG_CF("Major version number = %d\n", cfSHDInformation.major_ver_num);              
    DEBUG_CF("Minor version number = %d\n", cfSHDInformation.minor_ver_num);                  
    DEBUG_CF("Command set supported = 0x%x\n", cfSHDInformation.command_set_support);                      
    DEBUG_CF("Command sets supported 2 = 0x%x\n", cfSHDInformation.command_set_support2);                          
//    DEBUG_CF("Command set/feature supported extension = 0x%x\n", cfSHDInformation.command_set_feature_support_ext);      
//    DEBUG_CF("Command set/feature enabled = 0x%x\n", *(u32 *)(&(cfSHDInformation.command_set_feature_ena)));          
    DEBUG_CF("Command set/feature default = 0x%x\n", cfSHDInformation.command_set_feature_default);              
    DEBUG_CF("Ultra DMA mode = 0x%x\n", cfSHDInformation.ultra_DMA_mode);                  
    DEBUG_CF("Time required for security erase unit completion = %d\n", cfSHDInformation.security_erase_time);                      
    DEBUG_CF("Time required for Enhanced security erase completion = %d\n", cfSHDInformation.enhanced_security_erase_time);                          
    DEBUG_CF("Current advanced power management value = %d\n", cfSHDInformation.current_adv_pow_mana_val);                              
    DEBUG_CF("Removable Media Status Notification feature set support = %d\n", cfSHDInformation.removable_media_status_notification_supp);                                  
    DEBUG_CF("Security status = %d\n", cfSHDInformation.security_status);                                      
//    DEBUG_CF("Number of logical cylinders =%d\n", cfSHDInformation.cylinders);    

    DEBUG_CF("    This Test passes if the information are ok!\n");
    return TRUE;
}

/*

Routine Description:

    Write Commands Test

Arguments:

    None.

Return Value:

    None

*/
u32 cfTestWriteCommands(void)
{
    u32 uzIndex;
    u32 uzTotalReadWriteSector = CF_MULTI_BLK;           /* end access sector in the SD card */
    u32 uzStartSector = 100;                     /* starting access sector in the SD card */
    u32 uzMultipleStartSector = 2000;
    u32 testPass = TRUE;

    DEBUG_CF("(7) Write Commands Test: \n");


    DEBUG_CF("    Write Sector Command: \n");

    /* Write specific data pattern P with write sector command */
    cfWriteSectors(uzStartSector, uzTotalReadWriteSector, cfWriteBuf);    

    /* Clear the memory */
    memset((void *)cfReadBuf, 0x00, CF_BLK_LEN * uzTotalReadWriteSector);

    /* Get whole data pattern Ps' with read sector command */
    cfReadSectors(uzStartSector, uzTotalReadWriteSector, cfReadBuf);

    /* Compare P and Ps' */
    for (uzIndex = 0; uzIndex < uzTotalReadWriteSector * CF_BLK_LEN; uzIndex++)
    {
        if (cfWriteBuf[uzIndex] != cfReadBuf[uzIndex])
        {
            DEBUG_CF("    cfWriteBuf[%d]: 0x%x != cfReadBuf[%d]: 0x%x\n", uzIndex, cfWriteBuf[uzIndex],
                   uzIndex, cfReadBuf[uzIndex]);
            testPass = FALSE;
			break;
        }
    }


    if (testPass)
    {
        DEBUG_CF("    Result => Pass!\n");
        return TRUE;
    }
    else
    {
        DEBUG_CF("    Result => False!\n");
        return FALSE;
    }
}

/*

Routine Description:

    Read Commands Test

Arguments:

    None.

Return Value:

    None

*/
u32 cfTestReadCommands(void)
{
    u32 uzIndex;
    u32 uzTotalReadWriteSector = CF_MULTI_BLK;           /* end access sector in the SD card */
    u32 uzStartSector = 100;                     /* starting access sector in the SD card */
    u32 uzMultipleStartSector = 2000;
    u32 testPass = TRUE;

    DEBUG_CF("(8) Read Commands Test: \n");


    DEBUG_CF("    Read Sector Command: \n");

    /* Write specific data pattern P with write sector command */
    cfWriteSectors(uzStartSector, uzTotalReadWriteSector, cfWriteBuf);    

    /* Clear the memory */
    memset((void *)cfReadBuf, 0x00, CF_BLK_LEN * uzTotalReadWriteSector);

    /* Get whole data pattern Ps' with read sector command */
    cfReadSectors(uzStartSector, uzTotalReadWriteSector, cfReadBuf);

    /* Compare P and Ps' */
    for (uzIndex = 0; uzIndex < uzTotalReadWriteSector * CF_BLK_LEN; uzIndex++)
    {
        if (cfWriteBuf[uzIndex] != cfReadBuf[uzIndex])
        {
            DEBUG_CF("    cfWriteBuf[%d]: 0x%x != cfReadBuf[%d]: 0x%x\n", uzIndex, cfWriteBuf[uzIndex],
                   uzIndex, cfReadBuf[uzIndex]);
            testPass = FALSE;
			break;
        }
    }
    if (testPass)
    {
        DEBUG_CF("    Result => Pass!\n");
        return TRUE;
    }
    else
    {
        DEBUG_CF("    Result => False!\n");
        return FALSE;
    }
}

/*

Routine Description:

    Erase Commands Test
    Erase command is for CF card only.

Arguments:

    None.

Return Value:

    None

*/
u32 cfTestEraseCommands(void)
{
    u32 uzIndex;
    u32 uzTotalReadWriteSector = CF_MULTI_BLK;           /* end access sector in the SD card */
    u32 uzStartSector = 100;                     /* starting access sector in the SD card */
    u32 testPass = TRUE;

    DEBUG_CF("(9) Erase Command Test: \n");

    /* Write specific data pattern P with write sector command */
    cfWriteSectors(uzStartSector, uzTotalReadWriteSector, cfWriteBuf);    

    /* Erase sectors in the ATA device */
    cfEraseSectors(uzStartSector, uzTotalReadWriteSector);  

    /* Clear the memory */
    memset((void *)cfReadBuf, 0x00, CF_BLK_LEN * uzTotalReadWriteSector);

    /* Get whole data pattern Ps' with read sector command */
    cfReadSectors(uzStartSector, uzTotalReadWriteSector, cfReadBuf);

    /* Compare P and Ps' */
    for (uzIndex = 0; uzIndex < uzTotalReadWriteSector * CF_BLK_LEN; uzIndex++)
    {
        if (cfReadBuf[0] != cfReadBuf[uzIndex])
        {
            DEBUG_CF("    %d != cfReadBuf[%d]: %2x\n", cfReadBuf[0], uzIndex, cfReadBuf[uzIndex]);
            testPass = FALSE;
        }
    }
    
    if (testPass)
    {
        DEBUG_CF("    Result => Pass!\n");
        return TRUE;
    }
    else
    {
        DEBUG_CF("    Result => False!\n");
        return FALSE;
    }
}

/*

Routine Description:

    Write DMA Commands Test

Arguments:

    None.

Return Value:

    None

*/
u32 cfTestWriteDMACommands(void)
{
    u32 uzIndex;
    u32 uzTotalReadWriteSector = CF_MULTI_BLK;           /* end access sector in the SD card */
    u32 uzMultipleStartSector = 5000;
    u32 testPass = TRUE;

    DEBUG_CF("(10) Write Commands Test in DMA mode: \n");

#if 1
    /* Set transfer mode */
    cfSetTransferMode(DMA_MODE, MODE_0);

    /* Write specific data pattern P with write DMA command */
    cfWriteMultipleSectorsDMA(uzMultipleStartSector, (u8)uzTotalReadWriteSector, cfWriteBuf);    

    /* Clear the memory */
    memset((void *)cfReadBuf, 0x00, CF_BLK_LEN * uzTotalReadWriteSector);

    /* Set transfer mode */
    cfSetTransferMode(PIO_MODE, MODE_0);

    /* Get whole data pattern Ps' with read sector command */
    cfReadSectors(uzMultipleStartSector, uzTotalReadWriteSector, cfReadBuf);

    /* Compare P and Ps' */
    for (uzIndex = 0; uzIndex < uzTotalReadWriteSector * CF_BLK_LEN; uzIndex++)
    {
        if (cfWriteBuf[uzIndex] != cfReadBuf[uzIndex])
        {
            DEBUG_CF("    cfWriteBuf[%d]: %d != cfReadBuf[%d]: %d\n", uzIndex, cfWriteBuf[uzIndex],
                   uzIndex, cfReadBuf[uzIndex]);
            testPass = FALSE;
			break;
        }
    }
#endif

    if (testPass)
    {
        DEBUG_CF("    Result => Pass!\n");
        return TRUE;
    }
    else
    {
        DEBUG_CF("    Result => False!\n");
        return FALSE;
    }
}

/*

Routine Description:

    Read DMA Commands Test

Arguments:

    None.

Return Value:

    None

*/
u32 cfTestReadDMACommands(void)
{
    u32 uzIndex;
    u32 uzTotalReadWriteSector = CF_MULTI_BLK;           /* end access sector in the SD card */
    u32 uzMultipleStartSector = 2000;
    u32 testPass = TRUE;

    DEBUG_CF("(11) Read Commands Test in DMA mode: \n");

#if 1
    /* Write specific data pattern P with write sector command */
    cfWriteSectors(uzMultipleStartSector, uzTotalReadWriteSector, cfWriteBuf);    

    /* Clear the memory */
    memset((void *)cfReadBuf, 0x00, CF_BLK_LEN * uzTotalReadWriteSector);

    /* Set transfer mode */
    cfSetTransferMode(DMA_MODE, MODE_0);

    /* Get whole data pattern Ps' with read sector command */
    cfReadMultipleSectorsDMA(uzMultipleStartSector, uzTotalReadWriteSector, cfReadBuf);

    /* Compare P and Ps' */
    for (uzIndex = 0; uzIndex < uzTotalReadWriteSector * CF_BLK_LEN; uzIndex++)
    {
        if (cfWriteBuf[uzIndex] != cfReadBuf[uzIndex])
        {
            DEBUG_CF("    cfWriteBuf[%d]: %d != cfReadBuf[%d]: %d\n", uzIndex, cfWriteBuf[uzIndex],
                   uzIndex, cfReadBuf[uzIndex]);
            testPass = FALSE;
			break;
        }
    }
#endif

    if (testPass)
    {
        DEBUG_CF("    Result => Pass!\n");
        return TRUE;
    }
    else
    {
        DEBUG_CF("    Result => False!\n");
        return FALSE;
    }
}

/*

Routine Description:

    Read/Write Large Disk Size Test

Arguments:

    None.

Return Value:

    None

*/
u32 cfTestReadWriteLargeDiskSize(void)
{
    u8  ucCharPattern = 0x0;
    u32 uzIndex;
//    u32 uzTotalReadWriteCycle = 2000;           
    u32 uzTotalReadWriteCycle = 100;           
    u32 uzSectorStartSector = 21000;
    u32 unSectorEndSector = uzSectorStartSector + uzTotalReadWriteCycle * CF_MULTI_BLK - 1;
    u32 uzMultipleStartSector = unSectorEndSector;
    u32 unMultipleEndSector = uzMultipleStartSector + uzTotalReadWriteCycle * CF_MULTI_BLK - 1;
    u32 unDMAStartSector = unMultipleEndSector + 1;
    u32 unDMAEndSector = unDMAStartSector + uzTotalReadWriteCycle * CF_MULTI_BLK - 1;
    u32 unCurrentSector;
    u32 testPass = TRUE;
 
    DEBUG_CF("(9)(12) Read/Write Large Disk Size Test: \n");

    /* For debug */
//    cfReadMultipleSectorsDMA(41199, CF_MULTI_BLK, cfReadBuf);


#if 1
    DEBUG_CF("    Read/Write Sector: \n");

    do
    {
        /* Generate Test Pattern */
        for (uzIndex = 0; uzIndex < CF_BLK_LEN * CF_MULTI_BLK; uzIndex++)
            cfWriteBuf[uzIndex] = ucCharPattern;

        /* Write specific data pattern P with write sector command */
		 DEBUG_CF("  Write Sector: \n");
        for (unCurrentSector = uzSectorStartSector; unCurrentSector <= unSectorEndSector; unCurrentSector += CF_MULTI_BLK)
            cfWriteSectors(unCurrentSector, CF_MULTI_BLK, cfWriteBuf);    
 		DEBUG_CF("    Read Sector: \n");
        /* Get whole data pattern Ps' with read sector command */
        for (unCurrentSector = uzSectorStartSector; unCurrentSector <= unSectorEndSector; unCurrentSector += CF_MULTI_BLK)
        {
            /* Clear the memory */
            memset((void *)cfReadBuf, 0xCC, CF_BLK_LEN * CF_MULTI_BLK);
        
            cfReadSectors(unCurrentSector, CF_MULTI_BLK, cfReadBuf);

            /* Compare P and Ps' */
            for (uzIndex = 0; uzIndex < CF_MULTI_BLK * CF_BLK_LEN; uzIndex++)
            {
                if (cfWriteBuf[uzIndex] != cfReadBuf[uzIndex])
                {
                    DEBUG_CF("unCurrentSector[%d] cfWriteBuf[%d]: %x != cfReadBuf[%d]: %x\n", unCurrentSector, 
                           uzIndex, cfWriteBuf[uzIndex], uzIndex, cfReadBuf[uzIndex]);
                    testPass = FALSE;
                }
            }
        }
             
        ucCharPattern += 0x55;
    } while (ucCharPattern != 0x54);
#endif

#if 0
    DEBUG_CF("    Read/Write Multiple: \n");

    ucCharPattern = 0x0;
    do
    {
#if 0    
        /* Generate Test Pattern */
        for (uzIndex = 0; uzIndex < CF_BLK_LEN * CF_MULTI_BLK; uzIndex++)
            cfWriteBuf[uzIndex] = ucCharPattern;
#endif

        /* Write specific data pattern P with write sector command */
        for (unCurrentSector = uzMultipleStartSector; unCurrentSector <= unMultipleEndSector; 
             unCurrentSector += CF_MULTI_BLK)
            cfWriteMultipleSectors(unCurrentSector, CF_MULTI_BLK, cfWriteBuf);    

        /* Get whole data pattern Ps' with read sector command */
        for (unCurrentSector = uzMultipleStartSector; unCurrentSector <= unMultipleEndSector; 
             unCurrentSector += CF_MULTI_BLK)
        {
            /* Clear the memory */
            memset((void *)cfReadBuf, 0xCC, CF_BLK_LEN * CF_MULTI_BLK);
        
            cfReadMultipleSectors(unCurrentSector, CF_MULTI_BLK, cfReadBuf);

            /* Compare P and Ps' */
            for (uzIndex = 0; uzIndex < CF_MULTI_BLK * CF_BLK_LEN; uzIndex++)
            {
                if (cfWriteBuf[uzIndex] != cfReadBuf[uzIndex])
                {
                    DEBUG_CF("unCurrentSector[%d] cfWriteBuf[%d]: %x != cfReadBuf[%d]: %x\n", unCurrentSector, 
                           uzIndex, cfWriteBuf[uzIndex], uzIndex, cfReadBuf[uzIndex]);
                    testPass = FALSE;
                }
            }
        }
             
        ucCharPattern += 0x55;
    } while (ucCharPattern != 0x54);
#endif

#if 1
    DEBUG_CF("    Read/Write DMA: \n");

    /* Set transfer mode */
    cfSetTransferMode(DMA_MODE, MODE_0);

    ucCharPattern = 0x00;
    do
    {
#if 1    
        /* Generate Test Pattern */
        for (uzIndex = 0; uzIndex < CF_BLK_LEN * CF_MULTI_BLK; uzIndex++)
            cfWriteBuf[uzIndex] = ucCharPattern;
#endif        

        /* For debug */
         DEBUG_CF("        Write DMA\n");

        /* Write specific data pattern P with write DMA command */
        for (unCurrentSector = unDMAStartSector; unCurrentSector <= unDMAEndSector; 
             unCurrentSector += CF_MULTI_BLK)
            cfWriteMultipleSectorsDMA(unCurrentSector, CF_MULTI_BLK, cfWriteBuf);    
//            cfWriteMultipleSectors(unCurrentSector, CF_MULTI_BLK, cfWriteBuf);    

        /* For debug */
         DEBUG_CF("        Read DMA\n");
        
        /* Get whole data pattern Ps' with read DMA command */
        for (unCurrentSector = unDMAStartSector; unCurrentSector <= unDMAEndSector; 
             unCurrentSector += CF_MULTI_BLK)
        {
            /* Clear the memory */
            memset((void *)cfReadBuf, 0xCC, CF_BLK_LEN * CF_MULTI_BLK);
        
            cfReadMultipleSectorsDMA(unCurrentSector, CF_MULTI_BLK, cfReadBuf);
//            cfReadMultipleSectors(unCurrentSector, CF_MULTI_BLK, cfReadBuf);

            /* Compare P and Ps' */
            for (uzIndex = 0; uzIndex < CF_MULTI_BLK * CF_BLK_LEN; uzIndex++)
            {
                if (cfWriteBuf[uzIndex] != cfReadBuf[uzIndex])
                {
                    DEBUG_CF("unCurrentSector[%d] cfWriteBuf[%d]: 0x%x != cfReadBuf[%d]: 0x%x\n", unCurrentSector, 
                           uzIndex, cfWriteBuf[uzIndex], uzIndex, cfReadBuf[uzIndex]);
                    testPass = FALSE;
                }
            }
        }

        ucCharPattern += 0x55;
    } while (ucCharPattern != 0x54);
         
#endif

    if (testPass)
    {
        DEBUG_CF("    Result => Pass!\n");
        return TRUE;
    }
    else
    {
        DEBUG_CF("    Result => False!\n");
        return FALSE;
    }
}

/*

Routine Description:

    Read/Write Sector Test with different sector count (1~255)

Arguments:

    None.

Return Value:

    None

*/
u32 cfTestReadWriteWithDiffSectorCount(void)
{
    u32 unSectorCount;
    u32 uzIndex;
    u32 uzReadWriteCycleCurrent;     
    u32 uzTotalReadWriteCycle = 1000;           
    u32 uzSectorStartSector = 12000;
//    u32 uzMultipleStartSector = uzSectorStartSector + uzTotalReadWriteCycle * CF_MAX_SECTOR;
#if 0    

    u32 unMultipleEndSector = uzMultipleStartSector + uzTotalReadWriteCycle * CF_MULTI_BLK - 1;
    u32 unDMAStartSector = unMultipleEndSector + 1;
    u32 unDMAEndSector = unDMAStartSector + uzTotalReadWriteCycle * CF_MULTI_BLK - 1;
#endif    
    u32 unCurrentSector;
    u32 testPass = TRUE;
 
    DEBUG_CF(" (13) Read/Write Sector Test with different sector count (1~Max Sector Number): \n");

#if 1
    DEBUG_CF("    Read/Write Sector: \n");

    for (unSectorCount = 1; unSectorCount <= cfSHDInformation.support_max_sector_no; unSectorCount++)
    {
        DEBUG_CF("        Sector Count = %d\n", unSectorCount);

#if 1    
        /* Write specific data pattern P with write sector command */
        for (unCurrentSector = uzSectorStartSector, uzReadWriteCycleCurrent = 0; 
             uzReadWriteCycleCurrent < uzTotalReadWriteCycle; 
             unCurrentSector += unSectorCount, uzReadWriteCycleCurrent++)
            cfWriteSectors(unCurrentSector, unSectorCount, cfWriteBuf);    
//            cfWriteMultipleSectors(unCurrentSector, unSectorCount, cfWriteBuf);    
//            cfWriteMultipleSectorsDMA(unCurrentSector, unSectorCount, cfWriteBuf);    

#endif

        /* Get whole data pattern Ps' with read sector command */
        for (unCurrentSector = uzSectorStartSector, uzReadWriteCycleCurrent = 0; 
             uzReadWriteCycleCurrent < uzTotalReadWriteCycle; 
             unCurrentSector += unSectorCount, uzReadWriteCycleCurrent++)
        {
            /* Clear the memory */
            memset((void *)cfReadBuf, 0x00, unSectorCount * CF_BLK_LEN);
        
//            cfReadSectors(unCurrentSector, unSectorCount, cfReadBuf);
//            cfReadMultipleSectors(unCurrentSector, unSectorCount, cfReadBuf);
            cfReadMultipleSectorsDMA(unCurrentSector, unSectorCount, cfReadBuf);

            /* Compare P and Ps' */
            for (uzIndex = 0; uzIndex < unSectorCount * CF_BLK_LEN; uzIndex++)
            {
                if (cfWriteBuf[uzIndex] != cfReadBuf[uzIndex])
                {
                    DEBUG_CF("unCurrentSector[%d] cfWriteBuf[%d]: %x != cfReadBuf[%d]: %x\n", unCurrentSector, 
                           uzIndex, cfWriteBuf[uzIndex], uzIndex, cfReadBuf[uzIndex]);
                    testPass = FALSE;
                }
            }
        }
    }             
#endif

#if 0
    /*  */
    DEBUG_CF("    Read/Write Multiple: \n");

    ucCharPattern = 0x0;
    do
    {
        /* Generate Test Pattern */
        for (uzIndex = 0; uzIndex < CF_BLK_LEN * CF_MULTI_BLK; uzIndex++)
            cfWriteBuf[uzIndex] = ucCharPattern;

        /* Write specific data pattern P with write sector command */
        for (unCurrentSector = uzMultipleStartSector; unCurrentSector <= unMultipleEndSector; 
             unCurrentSector += CF_MULTI_BLK)
            cfWriteMultipleSectors(unCurrentSector, CF_MULTI_BLK, cfWriteBuf);    

        /* Get whole data pattern Ps' with read sector command */
        for (unCurrentSector = uzMultipleStartSector; unCurrentSector <= unMultipleEndSector; 
             unCurrentSector += CF_MULTI_BLK)
        {
            /* Clear the memory */
            memset((void *)cfReadBuf, 0xCC, CF_BLK_LEN * CF_MULTI_BLK);
        
            cfReadMultipleSectors(unCurrentSector, CF_MULTI_BLK, cfReadBuf);

            /* Compare P and Ps' */
            for (uzIndex = 0; uzIndex < CF_MULTI_BLK * CF_BLK_LEN; uzIndex++)
            {
                if (cfWriteBuf[uzIndex] != cfReadBuf[uzIndex])
                {
                    DEBUG_CF("unCurrentSector[%d] cfWriteBuf[%d]: %x != cfReadBuf[%d]: %x\n", unCurrentSector, 
                           uzIndex, cfWriteBuf[uzIndex], uzIndex, cfReadBuf[uzIndex]);
                    testPass = FALSE;
                }
            }
        }
             
        ucCharPattern += 0x55;
    } while (ucCharPattern != 0x54);
#endif

    if (testPass)
    {
        DEBUG_CF("    Result => Pass!\n");
        return TRUE;
    }
    else
    {
        DEBUG_CF("    Result => False!\n");
        return FALSE;
    }
}

/*

Routine Description:

    CF/ATA Write Performance Evaluation

Arguments:

    None.

Return Value:

    None

*/
u32 cfTestEvaluateWritePerformance(void)
{
    u32 unTimerCount;
    u32 unTimerCountDif;    
    u32 unTotalReadWriteCycle = 1000;      
    u32 unSectorStartSector = 20000;
    u32 unSectorEndSector = unSectorStartSector + unTotalReadWriteCycle * CF_MULTI_BLK - 1;
    u32 unCurrentSector;    

    DEBUG_CF(" (51) Write Performance Evaluation\n");        

    /* Set transfer mode */
    cfSetTransferMode(PIO_MODE, MODE_0);

    unTimerCount = OSTimeGet();

    /* Write specific data pattern P with write sector command */
    for (unCurrentSector = unSectorStartSector; unCurrentSector <= unSectorEndSector; 
         unCurrentSector += CF_MULTI_BLK)
        cfWriteSectors(unCurrentSector, CF_MULTI_BLK, cfWriteBuf);    

    unTimerCountDif = OSTimeGet() - unTimerCount;
    DEBUG_CF("Write Sector: Spend %d timer counts to write %d sectors of data\n", unTimerCountDif, 
           unTotalReadWriteCycle * CF_MULTI_BLK);


    unTimerCount = OSTimeGet();
#if 0
    /* Write specific data pattern P with write sector command */
    for (unCurrentSector = unSectorStartSector; unCurrentSector <= unSectorEndSector; 
         unCurrentSector += CF_MULTI_BLK)
        cfWriteMultipleSectors(unCurrentSector, CF_MULTI_BLK, cfWriteBuf);    

    unTimerCountDif = OSTimeGet() - unTimerCount;
    DEBUG_CF("Write Multiple: Spend %d timer counts to write %d sectors of data\n", unTimerCountDif, 
           unTotalReadWriteCycle * CF_MULTI_BLK);
#endif

    /* Set transfer mode */
    cfSetTransferMode(DMA_MODE, MODE_0);

    unTimerCount = OSTimeGet();

    /* Write specific data pattern P with write sector command */
    for (unCurrentSector = unSectorStartSector; unCurrentSector <= unSectorEndSector; 
         unCurrentSector += CF_MULTI_BLK)
        cfWriteMultipleSectorsDMA(unCurrentSector, CF_MULTI_BLK, cfWriteBuf);    

    unTimerCountDif = OSTimeGet() - unTimerCount;
    DEBUG_CF("Write DMA: Spend %d timer counts to write %d sectors of data\n", unTimerCountDif, 
           unTotalReadWriteCycle * CF_MULTI_BLK);

    return TRUE;
}

/*

Routine Description:

    The test routine of CF/ATA.

Arguments:

    None.

Return Value:

    None

*/
s8 cfTest(void)
{
    u32 i, j;
    u32 uzItemNo = 0;
    static u32 batchItemsIndex = 0,
               batchItems[] = {6, 7, 8, 10, 11, 12, 13, 99},
               batchItemsNo;
#if 1
    u8 *writePtr = cfWriteBuf;
#else
    u32 *writePtr = (u32 *)sdcWriteBuf;
#endif

    /* Set write data */
#if 0
    /* CF_MAX_SECTOR x 2 x 256 bytes */ 
    for (j = 0; j < CF_MAX_SECTOR * 2; j++)
        for (i = 0; i < 256; i++)
            *writePtr++ = i;
#else
    /* CF_MAX_SECTOR x 2 x 256 bytes */ 
    for (j = 0; j < CF_MAX_SECTOR * 2; j++)
        for (i = 0; i < 256; i++)
            *writePtr++ = 255 - i;
#endif
#if 0
  do
    {
        DEBUG_CF("---------Test Programs for PA9001 CF Controller---------\n");
        DEBUG_CF(" (1) Register Reset Test\n");
        DEBUG_CF(" (2) Register Read/Write Test\n");
        DEBUG_CF(" (5) Interrupt Test\n");
        DEBUG_CF(" (6) Identify Drive Command Test\n");
        DEBUG_CF(" (7) Write Commands Test in PIO mode\n");
        DEBUG_CF(" (8) Read Commands Test in PIO mode\n");
        DEBUG_CF(" (9) Read/Write Mass Data Test in PIO mode\n");
        DEBUG_CF(" (10) Write Commands Test in DMA mode\n");
        DEBUG_CF(" (11) Read Commands Test in DMA mode\n");
        DEBUG_CF(" (12) Read/Write Mass Data Test in DMA mode\n");
        DEBUG_CF(" (13) Read/Write Sector Test with different sector count (1~Max Sector Number): \n");
//        DEBUG_CF(" (14) Erase Command Test\n");
 
        DEBUG_CF(" (51) Write Performance Evaluation\n");        
 //       DEBUG_CF(" (98) Batch Test\n");
        DEBUG_CF(" (99) Exit\n");
        DEBUG_CF("-------------------------------------------\n");
        DEBUG_CF(" Please select a item to verify:");

        scanf("%d", &uzItemNo);
        DEBUG_CF("\n");

        if (uzItemNo == 98)
            batchItemsNo = sizeof(batchItems) / sizeof(u32);
        else
            batchItemsNo = 0;            
        do
        {
            if (batchItemsNo)
                uzItemNo = batchItems[batchItemsIndex++];

            if (uzItemNo > 2)
            {
                /* Initialize CF/ATA Device */
                cfInit();

                if (cfMount() != 1)
                {
                    /* Error */
                    DEBUG_CF("Error: Compact Flash Card identification error.\n");
                }
            }

            switch (uzItemNo)
            {
            case  1:
                cfTestRegisterReset();
                break;
            case 2:
                cfTestRWTest();
                break;
            case 6: 
                cfTestIdentifyDriveCommands();
                break;
            case 7: 
                cfTestWriteCommands();
                break;
            case 8:
                cfTestReadCommands();
                break;
            case 10: 
                cfTestWriteDMACommands();
                break;
            case 11: 
                cfTestReadDMACommands();
                break;
            case 12: 
                cfTestReadWriteLargeDiskSize();
                break;
            case 13: 
                cfTestReadWriteWithDiffSectorCount();
                break;
            case 14:
                // cfTestEraseCommands();
                break;
             case 51:
                cfTestEvaluateWritePerformance();
                break;
            case 98:
                // batchItemsNo = readBatchItemsSequence(batchItems);
                break;
            case 99:
                DEBUG_CF("\nBye bye!\n");
                break;
            default:
                DEBUG_CF("\nNon-defined or inexistent test item!!!\n");
            }
        } while (batchItemsIndex < batchItemsNo);

        if (uzItemNo != 99)
            DEBUG_CF("\nPress \'Enter\' to continue...\n\n");
        getchar();
        getchar();
    } while (uzItemNo != 99);
#else
    DEBUG_CF("***************************\n");
	DEBUG_CF("*                         *\n");
	DEBUG_CF("*    CF IDE Test          *\n");
	DEBUG_CF("*                         *\n");
    DEBUG_CF("***************************\n\n");
//                cfTestRegisterReset();

//                cfTestRWTest();

		if(cfInit()==0) return 0;

		cfTestIdentifyDriveCommands();
			   
                cfTestWriteCommands();

                cfTestWriteDMACommands();

                cfTestReadDMACommands();
#if 0
		cfTestReadWriteLargeDiskSize();

                cfTestEvaluateWritePerformance();
#endif			
		  return 1;   
				
#endif			   
    DEBUG_CF("\n--------- Finish testing ---------\n");
}
#endif
#endif 

