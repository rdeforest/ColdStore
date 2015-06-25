
// QVMM_BASE determines where in absolute space the store starts
// QVMM_SIZE determines the size of the store
// There can be no holes in this range.  Use contrib/prospect to find them

// Note that the supplied values are SUGGESTIONS.  
// They may still not work on your system unless they are verified!

#define HAVE_CHECKED_QVMM_DEFAULTS

#ifndef HAVE_CHECKED_QVMM_DEFAULTS
#warning **************************************************
#warning         Using untested defaults for qvmm!  
#warning Run prospect to be sure these work on your system!
#warning **************************************************
#endif

#ifdef linux

#define QVMM_BASE (void *)0x48000000
#define QVMM_SIZE 0x70000000

#endif


#ifdef __FreeBSD__
// These are very conservative figures, and I just haven't prospected out a
// larger range than this one yet.

#define QVMM_BASE (void *)0x10000000
// #define QVMM_SIZE 0x70000000

// last known good size according to prospect so far
#define QVMM_SIZE 0x20000000
// Deep Check 0x30fff000

#endif


