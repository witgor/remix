// Common platform functions

#ifndef __COMMON_H__
#define __COMMON_H__

#include "picoc_int.h"

// Virtual timers data
#define VTMR_FIRST_ID           ( 32 )
#define VTMR_GET_ID( x )        ( ( x ) - VTMR_FIRST_ID )
#define TIMER_IS_VIRTUAL( x )   ( ( VTMR_NUM_TIMERS > 0 ) && ( ( x ) >= VTMR_FIRST_ID ) && ( ( x ) < VTMR_NUM_TIMERS + VTMR_FIRST_ID ) )

// Functions exported by the common platform layer
void cmn_platform_init();
void cmn_virtual_timer_cb();
void cmn_int_handler( picoc_int_id id, picoc_int_resnum resnum );
// Timer-specific functions
int cmn_tmr_int_set_status( picoc_int_resnum resnum, int status );
int cmn_tmr_int_get_status( picoc_int_resnum resnum );
int cmn_tmr_int_get_flag( picoc_int_resnum resnum, int clear );
void cmn_uart_setup_sermux();

unsigned int intlog2( unsigned int v );

#endif // #ifndef __COMMON_H__

