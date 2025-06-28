

#define USE_AVERAGE  0

void AWB_main(
			    unsigned char *SrcImg,
				unsigned char *DstQQVGAImg,
				int Pic_W,
				int Pic_H,
				int SampleFactor
		     )
{
	unsigned char *srcPos;
	unsigned char *dstPos;
	int i,j;
#if USE_AVERAGE
    int k;
	int sum1,sum2;
#endif
#ifdef WIN32
    FILE *fp;
#endif

	SrcImg=SrcImg + Pic_W*2 + 2;
    dstPos=DstQQVGAImg;

	for(j=0;j<(Pic_H-4)/SampleFactor/2;j++)
	{
		srcPos=SrcImg + Pic_W*(j*SampleFactor*2);
	    for(i=0;i<(Pic_W-4)/SampleFactor/2;i++)
		{
#if USE_AVERAGE
			sum1=0;
			sum2=0;
			for(k=0;k<SampleFactor;k++)
			{
               sum1 += srcPos[0+k*2];
			   sum2 += srcPos[1+k*2];
			}
		    dstPos[0]=sum1/SampleFactor;
			dstPos[1]=sum2/SampleFactor;
#else
            dstPos[0]= srcPos[0];
			dstPos[1]= srcPos[1];
#endif
			dstPos +=2;
			srcPos +=SampleFactor*2;
		}
	    //===//
		srcPos=SrcImg + Pic_W*( j*SampleFactor*2 + 1 );
		for(i=0;i<(Pic_W-4)/SampleFactor/2;i++)
		{
#if USE_AVERAGE
			sum1=0;
			sum2=0;
			for(k=0;k<SampleFactor;k++)
			{
               sum1 += srcPos[0+k*2];
			   sum2 += srcPos[1+k*2];
			}
		    dstPos[0]=sum1/SampleFactor;
			dstPos[1]=sum2/SampleFactor;
#else
            dstPos[0]= srcPos[0];
			dstPos[1]= srcPos[1];
#endif
			dstPos +=2;
			srcPos +=SampleFactor*2;
		}
	}

#ifdef WIN32
    fp=fopen("AWB image\\AWBimg.raw","wb");
    fwrite(DstQQVGAImg, ((Pic_H-4)/SampleFactor) * ((Pic_W-4)/SampleFactor),1,  fp);
	fclose(fp);
#endif

}
