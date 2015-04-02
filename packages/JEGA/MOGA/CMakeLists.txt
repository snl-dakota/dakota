add_library(moga
  src/MOGA.cpp 
  src/Convergers/MOGAConverger.cpp 
  src/FitnessAssessors/LayerFitnessAssessor.cpp 
  src/FitnessAssessors/DominationCountFitnessAssessor.cpp 
  src/NichePressureApplicators/DistanceNichePressureApplicator.cpp 
  src/NichePressureApplicators/MaxDesignsNichePressureApplicator.cpp 
  src/NichePressureApplicators/RadialNichePressureApplicator.cpp 
  src/OperatorGroups/DominationCountOperatorGroup.cpp 
  src/OperatorGroups/MOGAOperatorGroup.cpp 
  src/PostProcessors/DistanceNichingPostProcessor.cpp
)
target_link_libraries(moga jega utilities)
INSTALL(TARGETS moga EXPORT ${ExportTarget} DESTINATION lib)
