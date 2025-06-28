#ifndef __AWBDEF_H__
#define __AWBDEF_H__



typedef struct _sensorinfo
 {
    unsigned short EL;
    unsigned char AG;
    unsigned char DG;    
 }DEF_SENSORINFO;
 
 typedef struct _siustatus
 {
    unsigned int SIU_CTL1;
    unsigned int OB_COMP;    
 }DEF_SIUSTATUS;
 
 typedef struct _aereport
 {
    unsigned int ae_YavgValue;
    int ae_logYdiff;
    unsigned short AECurSet;
    unsigned short EL;
    unsigned char ConvergeCount;    
    unsigned char AG;
    unsigned char MUL;
    unsigned char DG;    
    unsigned char AEwtab[28];
 }DEF_AE_REPORT;
 
 typedef struct _appendixinfo
 {
    unsigned short APP2Marker;
    unsigned short APP2Size;
        
    unsigned char ModelName[64];
    DEF_SENSORINFO sensorinfo;
    DEF_SIUSTATUS siu_status;
    DEF_AE_REPORT ae_report;
    
    unsigned char AwbImg[160*120];
    short CCM[16];
    unsigned short aeWinYsum[25]; 
    unsigned char AeWeightTab[25];
    
    short AWBgain_Preview[3];
    short AWBgain_Capture[3];
    unsigned char LightFeq;
    unsigned short GammaTabIdx;
   
    unsigned char siuY_TargetIndex;
    unsigned char AE_Y_Target;
    unsigned int siu_DGT_GAIN1;
    unsigned int siu_DGT_GAIN2;
   
 }DEF_APPENDIXINFO;


#endif
