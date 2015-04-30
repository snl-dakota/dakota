
#
# SLB: Temporary flag to prevent parallel compilation by gmake
#

include ../../../config/MakeMacros.mk

lib_LIBRARIES=libcolin.a

TOPINC=../../../include
TOPLIB=../../../lib
TOPBIN=${top_srcdir}/bin

if HAVE_ANSI_CPP_HEADERS
  ANSI_FLAG=-DANSI_HDRS 
else
  ANSI_FLAG=
endif

SUBDIRS = . colin

AMPL_INCLUDE=-I${top_srcdir}/tpl/ampl \
	-I${top_builddir}/tpl/ampl
UTILIB_INCLUDE=-I${top_srcdir}/packages/utilib/src \
	-I${top_builddir}/packages/utilib/src

#
# Paths to: config.h, other configure headers, colin headers, MPI
#
AM_CPPFLAGS=-I${srcdir} $(MPI_FLAG) $(ANSI_FLAG) $(AMPL_INCLUDE) \
	$(UTILIB_INCLUDE) $(TINYXML_INCLUDE) $(BOOST_CPPFLAGS) \
        -DMPICH_IGNORE_CXX_SEEK
LDADD=libcolin.a

libcolin_a_SOURCES= \
  libs/AppResponseInfo.cpp\
  libs/AppResponse.cpp\
  libs/AppResponseXML.cpp\
  libs/AppRequest.cpp\
  libs/Application_Base.cpp\
  libs/Application_SingleObjective.cpp\
  libs/Application_MultiObjective.cpp\
  libs/Application_NonD_Objective.cpp\
  libs/Application_Domain.cpp\
  libs/Application_IntDomain.cpp\
  libs/Application_RealDomain.cpp\
  libs/Application_Constraints.cpp\
  libs/Application_LinearConstraints.cpp\
  libs/Application_NonlinearConstraints.cpp\
  libs/Application_NonD_Constraints.cpp\
  libs/Application_Gradient.cpp\
  libs/Application_Jacobian.cpp\
  libs/Application_Hessian.cpp\
  libs/ApplicationMngr.cpp\
  libs/AsynchronousApplication.cpp\
  libs/ContextMngr.cpp\
  libs/SolverMngr.cpp\
  libs/ColinRegistrations.cpp\
  libs/AnalysisCode.cpp\
  libs/TinyXML_data_parser.cpp\
  libs/XMLProcessor.cpp\
  libs/ReformulationApplication.cpp\
  libs/AnalysisCodeApplication.cpp\
  libs/ConstraintPenaltyApplication.cpp\
  libs/DowncastApplication.cpp\
  libs/FiniteDiffApplication.cpp\
  libs/RelaxableMixedIntDomainApplication.cpp\
  libs/SamplingApplication.cpp\
  libs/SubspaceApplication.cpp\
  libs/SynchronousApplication.cpp\
  libs/UnconMultiobjApplication.cpp\
  libs/UpcastApplication.cpp\
  libs/WeightedSumApplication.cpp\
  libs/AmplApplication.cpp\
  libs/ResponseGenerator.cpp\
  libs/QueueManager.cpp\
  libs/LocalQueueManager.cpp\
  libs/SerialQueueManager.cpp\
  libs/EvaluationID.cpp\
  libs/EvaluationManager.cpp\
  libs/EvaluationManagerFactory.cpp\
  libs/ConcurrentEvaluator.cpp\
  libs/SerialEvaluator.cpp\
  libs/Solver_Base.cpp\
  libs/ExecuteMngr.cpp\
  libs/LibLoader.cpp\
  libs/ProcessManager_Local.cpp\
  libs/ProcessManager_MPI.cpp\
  libs/Cache.cpp\
  libs/Cache_Factory.cpp\
  libs/Cache_Local.cpp\
  libs/Cache_MasterSlave.cpp\
  libs/Cache_View_CommonBase.cpp\
  libs/Cache_View_Labeled.cpp\
  libs/Cache_View_Unlabeled.cpp\
  libs/Cache_View_Subset.cpp\
  libs/Cache_View_Pareto.cpp\
  libs/KeyGenerator_EpsilonMatch.cpp\
  libs/ColinGlobals.cpp\
  libs/ColinDriver.cpp\
  libs/AmplDriver.cpp\
  libs/SimpleMILocalSearch.cpp\
  libs/RandomMOO.cpp\
  libs/GradientDescent.cpp

#AppResponseAnalysis.cpp

all-local: libcolin.a
	@if test -d $(TOPLIB) ; then : ; else mkdir $(TOPLIB) ; fi
	@$(TOPBIN)/cp_u libcolin.a $(TOPLIB)

clean-local:
	@rm -f $(TOPLIB)/libcolin.a
	@rm -f *.i

MAINTAINERCLEANFILES = Makefile.in

