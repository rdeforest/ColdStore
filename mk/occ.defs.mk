#
# occ.defs.mk
#
# Sources compiled with openc++ should include this first
#
#OCXXFLAGS = -frtti -fPIC -pipe -Wall -Woverloaded-virtual
OCXXFLAGS = -frtti -fPIC -pipe -Wall

ifneq ($(METACOMPILING),yes)
OCCFLAGS += -SColdClass -SColdTemplate -SSlotClass
endif

OLDFLAGS = 

include $(TOP_SRCDIR)/mk/gcc.defs.mk

# MAKE SURE openc++ is compiled with the same compiler as the value of CXX!

OCC_HOME = $(TOP_SRCDIR)/openc++
OCC = occ

#OCCFLAGS = -I$(OCC_HOME)/src
OCCFLAGS += -v -I/usr/include/openc++/

# This is an evil and rude hack that allows the right backend to be used
# If occ is in your path and CXX=g++, you can safely elide this
# Note that you still need to use the right OCC_HOME for the MOP classes
#OCC_ENV = rm -f ./g++; ln -s `which $(CXX)` ./g++; PATH=.:$(OCC_HOME)/src/Unix:$$PATH

# use the system's occ
OCC_ENV = rm -f ./g++; ln -s `which $(CXX)` ./g++; 
OCC_ENV += LD_LIBRARY_PATH=.:$(TOP_SRCDIR)/metaclass

CLEAN += g++

RPCGEN = rpcgen
