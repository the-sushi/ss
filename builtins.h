typedef char * lstring;
#define DECL_BUILTIN(NAME) int NAME##_builtin(unsigned short argc, char ** argv)
DECL_BUILTIN(cd);
DECL_BUILTIN(set);
DECL_BUILTIN(echo);
DECL_BUILTIN(pwd);
DECL_BUILTIN(exit);
DECL_BUILTIN(exec);
DECL_BUILTIN(help);