#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#ifdef linux
	#include <linux/limits.h>
#else
	#include <sys/syslimits.h>
#endif

#include <editline/readline.h>

#include "builtins.h"

#define BUILTINS_NUM 7
#define CMD_ITEM(NAME) {#NAME, &NAME##_builtin}
#define FG_RED "\033[0;1;31m"
#define CLR "\033[0m"
#define BG_RED "\033[37;41m"
#define BG_WHITE "\033[30;47m"


/* ---------- VARIABLE DECLARATIONS ---------- */

int lin_ret;
unsigned short argc;


/* ---------- COMMAND TABLE ---------- */

struct builtin_func_s
{
	const char* cmd;
	int (*func)(unsigned short argc, char ** argv);
};

struct builtin_func_s builtin_table[] =
	{
		CMD_ITEM(cd),
		CMD_ITEM(set),
		CMD_ITEM(echo),
		CMD_ITEM(pwd),
		CMD_ITEM(exit),
		CMD_ITEM(exec),
		CMD_ITEM(help)
	};



/* ---------- HELPER FUNCTIONS ---------- */

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

	arg = strtok(line, " ");
	args[argc - 1] = arg;

	while (( arg = strtok(NULL, " ") ) != NULL)
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




/* ---------- MAIN CODE --------- */


int lin_eval(char current[])
{
	unsigned int i;
	const char *cmd;
	char ** args;
	char * tmp;

	tmp = NULL;
	args = split_cmd(current);
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