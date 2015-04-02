add_library(soga 
  src/FitnessAssessors/ExteriorPenaltyFitnessAssessor.cpp 
  src/FitnessAssessors/PenaltyFitnessAssessorBase.cpp 
  src/FitnessAssessors/WeightedSumOnlyFitnessAssessor.cpp 
  src/OperatorGroups/SOGAOperatorGroup.cpp 
  src/OperatorGroups/FavorFeasibleOperatorGroup.cpp 
  src/Selectors/FavorFeasibleSelector.cpp 
  src/SOGA.cpp
)
target_link_libraries(soga jega utilities)
INSTALL(TARGETS soga EXPORT ${ExportTarget} DESTINATION lib)
