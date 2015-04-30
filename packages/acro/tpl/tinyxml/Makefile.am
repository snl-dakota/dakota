#
# Wrapper Makefile for TinyXML
#

lib_LIBRARIES=libtinyxml.a

SRCDIR=tinyxml

TOPINC=${top_builddir}/include
TOPLIB=${top_builddir}/lib
TOPBIN=${top_srcdir}/bin

FULLPATH=${top_srcdir}/tpl/tinyxml/$(SRCDIR)

libtinyxml_a_SOURCES= \
 $(FULLPATH)/tinyxml.cpp\
 $(FULLPATH)/tinyxmlerror.cpp\
 $(FULLPATH)/tinyxmlparser.cpp

ALL_H_SRC=\
 $(FULLPATH)/tinyxml.h \
 $(FULLPATH)/tinystr.h

includetinyxmldir=$(includedir)
includetinyxml_HEADERS=$(ALL_H_SRC)


all-local: libtinyxml.a
	@if test -d $(TOPLIB) ; then : ; else mkdir $(TOPLIB) ; fi
	@$(TOPBIN)/cp_u libtinyxml.a $(TOPLIB)

clean-local:
	@rm -f $(TOPLIB)/libtinyxml.a
	@$(MAKE) clean-headers

copy-headers:
	@if test -d $(TOPINC) ; then : ; else mkdir $(TOPINC) ; fi
	@if test -d $(TOPINC)/tinyxml ; then : ; else mkdir $(TOPINC)/tinyxml ; fi
	@$(TOPBIN)/cp_u $(ALL_H_SRC) $(TOPINC)/tinyxml

clean-headers:
	@rm -rf $(TOPINC)/tinyxml

