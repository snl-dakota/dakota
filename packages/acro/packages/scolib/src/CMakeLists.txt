include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(scolib
  libs/Direct.cpp
  libs/DockingPS.cpp
  libs/PatternSearch.cpp
  libs/positive_basis.cpp
  libs/SolisWets.cpp
  libs/DynamicGSS.cpp
  libs/MultiStatePS.cpp
  libs/MultiStart.cpp
  libs/EAminlp.cpp
  libs/SCORegistrations.cpp
  libs/PIDOMS.cpp
  libs/PLGO.cpp
  libs/TGPO.cpp
  libs/StateMachineLS.cpp
)
target_link_libraries(scolib
  ${Boost_LIBRARIES}
  utilib
  pebbl
)

if(HAVE_AMPL)
target_link_libraries(scolib
  amplsolver
)
endif()

if(HAVE_AMPL)
  include_directories(${AMPL_INCLUDE_DIRS})
  add_executable(scolib_driver exec/scolib.cpp exec/funcadd.c)
else()
  add_executable(scolib_driver exec/scolib.cpp)
endif()
if(UNIX)
  target_link_libraries(scolib_driver dl)
endif()


target_link_libraries(scolib_driver
  colin
  scolib
  utilib
  pebbl
  ${Boost_LIBRARIES}
  tinyxml
)

add_executable(scolib_docking
  exec/docking.cpp
)

add_executable(scolib_simple
  exec/simple.cpp
)
target_link_libraries(scolib_simple
  colin
  scolib
)

install(TARGETS scolib EXPORT ${ExportTarget} DESTINATION lib)
# These testers will be built, but not installed:
#install(TARGETS scolib_driver EXPORT ${ExportTarget} DESTINATION bin)
#install(TARGETS scolib_docking EXPORT ${ExportTarget} DESTINATION bin)
#install(TARGETS scolib_simple EXPORT ${ExportTarget} DESTINATION bin)

install(DIRECTORY scolib DESTINATION include PATTERN .svn EXCLUDE)
