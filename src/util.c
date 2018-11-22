/* ----- INCLUDES ----- */

/* Standard library */
#include <stdlib.h>

/* Local */
#include "globals.h"

/* ----- VARIABLE DECLARATIONS ----- */
extern unsigned argc;

/* ----- CODE ----- */

char * tok_next(char * str, char delim, char quote)
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

	while (*ptr == delim) ptr++;

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
		else if (*loc == '\\' && *(loc + 1) != '$')
		{
			if (loc == start)
			{
				start++;
			}

			/* Swap quote at end of token */
			if ( *(loc + 1) == '"' && (*(loc + 2) == delim || *(loc + 2) == 0))
			{
				*loc = '"';
				*(loc + 1) = *(loc + 2);
			}

			loc++;
		}
		else if ( *loc == quote )
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
			*loc = 0;
			ptr = ++loc;
			break;
		}
	}

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

	arg = tok_next(line, ' ', '"');
	(*args)[argc - 1] = arg;

	while (( arg = tok_next(NULL, ' ', '"') ) != NULL)
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
		return 0;
	}

	(*args)[argc] = NULL;
	return argc;

malloc_fail:
	perror("malloc() failed");
	exit_flag = 1;
	return 0;
realloc_fail:
	perror("realloc() failed");
	exit_flag = 1;
	return 0;
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