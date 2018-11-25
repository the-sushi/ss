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
#ifdef DEBUG
	unsigned i;
#endif

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


	while (exit_flag == 0)
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

#ifdef DEBUG
	for (i = 0; i < routine_num; i++)
	{
		routine_clear(&routines[i]);
	}

	free(routines);
#endif

	return ret_num;
}

int line_eval(char current[], unsigned short routine_argc, char ** routine_args)
{
	unsigned i, j;
	const char * cmd;
	char ** args;
	char ** var_tmp;
	unsigned var_tmp_num;
	char * routine_tmp;
	unsigned argc;

	args        = NULL;
	routine_tmp = NULL;
	var_tmp     = NULL;
	var_tmp_num = 0;

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

						var_tmp = realloc(var_tmp, var_tmp_num * sizeof (char *));
						if (var_tmp == NULL) goto alloc_fail;

						asprintf(&var_tmp[var_tmp_num - 1], "%d", ret_num);
						if (var_tmp[var_tmp_num - 1] == NULL) goto alloc_fail;

						args[i] = var_tmp[var_tmp_num - 1];

						goto end_if;

					case '#':
						var_tmp_num++;

						var_tmp = realloc(var_tmp, var_tmp_num * sizeof (char *));
						if (var_tmp == NULL) goto alloc_fail;

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
			var_tmp_num++;

			var_tmp = realloc(var_tmp, var_tmp_num * sizeof (char *));
			if (var_tmp == NULL) goto alloc_fail;

			var_tmp[var_tmp_num - 1] = strdup(args[i] + 1);
			if (var_tmp[var_tmp_num - 1] == NULL) goto alloc_fail;

			args[i] = var_tmp[var_tmp_num - 1];
		}
	}

	/* Try routines */
	for (i = 0; i < routine_num; i++)
	{
		if (!strcmp(routines[i].name, args[0]))
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

		if ( ! strcmp(args[0], cmd) )
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

	free(args);
	return 0;

alloc_fail:
	perror("Allocation faliure");
	free(args);
	exit(1);
}