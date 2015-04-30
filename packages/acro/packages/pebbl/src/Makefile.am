
include ../../../config/MakeMacros.mk

lib_LIBRARIES=libpebbl.a

TOPINC=${abs_top_builddir}/include
TOPLIB=${abs_top_builddir}/lib
TOPSRCBIN=${top_srcdir}/bin
TOPBUILDBIN=${top_builddir}/bin

if HAVE_ANSI_CPP_HEADERS
  ANSI_FLAG=-DANSI_HDRS 
else
  ANSI_FLAG=
endif

if HAVE_NAMESPACES
  NS_FLAG=-DANSI_NAMESPACES 
else
  NS_FLAG=
endif

#
# Paths to:
#  all headers, to config.h, and to other configuration headers.  
#
AMPL_INCLUDE=-I${top_srcdir}/tpl/ampl \
	-I${top_builddir}/tpl/ampl
UTILIB_INCLUDE=-I${top_srcdir}/packages/utilib/src \
	-I${top_builddir}/packages/utilib/src
PEBBL_INCLUDE=-I${top_srcdir}/packages/pebbl/src/bb \
	-I${top_srcdir}/packages/pebbl/src/comm \
	-I${top_srcdir}/packages/pebbl/src/example \
	-I${top_srcdir}/packages/pebbl/src/misc \
	-I${top_srcdir}/packages/pebbl/src/pbb \
	-I${top_srcdir}/packages/pebbl/src/sched

# If we are built with CLP, put it in the include paths because the
# monomial application will use it (JE)
if BUILD_COIN_CBC
PEBBL_INCLUDE += -I${TOPINC}/coin
endif

AM_CPPFLAGS=\
  -I${srcdir} $(PEBBL_INCLUDE) $(AMPL_INCLUDE) $(UTILIB_INCLUDE)\
  $(MPI_FLAG) $(ANSI_FLAG) $(NS_FLAG)

libpebbl_a_SOURCES= \
./bb/pebbl/branching.cpp\
./bb/pebbl/loadObject.cpp\
./bb/pebbl/pebblBase.cpp\
./bb/pebbl/pebblParams.cpp\
./comm/pebbl/MessageID.cpp\
./comm/pebbl/coTree.cpp\
./comm/pebbl/outBufferQ.cpp\
./misc/pebbl/clusterTree.cpp\
./misc/pebbl/clustering.cpp\
./misc/pebbl/followerTree.cpp\
./misc/pebbl/gRandom.cpp\
./misc/pebbl/memUtil.cpp\
./misc/pebbl/scatterObj.cpp\
./misc/pebbl/treeTopology.cpp\
./pbb/pebbl/earlyOutputThread.cpp\
./pbb/pebbl/hubThread.cpp\
./pbb/pebbl/incumbCast.cpp\
./pbb/pebbl/incumbSearchThread.cpp\
./pbb/pebbl/loadBal.cpp\
./pbb/pebbl/loadBalPair.cpp\
./pbb/pebbl/parBranchThreads.cpp\
./pbb/pebbl/parBranching.cpp\
./pbb/pebbl/parLoadObject.cpp\
./pbb/pebbl/parPebblBase.cpp\
./pbb/pebbl/parPebblParams.cpp\
./pbb/pebbl/pbCheckpoint.cpp\
./pbb/pebbl/pbEnumerate.cpp\
./pbb/pebbl/pbHub.cpp\
./pbb/pebbl/pbOutput.cpp\
./pbb/pebbl/pbPack.cpp\
./pbb/pebbl/pbWorker.cpp\
./pbb/pebbl/reposThreads.cpp\
./pbb/pebbl/llChainer.cpp\
./pbb/pebbl/spReceiver.cpp\
./pbb/pebbl/spServer.cpp\
./pbb/pebbl/workerAuxThread.cpp\
./pbb/pebbl/workerInHeap.cpp\
./pbb/pebbl/workerThread.cpp\
./sched/pebbl/Scheduler.cpp\
./sched/pebbl/SelfAdjustThd.cpp\
./sched/pebbl/ThreadObj.cpp\
./sched/pebbl/ThreadQueue.cpp

SUBDIRS= headers .

LDADD=-L. -L$(TOPLIB) -L$(top_builddir)/packages/pebbl/src/ libpebbl.a\
		$(top_builddir)/packages/utilib/src/libutilib.la -lm

# If we are built with CLP, include it in the links.  This will allow
# the monomial application to use CLP to update the objective function
# used in branch and boud, and run multiple iterations.  (JE)
if BUILD_COIN_CBC
LDADD += -lClp -lCoinUtils $(LAPACK_LIBS) $(BLAS_LIBS)
endif

if BUILD_EXECUTABLES
noinst_PROGRAMS = \
knapMPS\
lipshitzian\
core_test

bin_PROGRAMS=\
knapsack\
logAnalyze\
Docking\
monomial

knapMPS_SOURCES=example/pebbl/knapMPS.cpp example/pebbl/serialKnapsack.cpp
lipshitzian_SOURCES=example/pebbl/lipshitzian.cpp
core_test_SOURCES=example/pebbl/core_test.cpp
knapsack_SOURCES=example/pebbl/knapsack.cpp \
                 example/pebbl/serialKnapsack.cpp \
                 example/pebbl/parKnapsack.cpp
Docking_SOURCES=example/pebbl/Docking.cpp
logAnalyze_SOURCES=example/pebbl/logAnalyze.cpp
monomial_SOURCES=example/pebbl/monomial.cpp \
                 example/pebbl/serialMonomial.cpp \
                 example/pebbl/parMonomial.cpp

if BUILD_STATIC_EXECUTABLES
#
#A libtool flag
#
AM_LDFLAGS=@STATIC_FLAGS@
endif

endif


all-local: Docking$(EXEEXT) knapsack$(EXEEXT) logAnalyze$(EXEEXT) \
           monomial$(EXEEXT)
	@if test -d $(TOPLIB) ; then : ; else mkdir $(TOPLIB) ; fi
	@$(TOPSRCBIN)/cp_u $(top_builddir)/packages/pebbl/src/libpebbl.a $(TOPLIB)
if BUILD_EXECUTABLES
	@$(TOPSRCBIN)/cp_u Docking$(EXEEXT) $(TOPBUILDBIN)
	@$(TOPSRCBIN)/cp_u Docking$(EXEEXT) ${top_builddir}/packages/pebbl/test/DockingTest$(EXEEXT)
	@$(TOPSRCBIN)/cp_u knapsack$(EXEEXT) $(TOPBUILDBIN)
	@$(TOPSRCBIN)/cp_u knapsack$(EXEEXT) ${top_builddir}/packages/pebbl/test/knapsackTest$(EXEEXT)
	@$(TOPSRCBIN)/cp_u monomial$(EXEEXT) $(TOPBUILDBIN)
	@$(TOPSRCBIN)/cp_u monomial$(EXEEXT) ${top_builddir}/packages/pebbl/test/monomialTest$(EXEEXT)
	@$(TOPSRCBIN)/cp_u logAnalyze$(EXEEXT) $(TOPBUILDBIN)
endif

clean-local:
	@rm -f *.gcda *.gcno
	@rm -f $(TOPLIB)/libpebbl.a
if BUILD_EXECUTABLES
	@rm -f ${top_builddir}/packages/pebbl/test/DockingTest$(EXEEXT)
	@rm -f $(TOPBUILDBIN)/Docking
	@rm -f ${top_builddir}/packages/pebbl/test/knapsackTest$(EXEEXT)
	@rm -f $(TOPBUILDBIN)/knapsack
	@rm -f ${top_builddir}/packages/pebbl/test/monomialTest$(EXEEXT)
	@rm -f $(TOPBUILDBIN)/monomial
	@rm -f $(TOPBUILDBIN)/logAnalyze
endif

MAINTAINERCLEANFILES = Makefile.in
