#pragma once

/* Macros and defines */
#define BUILTINS_NUM 10
#define DECL_BUILTIN(NAME) int NAME##_builtin(unsigned short argc, char ** argv)

/* Declarations */
DECL_BUILTIN(cd);
DECL_BUILTIN(set);
DECL_BUILTIN(echo);
DECL_BUILTIN(pwd);
DECL_BUILTIN(exit);
DECL_BUILTIN(exec);
DECL_BUILTIN(routine);
DECL_BUILTIN(unroutine);
DECL_BUILTIN(listroutines);
DECL_BUILTIN(help);