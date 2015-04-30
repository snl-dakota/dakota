
include ../../../config/MakeMacros.mk

TOPBIN=$(top_builddir)/bin
TOPINC=$(top_builddir)/include
TOPLIB=$(top_builddir)/lib

THREEPO_INCLUDE=-I${top_srcdir}/tpl/3po/cobyla2c
AMPL_INCLUDE=-I${top_srcdir}/tpl/ampl
APPSPACK_INCLUDE=-I${top_srcdir}/tpl/appspack/src
MOMH_INCLUDE=-I${top_srcdir}/tpl/momhlib/MOMHLib/Common \
	-I${top_srcdir}/tpl/momhlib/MOMHLib/HGA \
	-I${top_srcdir}/tpl/momhlib/MOMHLib/MO \
	-I${top_srcdir}/tpl/momhlib/MOMHLib/MOMSLS \
	-I${top_srcdir}/tpl/momhlib/MOMHLib/ParetoGA \
	-I${top_srcdir}/tpl/momhlib/MOMHLib/SA

UTILIB_INCLUDE=-I${top_srcdir}/packages/utilib/src \
	-I${top_builddir}/packages/utilib/src
COLIN_INCLUDE=-I${top_srcdir}/packages/colin/src
SCOLIB_INCLUDE=-I${top_srcdir}/packages/scolib/src

LOCAL_TEUCHOS_INCLUDE = @TEUCHOS_CPPFLAGS@

OPTPP_INCLUDE=-I$(top_srcdir)/packages/optpp/include 


if BUILD_APPSPACK
APPSPACKLIB=$(top_builddir)/tpl/appspack/src/libappspack.a\
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

LDADD=$(top_builddir)/packages/interfaces/src/libinterfaces.a\
	$(SCOLIB)\
	$(top_builddir)/packages/colin/src/libcolin.a\
	$(PEBBLLIB)\
	$(top_builddir)/packages/utilib/src/libutilib.la \
	$(THREEPOLIB) $(MOMHLIB) $(OPTPPLIB) $(TEUCHOSLIB) \
	$(AMPLLIB) $(APPSPACKLIB) \
	$(DLOPEN_LIBS) 

#
# Paths to: config.h, other configure headers, utilib headers, MPI
#
AM_CPPFLAGS=$(THREEPO_INCLUDE) $(AMPL_INCLUDE) $(APPSPACK_INCLUDE)\
	$(MOMH_INCLUDE) $(UTILIB_INCLUDE) $(COLIN_INCLUDE) $(SCOLIB_INCLUDE)\
	$(LOCAL_TEUCHOS_INCLUDE) $(OPTPP_INCLUDE)\
	$(BOOST_CPPFLAGS) $(TINYXML_INCLUDE)\
	-DMPICH_IGNORE_CXX_SEEK

check_PROGRAMS = namespace

namespace_SOURCES=namespace.cpp

TESTS = namespace

MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES=acrotest
