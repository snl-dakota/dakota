#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2023
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________


# BMA TODO: Do we want to have each module be its own project?
#project(DakotaUtil LANGUAGES CXX)

set(util_sources
  util_common.cpp
  UtilDataScaler.cpp
  UtilLinearSolvers.cpp
  util_metrics.cpp
  util_math_tools.cpp
  )

set(util_headers
  util_common.hpp
  UtilDataScaler.hpp
  UtilLinearSolvers.hpp
  util_metrics.hpp
  util_data_types.hpp
  util_eigen_plugins.hpp
  util_math_tools.hpp
  util_windows.hpp
  )

add_library(dakota_util ${util_sources} ${util_headers})

# Rationale: append includes from our source outward to system
target_include_directories(dakota_util PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
  )

# Rationale: Teuchos is included in API headers
target_include_directories(dakota_util PUBLIC
  "$<BUILD_INTERFACE:${Teuchos_INCLUDE_DIRS}>"
  $<INSTALL_INTERFACE:include>
  )
# Rationale: Teuchos libraries aren't used directly, but something is
# indirectly using CommandLine, RCP, and TimeMonitor, so this link is
# more liberal than should be needed
target_link_libraries(dakota_util PUBLIC ${Teuchos_LIBRARIES})

# Rationale: restrict Eigen to MPL2-licensed components
# Rationale: Eigen is referenced in API headers
target_compile_definitions(dakota_util PUBLIC "EIGEN_MPL2_ONLY")
target_link_libraries(dakota_util PUBLIC Eigen3::Eigen)

# Rationale: Boost headers are implementation details only, while
# serialization is referenced in API headers
target_link_libraries(dakota_util PRIVATE Boost::boost
  PUBLIC Boost::serialization)

dakota_strict_warnings(dakota_util)

install(FILES ${util_headers} DESTINATION "include")
# Need to make sure DLLs end up in bin and prefer lib over lib64 (for now)
install(TARGETS dakota_util EXPORT ${ExportTarget}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

add_subdirectory(unit)
