#include "builtins.h"
/*
	cd, set, echo, pwd, exit, help
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

int cd_builtin(unsigned short argc, char ** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: cd [directory]\n");
		return 1;
	}

	if (chdir(argv[1]))
	{
		perror("Failed to change directory");
		return 1;
	}
	return 0;
}

int set_builtin(unsigned short argc, char ** argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: set [variable] [value]\n");
		return 1;
	}
	if (setenv(argv[1], argv[2], 1))
	{
		perror("Failed to set variable");
		return 1;
	}
	return 0;
}

int echo_builtin(unsigned short argc, char ** argv)
{
	unsigned short i;

	if (argc == 1)
	{
		fprintf(stderr, "Usage: echo [text] <more text...>\n");
		return 1;
	}

	for (i = 1; i < argc; i++)
	{
		printf("%s ", argv[i]);
	}

	puts("");
	return 0;
}

int pwd_builtin(unsigned short argc, char ** argv)
{
	char * buf;
	unsigned int bufsize;

	(void)argv; /* shut up the compiler */

	if (argc != 1)
	{
		fprintf(stderr, "Usage: pwd\n");
		return 1;
	}

	buf = malloc(1024);
	bufsize = 1024;

	while(getcwd(buf, sizeof buf) == NULL)
	{
		if (errno == ERANGE)
		{
			bufsize += 1024;
			buf = realloc(buf, bufsize);
		}
		else
		{
			perror("Failed to get current working directory");
			free(buf);
			return 1;
		}
	}

	free(buf);
	return 0;
}

int exit_builtin(unsigned short argc, char ** argv)
{
	unsigned short i;
	unsigned short len;

	if (argc > 2)
	{
		fprintf(stderr, "Usage: exit <error code>\n");
		return 1;
	}

	if (argc == 1)
		exit(0);

	len = strlen(argv[1]);
	i = 0;
	if (argv[1][0] == '-') i++;

	for (; i < len; i++)
	{
		if (isdigit(argv[1][i]) == 0)
		{
			fprintf(stderr, "Not a valid exit code\n");
			return 1;
		}
	}

	exit( atoi(argv[1]) );
}

int exec_builtin(unsigned short argc, char ** argv)
{
	(void)argc;
	execvp(argv[0], argv);
	perror("Exec failed");
	return 1;
}

int help_builtin(unsigned short argc, char ** argv)
{
	(void)argc; (void)argv;

	puts
		(
			"Help:\n\n"

			"cd - Change directory\n"
			"\tUsage: cd [directory]\n"

			"set - Set enviromental variable\n"
			"\tUsage: set [variable] [value]\n"

			"echo - Echo arguments back\n"
			"\tUsage: echo [text] <more text...>\n"

			"pwd - Path of working directory\n"
			"\tUsage: pwd\n"

			"exit - Exits shell\n"
			"\tUsage: exit <error code>\n"

			"help - Display this.  If there's one that you don't want to forget, it's this one\n"
			"\tUsage: help\n"
		);

	return 0;
}