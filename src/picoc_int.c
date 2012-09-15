// eLua interrupt support

#include "picoc_int.h"
#include "platform.h"
#include "platform_conf.h"
#include "type.h"
#include <stdio.h>
#include <string.h>

// ****************************************************************************
// picoc handlers

#ifdef BUILD_PICOC_INT_HANDLERS

/* TODO: Support interrupts from picoc */

#else // #ifdef BUILD_PICOC_INT_HANDLERS

void picoc_int_disable_all()
{
}

// This too
void picoc_int_cleanup()
{
}

void picoc_int_enable( picoc_int_id inttype )
{
}

void picoc_int_disable( picoc_int_id inttype )
{
}

int picoc_int_add( picoc_int_id inttype, picoc_int_resnum resnum )
{
  return PLATFORM_ERR;
}

#endif // #ifdef BUILD_PICOC_INT_HANDLERS

// ****************************************************************************
// C handlers

#ifdef BUILD_C_INT_HANDLERS

static picoc_int_c_handler picoc_int_c_handler_list[ INT_PICOC_LAST ];

picoc_int_c_handler picoc_int_set_c_handler( picoc_int_id inttype, picoc_int_c_handler phandler )
{
  picoc_int_c_handler crthandler;

  if( inttype < PICOC_INT_FIRST_ID || inttype > INT_PICOC_LAST )
    return NULL;
  inttype -= PICOC_INT_FIRST_ID;
  crthandler = picoc_int_c_handler_list[ inttype ];
  picoc_int_c_handler_list[ inttype ] = phandler;
  return crthandler;
}

picoc_int_c_handler picoc_int_get_c_handler( picoc_int_id inttype )
{
  if( inttype < PICOC_INT_FIRST_ID || inttype > INT_PICOC_LAST )
    return NULL;
  return picoc_int_c_handler_list[ inttype - PICOC_INT_FIRST_ID ];
}

#else // #ifdef BUILD_C_INT_HANDLERS

picoc_int_c_handler picoc_int_set_c_handler( picoc_int_id inttype, picoc_int_c_handler phandler )
{
  return NULL;
}

picoc_int_c_handler picoc_int_get_c_handler( picoc_int_id inttype )
{
  return NULL;
}

#endif // #ifdef BUILD_C_INT_HANDLERS

