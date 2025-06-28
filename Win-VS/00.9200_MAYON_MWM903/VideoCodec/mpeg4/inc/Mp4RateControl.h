#ifndef __MP4RATECONTROL_H__
#define __MP4RATECONTROL_H__





#define TX_BUFFCNTL_THR_FHD     120
#define TX_BUFFCNTL_THR_FHD_LOW 120


#define TX_BUFFCNTL_THR_HD     75
#define TX_BUFFCNTL_THR_HD_LOW 35

#define TX_BUFFCNTL_THR_QHD  20

#define MAX_QP_P_EXT           8
#define MAX_QP_P_EXT2          9
#define MAX_QP_I_EXT           16

#if 1
/*
  #define RC_MAX_QP_I    34
  #define RC_MAX_QP_SUB  35

  #define RC_MAX_QP      35
  #define RC_MIN_QP      24
 */
 

#if(RATE_CONTROL_MODE == BALANCE_MODE)
  #define RC_MAX_QP_I    34
  #define RC_MIN_QP_I    28
  #define RC_MAX_QP_SUB  35

  #define RC_MAX_QP      34
  #define RC_MIN_QP      24 
#elif(RATE_CONTROL_MODE == SMOOTHFRAME_MODE)

 #if (Sensor_OPTION == Sensor_ZN220_YUV601) //ZN picture is higher quality
  #define RC_MAX_QP_I    36
  #define RC_MIN_QP_I    28
  #define RC_MAX_QP_SUB  36

  #define RC_MAX_QP      36
  #define RC_MIN_QP      24
#else   // other sensor
  #define RC_MAX_QP_I    36
  #define RC_MIN_QP_I    28
  #define RC_MAX_QP_SUB  38

  #define RC_MAX_QP      36
  #define RC_MIN_QP      24   
 #endif
 
#else   //Quality mode
  #define RC_MAX_QP_I    34
  #define RC_MIN_QP_I    28
  #define RC_MAX_QP_SUB  35

  #define RC_MAX_QP      34
  #define RC_MIN_QP      24   
#endif
  
#else
  #define RC_MAX_QP_I 24

  #define RC_MAX_QP   24
  #define RC_MIN_QP   24
#endif

#define DROP_FRAME_INTERVAL_LEVEL 6
#define START_DROP_FRAME_TRHESHOLD (RC_MAX_QP - DROP_FRAME_INTERVAL_LEVEL)  // DROP INTERVAL MIN = 2; drop 1 frame for every 2 frames

//#define PAST_PERCENT           0.05
#define START_MODEL_UPDATING   1
#define MAX_SLIDING_WINDOW     20 //


#define  MIN(a,b)              (((a) < (b)) ? (a) : (b))
#define  MAX3(x,y,z)      	  MAX(MAX(x,y),z)
#define  MIN3(x,y,z)      	  MIN(MIN(x,y),z)
#define  MAX(a,b)              (((a) > (b)) ? (a) : (b))



typedef struct 
{
	int	    frame_rate;			/* VOL frame rate */
	int	    bit_rate;			/* Target bitrate */
	int 	frame_skip;			/* Number of frames to skip between codings */
	int	    quantizer;			/* Initial value for H.263 INTER quantizer */
	int	    intra_quantizer;	/* Initial value for H.263 INTRA quantizer */
} VolConfig;

typedef struct
{
   int             size;            /* Storage capacity */      
   int             n;               /* Number of actually stored values */
   int             ptr;             /* Position where next data are going to be stored */						
   unsigned int    bits_text[20];   /* Bits for texture */
   int    Int_mad_text[20];
   int    bits_vop[20];    /* Total bits */ 
   unsigned int    qp[20];          /* Employed QP */						
   int   rate_history;    /* Record the pass rate*/
   int    history_length;  /* frame index */
} RC_HIST;
//
typedef struct
{
   int     bit_rate;    /* in bits/second */
   int     VBV_size;    /* specify VBV Buffer Size */
   int     VBV_fullness;/* buffer occupancy */
   int     X1[3];       /* second order complexity measure */
   int     QPfirst[3];  /* Initial QP*/
   int     decay;       /* = 1- 1/rc_period */
   int     rc_period;   /* Rate control operation interval*/
   int	   target_rate; /* =bit_rate/framerate */ 
} RCQ2_DATA;
typedef struct 
{
    short QP_I;
	short QP_P;
	int TargetBitrate;
	int Framerate;
	int enable_ratecontrol;
	int max_Qp;
	int min_Qp;
	int rc_period;   
	int InitQP;
	
    RC_HIST   rc_hist[3];        //for I,P,B
	RCQ2_DATA RCQ2_config;
	int Int_prev_PMad;
    int Sum_PMad;
    int Avg_PMad;
    int PMad_cnt;
} DEF_RATECONTROL_PARA;
//========= Function =============//
void RCQ2_Update2OrderModel(
							   DEF_RATECONTROL_PARA *pRateControlParam,
							   int vopbits, /* Total number of bits */
							   int frame_type                               
						   );

int rch_get_last_mad_text( RC_HIST *rch);
int rch_dec_mod(RC_HIST *rch,int i);
int rch_get_plast_mad_text(RC_HIST *rch);
void rch_store_after(
						RC_HIST      *rch,
						int          bits_text,
						int          bits_vop
					);
void rc_update_Q2_model(
							RC_HIST *rch,
							int  *X1 /* <-> Old and computed params. */
						);
void rch_store_before(
							RC_HIST *rch,
							int Int_mad,
							int      qp
                      );
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
					  );

int RCQ2_QuantAdjust(
					    DEF_RATECONTROL_PARA *pRateControlParam,
						int Int_mad,
						int    frame_type
					 );
int rch_get_last_qp(RC_HIST *rch);
int rch_get_last_bits_vop(RC_HIST *rch);
//
void rch_init(RC_HIST *rch,int size);
void rcQ2_init(
                 RCQ2_DATA  *rcd,
                 VolConfig *vol_config,
                 int *qp_first	        /* QP to use the first time RC is employed */
              );


#endif

