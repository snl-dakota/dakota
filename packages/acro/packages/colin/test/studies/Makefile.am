
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

check_PROGRAMS = tests contextMngr

tests_SOURCES=tests.cpp test1.cpp TestOptSolver.cpp test7.cpp test2.cpp test3.cpp test5.cpp test6.cpp test20.cpp test8.cpp test9.cpp test21.cpp test4.cpp

contextMngr_SOURCES=contextMngr.cpp

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST=$(srcdir)/TestOptSolver.h

clean-local: clean-temps
	\rm -f core* vgcore*

clean-temps:
	\rm -f shell_func5.in.* shell_func5.out.* shell_func7.in.* shell_func7.out.* colin.in.* colin.out.*
