

#include <stdio.h>


#include "general.h"
#include "AWBdef.h"

#define BAYERIMG_W   160
#define BAYERIMG_H   120

typedef struct _IPU_LUM_GAMMA {
	unsigned char		x;
	unsigned char		y;
} IPU_LUM_GAMMA;



void GetAwbGain(unsigned char *BayerImg,short *R_gain,short *B_gain);
unsigned char B_pregamma(unsigned char data);
unsigned char Gr_pregamma(unsigned char data);
unsigned char Gb_pregamma(unsigned char data);
unsigned char R_pregamma(unsigned char data);

unsigned char Cal_AWB_Weight(short Y,short I, short Q);
int Cal_AWB_Y_Dist(short Y, short Q);
int Cal_AWB_IQ_Dist(short I, short Q, unsigned char Region);
unsigned char remap_RGBGamma(unsigned char valueX, IPU_LUM_GAMMA *pGammaTbl);

#ifdef WIN32
  void WriteBmpFile(char *pDataBGR,int W,int H,char *PathName);
#endif

#ifdef WIN32
   unsigned char AwbWeightMap[BAYERIMG_W/2*BAYERIMG_H/2*3];
#endif

#ifdef WIN32
void AWB_Analysis(DEF_APPENDIXINFO *pPicAppInfo)
{
	FILE *awbfile;
	short R_gain,B_gain;
	int i,j;


	if(  pPicAppInfo->ModelName[0] !='H' ||
         pPicAppInfo->ModelName[1] !='I' ||
		 pPicAppInfo->ModelName[2] !='M' ||
		 pPicAppInfo->ModelName[3] !='A' ||
		 pPicAppInfo->ModelName[4] !='X' 
	  )

	    return;

    //Show Message//
    DEBUG_IPU("\n");
	DEBUG_IPU("-----------Model Name: %s----------\n",pPicAppInfo->ModelName);
    DEBUG_IPU("Sensor Info: EL=%d,AG=%d,DG=%d\n",pPicAppInfo->sensorinfo.EL,pPicAppInfo->sensorinfo.AG,pPicAppInfo->sensorinfo.DG);
	DEBUG_IPU("SIU status: SIU_CTL1=0x%08x, OB_COMP==0x%08x\n",pPicAppInfo->siu_status.SIU_CTL1,pPicAppInfo->siu_status.OB_COMP);
	DEBUG_IPU("AE report: \n");
	DEBUG_IPU("            AETabIndex=%d,ConvergeCount=%d\n",pPicAppInfo->ae_report.AECurSet,pPicAppInfo->ae_report.ConvergeCount);
	DEBUG_IPU("            ae_YavgValue=%d, ae_logYdiff=%d\n",pPicAppInfo->ae_report.ae_YavgValue,pPicAppInfo->ae_report.ae_logYdiff);
	DEBUG_IPU("            EL=%d, AG=%d,MUL=%d,DG=%d\n",pPicAppInfo->ae_report.EL,pPicAppInfo->ae_report.AG,pPicAppInfo->ae_report.MUL,pPicAppInfo->ae_report.DG);
    //Save Awb bayer image//
	awbfile=fopen("AWB_Image\\awb.raw","wb");
    fwrite(pPicAppInfo->AwbImg,BAYERIMG_W*BAYERIMG_H,1,awbfile);
	fclose(awbfile);
	//===Analysis AWB image===//
    GetAwbGain(pPicAppInfo->AwbImg,&R_gain,&B_gain);
	//========================//
    
	fclose(awbfile);
	DEBUG_IPU("Post R_gain=%d, B_gain=%d (x1000)\n",R_gain,B_gain);
	DEBUG_IPU("Capture mode: R_gain=%d, B_gain=%d (x1000)\n",pPicAppInfo->AWBgain_Capture[0],pPicAppInfo->AWBgain_Capture[2]);
	DEBUG_IPU("Preview mode: R_gain=%d, B_gain=%d (x1000)\n",pPicAppInfo->AWBgain_Preview[0],pPicAppInfo->AWBgain_Preview[2]);
    
	DEBUG_IPU("Color Correct Matrix={%d,%d,%d,%d,%d,%d,%d,%d,%d}\n",pPicAppInfo->CCM[0],pPicAppInfo->CCM[1],pPicAppInfo->CCM[2],
		pPicAppInfo->CCM[3],pPicAppInfo->CCM[4],pPicAppInfo->CCM[5],
		pPicAppInfo->CCM[6],pPicAppInfo->CCM[7],pPicAppInfo->CCM[8]
		);

	DEBUG_IPU("AeWeightTab=\n");
	for(j=0;j<5;j++)
	{
		DEBUG_IPU("          ");
		for(i=0;i<5;i++)
        {
			DEBUG_IPU("%3d ",pPicAppInfo->AeWeightTab[j*5+i]);
		}
		DEBUG_IPU("\n");
    }

    DEBUG_IPU("aeWinYsum=\n");
	for(j=0;j<5;j++)
	{
		DEBUG_IPU("          ");
		for(i=0;i<5;i++)
        {
			DEBUG_IPU("%8d ",pPicAppInfo->aeWinYsum[j*5+i]);
		}
		DEBUG_IPU("\n");
    }

	

	//DEBUG_IPU("CCM[i]=");
}
#endif

/*
    1. Do Bayer data transfer to RGB data.
    2. Do RGB pregama calibration.
*/
#define AWB_DROP_EDGE_THR    10
#define AWB_GB_DIFF_THR      90
#define AWB_GR_DIFF_THR      90
#define AWB_RB_DIFF_THR      90
#define AWB_Y_MAX_THR        230
#define AWB_Y_MIN_THR        15

#define R_GAIN_MAX           180
#define R_GAIN_MIN           80
#define B_GAIN_MAX           210
#define B_GAIN_MIN           90

#ifdef WIN32
  unsigned char AWB_CheckImage[80*60*3];
#endif

void GetAwbGain(unsigned char *BayerImg,short *R_gain,short *B_gain)
{
   int i,j;
   unsigned char *src;
   unsigned char Gr,Gb,G,R,B,Yx,Wt;
   unsigned char diff_Gx,diff_GB,diff_GR,diff_RB;
   unsigned int AWB_EdgeDrop_cnt;
   short Y,I,Q;
   unsigned int R_sum,G_sum,B_sum,temp,Gain;
   unsigned char minRGB,maxRGB;
   int R_ratio,B_ratio;

#ifdef WIN32
   int count=0;
   FILE *fp_AWB;
   unsigned char *pp=AWB_CheckImage;
#endif

   AWB_EdgeDrop_cnt =0;
   R_sum=0;
   G_sum=0;
   B_sum=0;

#ifdef WIN32
   fp_AWB=fopen("AWB_Image\\R_G_B.dat","wb");
#endif
   for(j=0;j<BAYERIMG_H/2;j++)
   {
	   src = BayerImg + j*BAYERIMG_W*2;
       for(i=0;i<BAYERIMG_W/2;i++)
	   {

		   B=B_pregamma( *(src+0) );
		   Gb=Gb_pregamma( *(src+1) );
		   Gr=Gr_pregamma( *(src+BAYERIMG_W+0)  );
		   G=( Gb + Gr +1 ) >>1;
		   R=R_pregamma( *(src+BAYERIMG_W+1) );

		   Yx=(299*R + 587*G  +114*B)/1000;

	       Y= (2607*R   -4131*G +3261 *B)/10000;  //Y          
		   I= (5960*R   -2750*G -3210*B)/10000;   //I
		   Q= (-2576*R  -6689*G -583*B)/10000 ;   //Q
		   //==//
		   diff_Gx=(Gb>Gr) ?  (Gb-Gr): (Gr-Gb);
		   diff_GB= (G>B) ? (G-B):(B-G);
		   diff_GR= (G>R) ? (G-R):(R-G);
		   diff_RB= (R>B) ? (R-B):(B-R);

           if(R==0) R=1;
		   R_ratio= G*100/R;
		   if(B==0) B=1;
		   B_ratio= G*100/B;

           //=Calculate weighting=//
		   if(  (diff_Gx > AWB_DROP_EDGE_THR) ||
			    (diff_GR > AWB_GR_DIFF_THR)   ||
				(diff_GB > AWB_GB_DIFF_THR)   ||
				(diff_RB > AWB_RB_DIFF_THR)   ||
				(R_ratio > R_GAIN_MAX)        ||
				(R_ratio < R_GAIN_MIN)        ||
				(B_ratio > B_GAIN_MAX)        ||
				(B_ratio < B_GAIN_MIN)        ||
				(Yx > AWB_Y_MAX_THR)          ||
				(Yx < AWB_Y_MIN_THR)          ||
				(Q>-10)
			 )
		   {
			  AWB_EdgeDrop_cnt ++;
			  Wt=0;

		   #ifdef WIN32			   
			   pp[0]=0;
			   pp[1]=255;
			   pp[2]=0;
			   pp+=3;
           #endif
		   }
           else
		   {
               Wt=Cal_AWB_Weight(Y,I,Q);
		   #ifdef WIN32	
             #if 0  //抓小區域做分析落點
			   if( 
				    (j>=9) &&
					(j<=15) &&
				    (i>=19) &&
					(i<=27)
				 )
             #endif	 
		       fprintf(fp_AWB,"%d %d %d \n",R,G,B);
			   if(Wt>=50)
			   {			   
				   pp[0]=B;
				   pp[1]=G;
				   pp[2]=R;
			   }
			   else
			   {
			       pp[0]=255;
				   pp[1]=0;
				   pp[2]=0;
			   }
			   pp+=3;
           #endif
		   }

		   //=====Human visual weight====//
           //Find minRGB
		   minRGB= (R<G) ? R:G;
		   minRGB= (minRGB<B) ? minRGB: B; 

		   //Find maxRGB
		   //maxRGB= (R>G) ? R:G;
		   //maxRGB= (maxRGB>B) ? maxRGB: B; 

		   if(minRGB<20)
                Wt =Wt/4;
		   else if(minRGB<30)
			   Wt =Wt/3;



	       //=====//
		   R_sum += R*Wt;
		   G_sum += G*Wt;
		   B_sum += B*Wt;
#ifdef WIN32
           AwbWeightMap[count]=Wt*2;
		   AwbWeightMap[count+1]=Wt*2;
		   AwbWeightMap[count+2]=Wt*2;
		   count +=3;
#endif

		   src +=2;
	   }
   }
   //===Lucian:Fix Div zero 2008/02/20===//
   temp=(R_sum/200);
   if(temp==0) temp=1;
   Gain=(G_sum*5)/temp;
   if(Gain>R_GAIN_MAX*10)
      Gain= R_GAIN_MAX*10;
   if(Gain<R_GAIN_MIN*10)
      Gain= R_GAIN_MIN*10;
   *R_gain=Gain;

   temp=(B_sum/200);
   if(temp==0) temp=1;
   Gain=(G_sum*5)/temp;
   if(Gain>B_GAIN_MAX*10)
      Gain= B_GAIN_MAX*10;   
   if(Gain<B_GAIN_MIN*10)
      Gain= B_GAIN_MIN*10;   
   *B_gain = Gain;
   //======//

}

#define AWB_NOISEMARGIN     0
#define AWB_IQ_DIST_MARGIN  0

#define UV_LINE_C  9193
#define UV_LINE_D  4080

#define U30_LINE_C 1077
#define U30_LINE_D  (-5000)//(-4803)//(-5429)

#define BOT_LINE_C (-221000)
#define BOT_LINE_D  0

unsigned char Cal_AWB_Weight(short Y,short I, short Q)
{
	/*
	Left line : y= 9.1932 + 4.0800x
    Right line: y=1.0771 - 5.000x
    Down  line: y=-221
    */

    unsigned char Wt;
    unsigned char Region;
	char A,B,C;
	int Dist_Y,Dist_IQ,Dist_sum;

	A= (Q*1000>UV_LINE_C+I*UV_LINE_D) ? 1:0 ;
    B= (Q*1000>U30_LINE_C+I*U30_LINE_D) ? 1:0 ;
	C= (Q*1000 < BOT_LINE_C) ? 1: 0;
	Region= A | (B<<1) | (C<<2);

	Dist_Y=Cal_AWB_Y_Dist(Y, Q);
    Dist_IQ=Cal_AWB_IQ_Dist(I,Q,Region);

	Dist_sum=Dist_Y + Dist_IQ;
	//y=1/x;
	if( Dist_sum <= (AWB_NOISEMARGIN+1)*1000 )
	{
	   Wt=1*100;
	}
	else
	{
	   Dist_sum -= AWB_NOISEMARGIN*1000;
	   Wt=100*1000/Dist_sum;
	   if(Wt>100)
		   DEBUG_IPU("Warning..\n");
	}

    return Wt;
}

int Cal_AWB_IQ_Dist(short I, short Q, unsigned char Region)
{
	
	int Dist_IQ;
	int Ix;
	int diff_I;
	int a,b;
	int x,y;

	switch(Region)
	{
	case 0:
		if(Q>-40) //低亮度去掉
           Dist_IQ=2000;
		else
		   Dist_IQ=0;  
		break;

	case 1:
		//Left line : y= 9.1932 + 4.0800x
        //sin(x)=0.9713
		Ix= (Q*1000-UV_LINE_C)*1000/UV_LINE_D; //(x1000)
		diff_I= (I*1000 > Ix) ? (I*1000 - Ix) : (Ix-I*1000);
		Dist_IQ= (971*diff_I)/1000;
		
		break;

    case 2:
		//Right line: y=1.0771 - 5.000x
		//sin(x)=0.9806
		Ix= (Q*1000-U30_LINE_C)*1000/U30_LINE_D; //(x1000)
		diff_I= (I*1000 > Ix) ? (I*1000 - Ix) : (Ix-I*1000);
		Dist_IQ= (981*diff_I)/1000;
        
		break;

	case 3:
		x= (U30_LINE_C-UV_LINE_C)*1000/(UV_LINE_D-U30_LINE_D);
		y= U30_LINE_C + U30_LINE_D*x/1000;

		a= (I*1000 > x) ? (I*1000-x) : (x-I*1000);
		b= (Q*1000 > y) ? (Q*1000-y) : (y-Q*1000);
        Dist_IQ= a+b;
		break;

	case 4:
		Dist_IQ = (Q*1000 > BOT_LINE_C) ? (Q*1000-BOT_LINE_C) : (BOT_LINE_C-Q*1000);
		break;

	case 5:
		x= (BOT_LINE_C- UV_LINE_C)*1000/UV_LINE_D;
		y=BOT_LINE_C;
        a= (I*1000 > x) ? (I*1000-x) : (x-I*1000);
		b= (Q*1000 > y) ? (Q*1000-y) : (y-Q*1000);
        Dist_IQ= a+b;
		break;

    case 6:
		x= (BOT_LINE_C- U30_LINE_C)*1000/U30_LINE_D;
		y=BOT_LINE_C;
        a= (I*1000 > x) ? (I*1000-x) : (x-I*1000);
		b= (Q*1000 > y) ? (Q*1000-y) : (y-Q*1000);
        Dist_IQ= a+b;
		break;
	}

	if(Dist_IQ<AWB_IQ_DIST_MARGIN*1000)
        Dist_IQ=0;
	else
        Dist_IQ=Dist_IQ - AWB_IQ_DIST_MARGIN*1000;

	return Dist_IQ;
}

#define U30_TOP_C  300
#define U30_TOP_D  (-80)

#define CWF_BOT_C  0//(-1000)
#define CWF_BOT_D  0

/*
	Top line: y= 0.3000 -0.0800x
    Bottom line: y=-2.000
*/
int Cal_AWB_Y_Dist(short Y, short Q)
{
	int Y_margin;
	int Dist_Y;

    if(Y>0)
	{
	   //Top line: y= 0.3000 -0.0800x
	   if(Q>0) Q=0;
	   Y_margin = U30_TOP_C + U30_TOP_D*Q; //(x1000)
	   if(Y*1000<=Y_margin)
		   Dist_Y=0;
	   else
	   {
		   Dist_Y=Y*1000 - Y_margin;
	   }
	}
	else//(Y<=0)
	{
	   if(Q>0) Q=0;
	   Y_margin = CWF_BOT_C + CWF_BOT_D*Q; //(x1000)
	   if(Y*1000 >= Y_margin)
		   Dist_Y=0;
	   else
	   {
		   Dist_Y= -(Y*1000 - Y_margin)+1000;
	   }
	}

	return Dist_Y;
}

unsigned char B_pregamma(unsigned char data)
{
   return data;
}

unsigned char Gr_pregamma(unsigned char data)
{
   return data;
}

unsigned char Gb_pregamma(unsigned char data)
{
   return data;
}

unsigned char R_pregamma(unsigned char data)
{
   return data;
}

unsigned char remap_RGBGamma(unsigned char valueX, IPU_LUM_GAMMA *pGammaTbl)
{
	unsigned int i;
	unsigned char valueY;

	i=0;

	while( valueX >= pGammaTbl[i].x )
		i++;
	if(i== 0)
		valueY = pGammaTbl[0].y;
	else
	{
		valueY = (valueX - pGammaTbl[i-1].x) * (pGammaTbl[i].y- pGammaTbl[i-1].y)/(pGammaTbl[i].x- pGammaTbl[i-1].x) + 
			     pGammaTbl[i-1].y;
	}
	return valueY;
}
#ifdef WIN32
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef long                LONG;


#include <pshpack2.h>
typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;
#include <poppack.h>

typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;


	void WriteBmpFile(char *pDataBGR,int W,int H,char *PathName)
	{
	   BITMAPFILEHEADER bmfh;
	   BITMAPINFOHEADER bmih;
	   FILE *fp;
	   int i,j;

	   fp=fopen(PathName,"wb");

	   bmfh.bfType=19778;
	   bmfh.bfSize=W*H*3+sizeof(BITMAPINFOHEADER)+sizeof(BITMAPFILEHEADER);
	   bmfh.bfReserved1=0;
	   bmfh.bfReserved2=0;
	   bmfh.bfOffBits=54;

	   bmih.biSize=40;
	   bmih.biWidth=W;
	   bmih.biHeight=H;
	   bmih.biPlanes=1;
	   bmih.biBitCount=24;
	   bmih.biCompression=0;
	   bmih.biSizeImage=0;
	   bmih.biXPelsPerMeter=2834;
	   bmih.biYPelsPerMeter=2834;
	   bmih.biClrUsed=0;
	   bmih.biClrImportant=0;
   
	   fwrite(&bmfh, sizeof(BITMAPFILEHEADER), 1,  fp);
	   fwrite(&bmih, sizeof(BITMAPINFOHEADER), 1,  fp);

	   for(j=H-1;j>=0;j--)
	   {  //For up-side-down
		  fwrite(pDataBGR+(j)*W*3, W*3, 1,  fp);
	   }

	   fclose(fp);
	}
#endif
