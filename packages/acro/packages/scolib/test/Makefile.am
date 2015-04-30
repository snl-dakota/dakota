
include ../../../config/MakeMacros.mk

TOPBIN=$(top_builddir)/bin
TOPINC=$(top_builddir)/include
TOPLIB=$(top_builddir)/lib

if BUILD_AMPL
AMPLLIB=$(top_builddir)/tpl/ampl/amplsolver.a
endif

LDADD=-L$(TOPLIB) $(top_builddir)/packages/scolib/src/libscolib.a\
	$(top_builddir)/packages/colin/src/libcolin.a\
	$(top_builddir)/packages/pebbl/src/libpebbl.a\
	$(top_builddir)/packages/utilib/src/libutilib.la \
	$(AMPLLIB) $(DLOPEN_LIBS)


#
# Paths to: config.h, other configure headers, utilib headers, MPI
#
AM_CPPFLAGS=-I$(top_builddir) -I$(top_srcdir)/config -I$(TOPINC) \
            -DMPICH_IGNORE_CXX_SEEK -I$(TOPINC)/tinyxml

check_PROGRAMS = #namespace

#namespace_SOURCES=namespace.cpp

TESTS = #namespace

MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES=scolibtest
