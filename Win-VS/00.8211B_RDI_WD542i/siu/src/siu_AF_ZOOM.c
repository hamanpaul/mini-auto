/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	siu_AF_ZOOM.c

Abstract:

   	5M Sensor about AF and ZOOM Unit.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2007/05/25	Lisa Lu	Create	

*/
#include "general.h"
#include "board.h"
#include "gpioapi.h"  

extern u8 siuAFCount;

s32 siuSetAF_StrPosition(void);	
void Rotate_F_Step(s32 step); 	
void Rotate_Z_Step(s32 step); 	

void sensor_HWrst()
{
    siuSetAF_StrPosition();
} 

void Set_F_reset(u8 level)
{
	gpioSetLevel(1,9,level);
}
void Set_Z_reset(u8 level)
{
	gpioSetLevel(1,23,level);
}
void Set_F_EN(u8 level)
{
	gpioSetLevel(1,10,level);
}
void Set_Z_EN(u8 level)
{
	gpioSetLevel(1,24,level);
}
void Set_F_Mode(u8 level)
{
	gpioSetLevel(1,18,level);
}
void Set_Z_Mode(u8 level)
{
	gpioSetLevel(1,26,level);
}
void Set_F_AN(u8 level)
{
	gpioSetLevel(1,17,level);
}
void Set_Z_AN(u8 level)
{
	gpioSetLevel(1,25,level);
}
void Set_F_CLK(u8 level)
{
	gpioSetLevel(1,8,level);
}
void Set_Z_CLK(u8 level)
{
	gpioSetLevel(1,22,level);
}
void Set_F_CW(u8 level)
{
	gpioSetLevel(1,7,level);
}
void Set_Z_CW(u8 level)
{
	gpioSetLevel(1,21,level);
}
u8 Get_F_em(void)
{
	u8 level;
	gpioGetLevel(0,8,&level);
	return level;
}
u8 Get_Z_em(void)
{
	u8 level;
	gpioGetLevel(0,7,&level);
	return level;
}

void Rotate_F_Step(s32 F_step)
{
	u32 i, j, k,m;
	if (F_step>0)
	{
		Set_F_CW(0);
		F_step=F_step;
	}
	else
	{
		Set_F_CW(1);
		F_step=0-F_step;
	}	
	for(k=0; k<F_step;k++)
	{
		Gpio1Level |= 0x00000100;  //GPIO1.8
		for(i=0; i<2500; i++)    ;  // delay
		Gpio1Level &= 0xfffffeff;  //GPIO1.8
		for(i=0; i<2500; i++)    ;  // delay   
	}
}
void Rotate_Z_Step(s32 Z_step)
{
	u32 i, j, k,m;
	if (Z_step>0)
	{
		Set_Z_CW(0);
		Z_step=Z_step;
	}
	else
	{
		Set_Z_CW(1);
		Z_step=0-Z_step;
	}	
	for(k=0; k<Z_step;k++)
	{
		Gpio1Level |= 0x00400000;  //GPIO1.22		//Z_clk
		for(i=0; i<2500; i++)    ;  // delay              
		Gpio1Level &= 0xffbfffff;  ////GPIO1.22		//Z_clk
		for(i=0; i<2500; i++)    ;  // delay  
	}
}
//Zoom_Focus_Step[Zoom][focus]
s32 Zoom_Focus_Step[12][9]=
{
	{56,136,112,88,-8,-72,-136,-200,-600}, // Zoom 1
	{736,816,792,768,672,608,544,480,80}, // Zoom 2
	{1816,1896,1864,1848,1752,1680,1624,1560,1152}, // Zoom 3
	{2416,2488,2464,2440,2352,2280,2224,2160,1752}, // Zoom 4
	{2736,2816,2792,2768,2672,2608,2544,2480,2072}, // Zoom 5
	{2856,2936,2904,2888,2792,2720,2664,2600,2192}, // Zoom 6
	{2696,2776,2752,2728,2632,2560,2504,2440,2032}, // Zoom 7
	{2576,2648,2624,2600,2504,2440,2376,2320,1912}, // Zoom 8
	{2184,2256,2232,2208,2112,2048,1984,1880,1520}, // Zoom 9
	{1696,1776,1752,1728,1632,1568,1504,1440,1032}, // Zoom max
	{400,1184,1936,2720,3552,4272,5016,5904,6496}, // Zoom facter
	{680,1080,600,320,120,-160,-128,-392,-480}  // Zoom step
};
s32 siuSetAF_StrPosition(void)
{
    u32 i, j, k,m;
    u32 flag;
    u32 uMaxAF_report0_H = 0;
    u32 uMaxAF_step = 0;
    u32 uMaxAF_step_F = 0;
    u8 dir= 0;
    u32 temp;
    u8 F_factor =0;	
    u8 Zoom=0;
    u32 step;
    u32 F_step;
    u32 Z_step;
		
    // reset Focus
	Set_F_reset(0);
	Set_Z_reset(0);
    for(i=0; i<100; i++);
    	Set_F_reset(1);
   	Set_Z_reset(1);
    // enable is active low
	Set_F_EN(0);
	Set_Z_EN(0);

    // mode is set to Hi
 	Set_F_Mode(1);    
	Set_Z_Mode(1);        
    
    // set motor address into start position
	Set_F_AN(1);    
	Set_Z_AN(1);    
   /*F_home S */

       if (Gpio0Level&0x00000100)      //F_em=1
        {   
          Gpio1Level |= 0x00000080;  //GPIO1.7   //CW=1
            while((Gpio0Level&0x00000100) != 0)     //F_em=1
            {
                Gpio1Level |= 0x00000100;  //GPIO1.8
                for(i=0; i<2500; i++)    ;  // delay
                Gpio1Level &= 0xfffffeff;  //GPIO1.8
                for(i=0; i<2500; i++)    ;  // delay   
            } 
            for(i=0; i<10000; i++)    ;     
            Gpio1Level &= 0xffffff7f;  //GPIO1.7   //CW=0                            
           while((Gpio0Level&0x00000100) == 0)        //F_em=0
            {
                Gpio1Level |= 0x00000100;  //GPIO1.8
                for(i=0; i<2500; i++)    ;  // delay
                Gpio1Level &= 0xfffffeff;  //GPIO1.8
                for(i=0; i<2500; i++)    ;  // delay   
            }           
        }
        else
        {  
            Gpio1Level &= 0xffffff7f;  //GPIO1.7   //CW=0
            while((Gpio0Level&0x00000100) == 0)        //F_em=0
            {
                Gpio1Level |= 0x00000100;  //GPIO1.8
                for(i=0; i<2500; i++)    ;  // delay  
                Gpio1Level &= 0xfffffeff;  //GPIO1.8
                for(i=0; i<2500; i++)    ;  // delay    
            }    
        }
  /*F_home E */
/*
	if (Gpio0Level&0x00000080)      //Z_em=1	//GPIO0.7 //	if (Get_Z_em())	// 
	{  
//		Gpio1Level &= 0xffdfffff;  //GPIO1.21    //Z_CW=0	   
Gpio1Level |= 0x00200000;  //GPIO1.21   //Z_CW=1 

		while((Gpio0Level&0x00000080) != 0)     //Z_em=1
		{      
			Gpio1Level |= 0x00400000;  //GPIO1.22		//Z_clk
			for(i=0; i<2500; i++)    ;  // delay              
			Gpio1Level &= 0xffbfffff;  ////GPIO1.22		//Z_clk
			for(i=0; i<2500; i++)    ;  // delay   
		} 
	}
        else
	{
Gpio1Level &= 0xffdfffff;  //GPIO1.21    //Z_CW=0
	//	Gpio1Level |= 0x00200000;  //GPIO1.21   //Z_CW=1 
    		while((Gpio0Level&0x00000080) == 0)        //Z_em=0
		{
			Gpio1Level |= 0x00400000;  //GPIO1.22		//Z_clk
			for(i=0; i<2500; i++)    ;  // delay              
			Gpio1Level &= 0xffbfffff;  ////GPIO1.22		//Z_clk
			for(i=0; i<2500; i++)    ;  // delay  
		}    
		for(i=0; i<10000; i++)    ;     
Gpio1Level |= 0x00200000;  //GPIO1.21   //Z_CW=1      
	//	Gpio1Level &= 0xffdfffff;  //GPIO1.21    //Z_CW=0	   
		while((Gpio0Level&0x00000080) != 0)     //Z_em=1
		{      
			Gpio1Level |= 0x00400000;  //GPIO1.22		//Z_clk
			for(i=0; i<2500; i++)    ;  // delay              
			Gpio1Level &= 0xffbfffff;  ////GPIO1.22		//Z_clk
			for(i=0; i<2500; i++)    ;  // delay   
		} 	
        }
*/
  /*Z_home S */

	if (Gpio0Level&0x00000080)      //Z_em=1	//GPIO0.7 //	if (Get_Z_em())	// 
	{  
		Gpio1Level |= 0x00200000;  //GPIO1.21   //Z_CW=1          	   
		while((Gpio0Level&0x00000080) != 0)     //Z_em=1
		{      
			Gpio1Level |= 0x00400000;  //GPIO1.22		//Z_clk
			for(i=0; i<2500; i++)    ;  // delay              
			Gpio1Level &= 0xffbfffff;  ////GPIO1.22		//Z_clk
			for(i=0; i<2500; i++)    ;  // delay   
		} 
		for(i=0; i<10000; i++)    ;          
		Gpio1Level &= 0xffdfffff;  //GPIO1.21    //Z_CW=0     
		while((Gpio0Level&0x00000080) == 0)        //Z_em=0
		{
			Gpio1Level |= 0x00400000;  //GPIO1.22		//Z_clk
			for(i=0; i<2500; i++)    ;  // delay              
			Gpio1Level &= 0xffbfffff;  ////GPIO1.22		//Z_clk
			for(i=0; i<2500; i++)    ;  // delay  
		}    
	}
        else
	{
		Gpio1Level &= 0xffdfffff;  //GPIO1.21    //Z_CW=0     
    		while((Gpio0Level&0x00000080) == 0)        //Z_em=0
		{
			Gpio1Level |= 0x00400000;  //GPIO1.22		//Z_clk
			for(i=0; i<2500; i++)    ;  // delay              
			Gpio1Level &= 0xffbfffff;  ////GPIO1.22		//Z_clk
			for(i=0; i<2500; i++)    ;  // delay  
		}    
        }

  /*Z_home E */ 
  
 /*F_start S */
	Rotate_F_Step(Zoom_Focus_Step[0][0]);
	siuAFCount = 0;
	while (siuAFCount < 5);
	uMaxAF_report0_H = IpuAFRpt0_1 & 0xFFFF;
	uMaxAF_step =Zoom_Focus_Step[0][0] ;
	DEBUG_SIU
		(
		        "AF_report0=%d.\n"
		        "uMaxAF_step0=%d.\n", 
			IpuAFRpt0_1 & 0xFFFF,
			uMaxAF_step 
	        );		
 /*F_start E */

/* Z_set S*/
  
 
 		if (Zoom==0)
			step=0;
		else
  			step=Zoom_Focus_Step[10][Zoom];
		Rotate_Z_Step(step);

/* Z_set E*/

/*F_fine S */   
 
	for(i=1;i<9;i++)
		{
			step=Zoom_Focus_Step[Zoom][i]-Zoom_Focus_Step[Zoom][i-1];
			Rotate_F_Step(step);
			siuAFCount = 0;
			while (siuAFCount < 5);
			
			if (uMaxAF_report0_H < (IpuAFRpt0_1 & 0xFFFF))
			{
				uMaxAF_report0_H = IpuAFRpt0_1 & 0xFFFF;
				uMaxAF_step =Zoom_Focus_Step[Zoom][i] ;
				F_factor =i ;
			}
			DEBUG_SIU
				(
				        "AF_report1=%d.\n"
				        "uMaxAF_step1=%d.\n", 
					IpuAFRpt0_1 & 0xFFFF,
					uMaxAF_step 
			        );		
		}
	    	uMaxAF_step_F = (uMaxAF_step-Zoom_Focus_Step[Zoom][8]);
		Rotate_F_Step(uMaxAF_step_F);
 
 /*F_fine E */   

return 0;    
}	
