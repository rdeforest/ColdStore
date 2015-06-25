#
# gcc.rules.mk
#
# Should be included last, but before any rules overrides
#

#### Common targets

all:

.PHONY: kdocs

kdocs:
	-mkdir kdoc
	-kdoc -d kdoc -n coldstore --strip-h-path *.hh

%.o : %.cc
	$(CXX) -c $(DEFINES) $(INCLUDES) $(CXXFLAGS) $<

%.cc : %.l
	flex++ -o $@ $^

# antlr uses .cpp suffix, still treated the same
%.o : %.cpp
	$(CXX) -c $(DEFINES) $(INCLUDES) $(CXXFLAGS) $<

# rule for making executables
%: %.o
	$(LD) $(LDFLAGS) -rdynamic -o $@ $^ $(LDLIBS)

%.so :
	$(LD) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

%.a :
	ar rsv $@ $^


.PHONY: clean distclean
clean:
	rm -rf $(CLEAN)

distclean:
	rm -rf $(CLEAN) $(DISTCLEAN)

#### Less common targets


%.cpp: %.g
	java -classpath $(CLASSPATH) antlr.Tool $^

# SP has some m4 stuff
# ??? what is SP ???  -- chuck
%.cxx: %.m4
	m4 $(TOP_SRCDIR)/include/instmac.m4 $^ >$@

# auto-construct dependencies
.%.d: %.cc
	$(SHELL) -ec '$(CXX) -MM $(INCLUDES) $(CXXFLAGS) $(TEMPLATES) $< \
		| sed '\''s/$*\\.o[ :]*/& $@/g'\'' > $@'

.%.d: %.cxx
	$(SHELL) -ec '$(CXX) -MM $(INCLUDES) $(CXXFLAGS) $(TEMPLATES) $< \
		| sed '\''s/$*\\.o[ :]*/& $@/g'\'' > $@'

.%.d: %.cpp
	$(SHELL) -ec '$(CXX) -MM $(INCLUDES) $(CXXFLAGS) $(TEMPLATES) $< \
		| sed '\''s/$*\\.o[ :]*/& $@/g'\'' > $@'

# ???? Looks like some kinda weird dependency trick ??? -- chuck
%.m:
	cd $(TOP_SRCDIR)/$(@:.m=); make all

%.html:	%.sgml
	SGML_CATALOG_FILES=/etc/sgml.catalog \
		jade -t sgml -ihtml -d $(TOP_SRCDIR)/include/docbook.dsl\#html $<

%.nana: %.cc
	nana $(CXXFLAGS) $(TEMPLATES) $< >$@

%.nana: %.cpp
	nana $(CXXFLAGS) $(TEMPLATES) $< >$@


# rpc support
%.h: %.x
	-rm -f $@
	$(RPCGEN) -N -h -o $@ $?

%.xdr.c: %.x
	-rm -f $@
	$(RPCGEN) -N -c -o $@ $?

%.svc.c: mount.x
	-rm -f $@
	$(RPCGEN) -N -m -o $@ $?
