#include "AsExtern.h"

#if RomPagerServer

extern rpObjectDescription Pglogo_mars;
extern rpObjectDescription Pgclose;
extern rpObjectDescription Pgopen;

rpObjectDescriptionPtr gRpObjectList_obimage[] = {
	&Pglogo_mars,
	&Pgclose,
	&Pgopen,
	(rpObjectDescriptionPtr) 0
};

#endif	/* RomPagerServer */
