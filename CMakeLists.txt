cmake_minimum_required(VERSION 2.8)

project("Dakota" C CXX Fortran)

include(FortranCInterface)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(FindCURL)

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE )
  message(FATAL_ERROR "In-source builds are not allowed.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

add_definitions("-DHAVE_CONFIG_H")

# Add CMAKE_BUILD_TYPE for DISTRIBUTION to match historical tests / releases
set(CMAKE_C_FLAGS_DISTRIBUTION "-O2")
set(CMAKE_CXX_FLAGS_DISTRIBUTION "-O2")
set(CMAKE_Fortran_FLAGS_DISTRIBUTION "-O2")

option(BUILD_SHARED_LIBS "Build shared libraries?" ON)

if(CMAKE_SYSTEM_NAME MATCHES Darwin)
  set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -undefined dynamic_lookup")
endif()

# first check for a system blas and lapack
find_library(BLAS_LIBS blas)
find_library(LAPACK_LIBS lapack)
if(NOT BLAS_LIBS OR NOT LAPACK_LIBS)
  # if not a system blas and lapack, then look for a cmake built LAPACK
  # with find_package
  find_package(LAPACK REQUIRED NO_MODULE)
  set(BLAS_LIBS blas)
  set(LAPACK_LIBS lapack)
endif()


# Mandate a system or user-provided Boost
if(HAVE_ACRO)
  # Acro requires Boost 1.40 or newer and the signals library
  find_package(Boost 1.40 REQUIRED COMPONENTS signals)  
else()
  # DAKOTA requires Boost 1.37 or newer;
  # there are additional probes in src/
  find_package(Boost 1.37 REQUIRED)
endif()

# check for a MPI-enabled build
if(CMAKE_CXX_COMPILER MATCHES mpicxx)

  find_program(mpicxx mpicxx)
  if(${mpicxx} MATCHES "mpicxx-NOTFOUND")
    message(FATAL_ERROR "Cannot build parallel configuration without mpicxx")
  endif()

  if ("$ENV{MPIHOME}")
    message(" ****      dakota build    ****    MPIHOME is: " $ENV{MPIHOME})
    set(MPI_INCLUDE_PATH "$ENV{MPIHOME}/include"
    	CACHE PATH "Always use $MPIHOME to find MessagePassing header files"
	FORCE)
  endif()
  
  find_package(MPI REQUIRED)
  if(MPI_FOUND)
    set(DAKOTA_HAVE_MPI "ON")

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


# Set the export name for install targets; parent packages likely want
# to override this to the name of their target
set(ExportTarget ${CMAKE_PROJECT_NAME}-targets CACHE STRING 
  "Name for the export target for ${CMAKE_PROJECT_NAME}")

# TODO: Have etphipp remove Trilinos-specific code and instead set in TriKota
if (BUILD_IN_TRILINOS)
  set(ExportTarget Trilinos)
endif()

# when building inside Trilinos, the path to Teuchos will already be set
if (NOT BUILD_IN_TRILINOS)
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
endif()


# Manage option for graphics (default ON if possible)
option(HAVE_X_GRAPHICS "Build the DAKOTA/Motif Graphics package." ON)
if (HAVE_X_GRAPHICS)
  find_package(X11)
  if (X11_FOUND)
    find_package(Motif)
    if (MOTIF_FOUND)
      include(FindThreads)
      if (Threads_FOUND)
        message("  All X_GRAPHICS dependencies have been found;
                2D graphics will be ENABLED in dakota as requested.")
      else()
        message(WARNING "HAVE_X_GRAPHICS requested, but threads not found;
                disabling.")
        set(HAVE_X_GRAPHICS OFF CACHE BOOL
            "X graphics OFF on MOST PLATFORMS" FORCE)
      endif(Threads_FOUND)
    else()
      message(WARNING "HAVE_X_GRAPHICS requested, but Motif not found;
              disabling.")
      set(HAVE_X_GRAPHICS OFF CACHE BOOL
          "X graphics OFF on MOST PLATFORMS" FORCE)
    endif(MOTIF_FOUND)
  else()
    message(WARNING "HAVE_X_GRAPHICS requested, but X11 not found; disabling.")
    set(HAVE_X_GRAPHICS OFF CACHE BOOL 
        "X graphics OFF on MOST PLATFORMS" FORCE)
  endif(X11_FOUND)
endif(HAVE_X_GRAPHICS)

add_subdirectory(packages)
add_subdirectory(src)

option(ENABLE_DAKOTA_TESTS "Enable Dakota tests." ON)
if(ENABLE_DAKOTA_TESTS)
  add_subdirectory(test)
endif()

option(ENABLE_SPEC_MAINT "Enable DAKOTA specification maintenance mode."
  OFF)
if(ENABLE_SPEC_MAINT AND NOT UNIX)
  message(FATAL_ERROR "DAKOTA specification maintenance mode only available on UNIX platforms")
endif()


option(ENABLE_DAKOTA_DOCS "Enable DAKOTA documentation build." OFF)
if(ENABLE_DAKOTA_DOCS)
  if(UNIX)
    add_subdirectory(docs)
  else()
    message(FATAL_ERROR "DAKOTA documentation build only available on UNIX platforms")
  endif()
endif()

