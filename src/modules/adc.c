// Module for interfacing with ADC

#include "interpreter.h"
#include "platform.h"
#include "common.h"
#include "picoc_mod.h"
#include "platform_conf.h"
#include "picoc_adc.h"
#include "rotable.h"

#ifdef BUILD_ADC

/* picoc: max = adc_maxval(id) */
static void adc_maxval(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	u32 res, id;
	id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(adc, id);
	res = platform_adc_op(id, PLATFORM_ADC_GET_MAXVAL, 0);
	ReturnValue->Val->UnsignedInteger = res;
}

/* picoc: realclock = adc_setclock(id, clock, [timer_id]) */
static void adc_setclock(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	u32 clock;
	unsigned id, timer_id = 0;

	id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(adc, id);

	clock = Param[1]->Val->UnsignedInteger;
	if (clock > 0) {
		timer_id = Param[2]->Val->UnsignedInteger;
		MOD_CHECK_ID(timer, timer_id);
		MOD_CHECK_RES_ID(adc, id, timer, timer_id);
	}

	platform_adc_op(id, PLATFORM_ADC_OP_SET_TIMER, timer_id);
	clock = platform_adc_op(id, PLATFORM_ADC_OP_SET_CLOCK, clock);
	ReturnValue->Val->UnsignedInteger = clock;
}

/* picoc: data = adc_isdone(id) */
static void adc_isdone(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id;
	id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(adc, id);
	ReturnValue->Val->UnsignedInteger = platform_adc_op(id, PLATFORM_ADC_IS_DONE, 0);
}

/* picoc: adc_setblocking(id, mode) */
static void adc_setblocking(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id, mode;
	id = Param[0]->Val->UnsignedInteger;  
	MOD_CHECK_ID(adc, id);
	mode = Param[1]->Val->UnsignedInteger;
  	ReturnValue->Val->UnsignedInteger = platform_adc_op(id, PLATFORM_ADC_SET_BLOCKING, mode);
}

/* picoc: adc_setsmoothing(id, length) */ 
static void adc_setsmoothing(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id, length, res;
	id = Param[0]->Val->UnsignedInteger;

  	MOD_CHECK_ID(adc, id);
	length = Param[1]->Val->UnsignedInteger;
  
  	if (!(length & (length - 1))) {
    		res = platform_adc_op(id, PLATFORM_ADC_SET_SMOOTHING, length);
    		if (res == PLATFORM_ERR) {
			ReturnValue->Val->Integer = 0;
			return ProgramFail(NULL, "Buffer allocation failed.");	
		} else {
      			ReturnValue->Val->UnsignedInteger = res;
		}
  	} else {
		ReturnValue->Val->Integer = 0;
		return ProgramFail(NULL, "Length must be power of 2");
	}
}

/* picoc: adc_sample(id, count) */
static void adc_sample(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id, count;
  	int res;
	count = Param[1]->Val->UnsignedInteger;
  
  	if ((count == 0 ) || count & (count - 1)) {
		ProgramFail(NULL, "count must be power of 2 and > 0");
		return;
	}
	
	id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(adc, id);
	res = adc_setup_channel(id, intlog2(count));

	if (res != PLATFORM_OK) {
		ProgramFail(NULL, "sampling setup failed");
		return;
	}
	platform_adc_start_sequence();	

	ReturnValue->Val->Integer = res;
}
		
  
/* picoc: val = adc_getsample(id) */
static void adc_getsample(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id;
	id = Param[0]->Val->UnsignedInteger;
  	MOD_CHECK_ID( adc, id );
  
  	/* If we have at least one sample, return it */
  	if (adc_wait_samples(id, 1) >= 1) {
		ReturnValue->Val->UnsignedInteger = adc_get_processed_sample(id);
		return;
  	}
  	ReturnValue->Val->UnsignedInteger = 0;
}

#if defined (BUF_ENABLE_ADC)
/* picoc: adc_getsamples(id, [count], array) */
static void adc_getsamples(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	int id, i, *arr;
	u16 bcnt, count = 0;
	
	id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(adc, id);
	
	count = (u16)Param[1]->Val->UnsignedInteger;
	arr = (int*)Param[2]->Val->Pointer;

  	bcnt = adc_wait_samples(id, count);
  
	/* If count is zero, grab all samples */
  	if (count == 0)
    		count = bcnt;
  
  	/* Don't pull more samples than are available */
  	if (count > bcnt)
    		count = bcnt;

	for (i = 1; i <= count; i++)
		arr[i] = adc_get_processed_sample(id);	
  
	ReturnValue->Val->UnsignedInteger = count;
}
#endif

#define MIN_OPT_LEVEL 2
#include "rodefs.h"

/* list of all library functions and their prototypes */
const PICOC_REG_TYPE adc_library[] = {
        {FUNC(adc_maxval), PROTO("unsigned int adc_maxval(unsigned int);")},
        {FUNC(adc_setclock), PROTO("unsigned int adc_setclock(unsigned int, unsigned int, unsigned int);")},
        {FUNC(adc_isdone), PROTO("unsigned int adc_isdone(unsigned int);")},
        {FUNC(adc_setblocking), PROTO("unsigned int adc_setblocking(unsigned int, unsigned int);")},
        {FUNC(adc_setsmoothing), PROTO("unsigned int adc_setsmoothing(unsigned int, unsigned int);")},
	{FUNC(adc_sample), PROTO("int adc_sample(unsigned int, unsigned int);")},
	{FUNC(adc_getsample), PROTO("unsigned int adc_getsample(unsigned int);")},
#if defined (BUF_ENABLE_ADC)
	{FUNC(adc_getsamples), PROTO("unsigned int adc_getsamples(unsigned int, unsigned int, int *);")},
#endif
	{NILFUNC, NILPROTO}
};

/* init library */
extern void adc_library_init(void)
{
	REGISTER("adc.h", NULL, &adc_library[0]);
}

#endif /* #ifdef BUILD_ADC */

