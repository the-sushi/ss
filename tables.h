#pragma once
#include "builtins.h"
/* Macros and defines */
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


struct routine_s * routines;
unsigned routine_num;