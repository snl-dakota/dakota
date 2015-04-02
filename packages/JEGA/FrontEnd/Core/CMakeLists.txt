add_library(jega_fe 
  src/AlgorithmConfig.cpp 
  src/ConfigHelper.cpp 
  src/Driver.cpp 
  src/ProblemConfig.cpp 
  src/SimpleFunctorEvaluatorCreator.cpp
)
target_link_libraries(jega_fe moga soga eutils)
INSTALL(TARGETS jega_fe EXPORT ${ExportTarget} DESTINATION lib)
