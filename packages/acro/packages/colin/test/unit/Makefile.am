
include ${top_srcdir}/config/MakeMacros.mk
SUFFIXES=.i

CXXTEST= ${top_builddir}/python/bin/cxxtestgen

TOPINC=$(top_builddir)/include
TOPLIB=$(top_builddir)/lib
TOPBUILDBIN=${top_builddir}/bin
TOPSRCBIN=${top_srcdir}/bin

if HAVE_ANSI_CPP_HEADERS
  ANSI_FLAG=-DANSI_HDRS
else
  ANSI_FLAG=
endif

if BUILD_AMPL
AMPLLIB=$(top_builddir)/tpl/ampl/amplsolver.a
endif

UTILIB_INCLUDE=-I${top_srcdir}/packages/utilib/src \
	-I${top_builddir}/packages/utilib/src
COLIN_INCLUDE=-I${top_srcdir}/packages/colin/src
CXXTEST_INCLUDE=-I${top_srcdir}/tpl/cxxtest

#
# Paths to: config.h, other configure headers, colin headers
#
LDADD=$(top_builddir)/packages/colin/src/libcolin.a \
	$(top_builddir)/packages/utilib/src/libutilib.la \
	$(AMPLLIB) -lm $(DLOPEN_LIBS)

#
# Paths to: config.h, other configure headers, utilib headers, MPI
#
AM_CPPFLAGS=$(CXXTEST_INCLUDE) $(UTILIB_INCLUDE) $(COLIN_INCLUDE) \
            -I$(TOPINC) $(MPI_FLAGS) $(ANSI_FLAG) $(BOOST_CPPFLAGS) \
            $(TINYXML_INCLUDE) -DMPICH_IGNORE_CXX_SEEK -DIGNORE_BROKEN

if BUILD_STATIC_EXECUTABLES
#
#A libtool flag
#
AM_LDFLAGS=@STATIC_FLAGS@
endif

#
# programs to build at "make check" time
#
check_PROGRAMS = runner


#if BUILD_SHARED
#EXECS=$(check_PROGRAMS:%=.libs/%)
#else
#EXECS=$(check_PROGRAMS)
#endif
EXECS=$(check_PROGRAMS)

BIN_COPIES=$(check_PROGRAMS:%=$(TOPBIN)/%)

runner_CXXTEST_SRC=\
	TObjectType.cpp \
	TRealDomain.cpp \
	TIntDomain.cpp \
	TGradient.cpp \
	TConstraints.cpp \
	TConstraintPenalty.cpp \
	TDowncast.cpp \
	TFiniteDifference_Real.cpp \
	TFiniteDifference_MixedInt.cpp \
	TLinearConstraints.cpp \
	TNonD_Constraints.cpp \
	TNonlinearConstraints.cpp \
	TRelaxableMixedIntDomain.cpp \
	TTestApplications.cpp \
	TUnconMultiobj.cpp \
	TWeightedSum.cpp \
	TAmplApplication.cpp \
	TCache.cpp \
	TView_Pareto.cpp \
	runner.cpp

#	TReformulation_NLP1_NLP0.cpp

runner_STATIC_SRC=

runner_SOURCES=$(runner_STATIC_SRC) $(runner_CXXTEST_SRC) 

runner.cpp:
	$(CXXTEST) --have-eh --have-std --root --xunit-printer \
		-o runner.cpp --include=runner.h

# Implicit rule to invoke cxxtest to autogenerate .cpp from .h test files
#  [test needed to prevent using cxxtest on non-test sources]
.h.cpp:
	@for x in $(runner_CXXTEST_SRC); do \
	   if [ "$$x" = "$@" ]; then \
	      echo $(CXXTEST) --have-eh --have-std --part -o $@ $<; \
	      $(CXXTEST) --have-eh --have-std --part -o $@ $<; \
	   fi \
	done

# prevent implicit rule for static sources
.PHONY:
$(runner_STATIC_SRC):
	@echo NO-OP

check-local:
	@if test -d $(TOPBUILDBIN) ; then : ; else mkdir $(TOPBUILDBIN); fi
	@if test -d $(TOPBUILDBIN)/colin ; then : ; else mkdir $(TOPBUILDBIN)/colin; fi
	@$(TOPSRCBIN)/cp_u $(EXECS) $(TOPBUILDBIN)/colin

clean-local:
	@rm -rf $(TOPBUILDBIN)/colin $(runner_CXXTEST_SRC)
	@for i in *.gcda *.gcno *.gcov ; \
        do\
                rm -f $$i ;\
        done


