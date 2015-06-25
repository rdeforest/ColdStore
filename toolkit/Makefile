# Generated automatically from Makefile.in by configure.
TOP_SRCDIR = ..
srcdir = .
prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
infodir = ${prefix}/info
libdir = ${exec_prefix}/lib
mandir = ${prefix}/man
includedir = ${prefix}/include
INSTALL = /usr/bin/install -c

include ../mk/occ.defs.mk


OCCFLAGS += -SColdClass -SColdTemplate -SSlotClass
YACC = bison++ 
INCLUDES += -I../include -I../coldstore -I/usr/local/include \
	    -I/usr/local/include/libelf

CXXFLAGS += $(INCLUDES)
LDFLAGS += $(WRAP) -rdynamic
LDLIBS += -L../libs -lqvmm -lgmp -lpcre  -lutil
LDLIBS += -ldl -liberty
OCC_ENV += LD_LIBRARY_PATH=.:../metaclass

all:	libcold_toolkit.so

include ../mk/occ.rules.mk

Tokenizer.o: Tokenizer.cc Tokenizer.hh
Tape.o: Tape.cc Tape.hh
Root.o: Root.cc common.hh
SequenceMethods.o: SequenceMethods.cc common.hh
initialize.o: initialize.cc common.hh

libcold_toolkit.so: Root.o SequenceMethods.o Tokenizer.o \
	            StringMethods.o Tape.o initialize.o

headers := Tokenizer.hh Tape.hh

install_headers: $(headers)
	$(TOP_SRCDIR)/mkinstalldirs $(includedir)
	@list='$(headers)'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  echo "(INSTALL) $$d$$p $(includedir)/$$p"; \
	  $(INSTALL) $$d$$p $(includedir)/$$p; \
	done

install: all install_headers
	$(TOP_SRCDIR)/mkinstalldirs $(libdir)
	$(TOP_SRCDIR)/mkinstalldirs $(includedir)
	$(INSTALL) libcold_toolkit.so $(libdir)/libcold_toolkit.so

CLEAN += 
DISTCLEAN += 

