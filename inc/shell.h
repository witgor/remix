// picoc shell 

#ifndef __SHELL_H__
#define __SHELL_H__

#define SHELL_WELCOMEMSG      "\npicoc-remix %s copyright (C) 2012 ramangopalan@gmail.com\n"
#define SHELL_PROMPT          "remix# "
#define SHELL_ERRMSG          "Invalid command, type 'help' for help\n"
#define SHELL_MAXSIZE         50
#define SHELL_MAX_PICOC_ARGS  3
#define SHELL_MAX_IV_ARGS     1
#define REMIX_VERSION	      "v0.1"

int shell_init();
void shell_start();

#endif // #ifndef __SHELL_H__
