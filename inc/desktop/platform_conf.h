// picoc platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#ifdef PICOC_RPC
#include "picoc_mod.h"

#define BUILD_RPC
#define PICOCRPC_ENABLE_SERIAL

#define LUA_PLATFORM_LIBS_ROM \
	_ROM(PICOCLIB_RPC, &rpc_lib_setup_func, &rpc_library[0], NULL)
#endif

#endif // #ifndef __PLATFORM_CONF_H__

