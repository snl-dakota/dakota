
include ${top_srcdir}/config/MakeMacros.mk
SUFFIXES=.i
if I_AM_SUBPACKAGE
  TOPINC=../../../include
  TOPLIB=../../../lib
  TOPBUILDBIN=../../../bin
else
  TOPINC=$(top_builddir)
  TOPLIB=$(top_builddir)/lib
  TOPBUILDBIN=${top_builddir}/bin
endif
TOPSRCBIN=${top_srcdir}/bin
LDADD=../src/libutilib.la
#
# Paths to: config.h, other configure headers, utilib headers
#
AM_CPPFLAGS=-I${top_srcdir}/src $(TINYXML_INCLUDE)


if BUILD_STATIC_EXECUTABLES
#
#A libtool flag
#
AM_LDFLAGS=@STATIC_FLAGS@
endif

bin_PROGRAMS=OptionParser1

if BUILD_SHARED
  EXECS=$(bin_PROGRAMS:%=.libs/%)
else
  EXECS=$(check_PROGRAMS)
endif

BIN_COPIES=$(bin_PROGRAMS:%=$(TOPBIN)/%)

OptionParser1_SOURCES= OptionParser1.cpp

check-local:
	@if test -d $(TOPBUILDBIN) ; then : ; else mkdir $(TOPBUILDBIN); fi
	@if test -d $(TOPBUILDBIN)/utilib ; then : ; else mkdir $(TOPBUILDBIN)/utilib; fi
	@$(TOPSRCBIN)/cp_u $(EXECS) $(TOPBUILDBIN)/utilib

clean-local:
	@rm -rf $(TOPBUILDBIN)/utilib
	@for i in *.gcda *.gcno ; \
	do\
		rm -f $$i ;\
	done


