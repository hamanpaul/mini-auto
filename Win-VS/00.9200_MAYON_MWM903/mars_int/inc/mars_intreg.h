

#ifndef __MARS_INTREG_H__
#define __MARS_INTREG_H__

#include "board.h"

#define REG_INT_BASE           IntCtrlBase
#define REG_INTFIQINPUT        (REG_INT_BASE)
#define REG_INTFIQMASK         (REG_INT_BASE + 0x0004)
#define REG_INTIRQINPUT        (REG_INT_BASE + 0x000c)
#define REG_INTIRQMASK         (REG_INT_BASE + 0x0010)

//=================================================================

#endif    // __MARS_INTREG_H__
