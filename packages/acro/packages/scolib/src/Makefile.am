
#
# SLB: Temporary flag to prevent parallel compilation by gmake
#

include ../../../config/MakeMacros.mk

lib_LIBRARIES=libscolib.a

SRCBIN=${top_srcdir}/bin
BLDBIN=../../../bin
TOPINC=../../../include
TOPLIB=../../../lib
FULLPATH_SRC=$(top_srcdir)/packages/scolib/src

if BUILD_STATIC_EXECUTABLES
#
#A libtool flag
#
AM_LDFLAGS=@STATIC_FLAGS@
endif

AMPL_INCLUDE=-I${top_srcdir}/tpl/ampl \
	-I${top_builddir}/tpl/ampl
UTILIB_INCLUDE=-I${top_srcdir}/packages/utilib/src \
	-I${top_builddir}/packages/utilib/src
COLIN_INCLUDE=-I${top_srcdir}/packages/colin/src
PEBBL_INCLUDE=-I${top_srcdir}/packages/pebbl/src/bb \
	-I${top_srcdir}/packages/pebbl/src/comm \
	-I${top_srcdir}/packages/pebbl/src/example \
	-I${top_srcdir}/packages/pebbl/src/misc \
	-I${top_srcdir}/packages/pebbl/src/pbb \
	-I${top_srcdir}/packages/pebbl/src/sched

ALL_H_SRC=\
  $(srcdir)/scolib/c_EAgeneric.h\
  $(srcdir)/scolib/Direct.h\
  $(srcdir)/scolib/DirectMisc.h\
  $(srcdir)/scolib/DockingPS.h\
  $(srcdir)/scolib/DomainInfoBase.h\
  $(srcdir)/scolib/DomainInfoMixedInteger.h\
  $(srcdir)/scolib/DomainOpsArray.h\
  $(srcdir)/scolib/DomainOpsBase.h\
  $(srcdir)/scolib/DomainOpsBinary.h\
  $(srcdir)/scolib/DomainOpsIntArray.h\
  $(srcdir)/scolib/DomainOpsMixedInteger.h\
  $(srcdir)/scolib/DomainOpsRealArray.h\
  $(srcdir)/scolib/DynamicGSS.h\
  $(srcdir)/scolib/EAbase.h\
  $(srcdir)/scolib/EAgeneric.h\
  $(srcdir)/scolib/EAindividual.h\
  $(srcdir)/scolib/EAminlp.h\
  $(srcdir)/scolib/EAops_array.h\
  $(srcdir)/scolib/EAops.h\
  $(srcdir)/scolib/EAops_intarray.h\
  $(srcdir)/scolib/EAops_real.h\
  $(srcdir)/scolib/EApoint.h\
  $(srcdir)/scolib/Grasp.h\
  $(srcdir)/scolib/MultiStart.h\
  $(srcdir)/scolib/MultiStatePS.h\
  $(srcdir)/scolib/panmictic_selection.h\
  $(srcdir)/scolib/PatternSearch.h\
  $(srcdir)/scolib/PDirect.h\
  $(srcdir)/scolib/PEAbase.h\
  $(srcdir)/scolib/PEAint.h\
  $(srcdir)/scolib/PEAreal.h\
  $(srcdir)/scolib/PIDOMS.h\
  $(srcdir)/scolib/PLGO.h\
  $(srcdir)/scolib/SCORegistrations.h\
  $(srcdir)/scolib/selection.h\
  $(srcdir)/scolib/SolisWets.h\
  $(srcdir)/scolib/StateMachineLS.h\
  $(srcdir)/scolib/TGPO.h

includescolibdir=$(includedir)/scolib

includescolib_HEADERS = $(ALL_H_SRC)


#
# Paths to:
#  all scolib headers, to acro_config.h, and to other configuration headers.  
#
AM_CPPFLAGS=-I$(srcdir) $(UTILIB_INCLUDE) $(COLIN_INCLUDE) $(PEBBL_INCLUDE)\
	$(TINYXML_INCLUDE) -I$(TOPINC)/ampl $(R_INCLUDE) \
	$(BOOST_CPPFLAGS) -DMPICH_IGNORE_CXX_SEEK

libscolib_a_SOURCES= \
  libs/Direct.cpp\
  libs/DockingPS.cpp\
  libs/PatternSearch.cpp\
  libs/positive_basis.cpp\
  libs/SolisWets.cpp\
  libs/DynamicGSS.cpp\
  libs/MultiStatePS.cpp\
  libs/MultiStart.cpp\
  libs/EAminlp.cpp\
  libs/PLGO.cpp\
  libs/PIDOMS.cpp\
  libs/SCORegistrations.cpp\
  libs/StateMachineLS.cpp\
  libs/TGPO.cpp
# NOTE: SCORegistrations MUST be the last object in this list
# WEH:  But it's not!  LOL

if BUILD_PEBBL
PEBBL_LIB=$(top_builddir)/packages/pebbl/src/libpebbl.a
endif

LDADD_ACRO_LIBS = libscolib.a\
	$(top_builddir)/packages/colin/src/libcolin.a\
	$(PEBBL_LIB)\
	$(top_builddir)/packages/utilib/src/libutilib.la

if BUILD_AMPL
AMPLLIB=$(top_builddir)/tpl/ampl/amplsolver.a
endif

LDADD=-L$(TOPLIB) $(LDADD_ACRO_LIBS) $(AMPLLIB) $(DLOPEN_LIBS) $(R_LIBS)

if BUILD_EXECUTABLES
bin_PROGRAMS = \
  scolib_driver\
  docking\
  simple

scolib_driver_SOURCES=exec/scolib.cpp exec/funcadd.c
docking_SOURCES=exec/docking.cpp
simple_SOURCES=exec/simple.cpp

scolib_driver_DEPENDENCIES=$(LDADD_ACRO_LIBS)
docking_DEPENDENCIES=$(LDADD_ACRO_LIBS)
simple_DEPENDENCIES=$(LDADD_ACRO_LIBS)
endif

SUBDIRS= .

all-local: libscolib.a scolib_driver$(EXEEXT)
	@if test -d $(TOPLIB) ; then : ; else mkdir $(TOPLIB) ; fi
	@$(SRCBIN)/cp_u libscolib.a $(TOPLIB)
	@$(SRCBIN)/cp_u scolib_driver$(EXEEXT) ../test/scolibtest$(EXEEXT)

clean-local:
	@rm -f $(TOPLIB)/libscolib.a
	@rm -f ../test/scolibtest

MAINTAINERCLEANFILES = Makefile.in
