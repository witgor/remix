/* Module for interfacing Lua code with a Controller Area Network (CAN) */

#include "platform.h"
#include "interpreter.h"
#include "picoc_mod.h"
#include "rotable.h"

/* platform variables */
const int IdStd = PICOC_CAN_ID_STD;
const int IdExt = PICOC_CAN_ID_EXT;

/* library setup function */
extern void can_lib_setup_func(void)
{
#if ((PICOC_OPTIMIZE_MEMORY == 0) && defined (BUILTIN_MINI_STDLIB))
        VariableDefinePlatformVar(NULL, "ID_STD", &IntType, (union AnyValue *)&IdStd, FALSE);
        VariableDefinePlatformVar(NULL, "ID_EXT", &IntType, (union AnyValue *)&IdExt, FALSE);
#endif
}

/* picoc: can_setup(id, clock) */
static void can_setup(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned id;
	u32 clock, res;
  
	id = Param[0]->Val->UnsignedInteger;
  	MOD_CHECK_ID(can, id);
	clock = Param[1]->Val->UnsignedInteger;
  	res = platform_can_setup(id, clock);
	
	ReturnValue->Val->UnsignedInteger = res;
}

/* picoc: can_send(id, canid, canidtype, message) */
static void can_send(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	size_t len;
  	int id, canid, idtype;
  	const char *data;
  
	id = Param[0]->Val->Integer;
  	MOD_CHECK_ID(can, id);
	canid = Param[1]->Val->Integer;
	idtype = Param[2]->Val->Integer;
	data = Param[3]->Val->Identifier;
	len = strlen(data);

  	if (len > PLATFORM_CAN_MAXLEN) {
		ReturnValue->Val->Integer = -1;
    		return ProgramFail(NULL, "message exceeds max length");
	}

  	platform_can_send(id, canid, idtype, len, (const u8 *)data);
	ReturnValue->Val->Integer = len;
}

/* picoc: canid, canidtype, message = recv(id, *message) */
static void can_recv(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	u8 len;
  	int id;
  	u32 canid;
  	u8  idtype, data[8];
  
	id = Param[0]->Val->Integer;
  	MOD_CHECK_ID(can, id);
  
  	if (platform_can_recv(id, &canid, &idtype, &len, data) == PLATFORM_OK)
    		ReturnValue->Val->Integer = len;
	else
		ReturnValue->Val->Integer = -1;
}

#define MIN_OPT_LEVEL 2
#include "rodefs.h"

#if ((PICOC_OPTIMIZE_MEMORY == 2) && !defined (BUILTIN_MINI_STDLIB))
/* rotable for platform variables */
const PICOC_RO_TYPE can_variables[] = {
	{STRKEY("ID_STD"), INT(IdStd)},
	{STRKEY("ID_EXT"), INT(IdExt)},
	{NILKEY, NILVAL}
};
#endif

/* list of all library functions and their prototypes */
const PICOC_REG_TYPE can_library[] = {
        {FUNC(can_setup), PROTO("unsigned int can_setup(unsigned int, unsigned int);")},
        {FUNC(can_send), PROTO("int can_send(int, int, int, char *);")},
        {FUNC(can_recv), PROTO("int can_recv(int);")},
	{NILFUNC, NILPROTO}
};

/* init library */
extern void can_library_init(void)
{
	REGISTER("can.h", &can_lib_setup_func, &can_library[0]);
}

