/* * 
 *  $Id: get_SDP_descr.c 133 2005-05-09 17:35:14Z federico $
 *  
 *  This file is part of Fenice
 *
 *  Fenice -- Open Media Server
 *
 *  Copyright (C) 2004 by
 *  	
 *	- Giampaolo Mancini	<giampaolo.mancini@polito.it>
 *	- Francesco Varano	<francesco.varano@polito.it>
 *	- Marco Penno		<marco.penno@polito.it>
 *	- Federico Ridolfo	<federico.ridolfo@polito.it>
 *	- Eugenio Menegatti 	<m.eu@libero.it>
 *	- Stefano Cau
 *	- Giuliano Emma
 *	- Stefano Oldrini
 * 
 *  Fenice is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Fenice is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Fenice; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 * */
#include "general.h" 
#include	"../LwIP_2.0/include/lwip/rtp.h"
#include	"../LwIP_2.0/include/lwip/rtsp.h"
#include	"../LwIP_2.0/include/lwip/sdp.h"
#include "Mpeg4api.h"


typedef struct _VIDEO_DESCRIPTION
{
    media_type mtype;
    int  port;
    int  payload_type;    
    char encoding_name[11];
    int  clock_rate;
    int  profile_level_id;
    char  *config;
} VIDEO_DESCRIPTION;   


typedef struct _AUDIO_DESCRIPTION
{
    media_type mtype;
    int  port;
    int  payload_type;    
    char encoding_name[11];
    int  clock_rate;    
} AUDIO_DESCRIPTION;  

extern u8  my_ipaddr[];

int get_MPEG4_descr(char* descr,int UseMpeg_Q)
{
    VIDEO_DESCRIPTION video_descr;
    char t[64];
    
    unsigned char MPEG4_config[0x1d] =    
    {
        0x00, 0x00, 0x01, 0xB0,
        0x03, 0x00, 0x00, 0x01,
        0xB5, 0x09, 
        0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x01, 0x20,
        0x00, 0xc4, 0x88, 0xba,
        0x98, 0x50, 0x00, 0x40,
        0x01, 0x44, 0x3f
    };

    char config[64]={0};
    char char_set[0x10] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
        
    int i;
    
    MPEG4_config[0x17] |= (unsigned char)(mpeg4Width >> 9); 
    MPEG4_config[0x18] |= (unsigned char)(mpeg4Width >> 1);     
    MPEG4_config[0x19] |= (unsigned char)(mpeg4Width << 7);     
    MPEG4_config[0x19] |= (unsigned char)(mpeg4Height >> 7);
    MPEG4_config[0x1A] |= (unsigned char)(mpeg4Height << 1);

    if(UseMpeg_Q)
    {
       MPEG4_config[0x1b]=0x49;
       MPEG4_config[0x1c]=0x0f;  
    }
    else
    {
       MPEG4_config[0x1b]=0x44;
       MPEG4_config[0x1c]=0x3f;  
    }

    for(i=0; i<0x1d; i++)
    {
        config[i*2]   =  char_set[(MPEG4_config[i]>>4)];
        config[i*2+1] =  char_set[(MPEG4_config[i]& 0x0F)];  
    }
    
    video_descr.mtype = video;
    video_descr.port  = 0; //Lsk ?
    video_descr.payload_type = PT_FOR_MPEG4; //Lsk ?
    sprintf(video_descr.encoding_name, "%s", "MP4V-ES");
    video_descr.clock_rate = 90000; //Lsk?
    video_descr.profile_level_id = 3;
    video_descr.config = config;

    /************** MPEG4-ES ********************
    m=video 49170/2 RTP/AVP 98
    a=rtpmap:98 MP4V-ES/90000
    a=fmtp:98 profile-level-id=3;config=
    *********************************************/
    strcat(descr,"m=");
    strcat(descr,"video ");
    sprintf(t, "%d ", video_descr.port);
    strcat(descr,t);
    strcat(descr,"RTP/AVP "); // Use UDP
    sprintf(t, "%d", video_descr.payload_type);
    strcat(descr,t);
    strcat(descr, SDP_EL);
    
    /*** bandwidth ***/    
    strcat(descr,"b=RR:0");
    strcat(descr, SDP_EL);
    
    strcat(descr,"a=rtpmap:");
    sprintf(t, "%d ", video_descr.payload_type);
    strcat(descr,t);
    strcat(descr,video_descr.encoding_name);
    strcat(descr,"/");
    sprintf(t, "%u", video_descr.clock_rate);
    strcat(descr,t);
    strcat(descr, SDP_EL);

    strcat(descr,"a=fmtp:");
    sprintf(t, "%d ", video_descr.payload_type);
    strcat(descr,t);
    strcat(descr,"profile-level-id=");
    sprintf(t, "%d; ", video_descr.profile_level_id);
    strcat(descr,t);
    strcat(descr,"config=");
    strcat(descr,video_descr.config);    
    strcat(descr, ";"SDP_EL);

    //strcat(descr,"a=framesize:");
    //sprintf(t, "%d ", video_descr.payload_type);
    //strcat(descr,"640-480"SDP_EL);    
    strcat(descr, "a=control:trackID=1"SDP_EL);        
    return 0;
}   
 

int get_PCM_descr(char* descr)
{
    AUDIO_DESCRIPTION audio_descr;
    char t[64];
    
    audio_descr.mtype = audio;
    audio_descr.port  = 0; //Lsk ?
    audio_descr.payload_type = PT_FOR_PCM_16K; //Lsk ?
    sprintf(audio_descr.encoding_name, "%s", "L8");
    audio_descr.clock_rate = 16000; //Lsk?

    /*********** PCM 8bit 16KByte *****************
    m=audio 49230 RTP/AVP 96
    a=rtpmap:96 L8/16000
    *********************************************/
    strcat(descr,"m=");
    strcat(descr,"audio ");
    sprintf(t, "%d ", audio_descr.port);
    strcat(descr,t);
    strcat(descr,"RTP/AVP "); // Use UDP
    sprintf(t, "%d", audio_descr.payload_type);
    strcat(descr,t);
    strcat(descr, SDP_EL);
    
    strcat(descr,"a=rtpmap:");
    sprintf(t, "%d ", audio_descr.payload_type);
    strcat(descr,t);
    strcat(descr,audio_descr.encoding_name);
    strcat(descr,"/");
    sprintf(t, "%u", audio_descr.clock_rate);
    strcat(descr,t);
    strcat(descr, SDP_EL);
    strcat(descr, "a=control:trackID=2"SDP_EL);
    return 0;
}
    
/*get media description, set each attributes in  buffer descr;yanf*/

int get_SDP_descr(char *descr,char *url)
{   
    /*** Protocol Version ***/
    strcpy(descr, "v=0"SDP_EL);
    strcat(descr, "o=- 1 1 IN IP4 127.0.0.1"SDP_EL);
    /*** Session name ***/    
    strcat(descr, "s=RTSP Server"SDP_EL); 
    /*** Connection Data ***/
    strcat(descr, "c=");
    strcat(descr, "IN ");       /* Network type: Internet. */
    strcat(descr, "IP4 ");      /* Address type: IP4. */
    strcat(descr, DEFAULT_MULTICAST_ADDRESS);
    strcat(descr, SDP_EL);	
    /*** bandwidth ***/    
    //strcat(descr, "b=AS:1090"SDP_EL);
    /*** Timing ***/    
    strcat(descr, "t=0 0"SDP_EL);
    /*** Attribute ***/    
    sprintf(descr + strlen(descr), "a=control:rtsp://%d.%d.%d.%d/"SDP_EL,my_ipaddr[0],my_ipaddr[1],my_ipaddr[2],my_ipaddr[3]);
    //strcat(descr, "a=type:broadcast"SDP_EL);
    /***  media specific ***/
    get_MPEG4_descr(descr,0);
    get_PCM_descr(descr);
    return ERR_NOERROR;
}

