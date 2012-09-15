// Module for interfacing with PIO

#include "platform.h"
#include "picoc_mod.h"
#include "platform_conf.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "rotable.h"

/* PIO public constants */
#define PIO_DIR_OUTPUT      0
#define PIO_DIR_INPUT       1

/* PIO private constants */
#define PIO_PORT_OP         0
#define PIO_PIN_OP          1

/* error check */
#define PIO_CHECK(x)\
	if (!x) {\
		ReturnValue->Val->Integer = -1;\
		return ProgramFail(NULL, "Invalid port/pin");\
	}

/* Local operation masks for all the ports */
static pio_type pio_masks[PLATFORM_IO_PORTS];

/**
 * declare proto. returns pin/port
 * numeric identifiers.
 */
static int pio_value_parse(char *key);

/* platform variables */
const int Input = PIO_DIR_INPUT;
const int Output = PIO_DIR_OUTPUT;
const int PullUp = PLATFORM_IO_PIN_PULLUP;
const int PullDown = PLATFORM_IO_PIN_PULLDOWN;
const int NoPull = PLATFORM_IO_PIN_NOPULL;

/* library setup function */
extern void pio_lib_setup_func(void)
{
#if ((PICOC_OPTIMIZE_MEMORY == 0) && defined (BUILTIN_MINI_STDLIB))
        VariableDefinePlatformVar(NULL, "INPUT", &IntType, (union AnyValue *)&Input, FALSE);
        VariableDefinePlatformVar(NULL, "OUTPUT", &IntType, (union AnyValue *)&Output, FALSE);
        VariableDefinePlatformVar(NULL, "PULLUP", &IntType, (union AnyValue *)&PullUp, FALSE);
        VariableDefinePlatformVar(NULL, "PULLDOWN", &IntType, (union AnyValue *)&PullDown, FALSE);
        VariableDefinePlatformVar(NULL, "NOPULL", &IntType, (union AnyValue *)&NoPull, FALSE);
#endif
}

/* Generic helper functions */

/* Helper function: clear all masks */
static void pioh_clear_masks(void)
{
	int i;
  	for (i = 0; i < PLATFORM_IO_PORTS; i++)
    		pio_masks[ i ] = 0;
}

static int pioh_set_pin(int v, int op)
{
	pio_type pio_mask = 0;
	int port, pin;
	pioh_clear_masks();
	port = PLATFORM_IO_GET_PORT(v);
	pin = PLATFORM_IO_GET_PIN(v);

	if (PLATFORM_IO_IS_PORT(v) || !platform_pio_has_port(port) || !platform_pio_has_pin(port, pin)) {
		ProgramFail(NULL, "Invalid pin");
		return -1;
	}

	pio_mask |= 1 << pin;
	if (pio_mask)
		if (!platform_pio_op(port, pio_mask, op)) {
			ProgramFail(NULL, "Invalid PIO operation");
			return -1;
		}
	return 0;
}

static int pioh_set_port(int v, int op, pio_type mask)
{
	int port;
	u32 port_mask = 0;
	port = PLATFORM_IO_GET_PORT(v);
	if (!PLATFORM_IO_IS_PORT(v) || !platform_pio_has_port(port)) {
		ProgramFail(NULL, "Invalid port");
		return -1;
	}
	port_mask |= (1ULL << port);
	if (port_mask & (1ULL << port))
		if (!platform_pio_op(port, mask, op)) {
			ProgramFail(NULL, "Invalid PIO operation");
			return -1;
		}
	return 0;
}

static int pio_gen_setdir(int v, int optype, int op)
{
	if (op == PIO_DIR_INPUT) {
    		op = optype == PIO_PIN_OP ? PLATFORM_IO_PIN_DIR_INPUT : PLATFORM_IO_PORT_DIR_INPUT;
	} else if (op == PIO_DIR_OUTPUT) {
    		op = optype == PIO_PIN_OP ? PLATFORM_IO_PIN_DIR_OUTPUT : PLATFORM_IO_PORT_DIR_OUTPUT;
	} else {
    		ProgramFail(NULL, "invalid direction");
		return -1;
	}

  	if (optype == PIO_PIN_OP)
    		pioh_set_pin(v, op);
  	else
    		pioh_set_port(v, op, PLATFORM_IO_ALL_PINS);

  	return 0;
}

int pio_gen_setpull(int v, int optype, int op)
{
	if ((op != PLATFORM_IO_PIN_PULLUP) &&
      		(op != PLATFORM_IO_PIN_PULLDOWN) &&
      		(op != PLATFORM_IO_PIN_NOPULL)) {
    		ProgramFail(NULL, "invalid pull type");
		return -1;
	}

  	if (optype == PIO_PIN_OP)
    		pioh_set_pin(v, op);
  	else
    		pioh_set_port(v, op, PLATFORM_IO_ALL_PINS);

  	return 0;
}

static int pio_gen_setval(int v, int optype, pio_type val)
{
	if ((optype == PIO_PIN_OP) && (val != 1) && (val != 0)) {
    		ProgramFail(NULL, "Invalid pin value");
		return -1;
	}

  	if (optype == PIO_PIN_OP)
    		pioh_set_pin(v, val == 1 ? PLATFORM_IO_PIN_SET : PLATFORM_IO_PIN_CLEAR);
  	else
    		pioh_set_port(v, val == 1 ? PLATFORM_IO_PIN_SET : PLATFORM_IO_PIN_CLEAR, val);
  	return 0;
}

// ****************************************************************************
// Pin operations

/* picoc: pin_setdir(INPUT | OUTPUT, "pin1") */
static void pio_pin_setdir(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	int ret = pio_value_parse(Param[1]->Val->Identifier);
	PIO_CHECK(ret);
	int dir = Param[0]->Val->Integer;

	ReturnValue->Val->Integer = pio_gen_setdir(ret, PIO_PIN_OP, dir);
}

/* picoc: pio_pin_setpull(PULLUP | PULLDOWN | NOPULL, "pin1") */
static void pio_pin_setpull(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	int ret = pio_value_parse(Param[1]->Val->Identifier);
	PIO_CHECK(ret);
	int dir = Param[0]->Val->Integer;

	ReturnValue->Val->Integer = pio_gen_setpull(ret, PIO_PIN_OP, dir);
}

/* picoc: pio_pin_setval(0 | 1, "pin1") */
static void pio_pin_setval(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	int ret = pio_value_parse(Param[1]->Val->Identifier);
	PIO_CHECK(ret);
	pio_type val = (pio_type)Param[0]->Val->UnsignedInteger;

	ReturnValue->Val->Integer = pio_gen_setval(ret, PIO_PIN_OP, val);
}

/* picoc: pio_pin_sethigh("pin1") */
static void pio_pin_sethigh(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	int ret = pio_value_parse(Param[0]->Val->Identifier);
	PIO_CHECK(ret);
	ReturnValue->Val->Integer = pio_gen_setval(ret, PIO_PIN_OP, 1);
}

/* picoc: pio_pin_setlow("pin1") */
static void pio_pin_setlow(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	int ret = pio_value_parse(Param[0]->Val->Identifier);
	PIO_CHECK(ret);
	ReturnValue->Val->Integer = pio_gen_setval(ret, PIO_PIN_OP, 0);
}

/* picoc: pin1_val = pio_pin_getval("pin1") */
static void pio_pin_getval(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	pio_type value;
  	int v, port, pin;

	v = pio_value_parse(Param[0]->Val->Identifier);
	PIO_CHECK(v);
    	port = PLATFORM_IO_GET_PORT(v);
    	pin = PLATFORM_IO_GET_PIN(v);

    	if (PLATFORM_IO_IS_PORT(v) || !platform_pio_has_port(port) || !platform_pio_has_pin(port, pin)) {
		ProgramFail(NULL, "Invalid pin");
		ReturnValue->Val->UnsignedInteger = 0;
	} else {
      		value = platform_pio_op(port, 1 << pin, PLATFORM_IO_PIN_GET);
		ReturnValue->Val->UnsignedInteger = value;
    	}
}

// ****************************************************************************
// Port operations

/* picoc: pio_port_setdir(INPUT | OUTPUT, "port1") */
static void pio_port_setdir(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
        int ret = pio_value_parse(Param[1]->Val->Identifier);
	PIO_CHECK(ret);
        int dir = Param[0]->Val->Integer;

        ReturnValue->Val->Integer = pio_gen_setdir(ret, PIO_PORT_OP, dir);
}

/* picoc: pio_port_setpull(PULLUP | PULLDOWN | NOPULL, "port1") */
static void pio_port_setpull(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
        int ret = pio_value_parse(Param[1]->Val->Identifier);
	PIO_CHECK(ret);
        int dir = Param[0]->Val->Integer;

        ReturnValue->Val->Integer = pio_gen_setpull(ret, PIO_PORT_OP, dir);
}

/* picoc: pio_port_setval(value, "port1") */
static void pio_port_setval(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
        int ret = pio_value_parse(Param[1]->Val->Identifier);
	PIO_CHECK(ret);
        pio_type val = (pio_type)Param[0]->Val->UnsignedInteger;

        ReturnValue->Val->Integer = pio_gen_setval(ret, PIO_PORT_OP, val);
}

/* picoc: pio_port_sethigh("port1") */
static void pio_port_sethigh(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
        int ret = pio_value_parse(Param[0]->Val->Identifier);
	PIO_CHECK(ret);
        ReturnValue->Val->Integer = pio_gen_setval(ret, PIO_PORT_OP, 1);
}

/* picoc: pio_port_setlow("port1") */
static void pio_port_setlow(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	int ret = pio_value_parse(Param[0]->Val->Identifier);
	PIO_CHECK(ret);
	ReturnValue->Val->Integer = pio_gen_setval(ret, PIO_PORT_OP, 0);
}

/* picoc: val1 = pio_port_getval("port1") */
static void pio_port_getval(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
        pio_type value;
        int v, port;

	v = pio_value_parse(Param[0]->Val->Identifier);
	PIO_CHECK(v);
        port = PLATFORM_IO_GET_PORT(v);

        if (!PLATFORM_IO_IS_PORT(v) || !platform_pio_has_port(port)) {
                ProgramFail(NULL, "Invalid port");
                ReturnValue->Val->UnsignedInteger = 0;
        } else {
                value = platform_pio_op(port, PLATFORM_IO_ALL_PINS, PLATFORM_IO_PORT_GET_VALUE);
                ReturnValue->Val->UnsignedInteger = value;
        }
}

/**
 * The 'decode' functions returns a port/pin pair from a platform code
 * picoc: pio_decode(code, &pin_code, &port_code)
 */
static void pio_decode(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	int code = Param[0]->Val->Integer;
	*((int *)Param[1]->Val->Pointer) = PLATFORM_IO_GET_PORT(code);
	*((int *)Param[2]->Val->Pointer) = PLATFORM_IO_GET_PIN(code);
	ReturnValue->Val->Integer = 0;
}

/* returns pin/port numeric identifiers */
static int pio_value_parse(char *key)
{
	int port = 0xFFFF, pin = 0xFFFF, isport = 0, sz;
  
  	if (!key || *key != 'P')
    		return 0;

	/* PA, PB .. */
  	if (isupper((int)key[1])) {
    		if (PIO_PREFIX != 'A') /* PA, PB .. */
      			return 0;
   		port = key[1] - 'A';
    		if (key[2] == '\0')
      			isport = 1;
    		else if (key[2] == '_')
      			if (sscanf(key + 3, "%d%n", &pin, &sz) != 1 || sz != strlen(key) - 3)
       		 		return 0;
  	} else { /* P0, P1 .. */
    		if (PIO_PREFIX != '0') /* P0, P1 .. */
      			return 0;
    		if (!strchr(key, '_')) {  /* parse port */
      			if(sscanf( key + 1, "%d%n", &port, &sz) != 1  || sz != strlen(key) - 1)
        			return 0;
      			isport = 1;
    		} else {   /* parse port_pin */
      			if (sscanf(key + 1, "%d_%d%n", &port, &pin, &sz) != 2 || sz != strlen(key) - 1)
        		return 0;
		}
  	}
  	sz = -1;
  	if (isport) {
    		if (platform_pio_has_port(port))
      			sz = PLATFORM_IO_ENCODE(port, 0, 1);
  	} else {
    		if (platform_pio_has_port(port) && platform_pio_has_pin(port, pin))
      			sz = PLATFORM_IO_ENCODE(port, pin, 0); printf ("\nsz: %d\n", sz);
  	}
  	if (sz == -1)
    		return 0;
  	else
    		return sz;
}

#define MIN_OPT_LEVEL 2
#include "rodefs.h"

#if ((PICOC_OPTIMIZE_MEMORY == 2) && !defined (BUILTIN_MINI_STDLIB))
/* rotable for platform variables */
const PICOC_RO_TYPE pio_variables[] = {
        {STRKEY("INPUT"), INT(Input)},
        {STRKEY("OUTPUT"), INT(Output)},
        {STRKEY("PULLUP"), INT(PullUp)},
        {STRKEY("PULLDOWN"), INT(PullDown)},
        {STRKEY("NOPULL"), INT(NoPull)},
        {NILKEY, NILVAL}
};
#endif

/* list of all library functions and their prototypes */
const PICOC_REG_TYPE pio_library[] = {
	/* pin functions */
        {FUNC(pio_pin_setdir), PROTO("int pio_pin_setdir(int, char *);")},
        {FUNC(pio_pin_setpull), PROTO("int pio_pin_setpull(int, char *);")},
        {FUNC(pio_pin_setval), PROTO("int pio_pin_setval(unsigned int, char *);")},
        {FUNC(pio_pin_sethigh), PROTO("int pio_pin_sethigh(char *);")},
        {FUNC(pio_pin_setlow), PROTO("int pio_pin_setlow(char *);")},
	{FUNC(pio_pin_getval), PROTO("unsigned int pio_pin_getval(char *);")},

	/* port functions */
        {FUNC(pio_port_setdir), PROTO("int pio_port_setdir(int, char *);")},
        {FUNC(pio_port_setpull), PROTO("int pio_port_setpull(int, char *);")},
        {FUNC(pio_port_setval), PROTO("int pio_port_setval(unsigned int, char *);")},
        {FUNC(pio_port_sethigh), PROTO("int pio_port_sethigh(char *);")},
        {FUNC(pio_port_setlow), PROTO("int pio_port_setlow(char *);")},
	{FUNC(pio_port_getval), PROTO("unsigned int pio_port_getval(char *);")},

	/* decode function */
	{FUNC(pio_decode), PROTO("int pio_decode(int, int *, int *);")},
	{NILFUNC, NILPROTO}
};

/* init library */
extern void pio_library_init(void)
{
	REGISTER("pio.h", &pio_lib_setup_func, &pio_library[0]);
}

