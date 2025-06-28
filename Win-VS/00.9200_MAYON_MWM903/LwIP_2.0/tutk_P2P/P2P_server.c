#include "general.h"
#include "board.h"
#if ((TUTK_SUPPORT) && !ICOMMWIFI_SUPPORT)
#include "TUTKIOTCAPI.h"
#include "TUTKAVAPI.h"
#include "AVFRAMEINFO.h"
#include "AVIOCTRLDEFs.h"
#include "gpiapi.h"
#include "lwipapi.h"
#include "Task.h"
#include "MPEG4api.h"
#include "IISapi.h"
#include "UIapi.h"
#include "uiKey.h"
#include "sysapi.h"
#include "fsapi.h"
#include "GlobalVariable.h"
#include <../rfiu/inc/rfiu.h>
#include "Dcfapi.h"
#include "timerapi.h"
#include <lwip/sockets.h>
#include "rtcapi.h"
#include "p2pserver_api.h"
#include "ispapi.h"
#include "../../ui/inc/ui.h"
#include "../../ui/inc/ui_project.h"
#include "encrptyapi.h"
#if HOME_RF_SUPPORT
#include "MR8200def_homeautomation.h"
#endif
#include "iduapi.h"
#include "out_of_range_720p_264.h"

/*
 *********************************************************************************************************
 *  SYS Constant
 *********************************************************************************************************
 */

#define FW_SERVER2	"ota.mars-semi.com.tw" //China Server

typedef struct _AudioIn
{
    int SID;
    int ch;
    int avIndex;
    unsigned long srvType;
    int dspFd;
} AudioIn;

struct hostent {
    char  *h_name;      /* Official name of the host. */
    char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                           terminated by a null pointer. */
    int    h_addrtype;  /* Address type. */
    int    h_length;    /* The length, in bytes, of the address. */
    char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                           network byte order) for the host, terminated by a null pointer. */
#define h_addr h_addr_list[0] /* for backward compatibility */
};
/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
#if APP_KEEP_ALIVE
#define KEEP_ALIVE_TIMEOUT	60

int gFlagKeepAlive[MAX_CLIENT+1] = {0}; /* 0:Default not connected; 1:APP connected*/
RTC_DATE_TIME gKeepAliveTime0;
RTC_DATE_TIME gKeepAliveTime1;
RTC_DATE_TIME gKeepAliveTime2;
RTC_DATE_TIME gKeepAliveTime3;
RTC_DATE_TIME gKeepAliveTime4;

#endif

#if UI_LIGHT_SUPPORT
extern u8 AppLightStatus;
#endif

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

//------------------------------------code--------------------------------------//



/* Convert numeric IP address into decimal dotted ASCII representation.
 * returns ptr to static buffer; not reentrant!
 */
#if 0
char * inet_ntoa(struct in_addr addr)
{
  static char str[16];
  u32_t s_addr = addr.s_addr;
  char inv[3];
  char *rp;
  u8_t *ap;
  u8_t rem;
  u8_t n;
  u8_t i;

  rp = str;
  ap = (u8_t *)&s_addr;
  for(n = 0; n < 4; n++) {
    i = 0;
    do {
      rem = *ap % (u8_t)10;
      *ap /= (u8_t)10;
      inv[i++] = '0' + rem;
    } while(*ap);
    while(i--)
      *rp++ = inv[i];
    *rp++ = '.';
    ap++;
  }
  *--rp = 0;
  return str;
}
#endif
#if (LWIP_PLATFORM_BYTESWAP == 0) && (BYTE_ORDER == LITTLE_ENDIAN)

u16_t
htons(u16_t n)
{
  return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

u16_t
ntohs(u16_t n)
{
  return htons(n);
}

u32_t
htonl(u32_t n)
{
  return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000) >> 8) |
    ((n & 0xff000000) >> 24);
}

u32_t
ntohl(u32_t n)
{
  return htonl(n);
}

#endif /* (LWIP_PLATFORM_BYTESWAP == 0) && (BYTE_ORDER == LITTLE_ENDIAN) */

#else

   //#include "arch/cc.h"
#include "TUTKIOTCAPI.h"
#include "TUTKAVAPI.h"
#include "AVFRAMEINFO.h"
#include "AVIOCTRLDEFs.h"
#include "gpiapi.h"
#include "lwipapi.h"
#include "Task.h"
#include "MPEG4api.h"
#include "IISapi.h"
#include "UIapi.h"
#include "rfiuapi.h"
#include "uiKey.h"
#include "sysapi.h"
#include "fsapi.h"
#include "GlobalVariable.h"
#include <../rfiu/inc/rfiu.h>
#include "Dcfapi.h"
#include "timerapi.h"
#include <lwip/sockets.h>
#include "rtcapi.h"
#include "p2pserver_api.h"
#include "ispapi.h"
#include "../../ui/inc/ui.h"
#include "../../ui/inc/ui_project.h"
#include "encrptyapi.h"

u8 renewIP;
int gOnlineNum = 0;
OS_EVENT* P2PAudioPlaybackCmpSemEvt;
u32 P2PPauseTime,P2PStartPauseTime,P2PStopPauseTime;
u8 Fileplaying=0;   /*for RDI 2: local playback*/
u8 * gUID;

int Get_network_status();
u8 GetFWDownProgress(s32* fw_length);
void LoadP2PPassword(char *password);
u8 Check_fw_ver_net(u8 connected);

    u16_t
    htons(u16_t n)
    {
      return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
    }

    u16_t
    ntohs(u16_t n)
    {
      return htons(n);
    }

    u32_t
    htonl(u32_t n)
    {
      return ((n & 0xff) << 24) |
        ((n & 0xff00) << 8) |
        ((n & 0xff0000) >> 8) |
        ((n & 0xff000000) >> 24);
    }

    u32_t
    ntohl(u32_t n)
    {
      return htonl(n);
    }


#endif


#if UI_LIGHT_SUPPORT

void UpdateAPPLightStatus(u8 Camid)
{
    AppLightStatus = Camid+1;
}

#endif