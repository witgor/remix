/**
 * eLua Module for LM3S RIT OLED Display Support
 * disp is a platform-dependent (LM3S) module, that binds to Lua the basic API
 * from Luminary Micro
 * Adapted for remix by Raman Gopalan
 */
#include "picoc.h"
#include "interpreter.h"
#include "platform.h"
#include "platform_conf.h"
#include "disp.h"
#include "rotable.h"

/* picoc: disp_init(frequency); */
static void disp_init(struct ParseState *Parser,
	struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	unsigned long freq = Param[0]->Val->UnsignedLongInteger;
	lm3s_disp_init(freq);
	ReturnValue->Val->Integer = 0;
}

/* picoc: enable(frequency); */
static void disp_enable(struct ParseState *Parser,
	struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	unsigned long freq = Param[0]->Val->UnsignedLongInteger;
	lm3s_disp_enable(freq);
	ReturnValue->Val->Integer = 0;
}

/* picoc: disp_disable() */
static void disp_disable(struct ParseState *Parser,
	struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	lm3s_disp_disable();
	ReturnValue->Val->Integer = 0;
}
   
/* picoc: disp_on() */
static void disp_on(struct ParseState *Parser,
	struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	lm3s_disp_displayOn();    
	ReturnValue->Val->Integer = 0;
}

/* picoc: disp_off() */
static void disp_off(struct ParseState *Parser,
	struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	lm3s_disp_displayOff();
	ReturnValue->Val->Integer = 0;
}
   
/* picoc: disp_clear() */
static void disp_clear(struct ParseState *Parser,
	struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	lm3s_disp_clear();
	ReturnValue->Val->Integer = 0;
}
   
/* picoc: disp_puts(str, x, y, depth) */
static void disp_puts(struct ParseState *Parser,
	struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	char *str;
	unsigned long x, y;
	unsigned char level;

	str = Param[0]->Val->Identifier;
	x = Param[1]->Val->UnsignedLongInteger;	
	y = Param[2]->Val->UnsignedLongInteger;
	level = Param[3]->Val->Character;	

	lm3s_disp_stringDraw(str, x, y, level);
	ReturnValue->Val->Integer = 0;
}
   
/* picoc: disp_imageDraw(img, x, y, width, height) */
static void disp_imagedraw(struct ParseState *Parser,
	struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	char *img;
	unsigned long x, y, width, height;

	/**
	 * FIXME: 'img' must have been of type
         * unsigned char * (from eLua). This
         * mightcause an issue (?). Anyway, supress
         * the compiler warning for now.
	 */
	img = Param[0]->Val->Identifier;
	x = Param[1]->Val->UnsignedLongInteger;
	y = Param[2]->Val->UnsignedLongInteger;
	width = Param[3]->Val->UnsignedLongInteger;
	height= Param[4]->Val->UnsignedLongInteger;

	lm3s_disp_imageDraw((const unsigned char*)img, x, y, width, height);
	ReturnValue->Val->Integer = 0;
}   

#define MIN_OPT_LEVEL 2
#include "rodefs.h"

/* list of all library functions and their prototypes */
const PICOC_REG_TYPE lm3s_display_library[] =
{
        {FUNC(disp_init), PROTO("int lm3s_disp_init(unsigned long);")},
        {FUNC(disp_enable), PROTO("int lm3s_disp_enable(unsigned long);")},
  	{FUNC(disp_disable), PROTO("int lm3s_disp_disable(void);")},
    	{FUNC(disp_on), PROTO("int lm3s_disp_on(void);")},
    	{FUNC(disp_off), PROTO("int lm3s_disp_off(void);")},
    	{FUNC(disp_clear), PROTO("int lm3s_disp_clear(void);")},
    	{FUNC(disp_puts), PROTO("int lm3s_disp_puts(char *, unsigned long, unsigned long, unsigned char);")},
    	{FUNC(disp_imagedraw), PROTO("int lm3s_disp_imageDraw(unsigned char *, unsigned long, unsigned long, unsigned long, unsigned long);")},
	{NILFUNC, NILPROTO}
};

/* init library */
void disp_library_init(void)
{
	REGISTER("lm3s_disp.h", NULL, &lm3s_display_library[0]);
}
