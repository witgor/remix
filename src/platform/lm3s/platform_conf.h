// picoc platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "picoc_mod.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "stacks.h"
#include "sysctl.h"
#include "picoc_int.h"
#include "disp.h"
#include "picoc_mod.h"

// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_BANNER
#define BUILD_XMODEM
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

#if defined (FORLM3S8962) || defined (FORLM3S9B92)
#define CANLINE  _ROM(PICOCLIB_CAN, &can_lib_setup_func, &can_library[0], NULL)
#define BUILD_CAN
#else
#define CANLINE
#endif

#ifdef FORLM3S6918
#define PWMLINE
#else
#define PWMLINE  _ROM(PICOCLIB_PWM, NULL, &pwm_library[0], NULL)
#endif

#ifdef BUILD_ADC
#define ADCLINE _ROM(PICOCLIB_ADC, NULL, &adc_library[0], NULL)
#else
#define ADCLINE
#endif

#ifdef BUILD_TERM
#define TERMLINE _ROM(PICOCLIB_TERM, &term_lib_setup_func, &term_library[0], NULL)
#else
#define TERMLINE
#endif

#ifdef ENABLE_DISP
#define DISPLINE _ROM(PICOCLIB_DISP, NULL, &lm3s_display_library[0], NULL)
#else
#define DISPLINE
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
	PWMLINE\
	_ROM(PICOCLIB_TMR, NULL, &tmr_library[0], NULL)\
	ADCLINE\
	_ROM(PICOCLIB_PIO, &pio_lib_setup_func, &pio_library[0], NULL)\
	CANLINE\
	TERMLINE\
	_ROM(PICOCLIB_PLATFORM_LIBS, NULL, &picoc_library[0], NULL)\
	_ROM(PICOCLIB_SPI, &spi_lib_setup_func, &spi_library[0], NULL)\
	_ROM(PICOCLIB_UART, &uart_lib_setup_func, &uart_library[0], NULL)\
	DISPLINE\
	_ROM(PICOCLIB_CPU, NULL, &cpu_library[0], NULL)

// *****************************************************************************
// system variables that will be compiled for this platform

#if defined (FORLM3S8962) || defined (FORLM3S9B92)
#define CANLINE_VAR  _ROM(PICOCVAR_CAN, &can_variables[0])
#else
#define CANLINE_VAR
#endif

#ifdef BUILD_TERM
#define TERMLINE_VAR _ROM(PICOCVAR_TERM, &term_variables[0])
#else
#define TERMLINE_VAR
#endif

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
	CANLINE_VAR\
	TERMLINE_VAR\
	_ROM(PICOCVAR_SPI, &spi_variables[0])\
	_ROM(PICOCVAR_UART, &uart_variables[0])

//******************************************************************************
// stack and heap config -
// values are set after experimentation. Needs validation.

#define HEAP_SIZE 	      (48*1024)
#define PICOC_STACK_SIZE      (42*1024)

// *****************************************************************************
// Configuration data

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       4
#define VTMR_FREQ_HZ          4

// Number of resources (0 if not available/not implemented)
#if defined (FORLM3S1968)
  #define NUM_PIO             8
#elif defined (FORLM3S9B92)
  #define NUM_PIO             7
#else
  #define NUM_PIO             7
#endif
#define NUM_SPI               1
#ifdef FORLM3S6965
  #define NUM_UART            3
#elif FORLM3S9B92
  #define NUM_UART            3
#else
  #define NUM_UART            2
#endif
#define NUM_TIMER             4
#ifndef FORLM3S6918
  #define NUM_PWM             6
#else
  #define NUM_PWM             0
#endif
#define NUM_ADC               4
#define NUM_CAN               1

// Enable RX buffering on UART
#define BUF_ENABLE_UART
#define CON_BUF_SIZE          BUF_SIZE_128

// ADC Configuration Params
#define ADC_BIT_RESOLUTION    10
#define BUF_ENABLE_ADC
#define ADC_BUF_SIZE          BUF_SIZE_2

// These should be adjusted to support multiple ADC devices
#define ADC_TIMER_FIRST_ID    0
#define ADC_NUM_TIMERS        NUM_TIMER  

// SD/MMC Filesystem Setup
#define MMCFS_TICK_HZ     4
#define MMCFS_TICK_MS     (1000 / MMCFS_TICK_HZ)

#if defined (PICOC_BOARD_EKLM3S6965)
  // EK-LM3S6965
  #define MMCFS_CS_PORT                3
  #define MMCFS_CS_PIN                 0
  #define MMCFS_SPI_NUM                0
#elif defined (PICOC_BOARD_EKLM3S8962)
  // EK-LM3S8962
  #define MMCFS_CS_PORT                6
  #define MMCFS_CS_PIN                 0
  #define MMCFS_SPI_NUM                0
#elif defined (PICOC_BOARD_EAGLE100)
  // Eagle-100
  #define MMCFS_CS_PORT                6
  #define MMCFS_CS_PIN                 1
  #define MMCFS_SPI_NUM                0
#elif defined (BUILD_MMCFS) && !defined (MMCFS_SPI_NUM)
  #warning "MMCFS was enabled, but required SPI & CS data are undefined, disabling MMCFS"
  #undef BUILD_MMCFS
#endif

// CPU frequency (needed by the CPU module, 0 if not used)
#define CPU_FREQUENCY         SysCtlClockGet()

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'

// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#if defined (FORLM3S1968)
  #define PIO_PIN_ARRAY         {8, 8, 8, 4, 4, 8, 8, 4}
#elif defined (FORLM3S9B92)
  #define PIO_PIN_ARRAY         {8, 8, 8, 8, 8, 6, 8, 8, 8}
#else
  #define PIO_PIN_ARRAY         {8, 8, 8, 8, 4, 4, 2}
#endif

#ifdef FORLM3S9B92
  #define SRAM_SIZE (0x18000)
#else
  #define SRAM_SIZE (0x10000)
#endif

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define MEM_START_ADDRESS     {(void* )end}
#define MEM_END_ADDRESS       {(void* )(SRAM_BASE + SRAM_SIZE - STACK_SIZE_TOTAL - 1)}

// Interrupt list
#define INT_UART_RX           PICOC_INT_FIRST_ID
#define INT_PICOC_LAST        INT_UART_RX

// *****************************************************************************
// CPU constants that should be exposed to the picoc "cpu" module

#include "hw_ints.h"

/**
 * For use in future when interrupts are
 * supported.
 */
/* #define PLATFORM_CPU_CONSTANTS\
	_CPU_CONSTANT(INT_GPIOA),\
  	_CPU_CONSTANT(INT_GPIOB),\
  	_CPU_CONSTANT(INT_GPIOC),\
	_CPU_CONSTANT(INT_GPIOD),\
  	_CPU_CONSTANT(INT_GPIOE),\
  	_CPU_CONSTANT(INT_UART0),\
  	_CPU_CONSTANT(INT_UART1),\
  	_CPU_CONSTANT(INT_SSI0),\
  	_CPU_CONSTANT(INT_I2C0),\
  	_CPU_CONSTANT(INT_PWM_FAULT),\
  	_CPU_CONSTANT(INT_PWM0),\
  	_CPU_CONSTANT(INT_PWM1),\
  	_CPU_CONSTANT(INT_PWM2),\
  	_CPU_CONSTANT(INT_QEI0),\
  	_CPU_CONSTANT(INT_ADC0),\
  	_CPU_CONSTANT(INT_ADC1),\
  	_CPU_CONSTANT(INT_ADC2),\
  	_CPU_CONSTANT(INT_ADC3),\
	_CPU_CONSTANT(INT_WATCHDOG),\
	_CPU_CONSTANT(INT_TIMER0A),\
	_CPU_CONSTANT(INT_TIMER0B),\
	_CPU_CONSTANT(INT_TIMER1A),\
	_CPU_CONSTANT(INT_TIMER1B),\
	_CPU_CONSTANT(INT_TIMER2A),\
	_CPU_CONSTANT(INT_TIMER2B),\
	_CPU_CONSTANT(INT_COMP0),\
	_CPU_CONSTANT(INT_COMP1),\
	_CPU_CONSTANT(INT_COMP2),\
	_CPU_CONSTANT(INT_SYSCTL),\
	_CPU_CONSTANT(INT_FLASH),\
	_CPU_CONSTANT(INT_GPIOF),\
	_CPU_CONSTANT(INT_GPIOG),\
	_CPU_CONSTANT(INT_GPIOH),\
	_CPU_CONSTANT(INT_UART2),\
	_CPU_CONSTANT(INT_SSI1),\
	_CPU_CONSTANT(INT_TIMER3A),\
	_CPU_CONSTANT(INT_TIMER3B),\
	_CPU_CONSTANT(INT_I2C1),\
	_CPU_CONSTANT(INT_QEI1),\
	_CPU_CONSTANT(INT_CAN0),\
	_CPU_CONSTANT(INT_CAN1),\
	_CPU_CONSTANT(INT_CAN2),\
	_CPU_CONSTANT(INT_ETH),\
	_CPU_CONSTANT(INT_HIBERNATE),\
	_CPU_CONSTANT(INT_USB0),\
	_CPU_CONSTANT(INT_PWM3),\
	_CPU_CONSTANT(INT_UDMA),\
	_CPU_CONSTANT(INT_UDMAERR),\
	_CPU_CONSTANT(INT_UART_RX) */

#endif // #ifndef __PLATFORM_CONF_H__
