#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "type.h"
#include "devman.h"
#include "platform.h"
#include "romfs.h"
#include "shell.h"
#include "picoc.h"
#include "term.h"
#include "platform_conf.h"

// Validate eLua configuration options
#include "validate.h"

#include "mmcfs.h"
#include "romfs.h"
#include "semifs.h"

// Define here your autorun/boot files, 
// in the order you want eLua to search for them
char *boot_order[] = {
#if defined(BUILD_MMCFS)
  "/mmc/autorun.c",
  "/mmc/autorun.pc",
#endif
#if defined(BUILD_ROMFS)
  "/rom/autorun.c",
  "/rom/autorun.pc",
#endif
};

extern char etext[];

// ****************************************************************************
//  Program entry point

int main( void )
{
  int i;
  FILE* fp;

  // Initialize platform first
  if( platform_init() != PLATFORM_OK )
  {
    // This should never happen
    while( 1 );
  }

  // Initialize device manager
  dm_init();

  // Register the ROM filesystem
  dm_register( romfs_init() );

  // Register the MMC filesystem
  dm_register( mmcfs_init() );

  // Register the Semihosting filesystem
  dm_register( semifs_init() );

  // Search for autorun files in the defined order and execute the 1st if found
  for( i = 0; i < sizeof( boot_order ) / sizeof( *boot_order ); i++ )
  {
    if( ( fp = fopen( boot_order[ i ], "r" ) ) != NULL )
    {
      fclose( fp );
      char* picoc_argv[] = { "picoc", boot_order[i], NULL };
      picoc_main( 2, picoc_argv );
      break; // autoruns only the first found
    }
  }

  // Run the shell
  if( shell_init() == 0 )
  {
    // Start picoc directly
    char* picoc_argv[] = { "picoc", NULL };
    picoc_main( 1, picoc_argv );
  }
  else
    shell_start();

  while( 1 );
}
