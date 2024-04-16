#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2023
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

if(DAKOTA_PYTHON_SURROGATES)
  list(APPEND surrogates_sources SurrogatesPython.cpp)
  list(APPEND surrogates_headers SurrogatesPython.hpp)
endif()

add_library(dakota_surrogates ${surrogates_sources} ${surrogates_headers})

target_include_directories(dakota_surrogates PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(dakota_surrogates PUBLIC dakota_util pybind11::embed)

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

install(FILES ${surrogates_headers} DESTINATION "include")
# Need to make sure DLLs end up in bin and prefer lib over lib64 (for now)
install(TARGETS dakota_surrogates EXPORT ${ExportTarget}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

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
  # On Windows, top-level Dakota must still be built static,
  # but we build the surrogates JNI only as a DLL via external project.
  # Assumes sub-project uses LAPACK_DIR and its specified DLLs
  # Assumes separate Boost include and library dir are given
  if(WIN32)
    # This variable prevents infinite recursion
    if(DAKOTA_JAVA_SURROGATES_EXTPROJ)
	  # We're in the external surrogates JNI DLL project
	  add_subdirectory(java)
	  install(FILES "$<TARGET_FILE:blas>" "$<TARGET_FILE:lapack>" DESTINATION bin)
	else()
	  # We're configuring top level and want to build the external DLL project
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
        -D BOOST_INCLUDEDIR=${Boost_INCLUDE_DIR}
	    -D BOOST_LIBRARYDIR=${Boost_LIBRARY_DIR}
        -D LAPACK_DIR=${DAKOTA_LAPACK_DLL_DIR}
	-D Python_ROOT_DIR=${Python_ROOT_DIR}
	    )
	  install(DIRECTORY ${Dakota_BINARY_DIR}/DjsDll/install/bin/ DESTINATION bin
	    FILES_MATCHING PATTERN "*.dll")
	endif()
  else()
    add_subdirectory(java)
  endif()
endif()

add_subdirectory(unit)
