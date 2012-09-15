// Interface with core services

#include "platform.h"
#include "picoc_mod.h"
#include "picoc.h"
#include "interpreter.h"
#include "platform_conf.h"
#include "linenoise.h"
#include <string.h>
#include "rotable.h"

/* picoc: picoc_version() */
static void picoc_version(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	ReturnValue->Val->Identifier = PICOC_VERSION;
}

/* picoc: picoc_save_history(filename). only available if linenoise is enabled */
static void picoc_save_history(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
#ifdef BUILD_LINENOISE
	const char *fname = Param[0]->Val->Identifier;
	int res;
  
	res = linenoise_savehistory(LINENOISE_ID_PICOC, fname);
	if (res == 0)
		printf("History saved to %s.\n", fname);
	else if (res == LINENOISE_HISTORY_NOT_ENABLED)
		printf("Linenoise not enabled for picoc.\n");
	else if (res == LINENOISE_HISTORY_EMPTY)
		printf("History empty, nothing to save.\n");
	else
		printf("Unable to save history to %s.\n", fname);
	ReturnValue->Val->Integer = 0;
#else
	ProgramFail(NULL, "Linenoise support was not enabled.");
	ReturnValue->Val->Integer = -1;
#endif	/* #ifdef BUILD_LINENOISE */
}

#define MIN_OPT_LEVEL 2
#include "rodefs.h"

/* list of all library functions and their prototypes */
const PICOC_REG_TYPE picoc_library[] = {
        /* picoc platform functions */
        {FUNC(picoc_version), PROTO("char *picoc_version(void);")},
	{FUNC(picoc_save_history), PROTO("int picoc_save_history(char *);")},
	{NILFUNC, NILPROTO}
};

/* init library */
extern void picoc_platform_library_init(void)
{
	REGISTER("picoc.h", NULL, &picoc_library[0]);
}
