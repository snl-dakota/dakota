add_library(jega
  BitManipulator.cpp 
  FitnessRecord.cpp 
  GeneticAlgorithm.cpp 
  GeneticAlgorithmConverger.cpp 
  GeneticAlgorithmCrosser.cpp 
  GeneticAlgorithmEvaluator.cpp 
  GeneticAlgorithmFitnessAssessor.cpp 
  GeneticAlgorithmInitializer.cpp 
  GeneticAlgorithmMainLoop.cpp 
  GeneticAlgorithmMutator.cpp 
  GeneticAlgorithmNichePressureApplicator.cpp 
  GeneticAlgorithmOperator.cpp 
  GeneticAlgorithmOperatorGroup.cpp 
  GeneticAlgorithmOperatorSet.cpp 
  GeneticAlgorithmPostProcessor.cpp 
  GeneticAlgorithmSelector.cpp 
  MetricTracker.cpp 
  Convergers/AverageFitnessTrackerConverger.cpp 
  Convergers/BestFitnessTrackerConverger.cpp 
  Convergers/MaxGenEvalConverger.cpp 
  Convergers/MaxGenEvalTimeConverger.cpp 
  Convergers/MaximumEvaluationConverger.cpp 
  Convergers/MaximumGenerationConverger.cpp 
  Convergers/MaximumWallClockTimeConverger.cpp 
  Convergers/MetricTrackerConvergerBase.cpp 
  Convergers/NullConverger.cpp 
  Crossers/NPointBinaryCrosser.cpp 
  Crossers/NPointCrosserBase.cpp 
  Crossers/NPointParameterizedBinaryCrosser.cpp 
  Crossers/NPointRealCrosser.cpp 
  Crossers/NullCrosser.cpp 
  Crossers/RandomDesignVariableCrosser.cpp 
  Evaluators/NullEvaluator.cpp 
  Evaluators/SimpleFunctorEvaluator.cpp 
  Evaluators/ExternalEvaluator.cpp 
  FitnessAssessors/NullFitnessAssessor.cpp 
  Initializers/DoubleMatrixInitializer.cpp 
  Initializers/FlatFileInitializer.cpp 
  Initializers/NullInitializer.cpp 
  Initializers/RandomInitializer.cpp 
  Initializers/RandomUniqueInitializer.cpp 
  MainLoops/DuplicateRemovingMainLoop.cpp 
  MainLoops/NullMainLoop.cpp 
  MainLoops/StandardMainLoop.cpp 
  Mutators/CauchyOffsetMutator.cpp 
  Mutators/GaussianOffsetMutator.cpp 
  Mutators/LocalDesignVariableMutator.cpp 
  Mutators/NullMutator.cpp 
  Mutators/OffsetMutatorBase.cpp 
  Mutators/RandomBitMutator.cpp 
  Mutators/RandomDesignVariableMutator.cpp 
  Mutators/UniformOffsetMutator.cpp 
  NichePressureApplicators/NullNichePressureApplicator.cpp 
  OperatorGroups/AllOperators.cpp 
  OperatorGroups/DuplicateFreeOperatorGroup.cpp 
  OperatorGroups/NullOperatorGroup.cpp 
  OperatorGroups/StandardOperatorGroup.cpp 
  PostProcessors/NullPostProcessor.cpp 
  Selectors/BelowLimitSelector.cpp 
  Selectors/ElitistSelector.cpp 
  Selectors/NonDuplicatingRouletteWheelSelector.cpp 
  Selectors/NullSelector.cpp 
  Selectors/RouletteWheelSelector.cpp
)

#set(jega_link_libs moga soga utilities eutils jega_fe)
#if(Threads_FOUND)
#  list(APPEND jega_link_libs ethreads)
#endif()  
#if(JEGA_HAVE_BOOST)
#  list(APPEND jega_link_libs jega_cffe)
#endif()
#target_link_libraries(jega ${jega_link_libs})

INSTALL(TARGETS jega EXPORT ${ExportTarget} DESTINATION lib)
