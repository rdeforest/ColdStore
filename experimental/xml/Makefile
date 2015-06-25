include ../mk/occ.defs.mk

OCCFLAGS += -SColdClass -SColdTemplate -SSlotClass
YACC = bison++ 
YFLAGS = -d -h xmlParser.hh -o xmlParser.cc
INCLUDES += -I../include -I../freon2 -I../coldstore -I/usr/local/include \
	    -I/usr/local/include/libelf -I/usr/include/gtk-1.2 \
	    -I/usr/include/glib-1.2 -I/usr/lib/glib/include \
	    -I/usr/include/gtk-- -I/usr/lib/gtkmm/include \
	    -I/usr/lib/sigc++/include -I/usr/include/gnome-- \
	    -I/usr/lib/gnome-libs/include -I/usr/local/include
	    
CXXFLAGS += $(INCLUDES)
LDFLAGS += $(WRAP) -rdynamic
LDLIBS += -L../libs -lqvmm -lgmp -lpcre  -lutil -L/usr/local/lib
LDLIBS += -ldl -liberty  -lexpat -lgtk  -lsigc -lpthread -lgdk \
	  -lgtkmm -lgdkmm -lgnomemm

OCC_ENV += LD_LIBRARY_PATH=.:../metaclass


all:	xml.so xml 

test: xml
	LD_LIBRARY_PATH=../libs/ ./xml < xml.test

include ../mk/occ.rules.mk

manual: xmlmanual.html

xmlmanual.html: xmlmanual.wml
	wml xmlmanual.wml > xmlmanual.html

xml.so: ../libs/libcold.so start.o 

global.o: global-linux.c global-freebsd.c
start.o: start.cc
xml: xml.o global.o ../libs/slop.o ../libs/intern.so

debug: xml
	-rm coldstore
	LD_LIBRARY_PATH=../libs ddd xml &

CLEAN += 
DISTCLEAN += xml coldstore
