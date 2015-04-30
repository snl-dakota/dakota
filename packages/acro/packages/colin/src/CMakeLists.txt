add_library(colin
  libs/AppResponseInfo.cpp
  libs/AppResponse.cpp
  libs/AppResponseXML.cpp
  libs/AppRequest.cpp
  libs/Application_Base.cpp
  libs/Application_SingleObjective.cpp
  libs/Application_MultiObjective.cpp
  libs/Application_NonD_Objective.cpp
  libs/Application_Domain.cpp
  libs/Application_IntDomain.cpp
  libs/Application_RealDomain.cpp
  libs/Application_Constraints.cpp
  libs/Application_LinearConstraints.cpp
  libs/Application_NonlinearConstraints.cpp
  libs/Application_NonD_Constraints.cpp
  libs/Application_Gradient.cpp
  libs/Application_Jacobian.cpp
  libs/Application_Hessian.cpp
  libs/ApplicationMngr.cpp
  libs/AsynchronousApplication.cpp
  libs/ContextMngr.cpp
  libs/SolverMngr.cpp
  libs/ColinRegistrations.cpp
  libs/AnalysisCode.cpp
  libs/TinyXML_data_parser.cpp
  libs/XMLProcessor.cpp
  libs/ReformulationApplication.cpp
  libs/AnalysisCodeApplication.cpp
  libs/ConstraintPenaltyApplication.cpp
  libs/DowncastApplication.cpp
  libs/FiniteDiffApplication.cpp
  libs/RelaxableMixedIntDomainApplication.cpp
  libs/SamplingApplication.cpp
  libs/SubspaceApplication.cpp
  libs/SynchronousApplication.cpp
  libs/UnconMultiobjApplication.cpp
  libs/UpcastApplication.cpp
  libs/WeightedSumApplication.cpp
  libs/AmplApplication.cpp
  libs/ResponseGenerator.cpp
  libs/QueueManager.cpp
  libs/LocalQueueManager.cpp
  libs/SerialQueueManager.cpp
  libs/EvaluationID.cpp
  libs/EvaluationManager.cpp
  libs/EvaluationManagerFactory.cpp
  libs/ConcurrentEvaluator.cpp
  libs/SerialEvaluator.cpp
  libs/Solver_Base.cpp
  libs/ExecuteMngr.cpp
  libs/LibLoader.cpp
  libs/ProcessManager_Local.cpp
  libs/ProcessManager_MPI.cpp
  libs/Cache.cpp
  libs/Cache_Factory.cpp
  libs/Cache_Local.cpp
  libs/Cache_MasterSlave.cpp
  libs/Cache_View_CommonBase.cpp
  libs/Cache_View_Labeled.cpp
  libs/Cache_View_Unlabeled.cpp
  libs/Cache_View_Subset.cpp
  libs/Cache_View_Pareto.cpp
  libs/KeyGenerator_EpsilonMatch.cpp
  libs/ColinGlobals.cpp
  libs/ColinDriver.cpp
  libs/AmplDriver.cpp
  libs/SimpleMILocalSearch.cpp
  libs/RandomMOO.cpp
  libs/GradientDescent.cpp
)

# It should suffice to link libcolin against libdl; shouldn't need to
# explicitly link all downstream exes due to CMake dep management
target_link_libraries(colin tinyxml utilib ${DL_LIBRARY})
if (HAVE_AMPL)
  target_link_libraries(colin amplsolver)
endif()

install(TARGETS colin EXPORT ${ExportTarget} DESTINATION lib)

install(DIRECTORY colin DESTINATION include PATTERN ".svn" EXCLUDE
  PATTERN "Makefile.am" EXCLUDE)
