.c:
	$(MAKE) $*.o
	$(CLINK) $*.o $(LDADD) $(LIBS)

.cc:
	$(MAKE) $*.o
	$(CXXLINK) $*.o $(LDADD) $(LIBS)

.cpp:
	$(MAKE) $*.o
	$(CXXLINK) $*.o $(LDADD) $(LIBS)

.c.i:
	$(CCOMPILE) -E $< > $*.i

.cc.i:
	$(CXXCOMPILE) -E $< > $*.i

.cpp.i:
	$(CXXCOMPILE) -E $< > $*.i

