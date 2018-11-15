/* ----- Includes  ----- */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

/* Local */
#include "builtins.h"
#include "tables.h"
#include "util.h"

/* OS/POSIX */
#include <unistd.h>
#include <signal.h>

#ifdef linux
#	include <linux/limits.h>
#else
#	include <sys/syslimits.h>
#endif

/* LIBRARY */
#include <editline/readline.h>

/* ----- DECLARATIONS ----- */
struct builtin_func_s builtin_table[] =
	{
		CMD_ITEM(cd),
		CMD_ITEM(set),
		CMD_ITEM(echo),
		CMD_ITEM(pwd),
		CMD_ITEM(exit),
		CMD_ITEM(exec),
		CMD_ITEM(routine),
		CMD_ITEM(unroutine),
		CMD_ITEM(listroutines),
		CMD_ITEM(help),
	};

/* ----- CODE ----- */

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

	setenv("OWD", path, 1);

	if (getwd(path) != NULL)
	{
		setenv("PWD", path, 1);
	}
	else
	{
		perror("Failed to set $PWD:");
		setenv("SHELL", "????", 1);
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
	void * tmp;
	unsigned int bufsize;

	(void)argv; /* shut up the compiler */

	if (argc != 1)
	{
		fprintf(stderr, "Usage: pwd\n");
		return 1;
	}

	buf = malloc(PATH_MAX);
	if (buf == NULL) goto malloc_fail;

	bufsize = PATH_MAX;

	while(getcwd(buf, bufsize) == NULL)
	{
		if (errno == ERANGE)
		{
			bufsize += 128;
			tmp = realloc(buf, bufsize);
			if (tmp == NULL) goto realloc_fail;
			buf = tmp;
		}
		else
		{
			perror("Failed to get current working directory");
			free(buf);
			return 1;
		}
	}

	puts(buf);

	free(buf);
	return 0;

malloc_fail:
	perror("malloc() failed");
	return 1;
realloc_fail:
	perror("realloc() failed");
	return 1;
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

	exit_flag = 1;
	return atoi(argv[1]);
}

int exec_builtin(unsigned short argc, char ** argv)
{
	(void)argc;
	execvp(argv[0], argv);
	perror("Exec failed");
	return 1;
}

int routine_builtin(unsigned short argc, char ** argv)
{
	char * line;
	void * tmp;
	struct routine_s * current;
	unsigned i;

	if (argc != 2)
	{
		fprintf(stderr,
			"Usage:\n"
			"  routine [name]\n"
			"    code\n"
			"    end\n"
		);
		return 1;
	}

	if (routine_num == 0)
	{
		routines = malloc(sizeof (struct routine_s));
		if (routines == NULL) goto malloc_fail;
	}
	else
	{
		for (i = 0; i < routine_num; i++)
		{
			if (! strcmp(routines[i].name, argv[1]))
			{
				fprintf(stderr, "Error: Routine exists\n");
				return 1;
			}
		}
		tmp = realloc(routines, sizeof (struct routine_s) * (routine_num+1));
		if (tmp == NULL) goto realloc_fail;
		routines = tmp;
	}
	routine_num++;

	current = &routines[routine_num - 1];

	current->code = NULL; /* realloc(NULL, size) acts like malloc(size) */

	current->name = strdup(argv[1]);
	if (current->name == NULL) goto malloc_fail;
	current->code_size = 0;

	while ( (line = readline("  routine> ")) && strcmp(line, "end") )
	{
		current->code_size++;

		current->code = realloc(current->code, sizeof(char *) * current->code_size);
		if (current->code == NULL) goto realloc_fail;

		current->code[current->code_size - 1] = line;
	}

	free(line);

	return 0;
malloc_fail:
	perror("malloc() failed");
	exit_flag = 1;
	return 1;
realloc_fail:
	perror("realloc() failed");
	exit_flag = 1;
	return 1;
}

int unroutine_builtin(unsigned short argc, char ** argv)
{
	unsigned i;
	struct routine_s * current;
	void * tmp;

	if (argc != 2)
	{
		fprintf(stderr,
			"Usage:\n"
			"\t~> routine [name]\n"
			"\t  routine> [code]\n"
			"\t  routine> end\n"
		);
		return 1;
	}

	for (i = 0; i < routine_num; i++)
	{
		current = &routines[i];
		if (!strcmp(current->name, argv[1]))
		{
			routine_clear(current);

			/* Shift routines over */
			for (; i < routine_num - 1; i++)
				routines[i] = routines[i + 1];

			if (--routine_num == 0)
			{
				free(routines);
			}
			else
			{
				tmp = realloc(routines, sizeof(struct routine_s) * routine_num);
	
				if (tmp == NULL)
				{
					fprintf(stderr, "Error: realloc failed");
					exit_flag = 1;
					return 1;
				}

				routines = tmp;
			}

			break;
		}
	}

	return 0;
}

int listroutines_builtin(unsigned short argc, char ** argv)
{
	unsigned i;

	(void)argv;

	if (argc != 1)
	{
		fprintf(stderr, "Usage: listroutines\n");
		return 1;
	}

	for (i = 0; i < routine_num; i++)
	{
		puts(routines[i].name);
	}

	return 0;
}

int help_builtin(unsigned short argc, char ** argv)
{
	(void)argc; (void)argv;

	puts
		(
			"Help:\n\n"

			"cd - Change directory\n"
			"\tUsage: cd [directory]\n"

			"\nset - Set enviromental variable\n"
			"\tUsage: set [variable] [value]\n"

			"\necho - Echo arguments back\n"
			"\tUsage: echo [text] <more text...>\n"

			"\npwd - Path of working directory\n"
			"\tUsage: pwd\n"

			"\nexit - Exits shell\n"
			"\tUsage: exit <error code>\n"

			"\nroutine - Creates a new routine\n"
			"\tUsage:\n"
			"\t~ > routine [name]\n"
			"\t  routine> [code]\n"
			"\t  routine> end\n"

			"\nunroutine - Deletes a routine\n"
			"\tUsage: unroutine [name]\n"

			"\nhelp - Display this.  If there's one command that you don't want to forget, it's this one\n"
			"\tUsage: help\n"
		);

	return 0;
}