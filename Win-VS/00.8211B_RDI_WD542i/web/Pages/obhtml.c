/* Created with PageBuilder version 4.2 on Mon Apr 26 10:34:05 2004 */

#include "AsExtern.h"
#include "Web_flag.h"

#if RomPagerServer

extern rpObjectDescription Pgstart;
extern rpObjectDescription PgjsComm;
extern rpObjectDescription Pgua;
extern rpObjectDescription Pgftiens4;
extern rpObjectDescription Pgweb_style;
extern rpObjectDescription PgHlogo;
extern rpObjectDescription PgHmain;
extern rpObjectDescription PgMntLogout;
extern rpObjectDescription PgMntLogout_Form_1;
extern rpObjectDescription PgMessageWindow;
extern rpObjectDescription Pgmenu;
extern rpObjectDescription PgRecSetting;
extern rpObjectDescription PgRecSetting_Form_1;
extern rpObjectDescription PgDisplaySetting;
extern rpObjectDescription PgDisplaySetting_Form_1;
extern rpObjectDescription PgImageSetting;
extern rpObjectDescription PgImageSetting_Form_1;

rpObjectDescriptionPtr gRpObjectList_obhtml[] = {
	&Pgstart,
	&PgjsComm,
	&Pgua,
	&Pgftiens4,
	&Pgweb_style,
	&PgHlogo,
	&PgHmain,
	&PgMntLogout,
	&PgMntLogout_Form_1,
    &PgMessageWindow,
    &Pgmenu,
	&PgRecSetting,
	&PgRecSetting_Form_1,
	&PgDisplaySetting,
	&PgDisplaySetting_Form_1,
	&PgImageSetting,
	&PgImageSetting_Form_1,
	(rpObjectDescriptionPtr) 0
};
#endif	/* RomPagerServer */
