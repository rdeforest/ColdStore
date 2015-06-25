# -*-Makefile-*-
# Makefile-occ.rules - common rules for compiling OpenC++
# 

%.o : %.cc
	$(OCC_ENV) $(OCC) $(DEFINES) $(OCCFLAGS) $(INCLUDES) -- -c $(CXXFLAGS) $<
	-rm $(*).ii

%.ii : %.cc
	$(OCC_ENV) $(OCC) $(DEFINES) $(OCCFLAGS) $(INCLUDES) -- -c $(CXXFLAGS) $<

# metaclass
%.so : %.mc
	$(OCC_ENV) $(OCC) $(DEFINES) -m $(OCCFLAGS) $(INCLUDES) -- $(OCXXFLAGS) $(OLDFLAGS) $<

%.so :
	$(LD) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)


.PHONY: clean distclean
clean:
	rm -rf $(CLEAN)

distclean:
	rm -rf $(CLEAN) $(DISTCLEAN)


#### Less common targets

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

%.xdr.cc: %.x
	-rm -f $@
	$(RPCGEN) -N -c -o $@ $?

%.svc.cc: mount.x
	-rm -f $@
	$(RPCGEN) -N -m -o $@ $?
