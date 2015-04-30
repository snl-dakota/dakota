
include ${top_srcdir}/config/MakeMacros.mk
SUFFIXES= .i

if I_AM_SUBPACKAGE
  TOPINC=../../../include
  TOPLIB=../../../lib
  TOPBUILDBIN=../../../bin
else
  TOPINC=${top_builddir}
  TOPLIB=${top_builddir}/lib
  TOPBUILDBIN=${top_builddir}/bin
endif
TOPSRCBIN=${top_srcdir}/bin

if BUILD_SHARED
if BUILD_CYGWIN
  DEPENDENCY_FLAG=-no-undefined -avoid-version -Wl,--add-stdcall-alias
else
  DEPENDENCY_FLAG=-avoid-version
endif
endif

ALL_H_SRC=\
  $(srcdir)/utilib/Binomial.h\
  $(srcdir)/utilib/DUniform.h\
  $(srcdir)/utilib/SampleGenerators.h\
  $(srcdir)/utilib/LCG.h\
  $(srcdir)/utilib/RNG.h\
  $(srcdir)/utilib/Triangular.h\
  $(srcdir)/utilib/Geometric.h\
  $(srcdir)/utilib/Random.h\
  $(srcdir)/utilib/RandomVariable.h\
  $(srcdir)/utilib/Cauchy.h\
  $(srcdir)/utilib/MUniform.h\
  $(srcdir)/utilib/LEC_rng.h\
  $(srcdir)/utilib/LogNormal.h\
  $(srcdir)/utilib/Normal.h\
  $(srcdir)/utilib/PM_LCG.h\
  $(srcdir)/utilib/Uniform.h\
  $(srcdir)/utilib/Exponential.h\
  $(srcdir)/utilib/MNormal.h\
  $(srcdir)/utilib/AnyRNG.h\
  $(srcdir)/utilib/default_rng.h\
  $(srcdir)/utilib/traits.h\
  $(srcdir)/utilib/HashedSet.h\
  $(srcdir)/utilib/SimpleHeap.h\
  $(srcdir)/utilib/StackList.h\
  $(srcdir)/utilib/Factory.h\
  $(srcdir)/utilib/BitArray.h\
  $(srcdir)/utilib/QueueList.h\
  $(srcdir)/utilib/bimap.h\
  $(srcdir)/utilib/TwoBitArray.h\
  $(srcdir)/utilib/EnumBitArray.h\
  $(srcdir)/utilib/ValuedContainer.h\
  $(srcdir)/utilib/BasicArray.h\
  $(srcdir)/utilib/AbstractHeap.h\
  $(srcdir)/utilib/CachedAllocator.h\
  $(srcdir)/utilib/SimpleSplayTree.h\
  $(srcdir)/utilib/Simple2DArray.h\
  $(srcdir)/utilib/LPHashTable.h\
  $(srcdir)/utilib/SmartHandle.h\
  $(srcdir)/utilib/SmartPtr.h\
  $(srcdir)/utilib/Num2DArray.h\
  $(srcdir)/utilib/Basic2DArray.h\
  $(srcdir)/utilib/stl_auxiliary.h\
  $(srcdir)/utilib/hash_fn.h\
  $(srcdir)/utilib/SimpleHashTable.h\
  $(srcdir)/utilib/GenericHeap.h\
  $(srcdir)/utilib/string_ops.h\
  $(srcdir)/utilib/pvector.h\
  $(srcdir)/utilib/RefCount.h\
  $(srcdir)/utilib/SharedValue.h\
  $(srcdir)/utilib/SharedPtr.h\
  $(srcdir)/utilib/NumArray.h\
  $(srcdir)/utilib/StringRegistry.h\
  $(srcdir)/utilib/AbstractSplayTree.h\
  $(srcdir)/utilib/Ereal.h\
  $(srcdir)/utilib/PersistentPointer.h\
  $(srcdir)/utilib/GenericSplayTree.h\
  $(srcdir)/utilib/IntVector.h\
  $(srcdir)/utilib/GenericKey.h\
  $(srcdir)/utilib/SparseMatrix.h\
  $(srcdir)/utilib/DoubleVector.h\
  $(srcdir)/utilib/ClassRef.h\
  $(srcdir)/utilib/BitArrayBase.h\
  $(srcdir)/utilib/LinkedList.h\
  $(srcdir)/utilib/MixedIntVars.h\
  $(srcdir)/utilib/ArrayBase.h\
  $(srcdir)/utilib/OrderedList.h\
  $(srcdir)/utilib/IntMatrix.h\
  $(srcdir)/utilib/CharString.h\
  $(srcdir)/utilib/AbstractHashTable.h\
  $(srcdir)/utilib/Tuple.h\
  $(srcdir)/utilib/Basic3DArray.h\
  $(srcdir)/utilib/GenericHashTable.h\
  $(srcdir)/utilib/DoubleMatrix.h\
  $(srcdir)/utilib/ReferenceCounted.h\
  $(srcdir)/utilib/compare.h\
  $(srcdir)/utilib/linpack.h\
  $(srcdir)/utilib/math_array.h\
  $(srcdir)/utilib/math_basic.h\
  $(srcdir)/utilib/_math.h\
  $(srcdir)/utilib/math_matrix.h\
  $(srcdir)/utilib/sort.h\
  $(srcdir)/utilib/_generic.h\
  $(srcdir)/utilib/CommonIO.h\
  $(srcdir)/utilib/exception_mngr.h\
  $(srcdir)/utilib/std_headers.h\
  $(srcdir)/utilib/memdebug.h\
  $(srcdir)/utilib/enum_def.h\
  $(srcdir)/utilib/mpi_utilib.h\
  $(srcdir)/utilib/seconds.h\
  $(srcdir)/utilib/file_util.h\
  $(srcdir)/utilib/ValidateParameter.h\
  $(srcdir)/utilib/ParameterList.h\
  $(srcdir)/utilib/Parameter.h\
  $(srcdir)/utilib/ParameterSet.h\
  $(srcdir)/utilib/Property.h\
  $(srcdir)/utilib/PropertyDict.h\
  $(srcdir)/utilib/PropertyDict_YamlPrinter.h\
  $(srcdir)/utilib/PropertyValidators.h\
  $(srcdir)/utilib/OptionParser.h\
  $(srcdir)/utilib/OStreamTee.h\
  $(srcdir)/utilib/TinyXML_helper.h\
  $(srcdir)/utilib/mpiUtil.h\
  $(srcdir)/utilib/comments.h\
  $(srcdir)/utilib/logEvent.h\
  $(srcdir)/utilib/nicePrint.h\
  $(srcdir)/utilib/Any.h\
  $(srcdir)/utilib/Any-deprecated.h\
  $(srcdir)/utilib/PackBuf.h\
  $(srcdir)/utilib/PackObject.h\
  $(srcdir)/utilib/Serialize.h\
  $(srcdir)/utilib/SerialStream.h\
  $(srcdir)/utilib/BinarySerialStream.h\
  $(srcdir)/utilib/fSerialStream.h\
  $(srcdir)/utilib/sSerialStream.h\
  $(srcdir)/utilib/MPIStream.h\
  $(srcdir)/utilib/XMLSerialStream.h\
  $(srcdir)/utilib/fXMLSerialStream.h\
  $(srcdir)/utilib/TypeManager_old.h\
  $(srcdir)/utilib/TypeManager.h

includeutilibdir=$(includedir)/utilib

includeutilib_HEADERS = $(ALL_H_SRC)

nodist_include_HEADERS = ${top_builddir}/src/utilib_config.h

#
# Paths to: utilib_config.h, other configure headers, utilib headers
#
AM_CPPFLAGS=$(TINYXML_INCLUDE) $(BOOST_CPPFLAGS)


libutilib_la_SOURCES= \
  libs/gencau.c   \
  libs/scauchy.c  \
  libs/genbet.c   \
  libs/genchi.c   \
  libs/genexp.c   \
  libs/genf.c     \
  libs/gengam.c   \
  libs/genmn.c    \
  libs/gennch.c   \
  libs/gennf.c    \
  libs/gennor.c   \
  libs/genunf.c   \
  libs/ignbin.c   \
  libs/ignpoi.c   \
  libs/sexpo.c    \
  libs/sgamma.c   \
  libs/snorm.c    \
  libs/genprm.c   \
  libs/globals.cpp  \
  libs/LCG.cpp    \
  libs/global_rng.c \
  libs/PM_LCG.cpp \
  libs/ParkMiller.c \
  libs/CharString.cpp   \
  libs/BitArray.cpp     \
  libs/MixedIntVars.cpp \
  libs/lookupa.cpp      \
  libs/hash_fn.cpp      \
  libs/Factory.cpp      \
  libs/Ereal.cpp        \
  libs/median.cpp   \
  libs/cov.c        \
  libs/cholesky.cpp \
  libs/math_basic.cpp \
  libs/math.c \
  libs/misc.c           \
  libs/seconds.cpp      \
  libs/memdebug.cpp     \
  libs/CommonIO.cpp     \
  libs/exception_mngr.cpp \
  libs/comments.cpp     \
  libs/mpiUtil.cpp      \
  libs/logEvent.cpp     \
  libs/nicePrint.cpp    \
  libs/OptionParser.cpp    \
  libs/TinyXML_helper.cpp    \
  libs/Property.cpp \
  libs/PropertyDict.cpp \
  libs/PropertyDict_YamlPrinter.cpp \
  libs/Parameter.cpp \
  libs/ParameterSet.cpp \
  libs/ParameterList.cpp \
  libs/Any.cpp      \
  libs/PackBuf.cpp      \
  libs/Serialize.cpp    \
  libs/SerialStream.cpp \
  libs/BinarySerialStream.cpp \
  libs/fSerialStream.cpp \
  libs/sSerialStream.cpp \
  libs/MPIStream.cpp \
  libs/XMLSerialStream.cpp \
  libs/fXMLSerialStream.cpp \
  libs/TypeManager_old.cpp \
  libs/TypeManager.cpp


EXTRA_INCLUDE_DIR=
EXTRA_LIB_FLAG=$(TINYXML_LIB)
EXTRA_LIBS=-lm

if USE_EXPAT_LIBRARY
if USE_EXPAT_INCDIR 
  EXTRA_INCLUDE_DIR += "-I@EXPAT_INCDIR@"
endif
if USE_EXPAT_LIBDIR 
  EXTRA_LIB_FLAG  += "-L/@EXPAT_LIBDIR@"
endif
  EXTRA_LIBS+= @EXPAT_LIBS@
endif

if HAVE_BOOST
  EXTRA_LIB_FLAG += $(BOOST_LDFLAGS)
  EXTRA_LIBS += -lboost_signals
endif

lib_LTLIBRARIES=libutilib.la

libutilib_la_LDFLAGS = $(DEPENDENCY_FLAG) $(EXTRA_LIB_FLAG)
libutilib_la_LIBADD = $(DEPENDENCIES) $(EXTRA_LIBS)
# next two lines are no-ops, but are required so objects for static lib
# have different base names than objects for dynamic lib
libutilib_la_CXXFLAGS=$(AM_CXXFLAGS)
libutilib_la_CPPFLAGS=$(AM_CPPFLAGS) $(EXTRA_INCLUDE_DIR)
libutilib_la_CFLAGS=$(AM_CFLAGS)

LDADD=libutilib.la

if !TARGET_CYGWIN
if !TARGET_MINGW
# sys/resource.h not available in Windows API
TIMER_EXECUTABLE=timer
timer_SOURCES=exec/timer.c
timer_LDADD=-lm
COPY_TIMER=$(TOPSRCBIN)/cp_u timer$(EXEEXT) $(TOPBUILDBIN)
CLEAN_TIMER=rm -f $(TOPBUILDBIN)/timer$(EXEEXT)
endif
endif

if BUILD_EXECUTABLES
bin_PROGRAMS=complete_doe$(EXEEXT) memmon$(EXEEXT) xu_doe$(EXEEXT) xu$(EXEEXT) $(TIMER_EXECUTABLE)

complete_doe_SOURCES=exec/doe/complete_doe.cpp
xu_doe_SOURCES=exec/doe/xu_doe.cpp
# NB: memmon and xu have no dependence on libutilib.a, so we override
# the default LDADD
memmon_SOURCES=exec/memmon.c
memmon_LDADD=-lm
xu_SOURCES=exec/doe/xu.c
xu_LDADD=-lm

if BUILD_STATIC_EXECUTABLES
#
#A libtool flag
#
AM_LDFLAGS=@STATIC_FLAGS@
endif
endif

SUBDIRS= .

all-local: copy-headers $(bin_PROGRAMS)
if BUILD_EXECUTABLES
	@if test -d $(TOPBUILDBIN) ; then : ; else mkdir $(TOPBUILDBIN) ; fi
	@$(TOPSRCBIN)/cp_u complete_doe$(EXEEXT) $(TOPBUILDBIN)
	@$(TOPSRCBIN)/cp_u memmon$(EXEEXT) $(TOPBUILDBIN)
	@$(COPY_TIMER)
	@$(TOPSRCBIN)/cp_u xu_doe$(EXEEXT) $(TOPBUILDBIN)
endif

clean-local: clean-headers
	@for i in *.gcda *.gcno *.gcov ; \
	do\
		rm -f $$i ;\
	done
if BUILD_EXECUTABLES
	@rm -f $(TOPBUILDBIN)/complete_doe$(EXEEXT)
	@rm -f $(TOPBUILDBIN)/memmon$(EXEEXT)
	@$(CLEAN_TIMER)
	@rm -f $(TOPBUILDBIN)/xu_doe$(EXEEXT)
endif

copy-headers:
if I_AM_SUBPACKAGE
	@if test -d $(TOPINC) ; then : ; else mkdir $(TOPINC) ; fi
	@if test -d $(TOPINC)/utilib ; then : ; else mkdir $(TOPINC)/utilib ; fi
	@$(TOPSRCBIN)/cp_u utilib_config.h $(TOPINC)
	@$(TOPSRCBIN)/cp_u $(ALL_H_SRC) $(TOPINC)/utilib
endif

clean-headers:
if I_AM_SUBPACKAGE
	@rm -rf $(TOPINC)/utilib
	@rm -f $(TOPINC)/utilib_config.h
else
#	@mv $(TOPINC)/utilib/utilib_config.h $(TOPINC)/utilib/utilib_config.sav
#	@rm -f $(TOPINC)/utilib/*.h
#	@mv $(TOPINC)/utilib/utilib_config.sav $(TOPINC)/utilib/utilib_config.h
endif
