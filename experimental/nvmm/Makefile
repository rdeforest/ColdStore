#
# Makefile for coldstore/qvmm
#

include ../mk/gcc.defs.mk

# define this for LOTS of spam on cerr
# DEFINES += -DDEBUGLOG

DEFINES += -DGUARD_ALLOC -DWRAP_MALLOC
LDFLAGS += $(WRAP)

include ../mk/gcc.rules.mk

all: libqvmm.so

libqvmm.so: QVmm.o new.o

QVmmtest: libqvmm.so

test: QVmmtest
