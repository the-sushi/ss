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
	#include <linux/limits.h>
#else
	#include <sys/syslimits.h>
#endif

/* LIB */
#include <editline/readline.h>

/* Local */
#include "builtins.h"
#include "util.h"
#include "builtin_table.h"


/* ----- ESCAPE SEQUENCES ----- */
#define FG_RED "\033[0;1;31m"
#define CLR "\033[0m"
#define BG_RED "\033[37;41m"
#define BG_WHITE "\033[30;47m"


/* ----- GLOBAL VARIABLE DECLARATIONS ----- */

int lin_ret;
unsigned short argc;


/* ----- MAIN CODE ---- */

int lin_eval(char current[])
{
	unsigned int i;
	const char *cmd;
	char ** args;
	char * tmp;

	tmp = NULL;
	args = split_cmd(current);

	if (args == NULL)
	{
		if (errno == 1)
			fprintf(stderr, "Error: Quote incomplete\n");
		else if (errno == 2)
			fprintf(stderr, "Error: Quote mid-token\n");

		lin_ret = 1;
		return 1;
	}
	if (args[0] == NULL)
	{
		lin_ret = 0;
		return 0;
	}

	/* Replace variables */
	for (i = 0; i < argc; i++)
	{
		if (args[i][0] == '$')
		{
			if (args[i][1] == '?' && args[i][2] == 0)
			{
				asprintf(&tmp, "%d", lin_ret);
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

		if ( ! strcmp(current, cmd) )
		{
			lin_ret = builtin_table[i].func(argc, args);
			goto end;
		}
	}

	lin_ret = execute(args);

end:
	free(tmp);
	free(args);
	return 0;
}


int main(int argc, char * argv[])
{
	char * line;
	char prompt[PATH_MAX + 128];
	char tmp[PATH_MAX];
	const char endp[] = " " FG_RED " > " CLR;

	(void)argc;

	signal(SIGINT, SIG_IGN);
	using_history();

	if (realpath(argv[0], tmp) == NULL) strcpy(tmp, "???");

	setenv("SHELL", tmp, 1);

	while (1)
	{
		if (!lin_ret)
			strcpy(prompt, BG_WHITE " ");
		else
			strcpy(prompt, BG_RED " ");

		getcwd(tmp, sizeof tmp);
		strcat(prompt, tmp);
		strcat(prompt, endp);

		line = readline(prompt);
		if (line && *line) add_history(line);
		lin_eval(line);
		free(line);
	}
	return 0;
}