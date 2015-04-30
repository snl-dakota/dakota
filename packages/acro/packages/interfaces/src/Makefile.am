
#
# SLB: Temporary flag to prevent parallel compilation by gmake
#

include ../../../config/MakeMacros.mk

lib_LIBRARIES=libinterfaces.a

SRCBIN=${top_srcdir}/bin
BLDBIN=../../../bin
TOPINC=../../../include
TOPLIB=../../../lib
FULLPATH_SRC=$(top_srcdir)/packages/interfaces/src

if BUILD_STATIC_EXECUTABLES
#
#A libtool flag
#
AM_LDFLAGS=@STATIC_FLAGS@
endif

ALL_H_SRC=\
  $(srcdir)/interfaces/APPS.h\
  $(srcdir)/interfaces/Cobyla.h\
  $(srcdir)/interfaces/DOT.h\
  $(srcdir)/interfaces/Hooke.h\
  $(srcdir)/interfaces/InterfacesRegistrations.h\
  $(srcdir)/interfaces/MOMHLib.h\
  $(srcdir)/interfaces/NPSOL.h\
  $(srcdir)/interfaces/OPTpp.h\
  $(srcdir)/interfaces/TCC.h

includeinterfacesdir=$(includedir)/interfaces

includeinterfaces_HEADERS = $(ALL_H_SRC)

THREEPO_INCLUDE=\
	-I${top_srcdir}/tpl/3po
AMPL_INCLUDE=\
	-I${top_srcdir}/tpl/ampl \
	-I${top_builddir}/tpl/ampl
APPSPACK_INCLUDE=\
	-I${top_srcdir}/tpl/appspack/src \
	-I${top_builddir}/tpl/appspack/src
MOMH_INCLUDE=\
	-I${top_srcdir}/tpl/momhlib/MOMHLib/Common \
	-I${top_srcdir}/tpl/momhlib/MOMHLib/HGA \
	-I${top_srcdir}/tpl/momhlib/MOMHLib/MO \
	-I${top_srcdir}/tpl/momhlib/MOMHLib/MOMSLS \
	-I${top_srcdir}/tpl/momhlib/MOMHLib/ParetoGA \
	-I${top_srcdir}/tpl/momhlib/MOMHLib/SA

UTILIB_INCLUDE=\
	-I${top_srcdir}/packages/utilib/src \
	-I${top_builddir}/packages/utilib/src
COLIN_INCLUDE=-I${top_srcdir}/packages/colin/src
SCOLIB_INCLUDE=-I${top_srcdir}/packages/scolib/src

LOCAL_TEUCHOS_INCLUDE = @TEUCHOS_CPPFLAGS@

OPTPP_INCLUDE=\
	-I$(top_srcdir)/packages/optpp/include \
	-I$(top_builddir)/packages/optpp/include


#
# Paths to:
#  all interfaces headers, to acro_config.h, and to other configuration headers.  
#
AM_CPPFLAGS=-I$(TOPINC) $(THREEPO_INCLUDE) $(AMPL_INCLUDE) $(APPSPACK_INCLUDE)\
	$(MOMH_INCLUDE) $(NSGA2_INCLUDE)\
	$(UTILIB_INCLUDE) $(COLIN_INCLUDE) $(SCOLIB_INCLUDE)\
	$(LOCAL_TEUCHOS_INCLUDE) $(OPTPP_INCLUDE) $(BOOST_CPPFLAGS)\
	$(TINYXML_INCLUDE)\
	-DMPICH_IGNORE_CXX_SEEK

libinterfaces_a_SOURCES= \
  libs/APPS.cpp\
  libs/Cobyla.cpp\
  libs/Hooke.cpp\
  libs/OPTpp.cpp\
  libs/MOMHLib.cpp\
  libs/NSGA2.cpp\
  libs/InterfacesRegistrations.cpp
# NOTE: InterfacesRegistrations MUST be the last object in this list

FOO= \
  libs/factory.cpp\
  libs/DOT.cpp\
  libs/NPSOL.cpp\
  libs/TCC.cpp

if BUILD_APPSPACK
APPSPACKLIB= $(top_builddir)/tpl/appspack/src/libappspack.a\
	 $(LAPACK_LIBS) $(BLAS_LIBS) 
endif
if BUILD_THREEPO
THREEPOLIB=$(top_builddir)/tpl/3po/lib3po.a
endif
if BUILD_MOMHLIB
MOMHLIB=$(top_builddir)/tpl/momhlib/libmomh.a
endif

if BUILD_AMPL
AMPLLIB=$(top_builddir)/tpl/ampl/amplsolver.a
endif


if BUILD_OPTPP
# only link if building OPT++
TEUCHOSLIB = @TEUCHOS_LDFLAGS@ -lteuchos
OPTPPLIB=$(top_builddir)/packages/optpp/lib/liboptpp.la
endif

if BUILD_PEBBL
PEBBLLIB=$(top_builddir)/packages/pebbl/src/libpebbl.a
endif
if BUILD_SCOLIB
SCOLIB=$(top_builddir)/packages/scolib/src/libscolib.a
endif

#$(top_builddir)/packages/coliny/src/libcoliny.a

LDADD_ACRO_LIBS = libinterfaces.a \
	$(SCOLIB) \
	$(top_builddir)/packages/colin/src/libcolin.a \
	$(PEBBLLIB) \
	$(top_builddir)/packages/utilib/src/libutilib.la

LDADD=$(LDADD_ACRO_LIBS) $(THREEPOLIB) $(MOMHLIB) $(TEUCHOSLIB) $(OPTPPLIB) \
	$(AMPLLIB) $(APPSPACKLIB) $(R_LIBS) -lm $(DLOPEN_LIBS)


if BUILD_EXECUTABLES
if HAVE_BOOST
COLINY_BIN=coliny
bin_PROGRAMS = \
$(COLINY_BIN)

coliny_SOURCES=exec/coliny.cpp exec/funcadd.c
coliny_DEPENDENCIES=$(LDADD_ACRO_LIBS)
endif
endif

SUBDIRS= .

all-local: libinterfaces.a coliny$(EXEEXT)
	@if test -d $(TOPLIB) ; then : ; else mkdir $(TOPLIB) ; fi
	@$(SRCBIN)/cp_u libinterfaces.a $(TOPLIB)
	@$(SRCBIN)/cp_u coliny$(EXEEXT) $(BLDBIN)
	@$(SRCBIN)/cp_u coliny$(EXEEXT) ../test/colinytest$(EXEEXT)

clean-local:
	@rm -f $(TOPLIB)/libinterfaces.a
	@rm -f $(BLDBIN)/coliny
	@rm -f ../test/colinytest

MAINTAINERCLEANFILES = Makefile.in
