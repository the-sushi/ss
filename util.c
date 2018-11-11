/* ----- INCLUDES ----- */

/* Standard library */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
/* POSIX */
#include <unistd.h>

/* ----- VARIABLE DECLARATIONS ----- */
extern unsigned argc;


/* ----- CODE ----- */

char * tok_next(char * str, char delim, char quote)
{
	static char * ptr;
	char *start, *end;
	static unsigned char quote_on;

	/* NULL - Continue with previous */
	if (str != NULL)
	{
		quote_on = 0;
		ptr = str;
	}

	if (ptr == NULL)
	{
		if (quote_on != 0)
			errno = 1;
		else
			errno = 0;
		return NULL;
	}

	for (start = ptr, end = ptr; ; end++)
	{
		if (*end == 0)
		{
			ptr = NULL;
			break;
		}

		if ( *end == quote )
		{
			/* There can't be a backslash if there's nothing before us */
			if (end == start)
			{
				start++;
				end++;
				quote_on = !quote_on;
			}
			else if
				(
					/* Check for backslash, make sure we're at the end of a token */
					(*(end - 1) != '\\')
					&&
					(*(end + 1) == delim || *(end + 1) == 0)
				)
			{
				*end = 0;
				quote_on = !quote_on;
			}
		}

		if (*end == delim && quote_on == 0)
		{
			*end = 0;
			ptr = ++end;
			break;
		}
	}
	return start;
}

char ** split_cmd(char * line)
{
	char ** args;
	char * arg;

	argc++;
	args = malloc(argc * sizeof (char *));
	if (args == NULL) goto malloc_fail;

	arg = tok_next(line, ' ', '"');
	args[argc - 1] = arg;

	while (( arg = tok_next(NULL, ' ', '"') ) != NULL)
	{
		argc++;
		args = realloc(args, argc * sizeof (char *));
		if (args == NULL) goto realloc_fail;

		args[argc - 1] = arg;
	}

	if (errno != 0)
	{
		free(args);
		return NULL;
	}

	args = realloc(args, (argc + 1) * sizeof (char *));
	if (args == NULL) goto realloc_fail;

	args[argc] = NULL;
	return args;

malloc_fail:
	perror("malloc() failed");
	return NULL;
realloc_fail:
	perror("realloc() failed");
	return NULL;
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