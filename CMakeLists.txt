cmake_minimum_required(VERSION 2.8)

project("Dakota" C CXX Fortran)

include(FortranCInterface)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(FindCURL)

option(BUILD_STATIC "build static libraries" OFF)

# first check for a system blas and lapack
find_library(BLAS_LIBS blas)
find_library(LAPACK_LIBS lapack)
if(NOT BLAS_LIBS OR NOT LAPACK_LIBS)
  # if not a system blas and lapack, then
  # look for a cmake built LAPACK with
  # find_package
  find_package(LAPACK REQUIRED NO_MODULE)
  set(BLAS_LIBS blas)
  set(LAPACK_LIBS lapack)
endif()

# hack to be fixed when CMake does it automatically
if(${CMAKE_Fortran_COMPILER_ID} MATCHES "Intel")
  set(FIXED_FLAG "-fixed")
elseif(${CMAKE_Fortran_COMPILER_ID} MATCHES "GNU")
  set(FIXED_FLAG "-ffixed-form")
elseif(${CMAKE_Fortran_COMPILER_ID} MATCHES "PGI")
  set(FIXED_FLAG "-Mfixed")
endif()

if(CMAKE_SYSTEM_NAME MATCHES Darwin)
  set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -undefined dynamic_lookup")
endif()

find_package(Boost 1.40 REQUIRED)

include(CTest)

set( CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/share
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/share/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/share/package_arch
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/share/config_tests
  ${CMAKE_MODULE_PATH}
)

set(BOOST_DISABLE_ASSERTS 1)
# WJB: Request a probe from Kitware to manage the OS/fork setting
set(HAVE_WORKING_FORK 1)


option(HAVE_PECOS "Build the Pecos package." ON)
option(HAVE_AMPL "Build the AMPL package." ON)
option(HAVE_DDACE "Build the DDACE package." ON)
option(HAVE_DOT "Build the DOT package." ON)
option(HAVE_CONMIN "Build the CONMIN package." ON)
option(HAVE_FSUDACE "Build the FSUDACE package." ON)
option(HAVE_JEGA "Build the JEGA package." ON)
option(HAVE_NCSUOPT "Build the NCSUOPT package." ON)
option(HAVE_NL2SOL "Build the NL2SOL package." ON)
option(HAVE_NPSOL "Build the NPSOL package." ON)
option(HAVE_NLPQL "Build the NLPQL package." ON)
option(HAVE_OPTPP "Build the OPTPP package." ON)
option(HAVE_PSUADE "Build the PSUADE package." ON)
option(HAVE_SURFPACK "Build the Surfpack package." OFF)
option(HAVE_TRACKING "Build the DAKOTA usage tracker module." ON)
option(HAVE_XDR "Build the XDR interface module." ON)

option(USE_SYSTEM_TEUCHOS
  "If false, DAKOTA will use the included version of Teuchos."
  OFF)

if(USE_SYSTEM_TEUCHOS)
  # WJB: suspect SYSTEM_TEUCHOS settings specify incorrect paths -- try to get
  #      a review from someone interested in this case being enabled
  find_package(Trilinos REQUIRED)
  ## Kitware version -- include(${Trilinos_DIR}/TrilinosConfig.cmake)
  include(${Trilinos_DIR}/TeuchosConfig.cmake)
  ##set(Teuchos_INCLUDE_DIRS ${Trilinos_INCLUDE_DIRS} CACHE FILEPATH "include dir for Teuchos" FORCE)
  ##set(Teuchos_LIBRARY_DIRS ${Trilinos_LIBRARY_DIRS} CACHE FILEPATH "library dir for Teuchos" FORCE)
else()
  set(Teuchos_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/packages/teuchos/src
                           ${CMAKE_CURRENT_BINARY_DIR}/packages/teuchos/src)
  set(Teuchos_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/packages/teuchos/src)
endif()


#Keep track of what libraries should be included in libdakota
## WJB - ToDo: determine whether this is needed at the top-level
set(SUBLIBS dakota_src)

add_subdirectory(packages)
add_subdirectory(methods)
add_subdirectory(src)
#add_subdirectory(test)

# Package-specific configuration options
include_directories(${Dakota_BINARY_DIR})

configure_file("${Dakota_SOURCE_DIR}/cmake/dakota_config.h.in"
  "${Dakota_BINARY_DIR}/dakota_config.h"
  @ONLY
)

add_definitions("-DHAVE_CONFIG_H")


file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)
set(docs_files
  Ref_Interface.dox
  Ref_Method.dox
  Ref_Model.dox
  Ref_Responses.dox
  Ref_Strategy.dox
  Ref_Variables.dox
)
foreach(doc ${docs_files})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/${doc}
    ${CMAKE_CURRENT_BINARY_DIR}/docs/${doc}
    COPYONLY)
endforeach()

#generate dakota.input.desc
find_package(Perl REQUIRED)
ADD_CUSTOM_COMMAND(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/dakota.input.desc"
  COMMAND ${PERL_EXECUTABLE} generate_desc.pl
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

#clean up binary dir
#file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/docs)


# WJB - ToDo: delete here to bottom once I am done debugging the build settings
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/AllVariables.txt "")
GET_CMAKE_PROPERTY(res VARIABLES)
FOREACH(var ${res})
  FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/AllVariables.txt
             "${var} \"${${var}}\"\n")
ENDFOREACH(var ${res})

