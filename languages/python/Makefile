include ../mk/occ.defs.mk

OCCFLAGS += -SColdClass -SColdTemplate -SSlotClass
INCLUDES += -I../include -I../coldstore -I/usr/include/libelf -I/usr/include/python2.0 -I. -I../freon2
CXXFLAGS += $(INCLUDES)
LDFLAGS += $(WRAP) -rdynamic
LDLIBS += -L../libs -lqvmm -lgmp -lpcre -lpython2.0 -lutil 
LDLIBS += -ldl -liberty

OCC_ENV += LD_LIBRARY_PATH=.:../metaclass

all:	pyc.so pyc coldmodule.so

test: pyc
	LD_LIBRARY_PATH=../libs/ ./pyc < pyc.test

include ../mk/occ.rules.mk

manual: pycmanual.html

pycmanual.html: pycmanual.wml
	wml pycmanual.wml > pycmanual.html

pyc.so: ../libs/libcold.so start.o 

global.o: global-linux.c global-freebsd.c
coldmodule.so: coldmodule.o ScalarType.o SequenceType.o MappingType.o ClosureType.o ObjectType.o PyColdObject.o Utility.o 
coldmodule.o: coldmodule.cc coldmodule.hh
PyColdObject.o: PyColdObject.cc PyColdObject.hh coldmodule.hh
ScalarType.o: ScalarType.cc ScalarType.hh coldmodule.hh
SequenceType.o: SequenceType.cc SequenceType.hh coldmodule.hh
MappingType.o: MappingType.cc MappingType.hh coldmodule.hh
ObjectType.o: ObjectType.cc ObjectType.hh coldmodule.hh
ClosureType.o: ClosureType.cc ClosureType.hh coldmodule.hh

Utility.o: Utility.cc coldmodule.hh

pyc: pyc.o global.o ../libs/slop.o ../libs/intern.so
start.o: start.cc
Pickled.o: Pickled.cc Pickled.hh

debug: pyc
	-rm coldstore
	LD_LIBRARY_PATH=../libs ddd pyc &

CLEAN += pycParser.cc pycParser.hh 
DISTCLEAN += pyc coldstore

