// picoc platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "picoc_mod.h"
#include "type.h"
#include "stacks.h"

// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_BANNER
#define BUILD_SHELL
#define BUILD_ROMFS
#define BUILD_CON_GENERIC
#define BUILD_TERM

#define TERM_LINES    25
#define TERM_COLS     80

// *****************************************************************************
// Auxiliary libraries that will be compiled for this platform

/* core library functions */
#define PICOC_CORE_LIBS_ROM\
        _ROM(PICOC_CORE_LIB_MATH, &MathSetupFunc, &MathFunctions[0], NULL)\
        _ROM(PICOC_CORE_LIB_STDIO, &StdioSetupFunc, &StdioFunctions[0], StdioDefs)\
        _ROM(PICOC_CORE_LIB_CTYPE, NULL, &StdCtypeFunctions[0], NULL)\
        _ROM(PICOC_CORE_LIB_STDBOOL, &StdboolSetupFunc, NULL, StdboolDefs)\
        _ROM(PICOC_CORE_LIB_STDLIB, &StdlibSetupFunc, &StdlibFunctions[0], NULL)\
        _ROM(PICOC_CORE_LIB_STRING, &StringSetupFunc, &StringFunctions[0], NULL)\
        _ROM(PICOC_CORE_LIB_ERRNO, &StdErrnoSetupFunc, NULL, NULL)

#define PICOC_PLATFORM_LIBS_ROM\
	_ROM(PICOCLIB_PD, NULL, &pd_library[0], NULL)\
	_ROM(PICOCLIB_TERM, &term_lib_setup_func, &term_library[0], NULL)
        //_ROM(PicocPlatform)

// Bogus defines for common.c
#define CON_UART_ID           0
#define CON_UART_SPEED        0
#define CON_TIMER_ID          0

// *****************************************************************************
// Configuration data

#define HEAP_SIZE        (48*1024)    /* space for the heap and the stack */
#define PICOC_STACK_SIZE (32*1024)    /* space for the the stack */

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       0

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               0
#define NUM_SPI               0
#define NUM_UART              0
#define NUM_TIMER             0
#define NUM_PWM               0
#define NUM_ADC               0
#define NUM_CAN               0

// CPU frequency (needed by the CPU module, 0 if not used)
#define CPU_FREQUENCY         0

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     0

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
u32 platform_get_lastmem();
#define MEM_START_ADDRESS     { ( void* )end }
#define MEM_END_ADDRESS       { ( void* )( platform_get_lastmem() - STACK_SIZE_TOTAL - 1 ) }

#endif // #ifndef __PLATFORM_CONF_H__
