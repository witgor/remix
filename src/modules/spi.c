// Module for interfacing with picoc SPI code

#include "platform.h"
#include "picoc_mod.h"
#include "picoc.h"
#include "interpreter.h"
#include "rotable.h"

/* platform variables */
const int Master = PLATFORM_SPI_MASTER;
const int Slave = PLATFORM_SPI_SLAVE;

/* library setup function */
extern void spi_lib_setup_func(void)
{
#if ((PICOC_OPTIMIZE_MEMORY == 0) && defined (BUILTIN_MINI_STDLIB))
        VariableDefinePlatformVar(NULL, "MASTER", &IntType, (union AnyValue *)&Master, FALSE);
        VariableDefinePlatformVar(NULL, "SLAVE", &IntType, (union AnyValue *)&Slave, FALSE);
#endif
}

/* picoc: spi_sson(id) */
static void spi_sson(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(spi, id);

  	platform_spi_select(id, PLATFORM_SPI_SELECT_ON);

	ReturnValue->Val->Integer = 0;
}

/* picoc: spi_ssoff(id) */
static void spi_ssoff(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArg)
{
	unsigned id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(spi, id);
  	platform_spi_select(id, PLATFORM_SPI_SELECT_OFF);

	ReturnValue->Val->Integer = 0;
}

/* picoc: clock = spi_setup(id, MASTER/SLAVE, clock, cpol, cpha, databits) */
static void spi_setup(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id, cpol, cpha, is_master, databits;
  	u32 clock, res;
	
	id = Param[0]->Val->UnsignedInteger;
	MOD_CHECK_ID(spi, id);
  
  	is_master = Param[1]->Val->UnsignedInteger == PLATFORM_SPI_MASTER;
  	if (!is_master) {
		ReturnValue->Val->UnsignedInteger = 0;
		return ProgramFail(NULL, "Invalid type (only MASTER is supported)");
	}

	clock = Param[2]->Val->UnsignedInteger;
	cpol = Param[3]->Val->UnsignedInteger;
	if ((cpol != 0) && (cpol != 1)) {
		ReturnValue->Val->UnsignedInteger = 0;
		return ProgramFail(NULL, "Invalid clock polarity.");
	}

	cpha = Param[4]->Val->UnsignedInteger;
	if ((cpha != 0) && (cpha != 1)) {
		ReturnValue->Val->UnsignedInteger = 0;
		return ProgramFail(NULL, "Invalid clock phase.");
	}

	databits = Param[5]->Val->UnsignedInteger;
  	res = platform_spi_setup(id, is_master, clock, cpol, cpha, databits);

	ReturnValue->Val->UnsignedInteger = res;
}

/* picoc: spi_write_num(id, num) */
static void spi_write_num(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id = Param[0]->Val->UnsignedInteger;
	spi_data_type value = Param[1]->Val->UnsignedInteger;

	platform_spi_send_recv(id, value);

	ReturnValue->Val->Integer = 1;
}

/* picoc: spi_write_string(id, string, len) */
static void spi_write_string(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned int id = Param[0]->Val->Integer, i;
	char *str  = Param[1]->Val->Identifier;
	unsigned int len = Param[2]->Val->UnsignedInteger;
	
	for (i = 0; i < len; i++)
		platform_spi_send_recv(id, str[i]);
	
	ReturnValue->Val->Integer = len;
}

#define MIN_OPT_LEVEL 2
#include "rodefs.h"

#if ((PICOC_OPTIMIZE_MEMORY == 2) && !defined (BUILTIN_MINI_STDLIB))
/* rotable for platform variables */
const PICOC_RO_TYPE spi_variables[] = {
	{STRKEY("MASTER"), INT(Master)},
	{STRKEY("SLAVE"), INT(Slave)},
	{NILKEY, NILVAL}	
};
#endif

/* list of all library functions and their prototypes */
const PICOC_REG_TYPE spi_library[] = {
        /* pin functions */
        {FUNC(spi_sson), PROTO("int spi_sson(int);")},
        {FUNC(spi_ssoff), PROTO("int spi_ssoff(int);")},
        {FUNC(spi_setup), PROTO("unsigned int spi_setup(unsigned int, int, unsigned int, unsigned int, unsigned int, unsigned int);")},
        {FUNC(spi_write_num), PROTO("int spi_write_num(unsigned int, unsigned int);")},
        {FUNC(spi_write_string), PROTO("int spi_write_string(unsigned int, char *, unsigned int);")},
	{NILFUNC, NILPROTO}
};

/* init library */
extern void spi_library_init(void)
{
	REGISTER("spi.h", &spi_lib_setup_func, &spi_library[0]);
}

