#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014 Sandia Corporation.
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________


set(surrogates_sources
  GaussianProcess.cpp 
  GP_Objective.cpp
  )

set(surrogates_headers
  GaussianProcess.hpp GP_Objective.hpp
)

add_library(dakota_surrogates ${surrogates_sources} ${surrogates_headers})

# BMA TODO: CMake config at top-level per RWH example
#set(EIGEN3_INCLUDE_DIRS
#  "${Dakota_SOURCE_DIR}/packages/external/eigen3/include/eigen3")

# BMA TODO: CMake best practice for PUBLIC?
target_include_directories(dakota_surrogates INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  PRIVATE
  "${ROL_INCLUDE_DIRS}"
  )

target_link_libraries(dakota_surrogates PUBLIC dakota_util)

install(FILES ${surrogates_headers} DESTINATION "include")

add_subdirectory(unit)
