#include "general.h"
#include "board.h"


#define SYSTEMCLOCK_27MHZ   0
#define SYSTEMCLOCK_48MHZ   1
#define SYSTEMCLOCK_64MHZ   2

static u32 Prev_IDUdiv;
extern u8 sysTVOutOnFlag;

void EnterClock64_8mFrom48m(void)
{}

void LeaveClokc64_8mTo48m(void)
{}

void ChangeSystemClock(u8 freq)
{}
