// Module for interfacing with PWM

#include "platform.h"
#include "interpreter.h"
#include "picoc_mod.h"
#include "rotable.h"

/* picoc: realfrequency = setup(id, frequency, duty) */
static void pwm_setup(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	u32 freq;
	unsigned duty, id;
  
	id = Param[0]->Val->UnsignedInteger;
  	MOD_CHECK_ID(pwm, id);
  	freq = Param[1]->Val->UnsignedInteger;
  	duty = Param[2]->Val->UnsignedInteger;

	if (duty > 100)
    		duty = 100;

  	freq = platform_pwm_setup(id, freq, duty);
	ReturnValue->Val->Integer = freq;
}

/* picoc: start(id) */
static void pwm_start(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id;
  
	id = Param[0]->Val->UnsignedInteger;
  	MOD_CHECK_ID(pwm, id);
  	platform_pwm_op(id, PLATFORM_PWM_OP_START, 0);
  	ReturnValue->Val->Integer = 0;
}

/* picoc: stop(id) */
static void pwm_stop(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id;
  
  	id = Param[0]->Val->UnsignedInteger;
  	MOD_CHECK_ID(pwm, id);
  	platform_pwm_op(id, PLATFORM_PWM_OP_STOP, 0);
	ReturnValue->Val->Integer = 0;
}

/* picoc: realclock = setclock(id, clock) */
static void pwm_setclock(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id;
  	u32 clk;
  
	id = Param[0]->Val->UnsignedInteger;
  	MOD_CHECK_ID(pwm, id);
	clk = Param[1]->Val->UnsignedInteger;
  	clk = platform_pwm_op(id, PLATFORM_PWM_OP_SET_CLOCK, clk);
	ReturnValue->Val->UnsignedInteger = clk;
}

/* picoc: clock = getclock(id) */
static void pwm_getclock(struct ParseState *Parser, struct Value *ReturnValue,	
			struct Value **Param, int NumArgs)
{
	unsigned id;
  	u32 clk;
  
	id = Param[0]->Val->UnsignedInteger;
  	MOD_CHECK_ID(pwm, id);
  	clk = platform_pwm_op(id, PLATFORM_PWM_OP_GET_CLOCK, 0);
	ReturnValue->Val->UnsignedInteger = clk;
}

#define MIN_OPT_LEVEL 2
#include "rodefs.h"

/* list of all library functions and their prototypes */
const PICOC_REG_TYPE pwm_library[] = {
        {FUNC(pwm_setup), PROTO("unsigned int pwm_setup(unsigned int, unsigned int, unsigned int);")},
        {FUNC(pwm_start), PROTO("int pwm_start(unsigned int);")},
        {FUNC(pwm_stop), PROTO("int pwm_stop(unsigned int);")},
        {FUNC(pwm_setclock), PROTO("unsigned int pwm_setclock(unsigned int, unsigned int);")},
        {FUNC(pwm_getclock), PROTO("unsigned int pwm_getclock(unsigned int);")},
	{NILFUNC, NILPROTO}
};

/* init library */
extern void pwm_library_init(void)
{
	REGISTER("pwm.h", NULL, &pwm_library[0]);
}

