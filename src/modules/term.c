// Module for interfacing with terminal functions

#include "platform.h"
#include "picoc_mod.h"
#include "term.h"
#include "platform_conf.h"
#include <string.h>
#include "rotable.h"

#ifdef BUILD_TERM

/* platform variables */
const int NoWait = TERM_INPUT_DONT_WAIT;
const int Wait = TERM_INPUT_WAIT;

/* library setup function */
extern void term_lib_setup_func(void)
{
#if ((PICOC_OPTIMIZE_MEMORY == 0) && defined (BUILTIN_MINI_STDLIB))
        VariableDefinePlatformVar(NULL, "WAIT", &IntType, (union AnyValue *)&Wait, FALSE);
        VariableDefinePlatformVar(NULL, "NOWAIT", &IntType, (union AnyValue *)&NoWait, FALSE);
#endif
}

/* picoc: term_clrscr() */
static void picoc_term_clrscr(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	term_clrscr();
	ReturnValue->Val->Integer = 0;
}

/* picoc: term_clreol() */
static void picoc_term_clreol(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	term_clreol();
  	ReturnValue->Val->Integer = 0;
}

/* picoc: term_moveto(x, y) */
static void picoc_term_moveto(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned x, y;
	x = Param[0]->Val->UnsignedInteger;
	y = Param[1]->Val->UnsignedInteger;
  
  	term_gotoxy(x, y);
	ReturnValue->Val->Integer = 0;
}

/* picoc: term_moveup(lines) */
static void picoc_term_moveup(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned delta;
  
	delta = Param[0]->Val->UnsignedInteger;
	term_up(delta);
	ReturnValue->Val->Integer = 0;
}

/* picoc: movedown(lines) */
static void picoc_term_movedown(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned delta;
	  
	delta = Param[0]->Val->UnsignedInteger;
	term_down(delta);
	ReturnValue->Val->Integer = 0;
}

/* picoc: term_moveleft(cols) */
static void picoc_term_moveleft(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned delta;
	delta = Param[0]->Val->UnsignedInteger;
	  
	term_left(delta);
	ReturnValue->Val->Integer = 0;
}

/* picoc: term_moveright(cols) */
static void picoc_term_moveright(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	unsigned delta;
	delta = Param[0]->Val->UnsignedInteger;
	  
	term_right(delta);
	ReturnValue->Val->Integer = 0;
}

/* picoc: lines = term_getlines() */
static void picoc_term_getlines(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	ReturnValue->Val->UnsignedInteger = term_get_lines();
}

/* picoc: columns = term_getcols() */
static void picoc_term_getcols(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	ReturnValue->Val->UnsignedInteger = term_get_cols();
}

/* picoc: term_puts(x, y, string) */
static void picoc_term_puts(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	const char* buf;
	term_gotoxy(Param[0]->Val->UnsignedInteger, Param[1]->Val->UnsignedInteger);
	buf = Param[2]->Val->Identifier;
	size_t len = strlen(buf), i;

	for (i = 0; i < len; i++)
		term_putch(buf[i]);

	ReturnValue->Val->UnsignedInteger = len;
}

/* picoc: term_putch(ch) */
static void picoc_term_putch(struct ParseState *Parser, struct Value *ReturnValue,
		struct Value **Param, int NumArgs)
{
	term_putch(Param[0]->Val->Character);
	ReturnValue->Val->UnsignedInteger = 1;
}

/* picoc: cursorx = term_getcx() */
static void picoc_term_getcx(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	ReturnValue->Val->UnsignedInteger = term_get_cx();
}

/* picoc: cursory = term_getcy() */
static void picoc_term_getcy(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	ReturnValue->Val->UnsignedInteger = term_get_cy();
}

/* picoc: key = term_getchar([mode]) */
static void picoc_term_getchar(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	int temp = TERM_INPUT_WAIT, res;
	temp = Param[0]->Val->Integer;
	res = term_getch(temp);
	if (!res) {
		ReturnValue->Val->Integer = -1;
		return;
	}
	ReturnValue->Val->Integer = res;
}

#undef _D
#define _D(x) #x
static const char* term_key_names[] = {TERM_KEYCODES};

/* picoc: term_decode(str) */
static void picoc_term_decode(struct ParseState *Parser, struct Value *ReturnValue,
			struct Value **Param, int NumArgs)
{
	const char *key = Param[0]->Val->Identifier;
  	unsigned i, total = sizeof(term_key_names) / sizeof(char*);

  	if (!key || *key != 'K') {
    		ProgramFail(NULL, "Key invalid. Parse error");
		ReturnValue->Val->Integer = -1;
	}
  	for (i = 0; i < total; i++)
    		if(!strcmp(key, term_key_names[i]))
      			break;
  	if (i == total)
		ReturnValue->Val->Integer = 0;
	else
		ReturnValue->Val->Integer = i + TERM_FIRST_KEY;
}

#define MIN_OPT_LEVEL 2
#include "rodefs.h"

#if ((PICOC_OPTIMIZE_MEMORY == 2) && !defined (BUILTIN_MINI_STDLIB))
/* rotable for platform variables */
const PICOC_RO_TYPE term_variables[] = {
	{STRKEY("WAIT"), INT(Wait)},
	{STRKEY("NOWAIT"), INT(NoWait)},
	{NILKEY, NILVAL}
};
#endif

/* list of all library functions and their prototypes */
const PICOC_REG_TYPE term_library[] = {
        {FUNC(picoc_term_clrscr), PROTO("int term_clrscr(void);")},
        {FUNC(picoc_term_clreol), PROTO("int term_clreol(void);")},
        {FUNC(picoc_term_moveto), PROTO("int term_moveto(unsigned int, unsigned int);")},
        {FUNC(picoc_term_moveup), PROTO("int term_moveup(unsigned int);")},
	{FUNC(picoc_term_movedown), PROTO("int term_movedown(unsigned int);")},
	{FUNC(picoc_term_moveleft), PROTO("int term_moveleft(unsigned int);")},
	{FUNC(picoc_term_moveright), PROTO("int term_moveright(unsigned int);")},
	{FUNC(picoc_term_getlines), "unsigned int term_getlines(void);"},
	{FUNC(picoc_term_getcols), PROTO("unsigned int term_getcols(void);")},
	{FUNC(picoc_term_puts), PROTO("unsigned int term_puts(unsigned int, unsigned int, char *);")},
	{FUNC(picoc_term_putch), PROTO("unsigned int term_putch(char);")},
	{FUNC(picoc_term_getcx), PROTO("unsigned int term_getcx(void);")},
	{FUNC(picoc_term_getcy), PROTO("unsigned int term_getcy(void);")},
	{FUNC(picoc_term_getchar), PROTO("int term_getchar(int);")},
	{FUNC(picoc_term_decode), PROTO("int term_decode(char *);")},
	{NILFUNC, NILPROTO}
};

/* init library */
extern void term_library_init(void)
{
	REGISTER("term.h", &term_lib_setup_func, &term_library[0]);
}

#endif /* #ifdef BUILD_TERM */
