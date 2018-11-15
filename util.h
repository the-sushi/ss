#include "tables.h"
#ifdef linux
#	include <linux/limits.h>
#else
#	include <sys/syslimits.h>
#endif

char ** split_cmd(char * line);
int execute(char ** args);
unsigned char exit_flag;
void routine_clear (struct routine_s * routine);
char path[PATH_MAX];