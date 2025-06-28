

#ifndef __MARS_INT_H__
#define __MARS_INT_H__

#include	<osapi.h>
#ifndef FP_VOID
typedef void   (*FP_VOID)(void);    // Function Point
#endif


//=================================================================
extern void marsIntInit(void);
extern void marsIntIRQDefIsr(INT32U dintno, FP_VOID hdl);
extern void marsIntFIQDefIsr(INT32U dintno, FP_VOID hdl);
extern void marsIntIRQEnable(INT32U intno);
extern void marsIntIRQDisable(INT32U intno);
extern void marsIntFIQEnable(INT32U intno);
extern void marsIntFIQDisable(INT32U intno);
//=================================================================

#endif    // __MARS_INT_H__
