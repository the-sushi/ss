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

/* ----- VARIBALE DECLARATIONS ----- */
int ret_num;
extern struct builtin_func_s builtin_table[];

/* ----- FUNCION DECLARATIONS ----- */
int mk_routine(char * current[]);
int line_eval(char current[], unsigned short routine_argc, char * routine_args[]);
int main(int argc, char * argv[]);


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

		line_eval(line, argc, argv);
		free(line);
	}

	return ret_num;
}

int line_eval(char current[], unsigned short routine_argc, char ** routine_args)
{
	unsigned i, j;

	const char * cmd;
	char ** args;
	unsigned argc;

	char ** var_tmp;
	unsigned var_tmp_num;
	char * routine_tmp;
	void * tmp;

	uint8_t stdout_redirect;

	args        = NULL;
	routine_tmp = NULL;
	var_tmp     = NULL;
	var_tmp_num = 0;
	stdout_redirect = 0;
	errno = 0;

	argc = split_cmd(&args, current);

	if (argc == 0)
	{
		if (errno == 1) fprintf(stderr, "Error: Quote incomplete\n");

		ret_num = 1;
		return 1;
	}

	/* Blank command */
	if (args[0] == NULL)
	{
		free(args);
		ret_num = 0;
		return 0;
	}

	/* Replace variables */
	for (i = 0; i < argc; i++)
	{
		if (args[i][0] == '$')
		{
			/* $? - Return value of the previous command */
			if (args[i][2] == 0)
			{
				switch (args[i][1])
				{
					case '?':
						var_tmp_num++;

						tmp = realloc(var_tmp, var_tmp_num * sizeof (char *));
						if (tmp == NULL) goto alloc_fail;
						var_tmp = tmp;

						asprintf(&var_tmp[var_tmp_num - 1], "%d", ret_num);
						if (var_tmp[var_tmp_num - 1] == NULL) goto alloc_fail;

						args[i] = var_tmp[var_tmp_num - 1];

						goto end_if;

					case '#':
						var_tmp_num++;

						tmp = realloc(var_tmp, var_tmp_num * sizeof (char *));
						if (tmp == NULL) goto alloc_fail;
						var_tmp = tmp;

						asprintf(&var_tmp[var_tmp_num - 1], "%d", routine_argc);
						if (var_tmp[var_tmp_num - 1] == NULL) goto alloc_fail;

						args[i] = var_tmp[var_tmp_num - 1];

						goto end_if;
				}

				if (args[i][1] >= '0' && args[i][1] <= '9')
				{
					if (routine_argc - 1 < args[i][1] - '0')
					{
						fprintf
							(
								stderr,
								"Error: $%c (%d) is out of range - argc is %d\n",
								args[i][1],
								args[i][1] - '0',
								routine_argc
							);
						goto end;
					}

					args[i] = routine_args[args[i][1] - '0'];
					goto end_if;
				}
			}

			args[i] = getenv(args[i] + 1);

end_if:
			;
		}

		else if (args[i][0] == '\\')
		{
			fflush(stdout);
			var_tmp_num++;

			var_tmp = realloc(var_tmp, var_tmp_num * sizeof (char *));
			if (var_tmp == NULL) goto alloc_fail;

			var_tmp[var_tmp_num - 1] = strdup(args[i] + 1);
			if (var_tmp[var_tmp_num - 1] == NULL) goto alloc_fail;

			args[i] = var_tmp[var_tmp_num - 1];
		}

		else if (strcmp(args[i], ">>") == 0)
		{
			if (stdout_set(args[i+1], "a") == 1)
			{
				ret_num = 1;
				goto end;
			}
			stdout_redirect = 1;
			args[i] = NULL;
			argc = i;
		}

		else if (strcmp(args[i], ">") == 0)
		{
			if (stdout_set(args[i+1], "w") == 1)
			{
				ret_num = 1;
				goto end;
			}
			stdout_redirect = 1;
			args[i] = NULL;
			argc = i;
		}
	}

	/* Try routines */
	for (i = 0; i < routine_num; i++)
	{
		if (strcmp(routines[i].name, args[0]) == 0)
		{
			for (j = 0; j < routines[i].code_size; j++)
			{
				routine_tmp = strdup(routines[i].code[j]);
				if (routine_tmp == NULL) goto alloc_fail;

				line_eval(routine_tmp, argc, args);
				free(routine_tmp);
			}

			goto end;
		}
	}

	/* Try builtins */
	for (i = 0; i < BUILTINS_NUM; i++)
	{
		cmd = builtin_table[i].cmd;

		if ( strcmp(args[0], cmd) == 0 )
		{
			ret_num = builtin_table[i].func(argc, args);
			goto end;
		}
	}

	ret_num = execute(args);

end:

	if (var_tmp != NULL)
	{
		for (var_tmp_num--; var_tmp_num != 0; var_tmp_num--) free(var_tmp[var_tmp_num]);
		free(var_tmp);
	}

	if (stdout_redirect == 1)
	{
		fflush(stdout);
		dup2(stdout_bak, STDOUT_FILENO);
		if (stdout == NULL)
		{
			perror("Failed to restore stdout");
			ret_num = 1;
		}
	}

	free(args);
	return 0;

alloc_fail:
	perror("Allocation faliure");
	free(args);
	if (var_tmp != NULL)
	{
		for (var_tmp_num--; var_tmp_num != 0; var_tmp_num--) free(var_tmp[var_tmp_num]);
		free(var_tmp);
	}
	exit(1);
}