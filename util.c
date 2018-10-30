extern int argc;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* ---------- HELPER FUNCTIONS ---------- */

char * tok_next(char * str, char delim, char quote)
{
	static char * ptr;
	char *start, *end;
	static unsigned char quote_on;
	unsigned i, str_size;

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

		if (*end == quote)
		{
			if (end == start)
			{
				quote_on = !quote_on;
			}
			else if (*(end - 1) != '\\')
			{
				if (*(end + 1) == delim || *(end + 1) == 0)
					quote_on = !quote_on;
				else
				{
					errno = 2;
					return NULL;
				}
			}
		}

		if (*end == delim && quote_on == 0)
		{
			*end = 0;
			ptr = ++end;
			break;
		}
	}

	str_size = strlen(start);
	for (i = 0; i < str_size; i++)
	{
		if (start[i] == quote)
		{
			char * j;
			for (j = (start + i); j < end; j++)
			{
				*j = *(j+1);
			}
		}
	}

	return start;
}

char ** split_cmd(char * line)
{
	char ** args;
	char * arg;

	argc = 1;
	args = malloc(argc * sizeof (char *));
	if (args == NULL)
	{
		perror("malloc() failed");
		exit(-1);
	}

	arg = tok_next(line, ' ', '"');
	args[argc - 1] = arg;

	while (( arg = tok_next(NULL, ' ', '"') ) != NULL)
	{
		argc++;
		args = realloc(args, argc * sizeof (char *));
		if (args == NULL)
		{
			perror("realloc() failed");
			exit(-1);
		}
		args[argc - 1] = arg;
	}

	if (errno == 1) return NULL;
	else if (errno == 2) return NULL;

	args = realloc(args, (argc + 1) * sizeof (char *));
	args[argc] = NULL;

	return args;
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
		perror("Failed");
		printf("\t(%s)\n", args[0]);
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
		
		return WEXITSTATUS(status);
	}
	return 0;
}