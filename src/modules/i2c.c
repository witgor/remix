// Module for interfacing with the I2C interface

#include "platform.h"
#include "picoc_mod.h"
#include "picoc.h"
#include "interpreter.h"
#include <string.h>
#include <ctype.h>
#include "rotable.h"

/* platform variables */
const int Fast = PLATFORM_I2C_SPEED_FAST;
const int Slow = PLATFORM_I2C_SPEED_SLOW;
const int Transmitter = PLATFORM_I2C_DIRECTION_TRANSMITTER;
const int Receiver = PLATFORM_I2C_DIRECTION_RECEIVER;

/* library setup function */
extern void i2c_lib_setup_func(void)
{
#if ((PICOC_OPTIMIZE_MEMORY == 0) && defined (BUILTIN_MINI_STDLIB))
        VariableDefinePlatformVar(NULL, "FAST", &IntType, (union AnyValue *)&Fast, FALSE);
        VariableDefinePlatformVar(NULL, "SLOW", &IntType, (union AnyValue *)&Slow, FALSE);
        VariableDefinePlatformVar(NULL, "TRANSMITTER", &IntType, (union AnyValue *)&Transmitter, FALSE);
        VariableDefinePlatformVar(NULL, "RECEIVER", &IntType, (union AnyValue *)&Receiver, FALSE);
#endif
}

/* picoc: speed = i2c_setup(id, speed) */
static void i2c_setup(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id = Param[0]->Val->UnsignedInteger;
	u32 speed = Param[1]->Val->UnsignedInteger;

  	MOD_CHECK_ID(i2c, id);
  	ReturnValue->Val->UnsignedInteger = platform_i2c_setup(id, speed);
}

/* picoc: i2c_start(id) */
static void i2c_start(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id = Param[0]->Val->UnsignedInteger;

  	MOD_CHECK_ID(i2c, id);
  	platform_i2c_send_start(id);

	ReturnValue->Val->Integer = 0;
}

/* picoc: i2c_stop(id) */
static void i2c_stop(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id = Param[0]->Val->UnsignedInteger;

	MOD_CHECK_ID(i2c, id);
  	platform_i2c_send_stop(id);

	ReturnValue->Val->Integer = 0;
}

/* picoc: i2c_address(id, address, direction) */
static void i2c_address(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id = Param[0]->Val->UnsignedInteger;
	u16 address = Param[1]->Val->UnsignedShortInteger;
	int direction = Param[2]->Val->Integer;

  	MOD_CHECK_ID(i2c, id);
  	ReturnValue->Val->Integer = platform_i2c_send_address(id, address, direction);
}

/* picoc: wrote = i2c_write_num(id, num) */
static void i2c_write_num(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id = Param[0]->Val->UnsignedInteger;
  	int numdata;

  	MOD_CHECK_ID(i2c, id);
	numdata = Param[1]->Val->Integer;
	if (numdata < 0 || numdata > 255) {
		ReturnValue->Val->Integer = 0;
		return ProgramFail(NULL, "Numeric data can be between 0 and 255 only.");
	}
	if (!platform_i2c_send_byte(id, numdata)) {
		ReturnValue->Val->Integer = 0;
		return ProgramFail(NULL, "i2c_send_byte failed.");
	}
	ReturnValue->Val->UnsignedInteger = 1;
}

/* picoc: wrote = i2c_write_string(id, str) */
static void i2c_write_string(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned int id = Param[0]->Val->UnsignedInteger;
        const char *pdata;
        size_t datalen, i;
        u32 wrote = 0;

  	MOD_CHECK_ID(i2c, id);
	pdata = Param[1]->Val->Identifier;
	datalen = strlen(pdata);
	for (i = 0; i < datalen; i++) {
		if (platform_i2c_send_byte(id, pdata[i]) == 0)
			break;
		wrote += 1;
	}	
	if (wrote < datalen) {
		ReturnValue->Val->UnsignedInteger = 0;
		return ProgramFail(NULL, "Unable to write entire string");
	}
	ReturnValue->Val->UnsignedInteger = wrote;
}

/* picoc: read = i2c_read(id, size) */
static void i2c_read(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id = Param[0]->Val->UnsignedInteger;
	u32 size = Param[1]->Val->UnsignedInteger, i;
	char *b = HeapAllocMem(size + 1);
	unsigned int count = 0;
  	int data;

  	MOD_CHECK_ID(i2c, id);
  	if (size == 0) {
    		ReturnValue->Val->Integer = 0;
		return;
	}
  	for (i = 0; i < size; i++) {
    		if ((data = platform_i2c_recv_byte(id, i < size - 1)) == -1) {
      			break;
		} else {
			b[count] = (char)data;
			count++;
		}
	}
	ReturnValue->Val->Identifier = b;
}

#define MIN_OPT_LEVEL 2
#include "rodefs.h"

#if ((PICOC_OPTIMIZE_MEMORY == 2) && !defined (BUILTIN_MINI_STDLIB))
/* rotable for platform variables */
const PICOC_RO_TYPE i2c_variables[] = {
	{STRKEY("FAST"), INT(Fast)},
	{STRKEY("SLOW"), INT(Slow)},
	{STRKEY("TRANSMITTER"), INT(Transmitter)},
	{STRKEY("RECEIVER"), INT(Receiver)},
	{NILKEY, NILVAL}
};
#endif

/* list of all library functions and their prototypes */
const PICOC_REG_TYPE i2c_library[] = {
        {FUNC(i2c_setup), PROTO("unsigned int i2c_setup(unsigned int, unsigned int);")},
        {FUNC(i2c_start), PROTO("int i2c_start(unsigned int);")},
        {FUNC(i2c_stop), PROTO("int i2c_stop(unsigned int);")},
        {FUNC(i2c_address), PROTO("int i2c_address(unsigned int, unsigned short int, int);")},
	{FUNC(i2c_write_num), PROTO("unsigned int i2c_write_num(unsigned int, int);")},
	{FUNC(i2c_write_string), PROTO("unsigned int i2c_write_string(unsigned int, char *);")},
	{FUNC(i2c_read), PROTO("char *i2c_read(unsigned int, unsigned int);")},
	{NILFUNC, NILPROTO}
};

/* init library */
extern void i2c_library_init(void)
{
	REGISTER("i2c.h", &i2c_lib_setup_func, &i2c_library[0]);
}

