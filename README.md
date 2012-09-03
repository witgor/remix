remix
=====

Remix Embedded

REMIX is an implementation of a paradigm for an interactive microcontroller development platform. It stives to be a minimal, free and open source operating system for 32-bit microcontrollers. It provides a convenient methord to do interactive embedded development in varius languages like  C (a subset of the ANSI C language based, on the PicoC interpreter), LISP and others (to be ported in the future). The system runs on bare metal or alternatively, can run as a part of complementary component of any kernel. Remix uses the MIT permissive free software license.
Features:
- UNIX-like shell 
- highly optimized for memory consumption
- FAT file system (e.g. for the SD/MMC card interface)
- supporet for significant part of the C std library
- TCP/uIP stack (in development) 
- XMODEM protocol for convenient way to share files code with the PC 
Changelog
1. First official release (0.1)
2. Supports various Cortex M3/M4 platforms
3. Shell commands (Basic UNIX like commands like ls, cp and cat which work directly with the SD/MMC interface)
4. Choice between FP and non FP
5. Generic platform modules (adc, can, i2c, platform data, gpio, pwm, spi, terminal, timer, uart, cpu specific module)
6. ROM file system (ROMFS)
7. Generic and portable SD/MMC card FAT file system
6. Linenoise support (Command line history and navigation -- an embedded replacement for GNU readline)
7. Samples: Morse code game, ADC poll (check romfs/)
8. C interrupt handler support
8. Tiny RAM patch. (It is now possible to accomodate cstdlib in PicoC running on M4, bare metal)
9. C files can be sent to target via XMODEM protocol.
10. Source under MIT free software license.


