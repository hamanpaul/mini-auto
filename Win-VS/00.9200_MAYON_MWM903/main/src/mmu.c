

/*

Routine Description:
	
	MMU Create   
	
Arguments:

	None.

Return Value:

	None.

*/

#include "general.h"
#include "board.h"
#include "mmu.h"
#include "mmuapi.h"
#ifdef MMU_SUPPORT
extern unsigned int *mmu_tlb_base;
extern unsigned int *mmu_L1_base;
extern void board_activate_mmu(int arg1);
#ifdef SECTION_PAGE
// Mark following since we use section page
static struct map_desc standard_io_desc[]  = {
	/* virtual              physical          length  domain         r w c b  last*/
	{ VA_StMemBase         ,StMemBase         ,SZ_1M ,DOMAIN_CLIENT, 0,0,0,0,0},
 #if(MEMORY_POOL_START_ADDR  == 0x80200000)
	#ifdef WRITE_BACK
	{ VA_SdramBase         ,SdramBase         ,SZ_2M ,DOMAIN_CLIENT, 0,0,1,1,0},
	#else  // Write Through
    { VA_SdramBase         ,SdramBase         ,SZ_2M ,DOMAIN_CLIENT, 0,0,1,0,0},
	#endif
	{ VA_SdramBase+SZ_2M   ,SdramBase+SZ_2M   ,DRAM_MEMORY_END-SdramBase+4-SZ_2M ,DOMAIN_CLIENT, 0,0,0,0,0},
 #elif(MEMORY_POOL_START_ADDR  == 0x80500000)
	#ifdef WRITE_BACK
	{ VA_SdramBase         ,SdramBase         ,SZ_5M ,DOMAIN_CLIENT, 0,0,1,1,0},
	#else  // Write Through
    { VA_SdramBase         ,SdramBase         ,SZ_5M ,DOMAIN_CLIENT, 0,0,1,0,0},
	#endif
	{ VA_SdramBase+SZ_5M   ,SdramBase+SZ_5M   ,DRAM_MEMORY_END-SdramBase+4-SZ_5M ,DOMAIN_CLIENT, 0,0,0,0,0},
 #else
	#ifdef WRITE_BACK
	{ VA_SdramBase         ,SdramBase         ,SZ_4M ,DOMAIN_CLIENT, 0,0,1,1,0},
	#else  // Write Through
    { VA_SdramBase         ,SdramBase         ,SZ_4M ,DOMAIN_CLIENT, 0,0,1,0,0},
	#endif
	{ VA_SdramBase+SZ_4M   ,SdramBase+SZ_4M   ,DRAM_MEMORY_END-SdramBase+4-SZ_4M ,DOMAIN_CLIENT, 0,0,0,0,0},
 #endif   
	{ VA_AHBARBBase        ,AHBARBBase        ,SZ_1M ,DOMAIN_CLIENT, 0,0,0,0,0},
	{ VA_SiuCtrlBase       ,SiuCtrlBase       ,SZ_1M ,DOMAIN_CLIENT, 0,0,0,0,0},
 	{ VA_IntCtrlBase       ,IntCtrlBase       ,SZ_2M ,DOMAIN_CLIENT, 0,0,0,0,0},
 	{ VA_CpuTCMBase        ,TcmSramBase       ,SZ_1M ,DOMAIN_CLIENT, 0,0,0,0,0},
 	{ VA_GfuCtrlBase       ,GFUCtrlBase       ,SZ_1M ,DOMAIN_CLIENT, 0,0,0,0,0},
	{ 0                    ,0                 ,0     ,0            , 0,1,0,0,1},
};

#else
static struct map_desc standard_io_desc[]  = {
	/* virtual        physical          length  c b domain     r w last*/
	{ VA_StMemBase,StMemBase   ,SZ_1M ,0,0,DOMAIN_CLIENT, 0,0,0},
       { VA_SdramBase + SZ_64K*0   ,SdramBase + SZ_64K*0       ,SZ_64K ,0xFFFF,0xFFFF,DOMAIN_CLIENT, 0,0,0}, //0x80000000 -- 0x8000FFFF
       { VA_SdramBase + SZ_64K*1   ,SdramBase + SZ_64K*1       ,SZ_64K ,0xFFFF,0xFFFF,DOMAIN_CLIENT, 0,0,0}, //0x80010000 -- 0x8001FFFF      
       { VA_SdramBase + SZ_64K*2   ,SdramBase + SZ_64K*2       ,SZ_64K ,0xFFFF,0xFFFF,DOMAIN_CLIENT, 0,0,0}, //0x80020000 -- 0x8002FFFF 
       { VA_SdramBase + SZ_64K*3   ,SdramBase + SZ_64K*3       ,SZ_64K ,0xFFFF,0xFFFF,DOMAIN_CLIENT, 0,0,0}, //0x80030000 -- 0x8003FFFF      
       { VA_SdramBase + SZ_64K*4   ,SdramBase + SZ_64K*4       ,SZ_64K ,0xFFFF,0xFFFF,DOMAIN_CLIENT, 0,0,0}, //0x80040000 -- 0x8004FFFF 
       { VA_SdramBase + SZ_64K*5   ,SdramBase + SZ_64K*5       ,SZ_64K ,0xFFFF,0xFFFF,DOMAIN_CLIENT, 0,0,0}, //0x80050000 -- 0x8005FFFF      
       { VA_SdramBase + SZ_64K*6   ,SdramBase + SZ_64K*6       ,SZ_64K ,0xFFFF,0xFFFF,DOMAIN_CLIENT, 0,0,0},  //0x800600000 -- 0x8006FFFF
       { VA_SdramBase + SZ_64K*7   ,SdramBase + SZ_64K*7       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},                   //0x800700000 -- 0x8007FFFF   
       { VA_SdramBase + SZ_64K*8   ,SdramBase + SZ_64K*8       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},                  //0x80080000 -- 0x8008FFFF
       { VA_SdramBase + SZ_64K*9   ,SdramBase + SZ_64K*9       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},                  //0x80090000 -- 0x8009FFFF
       { VA_SdramBase + SZ_64K*10   ,SdramBase + SZ_64K*10       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},              //0x800A0000 -- 0x800AFFFF
       { VA_SdramBase + SZ_64K*11   ,SdramBase + SZ_64K*11       ,SZ_64K ,0x0206,0x206,DOMAIN_CLIENT, 0,0,0},   //0x800B0000 -- 0x800BFFFF      
       { VA_SdramBase + SZ_64K*12   ,SdramBase + SZ_64K*12      ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},               //0x800C0000 -- 0x800CFFFF
       { VA_SdramBase + SZ_64K*13   ,SdramBase + SZ_64K*13       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},              //0x800D0000 -- 0x8009FFFF
       { VA_SdramBase + SZ_64K*14   ,SdramBase + SZ_64K*14       ,SZ_64K ,0x8020,0x8020,DOMAIN_CLIENT, 0,0,0},    //0x800E0000 -- 0x8009FFFF
       { VA_SdramBase + SZ_64K*15   ,SdramBase + SZ_64K*15       ,SZ_64K ,0x00FF,0x00FF,DOMAIN_CLIENT, 0,0,0},   //0x800F0000 -- 0x8009FFFF
       { VA_SdramBase + SZ_64K*16   ,SdramBase + SZ_64K*16       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},                      //0x80100000 -- 0x8010FFFF
       { VA_SdramBase + SZ_64K*17   ,SdramBase + SZ_64K*17       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},                      //0x80110000 -- 0x8011FFFF
       { VA_SdramBase + SZ_64K*18   ,SdramBase + SZ_64K*18       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},                      //0x80120000 -- 0x8012FFFF
       { VA_SdramBase + SZ_64K*19   ,SdramBase + SZ_64K*19       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},        //0x80130000 -- 0x8013FFFF
       { VA_SdramBase + SZ_64K*20   ,SdramBase + SZ_64K*29       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},        //0x80140000 -- 0x8014FFFF
       { VA_SdramBase + SZ_64K*21   ,SdramBase + SZ_64K*21       ,SZ_64K ,0xFFFF,0xFFFF,DOMAIN_CLIENT, 0,0,0},        //0x80150000 -- 0x8015FFFF 
       { VA_SdramBase + SZ_64K*22   ,SdramBase + SZ_64K*22       ,SZ_64K ,0x001F,0x001F,DOMAIN_CLIENT, 0,0,0},        //0x80160000 -- 0x8016FFFF
       { VA_SdramBase + SZ_64K*23   ,SdramBase + SZ_64K*23       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},        //0x80170000 -- 0x8017FFFF
       { VA_SdramBase + SZ_64K*24   ,SdramBase + SZ_64K*24       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},        //0x80180000 -- 0x8018FFFF
       { VA_SdramBase + SZ_64K*25   ,SdramBase + SZ_64K*25       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},        //0x80190000 -- 0x8019FFFF
       { VA_SdramBase + SZ_64K*26   ,SdramBase + SZ_64K*26       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},        //0x801A0000 -- 0x801AFFFF
       { VA_SdramBase + SZ_64K*27   ,SdramBase + SZ_64K*27       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},        //0x801B0000 -- 0x801BFFFF 
       { VA_SdramBase + SZ_64K*28   ,SdramBase + SZ_64K*28       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},        //0x801C0000 -- 0x801CFFFF
       { VA_SdramBase + SZ_64K*29   ,SdramBase + SZ_64K*29       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},        //0x801D0000 -- 0x801DFFFF
       { VA_SdramBase + SZ_64K*30   ,SdramBase + SZ_64K*30       ,SZ_64K ,0,0,DOMAIN_CLIENT, 0,0,0},        //0x801E0000 -- 0x801EFFFF
       { VA_SdramBase + SZ_64K*31   ,SdramBase + SZ_64K*31       ,SZ_64K ,0xFFFF,0xFFFF,DOMAIN_CLIENT, 0,0,0},        //0x801F0000 -- 0x801FFFFF
	{ VA_SdramBase+SZ_4M    ,SdramBase+SZ_4M       ,DRAM_MEMORY_END-SdramBase+4-SZ_4M ,0x0,0x0,DOMAIN_CLIENT, 0,0,0},       
	{ VA_AHBARBBase    ,AHBARBBase       ,SZ_1M ,0,0,DOMAIN_CLIENT, 0,0,0}, 
	{ VA_SiuCtrlBase  ,SiuCtrlBase     ,SZ_1M ,0,0,DOMAIN_CLIENT, 0,0,0},
 	{ VA_IntCtrlBase    ,IntCtrlBase       ,SZ_1M ,0,0,DOMAIN_CLIENT, 0,0,0},
	{ 0,0  ,0 ,0,0,0, 0,1,1},
};


#endif
void  create_mapping(struct map_desc *md)
{
    u32 descriptor_index, section_base, length,physical_base;
    u32 mmu_flag=0;
    u32* mmu_addr;
    // Set Section basic flag
    mmu_flag |=MMUTT_DESCTYPE_SECTION;

    if(md->bufferable)
    mmu_flag |=MMUTT_CACHATTR_B;    

    if(md->cacheable)    
    mmu_flag |=MMUTT_CACHATTR_C;
    
    if(md->prot_read && md->prot_write)     // R/W protection
    mmu_flag |=MMUTT_AP_DEFAULT;
    else if(md->prot_read==0 && md->prot_write==1)  //Read Only
    mmu_flag |=MMUTT_AP_RO_USER;
    else if(md->prot_read==0 && md->prot_write==0)  //Read Write
    mmu_flag |=MMUTT_AP_ALL;
    else   //Setting error
    {
    DEBUG_MAIN("Error: MMU AP flag error\n"); 
    }

    // Set domain flag
    mmu_flag |=  md->domain << MMUTT_DOMAIN_SHIFT;

    length= md->length;
    section_base = md->virtual;
    physical_base = md->physical;
    for (;length>0;length-=SZ_1M)
    {
         mmu_addr=mmu_tlb_base;
         descriptor_index = section_base>>20; 
         descriptor_index = descriptor_index<<2; // Align word
         mmu_addr+=descriptor_index/4;
         *mmu_addr = physical_base | mmu_flag;
         //descriptor_index+=1;
         physical_base += SZ_1M;
         section_base += SZ_1M;
    }

}

void  create_coarse_mapping(struct map_desc *md)
{
    u32 descriptor_index, MVA, length,physical_base;
    u32 mmu_flag=0,L1_flag=0;
    u32* mmu_addr;
    u32* L1_addr;
    u16 cache_mask=1,i=0;
    u32* coarse_tlb_addr;
    // Set L1 descriptor
    L1_flag |= MMUTT_DESCTYPE_TBL_COARSE;
    L1_flag |= md->domain << MMUTT_DOMAIN_SHIFT;

    // Set MMU flag
    mmu_flag |=MMUTT_DESCTYPE_PAGE_SMALL;
    
    if(md->prot_read && md->prot_write)     // R/W protection
    {
        mmu_flag |=MMUTT_AP_DEFAULT_UNSHFT<<MMUTT_AP0_SHIFT |
                           MMUTT_AP_DEFAULT_UNSHFT<<MMUTT_AP1_SHIFT |
                           MMUTT_AP_DEFAULT_UNSHFT<<MMUTT_AP2_SHIFT |
                           MMUTT_AP_DEFAULT_UNSHFT<<MMUTT_AP3_SHIFT  ;
    }
    else if(md->prot_read==0 && md->prot_write==1)  //Read Only
    {
        mmu_flag |=MMUTT_AP_RO_USER_UNSHFT<<MMUTT_AP0_SHIFT |
                           MMUTT_AP_RO_USER_UNSHFT<<MMUTT_AP1_SHIFT |
                           MMUTT_AP_RO_USER_UNSHFT<<MMUTT_AP2_SHIFT |
                           MMUTT_AP_RO_USER_UNSHFT<<MMUTT_AP3_SHIFT  ;
    }
    else if(md->prot_read==0 && md->prot_write==0)  //Read Write
    {
        mmu_flag |=MMUTT_AP_ALL_UNSHFT<<MMUTT_AP0_SHIFT |
                           MMUTT_AP_ALL_UNSHFT<<MMUTT_AP1_SHIFT |
                           MMUTT_AP_ALL_UNSHFT<<MMUTT_AP2_SHIFT |
                           MMUTT_AP_ALL_UNSHFT<<MMUTT_AP3_SHIFT  ;
    }
    else   //Setting error
    {
        DEBUG_MAIN("Error: MMU AP flag error\n"); 
    }

    length= md->length;
    MVA = md->virtual;
    physical_base = md->physical;
    if(MVA<(SdramBase+SZ_1M))
        coarse_tlb_addr=mmu_L1_base;
    else if(MVA>=(SdramBase+SZ_1M))
    {
        coarse_tlb_addr=mmu_L1_base;
        coarse_tlb_addr+=SZ_1K/4;    
    }

    for (;length>0;length-=SZ_4K)
    {
         L1_addr=mmu_tlb_base;
         descriptor_index = MVA>>20; 
         descriptor_index = descriptor_index<<2; // Align word
         L1_addr+= descriptor_index/4;
         *L1_addr = (u32)coarse_tlb_addr | L1_flag;

         // Start L2 transfer
         mmu_flag &=~(MMUTT_CACHATTR_MASK);
         #ifdef WRITE_BACK
         if(md->bufferable & cache_mask)
         mmu_flag |=MMUTT_CACHATTR_B;    
         #endif
        
         if(md->cacheable & cache_mask)    
         mmu_flag |=MMUTT_CACHATTR_C;

         L1_addr=coarse_tlb_addr;
         descriptor_index = MVA<<12; 
         descriptor_index=descriptor_index >>24;
         descriptor_index = descriptor_index<<2; // Align word
         L1_addr+= descriptor_index/4;
         *L1_addr = physical_base | mmu_flag;       

         physical_base += SZ_4K;
         MVA += SZ_4K;
         cache_mask <<= 1;
    }

}
void MMU_table_init(struct map_desc *mmu_desc)
{
	int i;

        for (i = 0; mmu_desc[i].last == 0; i++)
	{
	       if(mmu_desc[i].length>=SZ_1M) 
		create_mapping(mmu_desc + i);
              else if(mmu_desc[i].length==SZ_64K) 
              create_coarse_mapping(mmu_desc + i);  
	}
}

void MMU_mapping(void)
{
    mmu_tlb_base=(u32 *)MEMORY_POOL_START_ADDR;
    mmu_L1_base=(u32 *)(MEMORY_POOL_START_ADDR+SZ_16K);  // Since we are runing before memory pool init
    MMU_table_init(standard_io_desc);
    board_activate_mmu(MEMORY_POOL_START_ADDR);
}

u8 cehck_cache_by_address(u32 addr)
{
	 int i;
        u32 offset_addr;
        struct map_desc *mmu_desc;
        mmu_desc=standard_io_desc;
        
        for (i = 0; mmu_desc[i].last == 0; i++)
	{
	       if(mmu_desc[i].length==SZ_64K) 
	       {
                if(addr>=mmu_desc[i].physical && addr<= (mmu_desc[i].physical + mmu_desc[i].length))
                {
                    offset_addr=addr-mmu_desc[i].physical;
                    offset_addr=(offset_addr +SZ_4K-1)/SZ_4K;
                    if(mmu_desc[i].cacheable & (1<<offset_addr))  
                       return 1;  // need flush and clean 
                    else
                       return 0; 
                }
	       }
	}

        return 0;
}
void Read_cache_info(void)
{
    int r0_value=0;
    __asm
    {
        mrc     p15, 0, r0_value, c0, c0, 1
    };

}

void flush_dcache(void)
{
    int r0_value=0;
    __asm
    {
        mcr     p15, 0, r0_value, c7, c6, 0
    };

}

#endif
