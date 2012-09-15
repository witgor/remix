// picoc platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "picoc_mod.h"
#include "type.h"
#include "stacks.h"
#include "stm32f10x.h"
#include "picoc_int.h"
#include "sermux.h"


// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_SHELL
//#define BUILD_LINENOISE
//#define BUILD_ROMFS
#define BUILD_MMCFS
#define BUILD_TERM
#define BUILD_CON_GENERIC
#define BUILD_ADC
#define BUILD_CAN
#define BUILD_C_INT_HANDLERS

// *****************************************************************************
// UART/Timer IDs configuration data (used in main.c)

#define CON_UART_ID           0
#define CON_UART_SPEED        115200
#define CON_TIMER_ID          0
#define TERM_LINES            25
#define TERM_COLS             80

// *****************************************************************************
// libraries that will be compiled for this platform

/* FIXME: What comes here? Needs check on platform/stm32 */
#define LCDLINE

#ifdef BUILD_ADC
#define ADCLINE _ROM(PICOCLIB_ADC, NULL, &adc_library[0], NULL)
#else
#define ADCLINE
#endif

#ifndef NO_FP
#define MATHLINE _ROM(PICOC_CORE_LIB_MATH, &MathSetupFunc, &MathFunctions[0], NULL)
#else
#define MATHLINE
#endif

#if ((PICOC_OPTIMIZE_MEMORY == 2) && !defined (BUILTIN_MINI_STDLIB))
/* core library functions */
#define PICOC_CORE_LIBS_ROM\
        MATHLINE\
        _ROM(PICOC_CORE_LIB_STDIO, &StdioSetupFunc, &StdioFunctions[0], StdioDefs)\
        _ROM(PICOC_CORE_LIB_CTYPE, NULL, &StdCtypeFunctions[0], NULL)\
        _ROM(PICOC_CORE_LIB_STDBOOL, &StdboolSetupFunc, NULL, StdboolDefs)\
        _ROM(PICOC_CORE_LIB_STDLIB, &StdlibSetupFunc, &StdlibFunctions[0], NULL)\
        _ROM(PICOC_CORE_LIB_STRING, &StringSetupFunc, &StringFunctions[0], NULL)\
        _ROM(PICOC_CORE_LIB_ERRNO, &StdErrnoSetupFunc, NULL, NULL)
#endif

/* platform library functions */
#define PICOC_PLATFORM_LIBS_ROM\
        _ROM(PICOCLIB_PD, NULL, &pd_library[0], NULL)\
	_ROM(PICOCLIB_PWM, NULL, &pwm_library[0], NULL)\
        _ROM(PICOCLIB_TMR, NULL, &tmr_library[0], NULL)\
        ADCLINE\
        _ROM(PICOCLIB_PIO, &pio_lib_setup_func, &pio_library[0], NULL)\
	_ROM(PICOCLIB_CAN, &can_lib_setup_func, &can_library[0], NULL)\
        _ROM(PICOCLIB_TERM, &term_lib_setup_func, &term_library[0], NULL)\
        _ROM(PICOCLIB_PLATFORM_LIBS, NULL, &picoc_library[0], NULL)\
        _ROM(PICOCLIB_SPI, &spi_lib_setup_func, &spi_library[0], NULL)\
        _ROM(PICOCLIB_UART, &uart_lib_setup_func, &uart_library[0], NULL)\
        LCDLINE\
        _ROM(PICOCLIB_CPU, NULL, &cpu_library[0], NULL)

// *****************************************************************************
// system variables that will be compiled for this platform

#ifndef NO_FP
#define MATHLINE_VAR _ROM(PICOC_CORE_VAR_MATH, &math_variables[0])
#else
#define MATHLINE_VAR
#endif

/* core system variables */
#define PICOC_CORE_VARS_ROM\
        _ROM(PICOC_CORE_VAR_ERRNO, &errno_variables[0])\
        MATHLINE_VAR\
        _ROM(PICOC_CORE_VAR_STDBOOL, &stdbool_variables[0])\
        _ROM(PICOC_CORE_VAR_STDIO, &stdio_variables[0])

/* platform variables */
#define PICOC_PLATFORM_VARS_ROM\
        _ROM(PICOCVAR_PIO, &pio_variables[0])\
	_ROM(PICOCVAR_CAN, &can_variables[0])\
        _ROM(PICOCVAR_TERM, &term_variables[0])\
        _ROM(PICOCVAR_SPI, &spi_variables[0])\
        _ROM(PICOCVAR_UART, &uart_variables[0])

//******************************************************************************
// stack and heap config -
// values are set after experimentation. Needs validation.

#define HEAP_SIZE             (48*1024)
#define PICOC_STACK_SIZE      (42*1024)

// *****************************************************************************
// Configuration data

#define EGC_INITIAL_MODE      1

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       4
#define VTMR_FREQ_HZ          10

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               7
#define NUM_SPI               2
#define NUM_UART              5
#define NUM_TIMER             5
#define NUM_PWM               4
#define NUM_ADC               16
#define NUM_CAN               1

// Enable RX buffering on UART
#define BUF_ENABLE_UART
#define CON_BUF_SIZE          BUF_SIZE_128

// ADC Configuration Params
#define ADC_BIT_RESOLUTION    12
#define BUF_ENABLE_ADC
#define ADC_BUF_SIZE          BUF_SIZE_2

// These should be adjusted to support multiple ADC devices
#define ADC_TIMER_FIRST_ID    0
#define ADC_NUM_TIMERS        4

// RPC boot options
#define RPC_UART_ID           CON_UART_ID
#define RPC_TIMER_ID          CON_TIMER_ID
#define RPC_UART_SPEED        CON_UART_SPEED

// MMCFS Support (FatFs on SD/MMC)
// For STM32F103RET6 - PA5 = CLK, PA6 = MISO, PA7 = MOSI, PA8 = CS
#define MMCFS_TICK_HZ         10
#define MMCFS_TICK_MS         ( 1000 / MMCFS_TICK_HZ )
#define MMCFS_CS_PORT         0
#define MMCFS_CS_PIN          8
#define MMCFS_SPI_NUM         0

// CPU frequency (needed by the CPU module, 0 if not used)
u32 platform_s_cpu_get_frequency();
#define CPU_FREQUENCY         platform_s_cpu_get_frequency()

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PINS_PER_PORT     16

// Remote file system data
#define RFS_BUFFER_SIZE       BUF_SIZE_512
#define RFS_UART_ID           0
#define RFS_TIMER_ID          0
#define RFS_TIMEOUT           100000
#define RFS_UART_SPEED        115200

// Linenoise buffer sizes
#define LINENOISE_HISTORY_SIZE_PICOC  50
#define LINENOISE_HISTORY_SIZE_SHELL  10

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define SRAM_SIZE             (64 * 1024)
#define MEM_START_ADDRESS     {(void *)end}
#define MEM_END_ADDRESS       {(void *)(SRAM_BASE + SRAM_SIZE - STACK_SIZE_TOTAL - 1)}

// Interrupt queue size
#define PLATFORM_INT_QUEUE_LOG_SIZE 5

// Interrupt list
#define INT_TMR_MATCH         PICOC_INT_FIRST_ID
#define INT_UART_RX           (PICOC_INT_FIRST_ID + 1)
#define INT_PICOC_LAST        INT_UART_RX

/**
 * For use in future when interrupts are
 * supported.
 */
/* #define PLATFORM_CPU_CONSTANTS\
        _CPU_CONSTANT(INT_TMR_MATCH),\
        _CPU_CONSTANT(INT_UART_RX), */

#endif // #ifndef __PLATFORM_CONF_H__
