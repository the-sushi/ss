/* ----- INCLUDES ----- */

/* Standard library */
#include <stdlib.h>

/* Local */
#include "globals.h"

/* ----- VARIABLE DECLARATIONS ----- */
extern unsigned argc;

/* ----- CODE ----- */

char * tok_next(char * str, char delim)
{
	static char * ptr;
	char * start;
	char * loc;
	static unsigned char quote_on;

	/* NULL - Continue with previous */
	if (str != NULL)
	{
		quote_on = 0;
		ptr = str;
	}

	if (ptr == NULL)
	{
		if (quote_on != 0) errno = 1;
		else errno = 0;

		return NULL;
	}


	for
		(
			start = ptr, loc = ptr ;
			/* nothing */ ;
			loc++
		)
	{
		if (*loc == 0)
		{
			ptr = NULL;
			break;
		}

		/* Check for escape */
		if (*loc == '\\' && *(loc + 1) != '$')
		{
			if (loc == start)
			{
				start++;
			}

			/* Swap quote at end of token */
			if ( *(loc + 1) == '"' && (*(loc + 2) == delim || *(loc + 2) == 0))
			{
				*loc = '"';
				*(loc + 1) = 0;
			}

			loc++;
		}
		else if ( *loc == '"' )
		{
			/* There can't be a backslash if there's nothing before us */
			if (loc == start)
			{
				start++;
				loc++;
				quote_on = !quote_on;
			}
			else if
				(
					/* make sure we're at the end of a token */
					*(loc + 1) == delim ||
					*(loc + 1) == 0
				)
			{
				*loc = 0;
				quote_on = !quote_on;
			}
		}

		if (*loc == delim && quote_on == 0)
		{
			while (*loc == delim)
			{
				*loc = 0;
				ptr = ++loc;
			}
			break;
		}
	}

	if (start == loc) return NULL;
	return start;
}


unsigned split_cmd(char *** args, char * line)
{
	char * arg;
	void * tmp;
	unsigned argc;

	argc = 1;

	*args = malloc(argc * sizeof (char *));
	if (*args == NULL) goto malloc_fail;

	arg = tok_next(line, ' ');
	(*args)[0] = arg;

	while (( arg = tok_next(NULL, ' ') ) != NULL)
	{
		argc++;

		tmp = realloc(*args, (argc + 1) * sizeof (char *));
		if (tmp == NULL) goto realloc_fail;

		*args = tmp;
		(*args)[argc - 1] = arg;
	}

	if (errno != 0)
	{
		free(*args);
		fprintf(stderr, "Error: Quote incomplete");

		return 0;
	}

	(*args)[argc] = NULL;
	return argc;

malloc_fail:
	perror("malloc() failed");
	exit(1);

realloc_fail:
	perror("realloc() failed");
	exit(1);
}


int execute(char ** args)
{
	pid_t pid;
	int status;

	pid = fork();

	if (pid < 0)
	{
		perror("Fork failed");
		return 1;
	}
	else if (pid == 0)
	{
		/* Child */
		execvp(args[0], args);

		/* execvp() should've replaced the process */
		fprintf(stderr, "Failed: %s (%s)\n", strerror(errno), args[0]);
		exit(1);
	}
	else
	{
		/* Parent */
		do
		{
			waitpid(pid, &status, WUNTRACED | WCONTINUED);
		}
		while ( !WIFEXITED(status) && !WIFSIGNALED(status) );

		/* putchar('\n'); */
		return WEXITSTATUS(status);
	}
}


void routine_clear (struct routine_s * routine)
{
	free(routines->name);

	for (; routine->code_size == 0; routines->code_size--)
	{
		free(routines->code[routines->code_size - 1]);
	}

	free(routines->code);
}


int fp_set(FILE * fp, char * loc, char * mode)
{
	fflush(fp);
	if (freopen(loc, mode, fp) == NULL)
	{
		perror("Failed to redirect file pointer");
		return 1;
	}

	return 0;
}


int var_swap(char *** var_tmp, char ** arg, unsigned var_num, unsigned short routine_argc, char ** routine_args)
{
	void * tmp;

	/* Only one char long */
	if ((*arg)[2] == 0)
	{

		switch ((*arg)[1])
		{
			case '?':
				var_num++;

				tmp = realloc(*var_tmp, var_num * sizeof (char *));
				if (tmp == NULL) goto alloc_fail;
				*var_tmp = tmp;

				asprintf(var_tmp[var_num - 1], "%d", ret_num);
				if (*var_tmp[var_num - 1] == NULL) goto alloc_fail;

				*arg = *var_tmp[var_num - 1];

				return var_num;

			case '#':
				var_num++;

				tmp = realloc(*var_tmp, var_num * sizeof (char *));
				if (tmp == NULL) goto alloc_fail;
				*var_tmp = tmp;

				asprintf(var_tmp[var_num - 1], "%d", routine_argc);
				if (*var_tmp[var_num - 1] == NULL) goto alloc_fail;

				*arg = *var_tmp[var_num - 1];

				return var_num;
		}

		if ((*arg)[1] >= '0' && (*arg)[1] <= '9')
		{
			if (routine_argc - 1 < (*arg)[1] - '0')
			{
				fprintf
					(
						stderr,
						"Error: $%c (%d) is out of range - argc is %d\n",
						(*arg)[1],
						(*arg)[1] - '0',
						routine_argc
					);
				return -1;
			}

			*arg = routine_args[(*arg)[1] - '0'];
		}

		return var_num;
	}

	*arg = getenv(*arg + 1);
	return 0;

alloc_fail:
	perror("Allocation failed");
	free(*var_tmp);
	return -1;
}


void args_eval(unsigned short argc, char ** args)
{
	unsigned i = 0, j = 0;
	char * routine_tmp;
	const char * cmd;

	/* Try routines */
	for (; i < routine_num; i++)
	{
		if (strcmp(routines[i].name, args[0]) == 0)
		{
			for (j = 0; j < routines[i].code_size; j++)
			{
				routine_tmp = strdup(routines[i].code[j]);
				if (routine_tmp == NULL)
				{
					ret_num = errno;
					perror("String duplication faliure");
					return;
				}

				if (line_run(routine_tmp, argc, args) == -1)
				{
					free(routine_tmp);
					ret_num = 1;
					return;
				}

				free(routine_tmp);
			}

			ret_num = 0;
			return;
		}
	}

	/* Try builtins */
	for (; i < BUILTINS_NUM; i++)
	{
		cmd = builtin_table[i].cmd;

		if ( strcmp(args[0], cmd) == 0 )
		{
			ret_num = builtin_table[i].func(argc, args);
			return;
		}
	}

	ret_num = execute(args);
	return;
}