#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014 Sandia Corporation.
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________


# BMA TODO: Do we want to have each module be its own project?
#project(DakotaUtil LANGUAGES CXX)

set(util_sources
  CommonUtils.cpp
  DataScaler.cpp
  )

set(util_headers
  CommonUtils.hpp
  DataScaler.hpp
  )

add_library(dakota_util ${util_sources} ${util_headers})

# BMA TODO: CMake config at top-level per RWH example
#set(EIGEN3_INCLUDE_DIRS
#  "${Dakota_SOURCE_DIR}/packages/external/eigen3/include/eigen3")

SET( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../packages/external/eigen3/share/eigen3/cmake)
find_package(Eigen3 3.3 REQUIRED CONFIG)
if(TARGET Eigen3::Eigen)
  message(STATUS "Eigen3 v${EIGEN3_VERSION_STRING} found in
  ${EIGEN3_INCLUDE_DIR}")
endif()

target_include_directories(dakota_util PRIVATE
  ${Teuchos_INCLUDE_DIRS})

target_include_directories(dakota_util INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(dakota_util PUBLIC
  "EIGEN_MPL2_ONLY")

dakota_strict_warnings(dakota_util)
dakota_gcov_target(dakota_util)

target_link_libraries(dakota_util PUBLIC
  Eigen3::Eigen)

install(FILES ${util_headers} DESTINATION "include")
install(TARGETS dakota_util EXPORT ${ExportTarget} DESTINATION lib)

add_subdirectory(unit)
