/* Module for interfacing with CPU */

#include "platform.h"
#include "picoc_mod.h"
#include "rotable.h"
#include <string.h> 

#define _CPU_CONSTANT(x) {#x, x}
#include "platform_conf.h"

/* picoc: cpu_w32(address, data) */
static void cpu_w32(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	u32 addr = Param[0]->Val->UnsignedInteger;
	u32 data = Param[1]->Val->UnsignedInteger;
	*(u32 *)addr = data;
	ReturnValue->Val->Integer = 0;
}

/* picoc: data = cpu_r32(address) */
static void cpu_r32(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	u32 addr = Param[0]->Val->UnsignedInteger;
	ReturnValue->Val->UnsignedInteger = *(u32 *)addr;
}

/* picoc: cpu_w16(address, data) */
static void cpu_w16(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	u32 addr = Param[0]->Val->UnsignedInteger;
	u16 data = Param[1]->Val->UnsignedShortInteger;
	*(u16 *)addr = data;
	ReturnValue->Val->Integer = 0;
}

/* picoc: data = cpu_r16(address) */
static void cpu_r16(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	u32 addr = Param[0]->Val->UnsignedInteger;
	ReturnValue->Val->UnsignedShortInteger = *(u16 *)addr;
}

/* picoc: cpu_w8(address, data) */
static void cpu_w8(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	u32 addr = Param[0]->Val->UnsignedInteger;
	u8 data = Param[1]->Val->Character;
	*(u8 *)addr = data;
	ReturnValue->Val->Integer = 0;
}

/* picoc: data = cpu_r8(address) */
static void cpu_r8(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	u32 addr = Param[0]->Val->UnsignedInteger;
	ReturnValue->Val->UnsignedShortInteger = *(u8 *)addr;
}

/* picoc: frequency = cpu_clock() */
static void cpu_clock(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	ReturnValue->Val->UnsignedInteger = platform_cpu_get_frequency();
}

/* CPU constants list */
typedef struct {
	const char *name;
  	u32 val;
} cpu_const_t;

#ifdef PLATFORM_CPU_CONSTANTS

static const cpu_const_t cpu_constants[] = {
	PLATFORM_CPU_CONSTANTS,
  	{NULL, 0}
};

/**
 * returns the interrupt assignment values.
 * For use in future when interrupts are
 * supported.
 */
static int cpu_const_helper(char *key)
{
  	unsigned i = 0;
  	while(cpu_constants[i].name != NULL) {
    		if (!strcmp(cpu_constants[i].name, key))
			return cpu_constants[i].val;
    		i++;
  	}
  	return 0;
}

static void cpu_const_getval(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	ReturnValue->Val->UnsignedInteger = cpu_const_helper(Param[0]->Val->Identifier);
}

#endif /* #ifdef PLATFORM_CPU_CONSTANTS */

#define MIN_OPT_LEVEL 2
#include "rodefs.h"

/* list of all library functions and their prototypes */
const PICOC_REG_TYPE cpu_library[] = {
        /* pin functions */
        {FUNC(cpu_w32), PROTO("int cpu_w32(unsigned int, unsigned int);")},
        {FUNC(cpu_r32), PROTO("unsigned int cpu_r32(unsigned int);")},
        {FUNC(cpu_w16), PROTO("int cpu_w16(unsigned int, unsigned short);")},
        {FUNC(cpu_r16), PROTO("unsigned short cpu_r16(unsigned int);")},
        {FUNC(cpu_w8), PROTO("int cpu_w8(unsigned int, char);")},
        {FUNC(cpu_r8), PROTO("unsigned short cpu_r8(unsigned int);")},
	{FUNC(cpu_clock), PROTO("unsigned int cpu_clock(void);")},
#ifdef PLATFORM_CPU_CONSTANTS
        {FUNC(cpu_const_getval), PROTO("unsigned int cpu_const_getval(char *);")},
#endif
        {NILFUNC, NILPROTO}
};

/* init library */
extern void cpu_library_init(void)
{
        REGISTER("cpu.h", NULL, &cpu_library[0]);
}

