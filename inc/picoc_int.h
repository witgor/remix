// picoc interrupt handlers support

#ifndef __PICOC_INT_H__
#define __PICOC_INT_H__

#include "type.h"

typedef u8 picoc_int_id;
// The resource number is a 16-bit integer because it must be able to hold a 
// port/pin combination coded as specified in platform.h 
typedef u16 picoc_int_resnum;

#define PICOC_INT_EMPTY_SLOT             0
#define PICOC_INT_FIRST_ID               1
#define PICOC_INT_INVALID_INTERRUPT      0xFF

// This is what gets pushed in the interrupt queue
typedef struct {
	picoc_int_id id;
  	picoc_int_resnum resnum;
} picoc_int_element;

// Interrupt functions and descriptor
typedef int (*picoc_int_p_set_status)(picoc_int_resnum resnum, int state); 
typedef int (*picoc_int_p_get_status)(picoc_int_resnum resnum);
typedef int (*picoc_int_p_get_flag)(picoc_int_resnum resnum, int clear);
typedef struct {
  picoc_int_p_set_status int_set_status;
  picoc_int_p_get_status int_get_status;
  picoc_int_p_get_flag int_get_flag;
} picoc_int_descriptor;

// C interrupt handlers
typedef void(*picoc_int_c_handler)(picoc_int_resnum resnum);

// Handler key in the registry
#define PICOC_INT_HANDLER_KEY             (int)&picoc_int_add

// Maximum number of interrupt sources that can appear in ANY backend
// Must be a multiple of 32
#define PICOC_INT_MAX_SOURCES             128

// Function prototypes
int picoc_int_add( picoc_int_id inttype, picoc_int_resnum resnum );
void picoc_int_enable( picoc_int_id inttype );
void picoc_int_disable( picoc_int_id inttype );
int picoc_int_is_enabled( picoc_int_id inttype );
void picoc_int_cleanup();
void picoc_int_disable_all();
picoc_int_c_handler picoc_int_set_c_handler( picoc_int_id inttype, picoc_int_c_handler phandler );
picoc_int_c_handler picoc_int_get_c_handler( picoc_int_id inttype );

#endif

