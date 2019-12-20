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
  DataScaler.cpp
  )

set(util_headers
  DataScaler.hpp
  util_data_types.hpp
  )

add_library(dakota_util ${util_sources} ${util_headers})

# BMA TODO: CMake config at top-level per RWH example
set(EIGEN3_INCLUDE_DIRS
  "${Dakota_SOURCE_DIR}/packages/external/eigen3/include/eigen3")
# BMA TODO: CMake best practice for PUBLIC?
target_include_directories(dakota_util PUBLIC
  ${Teuchos_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIRS})
target_compile_definitions(dakota_util PUBLIC
  "HAVE_EIGEN" "EIGEN_MPL2_ONLY")
#target_link_directories()
#target_link_libraries()


install(FILES ${util_headers} DESTINATION "include")

add_subdirectory(unit)
