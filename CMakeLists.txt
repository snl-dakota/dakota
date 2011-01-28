cmake_minimum_required(VERSION 2.8)

project("Dakota" C CXX Fortran)

include(FortranCInterface)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(FindCURL)

option(BUILD_SHARED_LIBS "Build shared libraries?" ON)

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE )
  message(FATAL_ERROR "In-source builds are not allowed.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

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

# Prefer system Boost, otherwise fall back to in-source.  Neither
# Boost_DIR nor BOOST_ROOT work as expected (as in Teuchos example);
# likely due to custom FindBoost.cmake.  Therefore try include/libs.
option(USE_SYSTEM_BOOST "Use an outside build of Boost?" ON)
if(USE_SYSTEM_BOOST)
  find_package(Boost 1.40 QUIET)
endif()

if (NOT Boost_FOUND)
  if (NOT BOOST_INCLUDEDIR AND NOT USE_SYSTEM_BOOST)
    set (BOOST_INCLUDEDIR ${CMAKE_CURRENT_SOURCE_DIR}/packages/boost)
  endif() 
  find_package( Boost 1.40 REQUIRED)
  # Adding the subdirectory will build libs, but find_package's config
  # mode not working for Boost
  #if (NOT BOOST_LIBRARYDIR) 
  #  set (BOOST_LIBRARYDIR ${CMAKE_CURRENT_BINARY_DIR}/packages/boost)
  #endif()
  #add_subdirectory(packages/boost)
  #find_package( Boost 1.40 REQUIRED COMPONENTS signals)
endif(NOT Boost_FOUND)

# check for a MPI-enabled build
if(CMAKE_CXX_COMPILER MATCHES mpicxx)

  find_program(mpicxx mpicxx)
  if(${mpicxx} MATCHES "mpicxx-NOTFOUND")
    message(FATAL_ERROR "Cannot build parallel configuration without mpicxx")
  endif()

  message(" ****      dakota build    ****    MPIHOME is: " $ENV{MPIHOME})
  set(MPI_INCLUDE_PATH "$ENV{MPIHOME}/include"
      CACHE PATH "Always use $MPIHOME to find MessagePassing header files" FORCE)
  
  find_package(MPI REQUIRED)
  if(MPI_FOUND)
    #message("MPI found - path to include is: ${MPI_INCLUDE_PATH}")
    set(HAVE_MPI "ON")
    add_definitions(-DHAVE_MPI)
    include_directories(${MPI_INCLUDE_PATH})

    # Some TPLs have manage the setting with USE_MPI rather than HAVE_MPI
    set(USE_MPI "ON")

  else()
    message(FATAL_ERROR
            "Cannot find MPI.  Please build a serial configuration instead")
  endif(MPI_FOUND)

endif()


include(CTest)

set( CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/config/share
  ${CMAKE_CURRENT_SOURCE_DIR}/config/share/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/config/share/package_arch
  ${CMAKE_CURRENT_SOURCE_DIR}/config/share/config_tests
  ${CMAKE_MODULE_PATH}
)

# first probe for system-installed Trilinos
# this will respect Trilinos_DIR if already set
find_package(Trilinos QUIET)
if ( NOT Trilinos_DIR )

  # if no one has configured a local src Teuchos, do so
  # this will respect Teuchos_DIR if already set
  if( NOT Teuchos_DIR )
    set( Teuchos_DIR ${CMAKE_CURRENT_BINARY_DIR}/packages/teuchos )
    message( "Setting Teuchos_DIR to ${Teuchos_DIR}" )
    add_subdirectory(packages/teuchos)
  else()
    message( "in ${CMAKE_CURRENT_BINARY_DIR} Teuchos_DIR already set to ${Teuchos_DIR}" )
  endif()
  find_package( Teuchos NO_MODULE REQUIRED )
else()
  message( "Using system trilinos in ${Trilinos_DIR}" )
endif()


add_subdirectory(packages)
add_subdirectory(src)
add_subdirectory(test)

# Package-specific configuration options
include_directories(${Dakota_BINARY_DIR})

configure_file("${Dakota_SOURCE_DIR}/config/dakota_config.h.cmake.in"
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

