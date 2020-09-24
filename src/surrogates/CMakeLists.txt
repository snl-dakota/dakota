#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________


set(surrogates_sources
  SurrogatesBase.cpp
  SurrogatesGaussianProcess.cpp 
  SurrogatesGPObjective.cpp
  SurrogatesPolynomialRegression.cpp
  surrogates_tools.cpp
)

set(surrogates_headers
  SurrogatesBase.hpp
  SurrogatesGaussianProcess.hpp
  SurrogatesGPObjective.hpp
  SurrogatesPolynomialRegression.hpp
  surrogates_tools.hpp
)

add_library(dakota_surrogates ${surrogates_sources} ${surrogates_headers})

target_include_directories(dakota_surrogates PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(dakota_surrogates PUBLIC dakota_util)

# Rationale: Teuchos is included in API headers, and ParameterList
# library component is needed
target_include_directories(dakota_surrogates PUBLIC
  "$<BUILD_INTERFACE:${Teuchos_INCLUDE_DIRS}>"
  $<INSTALL_INTERFACE:include>
  )
target_link_libraries(dakota_surrogates PUBLIC ${Teuchos_LIBRARIES})


# BMA: Unless there's a better way, specify that ROL is a system
# library to suppress warnings
target_include_directories(dakota_surrogates SYSTEM PRIVATE
  "${ROL_INCLUDE_DIRS}"
)

# Rationale: Boost serialization is referenced in API headers
target_link_libraries(dakota_surrogates PUBLIC Boost::serialization)

# BMA TODO: Consider using a utility to add Dakota targets and do this
dakota_strict_warnings(dakota_surrogates)
dakota_gcov_target(dakota_surrogates)

install(FILES ${surrogates_headers} DESTINATION "include")
install(TARGETS dakota_surrogates EXPORT ${ExportTarget} DESTINATION lib)

if(DAKOTA_PYBIND11)
  # TODO: Need to always compile libs PIC when using Python
  pybind11_add_module(dakmod surrogatepy.cpp)
  # TODO: transitive includes
  target_include_directories(dakmod PRIVATE ${Teuchos_INCLUDE_DIRS})
  target_link_libraries(dakmod PRIVATE dakota_surrogates ${Teuchos_LIBRARIES})
endif()

add_subdirectory(unit)
