//under GPL v2

#include "ioq3-urt.h"

ioq3_urt_t			clu; //most new console variables and functions are called via this
cvar_t				*com_quiet; //excluded from the struct since it makes rebuilding hell.