/* ----- INCLUDES ----- */
/* libc */
#include <stdlib.h>

/* OS/POSIX */
#include <signal.h>

/* LIB */
#include <editline/readline.h>

/* Local */
#include "globals.h"


/* ----- PROMPT VALUES ----- */
#define FG_RED "\033[0;1;31m"
#define CLR "\033[0m"
#define BG_RED "\033[37;41m"
#define BG_WHITE "\033[30;47m"
#define PROMPT_END (" " FG_RED " > " CLR)

/* ----- MAIN CODE ---- */

int main(int argc, char * argv[])
{
	char * line;
	char prompt[PATH_MAX + 16];
	char * shell_path;

	signal(SIGINT, SIG_IGN);
	using_history();

	if ((shell_path = realpath(argv[0], NULL)) != NULL)
	{
		setenv("SHELL", shell_path, 1);
		free(shell_path);
	}
	else
	{
		fprintf(stderr, "Error: Failed to get shell path");
		setenv("SHELL", "????", 1);
	}

	if (getwd(path) != NULL)
	{
		setenv("PWD", path, 1);
	}
	else
	{
		fprintf(stderr, "Error: Failed to get current directory!\n");
		setenv("PWD", "????", 1);
	}

	stdout_bak = dup(STDOUT_FILENO);
	stdin_bak  = dup(STDOUT_FILENO);

	if (stdout_bak == -1)
	{
		perror("Failed to save stdout");
		return 1;
	}

	while (1)
	{
		if (ret_num) strcpy(prompt, BG_RED " ");
		else         strcpy(prompt, BG_WHITE " ");

		strncat(prompt, path, PATH_MAX);
		strcat(prompt, PROMPT_END);

		line = readline(prompt);

		if (line && *line) add_history(line);

		line_run(line, argc, argv);
		free(line);
	}

	return ret_num;
}

char line_run(char current[], unsigned short routine_argc, char ** routine_args)
{
	/** DECLARATIONS **/
	/* Loop counters and temp data */
	unsigned i = 0;

	/* Flags */
	uint8_t redirect = 0;
	/* 000000[stdin][stdout] */

	/* For the command itself */
	char ** args = NULL;
	unsigned argc;

	/* For variables */
	char ** var_tmp = NULL;
	int var_num = 0;


	/** CODE **/
	argc = split_cmd(&args, current);

	/* Failed */
	if (argc == 0)
	{
		ret_num = 1;
		return -1;
	}

	/* Blank command */
	if (args[0] == NULL)
	{
		free(args);
		ret_num = 0;
		return 0;
	}


	/* TODO: Rewrite this to allow for lisp-style nesting and `x >> y < z`, because this is awful.
	 * It would probably be simpler with xstr, but I'm not sure if I want to require that
	 */
	for (; i < argc; i++)
	{
		if (args[i][0] == '$')
		{
			var_num = var_swap(&var_tmp, args + i, var_num, routine_argc, routine_args);
			if (var_num == -1)
			{
				ret_num = 1;
				return -1;
			}
		}

		else if (args[i + 2] == NULL)
		{
			if (strcmp(args[i], ">>") == 0)
			{
				if (fp_set(stdout, args[i+1], "a") == 1)
				{
					ret_num = 1;
					goto cleanup;
				}
	
				redirect |= 1;
				args[i] = NULL;
				argc = i;
				break;
			}
	
			else if (strcmp(args[i], ">") == 0)
			{
				if (fp_set(stdout, args[i+1], "w") == 1)
				{
					ret_num = 1;
					goto cleanup;
				}
	
				redirect |= 1;
				args[i] = NULL;
				argc = i;
				break;
			}
	
			else if (strcmp(args[i], "<") == 0)
			{
				if (fp_set(stdin, args[i+1], "r") == 1)
				{
					ret_num = 1;
					goto cleanup;
				}
	
				redirect |= 2;
				args[i] = NULL;
				argc = i;
				break;
			}
		}
	}

	args_eval(argc, args);

	/** CLEANUP **/
cleanup:
	if (var_tmp != NULL)
	{
		for (var_num--; var_num != 0; var_num--) free(var_tmp[var_num]);
		free(var_tmp);
	}

	if ((redirect & 1) != 0)
	{
		fflush(stdout);
		dup2(stdout_bak, STDOUT_FILENO);
		if (stdout == NULL)
		{
			perror("Failed to restore stdout");
			ret_num = 1;
		}
	}

	if ((redirect & 2) != 0)
	{
		dup2(stdin_bak, STDIN_FILENO);
		if (stdin == NULL)
		{
			perror("Failed to restore stdin");
			ret_num = 1;
		}
	}

	free(args);
	return 0;
}