#pragma once

/* Includes */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* POSIX / OS */
#include <unistd.h>

#ifdef linux
#	include <linux/limits.h>
#else
#	include <sys/syslimits.h>
#endif

/* Macros and defines */
#define BUILTINS_NUM 11
#define DECL_BUILTIN(NAME) int NAME##_builtin(unsigned short argc, char ** argv)
#define CMD_ITEM(NAME) {#NAME, &NAME##_builtin}

/* Structure definitions */
struct builtin_func_s
{
	const char * cmd;
	int (*func)(unsigned short argc, char ** argv);
};

struct routine_s
{
	char *  name;
	char ** code;
	unsigned code_size;
};

extern struct builtin_func_s builtin_table[];

/* Builtin Declarations */
DECL_BUILTIN(cd);
DECL_BUILTIN(set);
DECL_BUILTIN(unset);
DECL_BUILTIN(echo);
DECL_BUILTIN(pwd);
DECL_BUILTIN(exit);
DECL_BUILTIN(exec);
DECL_BUILTIN(routine);
DECL_BUILTIN(unroutine);
DECL_BUILTIN(listroutines);
DECL_BUILTIN(help);

/* Normal function declarations */
unsigned split_cmd (char *** args, char * line);
int execute (char ** args);
void routine_clear (struct routine_s * routine);
int fp_set(FILE * fp, char * loc, char * mode);
int var_swap (char *** var_tmp, char ** arg, unsigned var_num, unsigned short routine_argc, char ** routine_args);
char line_run(char current[], unsigned short routine_argc, char ** routine_args);
void args_eval(unsigned short argc, char ** args);

/* Variable declarations */
char path[PATH_MAX];
struct routine_s * routines;
unsigned routine_num;
int stdout_bak;
int stdin_bak;
int ret_num;
