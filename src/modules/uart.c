// Module for interfacing with UART

#include "platform.h"
#include "picoc_mod.h"
#include "common.h"
#include "sermux.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "platform_conf.h"
#include "rotable.h"

/* max size for uart recv buffer */
#define PICOC_UART_BUF	100

/* helper macro */
#define PICOC_UART_ERROR(msg)\
	ReturnValue->Val->Integer = -1;\
	return ProgramFail(NULL, msg)

/* Modes for the UART read function */
enum {
	UART_READ_MODE_LINE,
  	UART_READ_MODE_NUMBER,
  	UART_READ_MODE_SPACE,
  	UART_READ_MODE_MAXSIZE
};

/* platform variables */
const int ParEven = PLATFORM_UART_PARITY_EVEN;
const int ParOdd = PLATFORM_UART_PARITY_ODD;
const int ParNone = PLATFORM_UART_PARITY_NONE;
const int Stop_1 = PLATFORM_UART_STOPBITS_1;
const int Stop_1_5 = PLATFORM_UART_STOPBITS_1_5;
const int Stop_2 = PLATFORM_UART_STOPBITS_2;
const int NoTime = 0;
const int InfTime = PLATFORM_UART_INFINITE_TIMEOUT;
const int FlowNone = PLATFORM_UART_FLOW_NONE;
const int FlowRts = PLATFORM_UART_FLOW_RTS;
const int FlowCts = PLATFORM_UART_FLOW_CTS;

/* library setup function */
extern void uart_lib_setup_func(void)
{
#if ((PICOC_OPTIMIZE_MEMORY == 0) && defined (BUILTIN_MINI_STDLIB))
        VariableDefinePlatformVar(NULL, "PAR_EVEN", &IntType, (union AnyValue *)&ParEven, FALSE);
        VariableDefinePlatformVar(NULL, "PAR_ODD", &IntType, (union AnyValue *)&ParOdd, FALSE);
        VariableDefinePlatformVar(NULL, "PAR_NONE", &IntType, (union AnyValue *)&ParNone, FALSE);
        VariableDefinePlatformVar(NULL, "STOP_1", &IntType, (union AnyValue *)&Stop_1, FALSE);
        VariableDefinePlatformVar(NULL, "STOP_1_5", &IntType, (union AnyValue *)&Stop_1_5, FALSE);
        VariableDefinePlatformVar(NULL, "STOP_2", &IntType, (union AnyValue *)&Stop_2, FALSE);
        VariableDefinePlatformVar(NULL, "NO_TIMEOUT", &IntType, (union AnyValue *)&NoTime, FALSE);
        VariableDefinePlatformVar(NULL, "INF_TIMEOUT", &IntType, (union AnyValue *)&InfTime, FALSE);
        VariableDefinePlatformVar(NULL, "FLOW_NONE", &IntType, (union AnyValue *)&FlowNone, FALSE);
        VariableDefinePlatformVar(NULL, "FLOW_RTS", &IntType, (union AnyValue *)&FlowRts, FALSE);
        VariableDefinePlatformVar(NULL, "FLOW_CTS", &IntType, (union AnyValue *)&FlowCts, FALSE);
#endif
}

/* picoc: actualbaud = uart_setup(id, baud, databits, parity, stopbits) */
static void uart_setup(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id, databits, parity, stopbits;
  	u32 baud;
	
	id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(uart, id);

  	if (id >= SERMUX_SERVICE_ID_FIRST) {
		ReturnValue->Val->UnsignedInteger = 0;
		return ProgramFail(NULL, "uart_setup() can't be called on virtual UART's");
	}

  	baud = Param[1]->Val->UnsignedInteger;
	databits = Param[2]->Val->UnsignedInteger;
	parity = Param[3]->Val->UnsignedInteger;
	stopbits = Param[4]->Val->UnsignedInteger;

  	ReturnValue->Val->UnsignedInteger = platform_uart_setup(id, baud, databits, parity, stopbits);
}

/* picoc: uart_write_string(id, string, len) */
static void uart_write_string(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id;
	const char *buf;
	size_t len, i;

	id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(uart, id);

	buf = Param[1]->Val->Identifier;
	len = Param[2]->Val->UnsignedInteger;

	for (i = 0; i < len; i++)
		platform_uart_send(id, buf[i]);

	ReturnValue->Val->Integer = 0;
}

/* picoc: uart_write_num(id, num) */
static void uart_write_num(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id;
	int num;

	id = Param[0]->Val->UnsignedInteger;
  	MOD_CHECK_ID(uart, id);

	num = Param[1]->Val->UnsignedInteger;  
      	if ((num < 0) || (num > 255))
		PICOC_UART_ERROR("invalid number");

        platform_uart_send(id, (u8)num);

	ReturnValue->Val->Integer = 0;
}


/* picoc: uart_read(id, pattern, timeer, timeout) */
static void uart_read(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	int id, res, mode, issign;
  	unsigned timer_id = 0;
  	s32 timeout = PLATFORM_UART_INFINITE_TIMEOUT, maxsize = 0, count = 0;
  	const char *fmt;
	char *buf = (char *)HeapAllocMem(PICOC_UART_BUF);
	unsigned int buf_count = 0;
  	char cres;
	
	id = Param[0]->Val->UnsignedInteger;
  	MOD_CHECK_ID(uart, id);

	/* check format */
	fmt = Param[1]->Val->Identifier;
	maxsize = atoi(fmt);
	if (maxsize)
		mode = UART_READ_MODE_MAXSIZE;
	else if (!strcmp(fmt, "*l"))
		mode = UART_READ_MODE_LINE;
	else if (!strcmp(fmt, "*n"))
		mode = UART_READ_MODE_NUMBER;
	else if (!strcmp(fmt, "*s"))
		mode = UART_READ_MODE_SPACE;
	else
		return ProgramFail(NULL, "invalid max size");

	/* check timeout and timer id */
	timeout = Param[2]->Val->UnsignedInteger;
	if ((timeout < 0) && (timeout != PLATFORM_UART_INFINITE_TIMEOUT))
		return ProgramFail(NULL, "invalid timeout value");
	if ((timeout != PLATFORM_UART_INFINITE_TIMEOUT) && (timeout != 0))
		timer_id = Param[3]->Val->UnsignedInteger;

	while (1) {
		if ((res = platform_uart_recv(id, timer_id, timeout)) == -1)
			break;
		cres = (char)res;
		count++;
		issign = (count == 1) && ((res == '-') || (res == '+'));
		/*  TODO: this only works for lines that actually end
	         *  with '\n', other line endings are not supported.
		 */
    		if ((cres == '\n') && (mode == UART_READ_MODE_LINE))
      			break;
    		if (!isdigit((int)cres) && !issign && (mode == UART_READ_MODE_NUMBER))
      			break;
    		if (isspace((int)cres) && (mode == UART_READ_MODE_SPACE))
      			break;
		buf[buf_count++] = cres;
    		if ((count == maxsize) && (mode == UART_READ_MODE_MAXSIZE))
      			break;
  	}
	ReturnValue->Val->Identifier = buf;
	
	/* return an integer if needed */
	if (mode == UART_READ_MODE_NUMBER)
		sprintf(ReturnValue->Val->Identifier, "%d", res);
}

/* picoc: data = uart_getchar(id) */
static void uart_getchar(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	int id, res;
  	char cres;
  	unsigned timer_id = 0;
  	s32 timeout = PLATFORM_UART_INFINITE_TIMEOUT;
  
  	id = Param[0]->Val->UnsignedInteger;
  	MOD_CHECK_ID(uart, id);

  	res = platform_uart_recv(id, timer_id, timeout);
  	if (res == -1) {
    		ReturnValue->Val->Character = (char)0;
	} else {
		cres = (char)res;
		ReturnValue->Val->Character = cres;
	}
}

/* picoc: uart_set_buffer(id, size) */
static void uart_set_buffer(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	int id;
	u32 size;
  
	id = Param[0]->Val->UnsignedInteger;
  	MOD_CHECK_ID(uart, id);

	size = Param[1]->Val->UnsignedInteger;
	if (size && (size & (size - 1)))
		PICOC_UART_ERROR("the buffer size must be a power of 2 or 0");

	if (size == 0 && id >= SERMUX_SERVICE_ID_FIRST)
		PICOC_UART_ERROR("disabling buffers on virtual UART's is not allowed");

  	if (platform_uart_set_buffer(id, intlog2(size)) == PLATFORM_ERR)
		PICOC_UART_ERROR("unable to set UART buffer");
	
	ReturnValue->Val->Integer = 0;
}

/* picoc: uart_set_flow_control(id, type) */
static void uart_set_flow_control(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	int id = Param[0]->Val->Integer;
  	int type = Param[1]->Val->Integer;

	MOD_CHECK_ID( uart, id );
  	if (platform_uart_set_flow_control(id, type) != PLATFORM_OK)
    		PICOC_UART_ERROR("unable to set the flow control on interface");

  	ReturnValue->Val->Integer = 0;
}

#ifdef BUILD_SERMUX
#define MAX_VUART_NAME_LEN    6
#define MIN_VUART_NAME_LEN    6

/**
 * Look for all VUARTx timer identifiers
 * picoc: uart_value_parse(str)
 */
static void uart_value_parse(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	const char *key = Param[1]->Val->Identifier;
  	char* pend;
  	long res;
  
  	if (strlen(key) > MAX_VUART_NAME_LEN || strlen(key) < MIN_VUART_NAME_LEN)
    		return 0;
  	if (strncmp( key, "VUART", 5))
    		return 0;  
  	res = strtol(key + 5, &pend, 10);
  	if (*pend != '\0')
    		return 0;
  	if (res >= SERMUX_NUM_VUART)
    		return 0;
	ReturnValue->Val->LongInteger = SERMUX_SERVICE_ID_FIRST + res;
}
#endif /* #ifdef BUILD_SERMUX */

#define MIN_OPT_LEVEL 2
#include "rodefs.h"

#if ((PICOC_OPTIMIZE_MEMORY == 2) && !defined (BUILTIN_MINI_STDLIB))
/* rotable for platform variables */
const PICOC_RO_TYPE uart_variables[] = {
	{STRKEY("PAR_EVEN"), INT(ParEven)},
	{STRKEY("PAR_ODD"), INT(ParOdd)},
	{STRKEY("PAR_NONE"), INT(ParNone)},
	{STRKEY("STOP_1"), INT(Stop_1)},
	{STRKEY("STOP_1_5"), INT(Stop_1_5)},
	{STRKEY("STOP_2"), INT(Stop_2)},
	{STRKEY("NO_TIMEOUT"), INT(NoTime)},
	{STRKEY("INF_TIMEOUT"), INT(InfTime)},
        {STRKEY("FLOW_NONE"), INT(FlowNone)},
        {STRKEY("FLOW_RTS"), INT(FlowRts)},
        {STRKEY("FLOW_CTS"), INT(FlowCts)},
	{NILKEY, NILVAL}
};
#endif

/* list of all library functions and their prototypes */
const PICOC_REG_TYPE uart_library[] = {
	{FUNC(uart_setup), PROTO("unsigned int uart_setup(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);")},
	{FUNC(uart_write_string), PROTO("int uart_write_string(unsigned int, char *, unsigned int);")},
	{FUNC(uart_write_num), PROTO("int uart_write_num(unsigned int, unsigned int);")},
	{FUNC(uart_read), PROTO("char *uart_read(unsigned int, char *, unsigned int, unsigned int);")},
	{FUNC(uart_getchar), PROTO("char uart_getchar(unsigned int);")},
	{FUNC(uart_set_buffer), PROTO("int uart_set_buffer(unsigned int, unsigned int);")},
	{FUNC(uart_set_flow_control), PROTO("int uart_set_flow_control(int, int);")},
#ifdef BUILD_SERMUX
        /* decode function */
        {FUNC(uart_value_parse), PROTO("long int uart_value_parse(char *);")},
#endif
	{NILFUNC, NILPROTO}
};

/* init library */
extern void uart_library_init(void)
{
	REGISTER("uart.h", &uart_lib_setup_func, &uart_library[0]);
}
