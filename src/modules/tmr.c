// Module for interfacing with timers

#include "platform.h"
#include "picoc_mod.h"
#include "platform_conf.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rotable.h"

#define MAX_VTIMER_NAME_LEN     6
#define MIN_VTIMER_NAME_LEN     5

/* picoc: tmr_delay(id, period) */
static void tmr_delay(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id, period;
	id = Param[0]->Val->UnsignedInteger;
  	MOD_CHECK_ID(timer, id);

	period = Param[1]->Val->UnsignedInteger;
  	platform_timer_delay(id, period);
	ReturnValue->Val->Integer = 0;
}

/* picoc: tmr_timervalue = read(id) */
static void tmr_read(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(timer, id);

	ReturnValue->Val->UnsignedInteger = platform_timer_op(id, PLATFORM_TIMER_OP_READ, 0);
}

/* picoc: timervalue = tmr_start(id) */
static void tmr_start(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(timer, id);

	ReturnValue->Val->UnsignedInteger = platform_timer_op(id, PLATFORM_TIMER_OP_START, 0);
}

/* picoc: time_us = tmr_gettimediff(id, end, start) */
static void tmr_gettimediff(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	timer_data_type end, start;
  	u32 res;
  	unsigned id;

	id = Param[0]->Val->UnsignedInteger;
  	MOD_CHECK_ID(timer, id);

	end = Param[1]->Val->UnsignedInteger;
	start = Param[2]->Val->UnsignedInteger;
	res = platform_timer_get_diff_us(id, end, start);
	ReturnValue->Val->UnsignedInteger = res;
}

/* picoc: res = tmr_getmindelay(id) */
static void tmr_getmindelay(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	u32 res;
	unsigned id;

	id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(timer, id);

	res = platform_timer_op(id, PLATFORM_TIMER_OP_GET_MIN_DELAY, 0);
	ReturnValue->Val->UnsignedInteger = res;
}

/* picoc: res = tmr_getmaxdelay(id) */
static void tmr_getmaxdelay(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	u32 res;
	unsigned id;
  
	id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(timer, id);

	res = platform_timer_op(id, PLATFORM_TIMER_OP_GET_MAX_DELAY, 0);
	ReturnValue->Val->UnsignedInteger = res;
}

/* picoc: realclock = tmr_setclock(id, clock) */
static void tmr_setclock(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	u32 clock;
	unsigned id;

	id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(timer, id);
	clock = Param[1]->Val->UnsignedInteger;
	clock = platform_timer_op(id, PLATFORM_TIMER_OP_SET_CLOCK, clock);
	ReturnValue->Val->UnsignedInteger = clock;
}

/* picoc: clock = tmr_getclock(id) */
static void tmr_getclock(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	u32 res;
	unsigned id;

	id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(timer, id);

	res = platform_timer_op(id, PLATFORM_TIMER_OP_GET_CLOCK, 0);
	ReturnValue->Val->UnsignedInteger = res;
}

#ifdef BUILD_PICOC_INT_HANDLERS
/* picoc: tmr_set_match_int(id, timeout, type) */
static void tmr_set_match_int(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id;
	u32 res;

	id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(timer, id);

	res = platform_timer_set_match_int(id, Param[1]->Val->UnsignedInteger, ParamValue[2]->Val->Integer);
	if (res == PLATFORM_TIMER_INT_TOO_SHORT)
		return ProgramFail(NULL, "timer interval too small");
  	else if (res == PLATFORM_TIMER_INT_INVALID_ID)
		return ProgramFail(NULL, "mach interrupt cannot be set on this timer");
  	ReturnValue->Val->Integer = 0;
}
#endif /* #ifdef BUILD_PICOC_INT_HANDLERS */

#if VTMR_NUM_TIMERS > 0
/* Look for all VIRTx timer identifiers */
static void tmr_decode(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	const char *key = Param[0]->Val->Identifier;
  	char* pend;
  	long res;
  
  	if (strlen(key) > MAX_VTIMER_NAME_LEN || strlen(key) < MIN_VTIMER_NAME_LEN) {
    		ReturnValue->Val->Integer = 0;
		return;
	}
  	if (strncmp(key, "VIRT", 4)) {
    		ReturnValue->Val->Integer = 0;
		return;
	}
  	res = strtol(key + 4, &pend, 10);
  	if (*pend != '\0') {
		ReturnValue->Val->Integer = 0;
    		return;
	}
  	if (res >= VTMR_NUM_TIMERS) {
		ReturnValue->Val->Integer = 0;
		return;
	}
	ReturnValue->Val->LongInteger = VTMR_FIRST_ID + res;
}
#endif /* VTMR_NUM_TIMERS > 0 */

#define MIN_OPT_LEVEL 2
#include "rodefs.h"

/* list of all library functions and their prototypes */
const PICOC_REG_TYPE tmr_library[] =
{
        {FUNC(tmr_delay), PROTO("int tmr_delay(unsigned int, unsigned int);")},
        {FUNC(tmr_read), PROTO("unsigned int tmr_read(unsigned int);")},
        {FUNC(tmr_start), PROTO("unsigned int tmr_start(unsigned int);")},
        {FUNC(tmr_gettimediff), PROTO("unsigned int tmr_gettimediff(unsigned int, unsigned int, unsigned int);")},
        {FUNC(tmr_getmindelay), PROTO("unsigned int tmr_getmindelay(unsigned int);")},
	{FUNC(tmr_getmaxdelay), PROTO("unsigned int tmr_getmaxdelay(unsigned int);")},
	{FUNC(tmr_setclock), PROTO("unsigned int tmr_setclock(unsigned int, unsigned int);")},
	{FUNC(tmr_getclock), PROTO("unsigned int tmr_getclock(unsigned int);")},
#if VTMR_NUM_TIMERS > 0
	{FUNC(tmr_decode), PROTO("long tmr_decode(char *);")},
#endif /* VTMR_NUM_TIMERS > 0 */
#ifdef BUILD_PICOC_INT_HANDLERS
	{FUNC(tmr_set_match_int), PROTO("int tmr_set_match_int(unsigned int, unsigned int, int);")},
#endif /* BUILD_PICOC_INT_HANDLERS */
	{NILFUNC, NILPROTO}
};

/* init library */
extern void tmr_library_init(void)
{
	REGISTER("tmr.h", NULL, &tmr_library[0]);
}
