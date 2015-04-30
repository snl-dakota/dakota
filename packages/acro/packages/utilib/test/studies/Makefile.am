
include ${top_srcdir}/config/MakeMacros.mk
SUFFIXES=.i

if I_AM_SUBPACKAGE
  TOPINC=../../../../include
  TOPLIB=../../../../lib
  TOPBUILDBIN=../../../../bin
  LDADD=../../src/libutilib.la
else
  TOPINC=$(top_builddir)
  TOPLIB=$(top_builddir)/lib
  TOPBUILDBIN=${top_builddir}/bin
  LDADD=$(top_builddir)/src/libutilib.la
endif

TOPSRCBIN=${top_srcdir}/bin

#SUBDIRS=unit

#
# Paths to: config.h, other configure headers, utilib headers
#
AM_CPPFLAGS=-I$(srcdir)/../../src $(TINYXML_INCLUDE) $(BOOST_CPPFLAGS)

if BUILD_STATIC_EXECUTABLES
#
#A libtool flag
#
AM_LDFLAGS=@STATIC_FLAGS@
endif

#
# programs to build at "make check" time
#
check_PROGRAMS = tests mpitests


if BUILD_SHARED
  EXECS=$(check_PROGRAMS:%=.libs/%)
else
  EXECS=$(check_PROGRAMS)
endif

BIN_COPIES=$(check_PROGRAMS:%=$(TOPBIN)/%)

##
## Sadly, "program_SOURCES" defaults to "program.c" if it's
## not supplied.  There's no way to say make it default to
## "program.cpp".
## 

tests_SOURCES= \
	olist.cpp \
	sort_test.cpp \
	array.cpp \
	bitarray.cpp \
	multibitarray.cpp \
	timing.cpp \
	tuple.cpp \
	iotest.cpp \
	rngtest.cpp \
	triang.cpp \
	vectest.cpp \
	stltest.cpp \
	hash.cpp \
	splay.cpp \
	factory.cpp \
	ereal.cpp \
	heap.cpp \
	calloc.cpp \
	malloc.cpp \
	pvector.cpp \
	exception_test.cpp \
	class_options.cpp \
	option_parser.cpp \
	sharedval.cpp \
	smartptr.cpp \
	commonio.cpp \
	stringtest.cpp \
	listtest.cpp \
	iotest_debug.cpp \
	commonio_debug.cpp \
	packbuf.cpp \
	lphash.cpp \
	hashedset.cpp \
	matrix.cpp \
	namespace.cpp \
	qlist.cpp \
	typeManager.cpp \
	math.cpp \
	sregistry.cpp \
	paramTest.cpp \
	tests.cpp 

OLD=\
	any.cpp \
	any-deprecated.cpp \
	serialize.cpp \
	sserialstream.cpp \
	fserialstream.cpp \
	fserialstream-ref.cpp \
	fxmlserialstream.cpp

mpitests_SOURCES= \
	mpitest.cpp \
	mpistream.cpp \
	mpitests.cpp 

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


