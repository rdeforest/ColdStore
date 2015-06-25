// This may be more dependent on compiler version than OS

#ifdef linux
#include "global-linux.c"
#endif

#ifdef __FreeBSD__
#include "global-freebsd.c"
#endif

