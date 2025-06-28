// --- come from MIB_CST_LINK_STATUS;

#ifdef WIN32
typedef enum {
    LINK_PASS = 2,       /* Port link up */
    LINK_FAIL            /* Port link down */
} WEB_PORT_LINK_STATUS;
#else
#include <Inc/Mib_cst.h>
typedef MIB_CST_LINK_STATUS		WEB_PORT_LINK_STATUS;
#endif

typedef struct _WEB_PORT_INFO 
{
	int index;		// port Number
	int moduleType; // module
	int nway;		// speed, 10/100M
	WEB_PORT_LINK_STATUS linkStatus;
}WEB_PORT_INFO;

typedef struct _WEB_PANEL_INFO
{	
	WEB_PORT_INFO portNode[26];
	int maxPort;
	int unit;
	int module;
	int slot; 

	int currentPort;
	int currentUnit;

} WEB_PANEL_INFO;

typedef enum {
	WEB_SIOLED_NONE=0,
	WEB_SIOLED_SIO1=1,
	WEB_SIOLED_SIO2=2,
	WEB_SIOLED_BOTH=3
}WEB_SIOLED_STATE;

typedef enum {
	WEB_SIOLED_OFF=0,
	WEB_SIOLED_ON=1
}WEB_SIOLED_3324SRi_STATE;

// get sio led status
int WEB_FPANEL_UnitSIOLedState_Get(int ,WEB_SIOLED_STATE *);

int WEB_FPANEL_UnitSIOLedState_3324SRi_Get(int ,WEB_SIOLED_STATE *);


// get console led state
int WEB_FPANEL_UnitConsoleLedState_Get(int unit);

// get RPS led state
int WEB_FPANEL_UnitRPSLedState_Get(int unit);

// get Link/ACT/Speed led state    /* Shanlu 94/06/10 */
int WEB_FPANEL_UnitLASLedState_Get(int unit);

// get PoE led state                       /* Shanlu 94/06/10 */
int WEB_FPANEL_UnitPoELedState_Get(int unit);

/* constant for swPortInfoLinkStatus */

int WEB_FPANEL_UnitPowerLedState_Get(int unit);
int fWeb_PanelPowerFanState_Get(int );//2004/6/8 05:51¤U¤È iris chu 04443 modify


