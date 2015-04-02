add_library(eutils
  src/EDDY_DebugScope.cpp 
  src/Math.cpp 
  src/RandomNumberGenerator.cpp
)
INSTALL(TARGETS eutils EXPORT ${ExportTarget} DESTINATION lib)
