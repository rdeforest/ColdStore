#PROFILING := no
include ../mk/occ.defs.mk

OCCFLAGS += -SColdClass -SColdTemplate -SSlotClass

INCLUDES += -I../include -I../coldstore -I/usr/local/include -I/usr/local/include/libelf

LDFLAGS += $(WRAP) -rdynamic

#LDLIBS += -L../libs -lqvmm -lcold -lgmp -lpcre
LDLIBS += -L../libs -lgmp -lpcre -lqvmm -liberty
ifeq ($(OSNAME),Linux)
LDLIBS += -ldl
endif

OCC_ENV += LD_LIBRARY_PATH=.:../metaclass:$$LD_LIBRARY_PATH

# DEFINES += -DEBUGLOG

all:	skeleton.so skeleton

skeleton.so: ../libs/libcold.so ../libs/libqvmm.so semantics.o

global.o: global-linux.c global-freebsd.c

skeleton: skeleton.o global.o ../libs/slop.o ../libs/intern.so

include ../mk/occ.rules.mk

global.so: global.o
	g++ -shared -o global.so -Xlinker -Bsymbolic global.o

profile: skeleton
	./skeleton
	LD_PRELOAD=./global.so LD_LIBRARY_PATH=../libs:. sprof skeleton.so skeleton.so.profile

debug: skeleton
	-rm coldstore
	LD_LIBRARY_PATH=../libs ddd skeleton &

DISTCLEAN += skeleton coldstore

