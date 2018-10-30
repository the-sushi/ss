#define CMD_ITEM(NAME) {#NAME, &NAME##_builtin}
#define BUILTINS_NUM 7

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