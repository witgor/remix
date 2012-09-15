// picoc shell

#include "shell.h"
#include "picoc.h"
#include "xmodem.h"
#include "version.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "platform.h"
#include "devman.h"
#include "buf.h"
#include "linenoise.h"

#include "platform_conf.h"
#ifdef BUILD_SHELL

// Shell alternate ' ' char
#define SHELL_ALT_SPACE           '\x07'

// EOF is different in UART mode and TCP/IP mode
#ifdef BUILD_CON_GENERIC
  #define SHELL_EOF_STRING        "CTRL+Z"
#else
  #define SHELL_EOF_STRING        "CTRL+D"
#endif

// Shell command handler function
typedef void( *p_shell_handler )( char* args );

// Command/handler pair structure
typedef struct
{
  const char* cmd;
  p_shell_handler handler_func;
} SHELL_COMMAND;

// Shell data
static char* shell_prog;

// ****************************************************************************
// Shell functions

// 'help' handler
static void shell_help( char* args )
{
  args = args;
  printf( "Shell commands:\n" );
  printf( "  exit         - exit from this shell\n" );
  printf( "  help         - print this help\n" );
  printf( "  ls or dir    - lists filesystems files and sizes\n" );
  printf( "  cat or type  - lists file contents\n" );
  printf( "  picoc [args] - run picoc with the given arguments\n" );
  printf( "  recv [path]  - receive a file via XMODEM and save in path\n" );
  printf( "  cp <src> <dst> - copy source file 'src' to 'dst'\n" );
  printf( "  ver          - print version details\n" );
}

// 'picoc' handler
static void shell_picoc( char* args )
{
  int nargs = 0;
  char* picoc_argv[ SHELL_MAX_PICOC_ARGS + 2 ];
  char *p, *prev, *temp;

  picoc_argv[ 0 ] = "picoc";
  // Process "args" if needed
  if( *args )
  {
    prev = args;
    p = strchr( args, ' ' );
    while( p )
    {
      if( nargs == SHELL_MAX_PICOC_ARGS )
      {
        printf( "Too many arguments to 'picoc' (maxim %d)\n", SHELL_MAX_PICOC_ARGS );
        return;
      }
      *p = 0;
      picoc_argv[ nargs + 1 ] = temp = prev;
      nargs ++;
      prev = p + 1;
      p = strchr( p + 1, ' ' );
      // If the argument is quoted, remove the quotes and transform the 'alternate chars' back to space
      if( *temp == '\'' || *temp == '"' )
      {
        temp ++;
        picoc_argv[ nargs ] = temp;
        while( *temp )
        {
          if( *temp == SHELL_ALT_SPACE )
            *temp = ' ';
          temp ++;
        }
        *( temp - 1 ) = '\0';
      }
    }
  }
  picoc_argv[ nargs + 1 ] = NULL;
  printf( "Press " SHELL_EOF_STRING " to exit picoc\n" );
  picoc_main( nargs + 1, picoc_argv );
  clearerr( stdin );
}

// 'recv' handler
static void shell_recv( char* args )
{
  args = args;

#ifndef BUILD_XMODEM
  printf( "XMODEM support not compiled, unable to recv\n" );
  return;
#else // #ifndef BUILD_XMODEM

  char *p;
  long actsize;
  
  if( strcmp( args, "" ) == 0 )
  {
    printf("Provide a file name to save. Ex: recv /mmc/foo.c\n" );
    return;
  }

  if( ( shell_prog = malloc( XMODEM_INITIAL_BUFFER_SIZE ) ) == NULL )
  {
    printf( "Unable to allocate memory\n" );
    return;
  }
  printf( "Waiting for file ... " );
  if( ( actsize = xmodem_receive( &shell_prog ) ) < 0 )
  {
    free( shell_prog );
    shell_prog = NULL;
    if( actsize == XMODEM_ERROR_OUTOFMEM )
      printf( "File too big\n" );
    else
      printf( "XMODEM error\n" );
    return;
  }
  // Eliminate the XMODEM padding bytes
  p = shell_prog + actsize - 1;
  while( *p == '\x1A' )
    p --;
  p ++;
  printf( "Done, got %u bytes\n", ( unsigned )( p - shell_prog ) );

  // we've received an argument, try saving it to a file
  if( strcmp( args, "") != 0 )
  {
    FILE *foutput = fopen( args, "w" );
    size_t file_sz = p - shell_prog;
    if( foutput == NULL )
    {
      printf( "Unable to open file %s\n", args);
      free( shell_prog );
      shell_prog = NULL;
      return;
    }
    if( fwrite( shell_prog, sizeof( char ), file_sz, foutput ) == file_sz )
      printf( "Received and saved as %s\n", args );
    else
      printf( "Disk full, unable to save file %s\n", args );
    fclose( foutput );
  }
  
  // automatically exec the file with picoc
  printf( "Executing the file ...\n\n" );
  char *argv[] = { "picoc", args, NULL };
  picoc_main( 2, argv );

  free( shell_prog );
  shell_prog = NULL;
#endif // #ifndef BUILD_XMODEM
}

// 'ver' handler
static void shell_ver( char* args )
{
  args = args;
  printf( "picoc version: %s\n", PICOC_VERSION );
  printf( "remix version: %s\n", REMIX_VERSION );
}

// 'ls' and 'dir' handler
static void shell_ls( char* args )
{
  const DM_DEVICE *pdev;
  unsigned dev, i;
  DM_DIR *d;
  struct dm_dirent *ent;
  u32 total;

  // Iterate through all devices, looking for the ones that can do "opendir"
  for( dev = 0; dev < dm_get_num_devices(); dev ++ )
  {  
    pdev = dm_get_device_at( dev );
    if( pdev->p_opendir_r == NULL || pdev->p_readdir_r == NULL || pdev->p_closedir_r == NULL )
      continue;
    d = dm_opendir( pdev->name );
    if( d )
    {
      total = 0;
      printf( "\n%s", pdev->name );
      while( ( ent = dm_readdir( d ) ) != NULL )
      {
        printf( "\n%s", ent->fname );
        for( i = strlen( ent->fname ); i <= DM_MAX_FNAME_LENGTH; i++ )
          printf( " " );
        printf( "%u bytes", ( unsigned )ent->fsize );
        total = total + ent->fsize;
      }
      printf( "\n\nTotal on %s: %u bytes\n", pdev->name, ( unsigned )total );
      dm_closedir( d );
    }
  }   
  printf( "\n" );
}

// 'cat' and 'type' handler
static void shell_cat( char *args )
{
  FILE *fp;
  int c;
  char *p;

// *args has an appended space. Replace it with the string terminator.
//  *(strchr( args, ' ' )) = 0;
  if ( *args )
    while ( *args ) 
    {
      p = strchr( args, ' ' );
      *p = 0;
      if( ( fp = fopen( args , "rb" ) ) != NULL )
      {
        c = fgetc( fp );
        while( c != EOF ) 
        {
          printf("%c", (char) c );  
          c = fgetc( fp );
        }
        fclose ( fp );
      }
      else
        printf( "File %s not found\n", args );
      args = p + 1;
    }      
  else
      printf( "Usage: cat (or type) <filename1> [<filename2> ...]\n" );
}

// 'copy' handler
#define SHELL_COPY_BUFSIZE    256
static void shell_cp( char *args )
{
  char *p1, *p2;
  int res = 0;
  FILE *fps = NULL, *fpd = NULL;
  void *buf = NULL;
  size_t datalen, total = 0;

  if( *args )
  {
    p1 = strchr( args, ' ' );
    if( p1 )
    {
      *p1 = 0;
      p2 = strchr( p1 + 1, ' ' );
      if( p2 )
      {
        *p2 = 0;
        // First file is at args, second one at p1 + 1
        if( ( fps = fopen( args, "rb" ) ) == NULL )
          printf( "Unable to open %s for reading\n", args );
        else
        {
          if( ( fpd = fopen( p1 + 1, "wb" ) ) == NULL )
            printf( "Unable to open %s for writing\n", p1 + 1 );
          else
          {
            // Alloc memory
            if( ( buf = malloc( SHELL_COPY_BUFSIZE ) ) == NULL )
              printf( "Not enough memory\n" );
            else
            {
              // Do the actual copy
              while( 1 )
              {
                datalen = fread( buf, 1, SHELL_COPY_BUFSIZE, fps );
                fwrite( buf, 1, datalen, fpd );
                total += datalen;
                if( datalen < SHELL_COPY_BUFSIZE )
                  break;
              }
              printf( "%u bytes copied\n", ( unsigned int )total );
              res = 1;
            }
          }
        } 
      }
    }
  }
  if( !res )
    printf( "Syntax error.\n" );
  if( fps )
    fclose( fps );
  if( fpd )
    fclose( fpd );
  if( buf )
    free( buf );
}

// Insert shell commands here
static const SHELL_COMMAND shell_commands[] =
{
  { "help", shell_help },
  { "picoc", shell_picoc },
  { "recv", shell_recv },
  { "ver", shell_ver },
  { "exit", NULL },
  { "ls", shell_ls },
  { "dir", shell_ls },
  { "cat", shell_cat },
  { "type", shell_cat },
  { "cp", shell_cp },
  { NULL, NULL }
};

#ifdef BUILD_BANNER
/* PicoC-remix banner */
static void shell_banner( void )
{
  printf( "\n\n ____  _            ____                         _      \n" );
  printf( "|  _ \\(_) ___ ___  / ___|     _ __ ___ _ __ ___ (_)_  __\n" );
  printf( "| |_) | |/ __/ _ \\| |   _____| '__/ _ \\ '_ ` _ \\| \\ \\/ /\n" );
  printf( "|  __/| | (_| (_) | |__|_____| | |  __/ | | | | | |>  < \n" );
  printf( "|_|   |_|\\___\\___/ \\____|    |_|  \\___|_| |_| |_|_/_/\\_\\\n" );
}
#endif

// Execute the remix "shell" in an infinite loop
void shell_start()
{
  char cmd[ SHELL_MAXSIZE + 1 ];
  char *p, *temp;
  const SHELL_COMMAND* pcmd;
  int i, inside_quotes;
  char quote_char;

#ifdef BUILD_BANNER
  /* print the banner */
  shell_banner();
#endif
  printf( SHELL_WELCOMEMSG, REMIX_VERSION );
  while( 1 )
  {
    while( linenoise_getline( LINENOISE_ID_SHELL, cmd, SHELL_MAXSIZE, SHELL_PROMPT ) == -1 )
    {
      printf( "\n" );
      clearerr( stdin );
    }
    if( strlen( cmd ) == 0 )
      continue;
    linenoise_addhistory( LINENOISE_ID_SHELL, cmd );
    if( cmd[ strlen( cmd ) - 1 ] != '\n' )
      strcat( cmd, "\n" );

    // Change '\r' and '\n' chars to ' ' to ease processing
    p = cmd;
    while( *p )
    {
      if( *p == '\r' || *p == '\n' )
        *p = ' ';
      p ++;
    }

    // Transform ' ' characters inside a '' or "" quoted string in
    // a 'special' char. We do this to let the user execute something
    // like "picoc -s 'quoted string'" without disturbing the quoted
    // string in any way.
    for( i = 0, inside_quotes = 0, quote_char = '\0'; i < strlen( cmd ); i ++ )
      if( ( cmd[ i ] == '\'' ) || ( cmd[ i ] == '"' ) )
      {
        if( !inside_quotes )
        {
          inside_quotes = 1;
          quote_char = cmd[ i ];
        }
        else
        {
          if( cmd[ i ] == quote_char )
          {
            inside_quotes = 0;
            quote_char = '\0';
          }
        }
      }
      else if( ( cmd[ i ] == ' ' ) && inside_quotes )
        cmd[ i ] = SHELL_ALT_SPACE;
    if( inside_quotes )
    {
      printf( "Invalid quoted string\n" );
      continue;
    }

    // Transform consecutive sequences of spaces into a single space
    p = strchr( cmd, ' ' );
    while( p )
    {
      temp = p + 1;
      while( *temp && *temp == ' ' )
        memmove( temp, temp + 1, strlen( temp ) );
      p = strchr( p + 1, ' ' );
    }
    if( strlen( cmd ) == 1 )
      continue;

    // Look for the first ' ' to separate the command from its args
    temp = cmd;
    if( *temp == ' ' )
      temp ++;
    if( ( p = strchr( temp, ' ' ) ) == NULL )
    {
      printf( SHELL_ERRMSG );
      continue;
    }
    *p = 0;
    i = 0;
    while( 1 )
    {
      pcmd = shell_commands + i;
      if( pcmd->cmd == NULL )
      {
        printf( SHELL_ERRMSG );
        break;
      }
      if( !strcasecmp( pcmd->cmd, temp ) )
      {
        // Special case: the "exit" command has a NULL handler
        if( pcmd->handler_func )
          pcmd->handler_func( p + 1 );
        break;
      }
      i ++;
    }
    // Check for 'exit' command
    if( pcmd->cmd && !pcmd->handler_func )
      break;
  }
  // Shell exit point
  if( shell_prog )
    free( shell_prog );
}

// Initialize the shell, returning 1 for OK and 0 for error
int shell_init()
{
  shell_prog = NULL;
  return 1;
}

#else // #ifdef BUILD_SHELL

int shell_init()
{
  return 0;
}

void shell_start()
{
}

#endif // #ifdef BUILD_SHELL
