
#include "general.h"
#include "mpeg4api.h"
#include "Mp4RateControl.h"


extern u32 sysTVinFormat;
//=================================//
void RCQ2_Update2OrderModel(
							   DEF_RATECONTROL_PARA *pRateControlParam,
							   int vopbits, /* Total number of bits */
							   int frame_type                               
						   )
{
	RCQ2_DATA *rcd=&pRateControlParam->RCQ2_config;
	RC_HIST   *rch=&pRateControlParam->rc_hist[frame_type];

	int decay;
	int history;
	int history_length, rc_period;

	// test RC - adamli
	history_length = pRateControlParam->rc_hist[0].history_length;
	history = pRateControlParam->rc_hist[0].rate_history;
	decay = pRateControlParam->RCQ2_config.decay;
	rc_period = pRateControlParam->RCQ2_config.rc_period;

	// do average//
	if (history_length < rc_period) 
	{
		history += vopbits;
	} 
	else if (history_length == rc_period) 
	{
		history += vopbits;
		history = history * rc_period / (rc_period + 1) ;
	} 
	else 
		history = vopbits + ( (history * decay)>>10 );

	pRateControlParam->rc_hist[0].rate_history = history;
	pRateControlParam->rc_hist[0].history_length ++;

	rch_store_after(rch, vopbits/*-vopheader*/, vopbits);

    //DEBUG_MP4("---rc_update_Q2_model---\n");
	rc_update_Q2_model(rch, &rcd->X1[frame_type]);

}

//

//
void rc_update_Q2_model(
							RC_HIST *rch,
							int  *X1  /* <-> Old and computed params. */
						)
{

	int   n;									  /* Number of data to employ */
	int   Int_last_mad;
	int   Int_plast_mad=0;
	int   i, count;
	unsigned int   *bits_text= rch->bits_text;
	int   *Int_mad_text=rch->Int_mad_text;
	unsigned int   *qp = rch->qp;
	int temp;

	int Int_sum;
	//
	if ( rch->n <START_MODEL_UPDATING)
		return;

    // Calculate the window size//
	//The data points are selected using a window whose size depends 
	//on the change in complexity. If the complexity changes significantly, 
	//a smaller window with more recent data points is used.

	Int_last_mad =rch_get_last_mad_text(rch);
	if (  rch->n > 1 )
		Int_plast_mad=rch_get_plast_mad_text(rch);

	n=MIN( rch->n,MAX_SLIDING_WINDOW);


	if ( rch->n>1 )
	{		
		n = (Int_plast_mad > Int_last_mad ) ? 
		     (Int_last_mad*n+Int_plast_mad)/Int_plast_mad : (Int_plast_mad*n+Int_last_mad)/Int_last_mad;		
	}
	else
		n = 1;

	n=MIN(n,MIN( rch->n,MAX_SLIDING_WINDOW));
    //============================================//
	/* Arrays initialization */
	// Instead of the 2nd order RC model, we now use a more reliable and much simple
	// 1st order RC model.
	Int_sum=0;
	for (i=rch_dec_mod(rch,rch->ptr),count=0; count<n; count++,i=rch_dec_mod(rch,i))
	{
		/* As the VM says, model only takes into account texture bits, although it is
		   used to estimate the total number of bits !!! */	
		temp=((Int_mad_text[i]+16)>>5);	
		if(temp==0 )  temp=1; //Avoid Divide-by-Zero
                  
		Int_sum += ( (bits_text[i]*qp[i])<<7) /temp;
		
#if DEBUG_FLAG
		if( (bits_text[i]*qp[i]) > (1<<24))
			DEBUG_MP4("Warning! RC predictor overflow!!\n");
#endif
    }
    
	*X1=(Int_sum)/n;
	
	
}
//
int rch_get_last_mad_text( RC_HIST *rch)
{
	if (rch->n == 0)
	{		
		return 0;
	}

	if (rch->ptr == 0)
	{
		return rch->Int_mad_text[rch->n-1];
	}
	else
	{
		return rch->Int_mad_text[rch->ptr-1];
	}
}
//
int rch_dec_mod(RC_HIST *rch,int i)
{
	i--;
	if (i==-1)
		return rch->size-1;
	else
		return i;
}
//
int rch_get_plast_mad_text(RC_HIST *rch)
{
	if (rch->ptr == 0)
	{
		return rch->Int_mad_text[rch->n-2];
	}
	else if (rch->ptr == 1)
	{
		return rch->Int_mad_text[rch->n-1];
	}
	else
	{
		return rch->Int_mad_text[rch->ptr-2];
	}
}
//
void rch_store_after(
						RC_HIST      *rch,
						int          bits_text,
						int          bits_vop
					)
{

	/* Save only for actual frame type: */
	if (rch->Int_mad_text[rch->ptr] != 0) 
	{
		rch->bits_text[rch->ptr] = bits_text;
		rch->bits_vop[rch->ptr]  = bits_vop;

		rch->ptr++;

		if (rch->ptr == rch->size)
			rch->ptr = 0;
		if (rch->n < rch->size)
			rch->n++;
	}


}

//===============RC model update========================//
int RCQ2_QuantAdjust(
					    DEF_RATECONTROL_PARA *pRateControlParam,
						int Int_mad,
						int frame_type
					)
{
	int qp, bits_P, bits_frame=0;
	//int decay;
	int history;
	int history_length, rc_period;
	int target_rate, average_rate;
	int max_Qp;
	int min_Qp;	
	RC_HIST   *rch=&pRateControlParam->rc_hist[frame_type];
	RCQ2_DATA *rcd=&pRateControlParam->RCQ2_config;
    //===============================//
	max_Qp=pRateControlParam->max_Qp;
	min_Qp=pRateControlParam->min_Qp;
	history_length = pRateControlParam->rc_hist[0].history_length;
	history = pRateControlParam->rc_hist[0].rate_history;
	//decay = pRateControlParam->RCQ2_config.decay;
	rc_period = pRateControlParam->RCQ2_config.rc_period;
	if ((history_length < rc_period) && (history_length != 0)) 
		average_rate = (int)(history / history_length); // cal average bit rate
	else 
		average_rate = (int)(history / rc_period);

	/* Computation of QP */
	if (rch->n==0)
	    qp=rcd->QPfirst[frame_type];
	else
	{
		/* RC-BIP: Calculate bits_frame (target0) depending on frame type */
		target_rate = rcd->target_rate;
		if (history_length < rc_period / 2)
			bits_P = target_rate;
		else
			bits_P = target_rate + (target_rate - average_rate);

		switch (frame_type)
		{
			case I_VOP:
				bits_frame = (int)( 3 * bits_P);  //alpha_I=3
				break;
			case B_VOP:
				bits_frame = (int)( bits_P/2 );  //alpha_B=0.5
				break;
			case P_VOP:
				bits_frame = bits_P;
				break;
			default:
				break;
		}		
		qp=RCQ2_compute_quant(
								rcd->bit_rate,
								bits_frame, /* RC-BIP */
								rch->n>0/*1*/ ? rch_get_last_bits_vop(rch) : 0,
								rcd->VBV_size,
								rcd->VBV_fullness,
								rcd->X1[frame_type],				  /* RC-BIP */
								Int_mad,
								rch_get_last_qp(rch),				  /* RC-BIP */
								max_Qp,
								min_Qp
			                 );
	}
	/* Storage of result */
	rch_store_before(rch,Int_mad, qp);
	return qp;
}

//
void rch_store_before(
							RC_HIST *rch,
							int Int_mad,
							int      qp
                      )
{

	rch->Int_mad_text[rch->ptr]=Int_mad;
	rch->qp[rch->ptr]      = qp;
}
//
int RCQ2_compute_quant(
						    int    bit_rate,   /* Bit rate (bits per second) */
							int    bits_frame, /* Target bits per frame */
							int    bits_prev,  /* Total bits previous frame */
							int    buff_size,  /* Buffer size */
							int    buff_occup, /* Buffer occupancy */
							int    X1,		   /* Model parameters */
							int    Int_mad,
							int    last_qp,	   /* Previous QP */
							int    max_Qp,
							int    min_Qp
					  )
{
	int target=bits_frame;
	int qp;

		
	/*** Low-pass filtering of the target ***/
	//T=max(Rs/40,Rr/Nr*0.95+S*0.05)
	if (bits_prev > 0)
	{
		target= target*19/20+ bits_prev/20;
		//target = (int)( target*(1.-PAST_PERCENT)+bits_prev*PAST_PERCENT);
	}
	/*** Lower-bounding of target ***/
	target = MAX(bit_rate/40, target);
	if (target<=0)
		return (max_Qp);
	qp=( ( (X1* (Int_mad>>4) )>>8 ) + (target>>1) )/target;
	
	/*** Bounding of QP variation ***/	
	qp = MIN3( (last_qp*5+3)/4, qp, max_Qp);
	qp = MAX3( (last_qp+1)/2  , qp, min_Qp);
	
	return qp;
}
//
int rch_get_last_qp(RC_HIST *rch)
{

	if (rch->ptr == 0)
		return rch->qp[rch->n-1];
	else
		return rch->qp[rch->ptr-1];
}
//
int rch_get_last_bits_vop(RC_HIST *rch)
{

	if (rch->ptr == 0)
		return rch->bits_vop[rch->n-1];
	else
		return rch->bits_vop[rch->ptr-1];
}
//=========Initial=============//
void RCQ2_init(DEF_RATECONTROL_PARA *pRateControlParam)
{
	static const int decaytab[31]=    // *1024
	{
	     0, //0
		 0,   512,   683,   768,   819,   
       853,   878,   896,   910,   922,   
       931,   939,   945,   951,   956,   
       960,   964,   967,   970,   973,   
       975,   977,   979,   981,   983,
       985,   986,   987,   989,   990	
	};
	int qp_first[3];
	VolConfig RVOL_Config;

    //======================================//
    //Lsk 090609 : default value
    //pRateControlParam->enable_ratecontrol=1;       //0  //Rate Control enable
	pRateControlParam->rc_period=30;

	if(pRateControlParam->TargetBitrate == 0)
		pRateControlParam->TargetBitrate=1950*1000;     

	if(pRateControlParam->Framerate == 0)
	{
    	if(sysTVinFormat == TV_IN_NTSC)
		   pRateControlParam->Framerate=30;               //Target Framerate
    	else
	       pRateControlParam->Framerate=25;               //Target Framerate
	}

    if(pRateControlParam->QP_I == 0)
	  pRateControlParam->QP_I=RC_MIN_QP+5;                     // 1 - 31

    if(pRateControlParam->QP_P == 0)
	  pRateControlParam->QP_P=RC_MIN_QP+5;                     // 1 - 31

    if(pRateControlParam->InitQP == 0)
	  pRateControlParam->InitQP=RC_MIN_QP+5;   

    if(pRateControlParam->max_Qp==0)
       pRateControlParam->max_Qp=RC_MAX_QP;

    if(pRateControlParam->min_Qp==0)
       pRateControlParam->min_Qp=RC_MIN_QP;
	

	
    RVOL_Config.frame_rate=pRateControlParam->Framerate;         //Frame Rate
	RVOL_Config.bit_rate=pRateControlParam->TargetBitrate;       //Bitrate 
	RVOL_Config.frame_skip=1;                                    // 1 is no skip.
	RVOL_Config.quantizer=pRateControlParam->QP_P;               // QP of P frame.
    RVOL_Config.intra_quantizer=pRateControlParam->QP_I;         // QP of I frame.

	qp_first[0] = pRateControlParam->QP_I;
	qp_first[1] = pRateControlParam->QP_P;
	qp_first[2] = 1;

    rcQ2_init(&pRateControlParam->RCQ2_config, &RVOL_Config, qp_first);
	rch_init(pRateControlParam->rc_hist, MAX_SLIDING_WINDOW);
	// test RC - adamli
	pRateControlParam->RCQ2_config.decay = decaytab[pRateControlParam->rc_period];
	pRateControlParam->RCQ2_config.rc_period = pRateControlParam->rc_period;
	pRateControlParam->RCQ2_config.target_rate = (int)(pRateControlParam->TargetBitrate / pRateControlParam->Framerate);
	pRateControlParam->rc_hist[0].rate_history = 0;
	pRateControlParam->rc_hist[0].history_length = 0;

}



void rch_init(RC_HIST *rch,int size)
{
	int i;
	for(i=0; i<3; i++)
	{
		rch[i].size=size;
		rch[i].n=0;
		rch[i].ptr=0;		
	}
}
//
void rcQ2_init(
                 RCQ2_DATA  *rcd,
                 VolConfig *vol_config,
                 int *qp_first	/* QP to use the first time RC is employed */
              )
{
	int   i, frame_inc;
	//int frame_rate;

	//frame_rate = vol_config->frame_rate;   
	frame_inc  = vol_config->frame_skip;          

	rcd->bit_rate = vol_config->bit_rate;  

	/* Initialization of model parameters: */
	for (i=0; i<3; i++)
	{
		rcd->X1[i] = rcd->bit_rate*frame_inc/2;
		/* Initial value specified in VM,
		but only used if the first model
		updating is delayed (by setting
		START_MODEL_UPDATING > 1) */
	}
	for (i=0; i<3; i++)
		rcd->QPfirst[i] = qp_first[i];
}
//
