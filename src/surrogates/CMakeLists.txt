#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2020
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

include(ExternalProject)

set(surrogates_sources
  SurrogatesBase.cpp
  SurrogatesGaussianProcess.cpp 
  SurrogatesGPKernels.cpp
  SurrogatesGPObjective.cpp
  SurrogatesPolynomialRegression.cpp
  surrogates_tools.cpp
)

set(surrogates_headers
  SurrogatesBase.hpp
  SurrogatesGaussianProcess.hpp
  SurrogatesGPKernels.hpp
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
install(TARGETS dakota_surrogates EXPORT ${ExportTarget})

if(DAKOTA_PYTHON_SURROGATES)
  # NOTE: Library name is really generic due to overly simple Python
  # packaging scheme we're using
  # TODO: Need to always compile libs PIC when using Python
  # TODO: Consider -undefined dynamic_lookup link flag on Mac
  pybind11_add_module(surrogates surrogates_python.cpp)
  # TODO: transitive includes when merging from devel
  target_include_directories(surrogates PRIVATE ${Teuchos_INCLUDE_DIRS})
  target_link_libraries(surrogates PRIVATE dakota_surrogates
    ${Teuchos_LIBRARIES})
  if (APPLE)
    set_target_properties(surrogates PROPERTIES INSTALL_RPATH
      "@rpath;@rpath/../../../../../lib;@rpath/../../../../../bin")
  elseif (UNIX)
    set_target_properties(surrogates PROPERTIES INSTALL_RPATH
      "\$ORIGIN;\$ORIGIN/../../../../../lib;\$ORIGIN/../../../../../bin")
  endif()
  install(TARGETS surrogates EXPORT ${ExportTarget}
    DESTINATION share/dakota/Python/dakota/surrogates/)
  install(FILES __init__.py DESTINATION share/dakota/Python/dakota/surrogates)
endif()

if(DAKOTA_JAVA_SURROGATES)
  # TODO: Move to helper file
  # Prevent infinite recursion
  if(WIN32)
    if(DAKOTA_JAVA_SURROGATES_EXTPROJ)
	  # We're in the external project
	  add_subdirectory(java)
	  install(FILES "$<TARGET_FILE:blas>" "$<TARGET_FILE:lapack>" DESTINATION bin)
	  ##include(InstallRequiredSystemLibraries)
	else()
	  # We're configuring top level and want the external DLL project
	  ExternalProject_Add(DakotaJavaSurrogatesDLL
	    PREFIX ${Dakota_BINARY_DIR}/DjsDll
	    BINARY_DIR ${Dakota_BINARY_DIR}/DjsDll/build
	    STAMP_DIR ${Dakota_BINARY_DIR}/DjsDll/stamp
	    SOURCE_DIR "${Dakota_SOURCE_DIR}"
	    CMAKE_ARGS
        ##-D CMAKE_BUILD_TYPE:STRING=RELEASE
        ##-D CMAKE_CONFIGURATION_TYPES:STRING=Release
        -D CMAKE_INSTALL_PREFIX:PATH=${Dakota_BINARY_DIR}/DjsDll/install
	    -D DAKOTA_JAVA_SURROGATES_EXTPROJ:BOOL=TRUE
	    -D BUILD_SHARED_LIBS:BOOL=TRUE
	    -D CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS:BOOL=TRUE
	    -D DAKOTA_MODULE_DAKOTA:BOOL=FALSE
	    -D DAKOTA_JAVA_SURROGATES:BOOL=TRUE
	    -D JAVA_HOME=${JAVA_HOME}
        # Assumes separate include and library dir
	    -D BOOST_INCLUDEDIR=${Boost_INCLUDE_DIR}
	    -D BOOST_LIBRARYDIR=${Boost_LIBRARY_DIR}
        -D LAPACK_DIR=${DAKOTA_LAPACK_DLL_DIR}
	    )
	  #ExternalProject_Get_property(DakotaJavaSurrogatesDLL INSTALL_DIR)
	  #install(DIRECTORY ${INSTALL_DIR}/bin TYPE BIN
	  #install(DIRECTORY ${Dakota_BINARY_DIR}/DjsDll/install/bin TYPE BIN
	  #  FILES_MATCHING PATTERN "*.dll")
	  install(DIRECTORY ${Dakota_BINARY_DIR}/DjsDll/install/bin/ DESTINATION bin)
	endif()
  else()
    # To get this working, need care w.r.t. imported BLAS/LAPACK targets
    add_subdirectory(java)
  endif()
endif()

add_subdirectory(unit)
