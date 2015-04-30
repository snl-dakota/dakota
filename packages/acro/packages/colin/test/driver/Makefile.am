
include ${top_srcdir}/config/MakeMacros.mk

if HAVE_ANSI_CPP_HEADERS
  ANSI_FLAG=-DANSI_HDRS 
else
  ANSI_FLAG=
endif

if BUILD_AMPL
AMPLLIB=$(top_builddir)/tpl/ampl/amplsolver.a
endif

TOPINC=$(top_builddir)/include
TOPLIB=$(top_builddir)/lib

UTILIB_INCLUDE=-I${top_srcdir}/packages/utilib/src \
	-I${top_builddir}/packages/utilib/src
COLIN_INCLUDE=-I${top_srcdir}/packages/colin/src

LDADD=$(top_builddir)/packages/colin/src/libcolin.a \
	$(top_builddir)/packages/utilib/src/libutilib.la \
	$(AMPLLIB) -lm $(DLOPEN_LIBS)

#
# Paths to: config.h, other configure headers, utilib headers, MPI
#
AM_CPPFLAGS=$(UTILIB_INCLUDE) $(COLIN_INCLUDE) \
            $(MPI_FLAGS) $(ANSI_FLAG) $(BOOST_CPPFLAGS) \
            $(TINYXML_INCLUDE) -DMPICH_IGNORE_CXX_SEEK -DIGNORE_BROKEN

check_PROGRAMS=colintest 

colintest_SOURCES=colintest.cpp TestOptSolver.cpp

