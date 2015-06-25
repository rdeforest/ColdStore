include ../mk/occ.defs.mk

OCCFLAGS += -SColdClass -SColdTemplate -SSlotClass
YACC = bison++ 
YFLAGS = -d -h browserParser.hh -o browserParser.cc
INCLUDES += -I../include -I../freon2 -I../coldstore -I/usr/local/include \
	    -I/usr/local/include/libelf -I/usr/include/gtk-1.2 \
	    -I/usr/include/glib-1.2 -I/usr/lib/glib/include \
	    -I/usr/include/gtk-- -I/usr/lib/gtkmm/include \
	    -I/usr/lib/sigc++/include -I/usr/include/gnome-- \
	    -I/usr/lib/gnome-libs/include

CXXFLAGS += $(INCLUDES)
LDFLAGS += $(WRAP) -rdynamic
LDLIBS += -L../libs -lqvmm -lgmp -lpcre  -lutil
LDLIBS += -ldl -liberty -lgtk  -lsigc -lpthread -lgdk -lgtkmm -lgdkmm -lgnomemm

OCC_ENV += LD_LIBRARY_PATH=.:../metaclass


all:	browser.so browsertest.so browser 

test: browser
	LD_LIBRARY_PATH=../libs/ ./browser < browser.test

include ../mk/occ.rules.mk

manual: browsermanual.html

browsermanual.html: browsermanual.wml
	wml browsermanual.wml > browsermanual.html

browser.so:  coldBrowser.o coldBrowser_glade.o 
browsertest.so: ../libs/libcold.so start.o
global.o: global-linux.c global-freebsd.c
start.o: start.cc
browser: browser.o global.o ../libs/slop.o ../libs/intern.so

debug: browser
	-rm coldstore
	LD_LIBRARY_PATH=../libs ddd browser &

CLEAN += 
DISTCLEAN += browser coldstore
