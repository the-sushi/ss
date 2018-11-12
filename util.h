#include "tables.h"
char ** split_cmd(char * line);
int execute(char ** args);
unsigned char exit_flag;
void routine_clear (struct routine_s * routine);