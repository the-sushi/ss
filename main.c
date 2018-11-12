/* ----- INCLUDES ----- */
/* libc */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
/* OS/POSIX */
#include <unistd.h>
#include <signal.h>

#ifdef linux
#	include <linux/limits.h>
#else
#	include <sys/syslimits.h>
#endif

/* LIB */
#include <editline/readline.h>
/* Local */
#include "util.h"
#include "builtins.h"
#include "tables.h"


/* ----- PROMPT VALUES ----- */
#define FG_RED "\033[0;1;31m"
#define CLR "\033[0m"
#define BG_RED "\033[37;41m"
#define BG_WHITE "\033[30;47m"
#define PROMPT_END (" " FG_RED " > " CLR)

/* ----- VARIBALE DECLARATIONS ----- */

int ret_num;
unsigned argc;
extern struct builtin_func_s builtin_table[];

/* ----- FUNCION DECLARATIONS ----- */
int mk_routine(char * current[]);
int line_eval(char current[]);
int main(int argc, char * argv[]);


/* ----- MAIN CODE ---- */

int main(int argc, char * argv[])
{
	char * line;
	char prompt[PATH_MAX + 32];
	char tmp[PATH_MAX];
	unsigned i;

	(void)argc; /* Shut up the compiler */

	signal(SIGINT, SIG_IGN);
	using_history();

	if (realpath(argv[0], tmp) == NULL) strcpy(tmp, "???");

	setenv("SHELL", tmp, 1);

	while (exit_flag == 0)
	{
		if (ret_num)
			strcpy(prompt, BG_RED " ");
		else
			strcpy(prompt, BG_WHITE " ");

		getcwd(tmp, sizeof tmp);
		strcat(prompt, tmp);
		strcat(prompt, PROMPT_END);

		line = readline(prompt);
		if (line && *line) add_history(line);

		line_eval(line);
		free(line);
	}

	for (i = 0; i < routine_num; i++)
	{
		free(routines[i].name);

		for (routines[i].code_size--; routines[i].code_size > 0; routines[i].code_size--)
		{
			free(routines[i].code[routines[i].code_size]);
		}

		free(routines[i].code);
	}

	free(routines);

	return 0;
}

int line_eval(char current[])
{
	unsigned i, j;
	const char *cmd;
	char ** args;
	char * tmp;

	tmp = NULL;
	argc = 0;

	args = split_cmd(current);

	if (args == NULL)
	{
		if (errno == 1)
			fprintf(stderr, "Error: Quote incomplete\n");

		ret_num = 1;
		return 1;
	}

	if (args[0] == NULL)
	{
		ret_num = 0;
		return 0;
	}

	for (i = 0; i < routine_num; i++)
	{
		if (!strcmp(routines[i].name, args[0]))
		{
			for (j = 0; j < routines[i].code_size; j++)
				line_eval(routines[i].code[j]);
			return 0;
		}
	}

	/* Replace variables */
	for (i = 0; i < argc; i++)
	{
		if (args[i][0] == '$')
		{
			/* $? - Return value of the previous command */
			if (args[i][1] == '?' && args[i][2] == 0)
			{
				asprintf(&tmp, "%d", ret_num);
				args[i] = tmp;
			}
			else
			{
				args[i] = getenv(args[i] + 1);
			}
		}
	}

	/* Try builtins */
	for (i = 0; i < BUILTINS_NUM; i++)
	{
		cmd = builtin_table[i].cmd;

		if ( ! strcmp(args[0], cmd) )
		{
			ret_num = builtin_table[i].func(argc, args);
			goto end;
		}
	}

	ret_num = execute(args);

end:
	free(tmp);
	free(args);
	return 0;
}