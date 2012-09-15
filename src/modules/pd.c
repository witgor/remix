// Module for interfacing with platform data

#include "platform.h"
#include "interpreter.h"
#include "rotable.h"

#define MACRO_NAME(x) MACRO_AGAIN(x)
#define MACRO_AGAIN(x) #x

/* picoc: platform = pd_platform() */
static void pd_platform(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	ReturnValue->Val->Identifier = MACRO_NAME(PICOC_PLATFORM);
}

/* picoc: cpuname = pd_cpu() */
static void pd_cpu(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	ReturnValue->Val->Identifier = MACRO_NAME(PICOC_CPU);
}

/* picoc: boardname = pd_board() */
static void pd_board(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	ReturnValue->Val->Identifier = MACRO_NAME(PICOC_BOARD);
}

#define MIN_OPT_LEVEL 2
#include "rodefs.h"

/* list of all library functions and their prototypes */
const PICOC_REG_TYPE pd_library[] = {
        {FUNC(pd_platform), PROTO("char *pd_platform(void);")},
        {FUNC(pd_board), PROTO("char *pd_board(void);")},
        {FUNC(pd_cpu), PROTO("char *pd_cpu(void);")},
        {NILFUNC, NILPROTO}
};

/* init library */
extern void pd_library_init(void)
{
	REGISTER("pd.h", NULL, &pd_library[0]);
}

