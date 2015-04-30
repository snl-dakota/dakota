
#
# SLB: Temporary flag to prevent parallel compilation by gmake
#
.NOTPARALLEL:

include ../../../../config/MakeMacros.mk

TOPINC=../../../../include
TOPBIN=${top_srcdir}/bin

MAINTAINERCLEANFILES = Makefile.in

FULLPATH_SRC=${srcdir}

COLIN_H_SRC=\
  $(FULLPATH_SRC)/OptPointDB.h\
  $(FULLPATH_SRC)/AmplApplication.h\
  $(FULLPATH_SRC)/AmplDriver.h\
  $(FULLPATH_SRC)/AnalysisCode.h\
  $(FULLPATH_SRC)/AnalysisCodeApplication.h\
  $(FULLPATH_SRC)/Application.h\
  $(FULLPATH_SRC)/ApplicationMngr.h\
  $(FULLPATH_SRC)/AppResponseAnalysis.h\
  $(FULLPATH_SRC)/AppResponse.h\
  $(FULLPATH_SRC)/AppRequest.h\
  $(FULLPATH_SRC)/AppResponseInfo.h\
  $(FULLPATH_SRC)/AppResponseXML.h\
  $(FULLPATH_SRC)/AppResponse_Impl.h\
  $(FULLPATH_SRC)/AppRequest_Impl.h\
  $(FULLPATH_SRC)/AsynchronousApplication.h\
  $(FULLPATH_SRC)/BoostExtras.h\
  $(FULLPATH_SRC)/BoundTypeArray.h\
  $(FULLPATH_SRC)/ColinGlobals.h\
  $(FULLPATH_SRC)/ColinDriver.h\
  $(FULLPATH_SRC)/ColinRegistrations.h\
  $(FULLPATH_SRC)/CommandShell.h\
  $(FULLPATH_SRC)/CommonOptions.h\
  $(FULLPATH_SRC)/ConfigurableApplication.h\
  $(FULLPATH_SRC)/ContextMngr.h\
  $(FULLPATH_SRC)/DirectFuncApplication.h\
  $(FULLPATH_SRC)/DirectSimpleApplication.h\
  $(FULLPATH_SRC)/EvaluationID.h\
  $(FULLPATH_SRC)/EvaluationManager.h\
  $(FULLPATH_SRC)/EvaluationManagerFactory.h\
  $(FULLPATH_SRC)/ExecuteMngr.h\
  $(FULLPATH_SRC)/Handle.h\
  $(FULLPATH_SRC)/MasterSlaveOptExec.h\
  $(FULLPATH_SRC)/MSApplication.h\
  $(FULLPATH_SRC)/ObjectType.h\
  $(FULLPATH_SRC)/OptApplications.h\
  $(FULLPATH_SRC)/OptResponse.h\
  $(FULLPATH_SRC)/OptSolverUtil.h\
  $(FULLPATH_SRC)/ParetoSet.h\
  $(FULLPATH_SRC)/PointSet.h\
  $(FULLPATH_SRC)/Problem.h\
  $(FULLPATH_SRC)/ProblemTraits.h\
  $(FULLPATH_SRC)/ProcessManager.h\
  $(FULLPATH_SRC)/QueueManager.h\
  $(FULLPATH_SRC)/real.h\
  $(FULLPATH_SRC)/Cache.h\
  $(FULLPATH_SRC)/ResponseFunctors.h\
  $(FULLPATH_SRC)/ResponseGenerator.h\
  $(FULLPATH_SRC)/Solver.h\
  $(FULLPATH_SRC)/SolverMngr.h\
  $(FULLPATH_SRC)/StatusCodes.h\
  $(FULLPATH_SRC)/SynchronousApplication.h\
  $(FULLPATH_SRC)/SysCallAnalysisCode.h\
  $(FULLPATH_SRC)/TinyXML_data_parser.h\
  $(FULLPATH_SRC)/TinyXML.h\
  $(FULLPATH_SRC)/Utilities.h\
  $(FULLPATH_SRC)/XMLProcessor.h

APPLICATION_H_SRC=\
  $(FULLPATH_SRC)/application/Base.h\
  $(FULLPATH_SRC)/application/Constraints.h\
  $(FULLPATH_SRC)/application/Domain.h\
  $(FULLPATH_SRC)/application/Gradient.h\
  $(FULLPATH_SRC)/application/Jacobian.h\
  $(FULLPATH_SRC)/application/Hessian.h\
  $(FULLPATH_SRC)/application/IntDomain.h\
  $(FULLPATH_SRC)/application/LinearConstraints.h\
  $(FULLPATH_SRC)/application/NonlinearConstraints.h\
  $(FULLPATH_SRC)/application/MultiObjective.h\
  $(FULLPATH_SRC)/application/NonD_Constraints.h\
  $(FULLPATH_SRC)/application/NonD_Objective.h\
  $(FULLPATH_SRC)/application/RealDomain.h\
  $(FULLPATH_SRC)/application/SingleObjective.h

CACHE_H_SRC=\
  $(FULLPATH_SRC)/cache/Factory.h\
  $(FULLPATH_SRC)/cache/KeyGenerator_EpsilonMatch.h\
  $(FULLPATH_SRC)/cache/Local.h\
  $(FULLPATH_SRC)/cache/MasterSlave.h\
  $(FULLPATH_SRC)/cache/View_CommonBase.h\
  $(FULLPATH_SRC)/cache/View_Labeled.h\
  $(FULLPATH_SRC)/cache/View_Unlabeled.h\
  $(FULLPATH_SRC)/cache/View_Pareto.h\
  $(FULLPATH_SRC)/cache/View_Subset.h

EVAL_H_SRC=\
  $(FULLPATH_SRC)/eval/ConcurrentEvaluator.h\
  $(FULLPATH_SRC)/eval/SerialEvaluator.h\
  $(FULLPATH_SRC)/eval/LocalQueueManager.h\
  $(FULLPATH_SRC)/eval/SerialQueueManager.h

REFORMULATION_H_SRC=\
  $(FULLPATH_SRC)/reformulation/Base.h\
  $(FULLPATH_SRC)/reformulation/ConstraintPenalty.h\
  $(FULLPATH_SRC)/reformulation/Downcast.h\
  $(FULLPATH_SRC)/reformulation/FiniteDifference.h\
  $(FULLPATH_SRC)/reformulation/InvertSense.h\
  $(FULLPATH_SRC)/reformulation/Sampling.h\
  $(FULLPATH_SRC)/reformulation/Subspace.h\
  $(FULLPATH_SRC)/reformulation/UnconMultiobj.h\
  $(FULLPATH_SRC)/reformulation/Upcast.h\
  $(FULLPATH_SRC)/reformulation/WeightedSum.h\
  $(FULLPATH_SRC)/reformulation/RelaxableMixedIntDomain.h

SOLVER_H_SRC=\
  $(FULLPATH_SRC)/solver/Base.h\
  $(FULLPATH_SRC)/solver/ColinSolver.h\
  $(FULLPATH_SRC)/solver/GradientDescent.h\
  $(FULLPATH_SRC)/solver/RandomMOO.h\
  $(FULLPATH_SRC)/solver/SimpleMILocalSearch.h


includecolindir=$(includedir)/colin
includecolin_HEADERS = $(COLIN_H_SRC)

includeapplicationdir=$(includedir)/colin/application
includeapplication_HEADERS = $(APPLICATION_H_SRC)

includecachedir=$(includedir)/colin/cache
includecache_HEADERS = $(CACHE_H_SRC)

includeevaldir=$(includedir)/colin/eval
includeeval_HEADERS = $(EVAL_H_SRC)

includereformulationdir=$(includedir)/colin/reformulation
includereformulation_HEADERS = $(REFORMULATION_H_SRC)

includesolverdir=$(includedir)/colin/solver
includesolver_HEADERS = $(SOLVER_H_SRC)

all-local:
	@if test -d $(TOPINC) ; then : ; else mkdir $(TOPINC) ; fi
	@if test -d $(TOPINC)/colin ; then : ; else mkdir $(TOPINC)/colin ; fi
	@mkdir -p $(TOPINC)/colin/application
	@mkdir -p $(TOPINC)/colin/cache
	@mkdir -p $(TOPINC)/colin/eval
	@mkdir -p $(TOPINC)/colin/reformulation
	@mkdir -p $(TOPINC)/colin/solver
	@$(TOPBIN)/cp_u $(COLIN_H_SRC) $(TOPINC)/colin
	@$(TOPBIN)/cp_u $(APPLICATION_H_SRC) $(TOPINC)/colin/application
	@$(TOPBIN)/cp_u $(CACHE_H_SRC) $(TOPINC)/colin/cache
	@$(TOPBIN)/cp_u $(EVAL_H_SRC) $(TOPINC)/colin/eval
	@$(TOPBIN)/cp_u $(REFORMULATION_H_SRC) $(TOPINC)/colin/reformulation
	@$(TOPBIN)/cp_u $(SOLVER_H_SRC) $(TOPINC)/colin/solver

clean-local:
	@rm -rf $(TOPINC)/colin
