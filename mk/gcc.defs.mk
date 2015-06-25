#
# gcc.defs.mk
#
# Sources compiled with gcc should include this first
#

OSNAME = $(shell uname)
export OSNAME

#Enable profiling using http://www.pdos.lcs.mit.edu/~cblake/pct-0.97.tgz
#PROFILING = yes
#export PROFILING

ifeq ($(OSNAME),FreeBSD)
# The only FreeBSD reference platform requires the bleeding edge
CC = egcc
CXX = eg++
LD = eg++
else
ifeq ($GCCV3,true)
CC = gcc-3.0
CXX = g++-3.0
LD = g++-3.0
else
CC = gcc
CXX = g++
LD = g++
endif
endif

CFLAGS = -pipe -fPIC
CXXFLAGS = -frtti -fPIC
#CXXFLAGS += -pipe -Wall -Woverloaded-virtual
CXXFLAGS += -pipe -Wall

ifeq ($(PROFILING),yes)
#CXXFLAGS += -pg
#CXXFLAGS += -a
#CXXFLAGS += -fprofile-arcs
#LDFLAGS += -pg
#LDFLAGS += -a
#LDFLAGS += -fprofile-arcs
LDLIBS += -L/usr/local/lib/ -lvtalrm
DEFINES += -DPROFILING
endif

INCLUDES += -I$(TOP_SRCDIR)/include


# WRAP = --wrap malloc --wrap free --wrap realloc
WRAP = -Xlinker --wrap -Xlinker malloc -Xlinker --wrap -Xlinker free -Xlinker --wrap -Xlinker realloc
WRAP += -Xlinker --wrap -Xlinker __libc_malloc -Xlinker --wrap -Xlinker __libc_free

ifeq ($(OSNAME),Linux)
CXXFLAGS += -O2 -ggdb3
else
# gcc 2.96 (our reference compiler on FreeBSD) doesn't like -O2 in parts
CXXFLAGS += -ggdb3
endif

# CXXFLAGS += -fprofile-arcs -ftest-coverage

# XXX ??? I presume these are magic ??? -- chuck
#DEFINES += -D_NOTHREADS -DGDB
DEFINES += -DGDB
# enable for runtime debugging output
# DEFINES += -DDEBUGLOG

# define this to get extra checks on RefCount
#DEFINES += -DREFMONITOR

CLEAN = *.a *.so *.o *~ kdoc *.da *.bb *.bbg *.gcov *.ii *.occ core *.core .*.d

RPCGEN = rpcgen
