cmake_minimum_required(VERSION 2.8.4)

set(CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/config
  ${CMAKE_CURRENT_SOURCE_DIR}/config/share
  ${CMAKE_CURRENT_SOURCE_DIR}/config/share/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/config/share/package_arch
  ${CMAKE_CURRENT_SOURCE_DIR}/config/share/config_tests
  ${CMAKE_CURRENT_SOURCE_DIR}/config/semsCMake
  ${CMAKE_MODULE_PATH}
)

# Check for compilers after initializing MPI
project("Dakota" NONE)

# DAKOTA versions for packaging
set(Dakota_VERSION_MAJOR 5)
set(Dakota_VERSION_MINOR 1)
# Set this to append to the version number
set(Dakota_VERSION_APPEND_STABLE "+")
#set(Dakota_VERSION_APPEND_STABLE "")

# Manage developer convenience options (experimental)
if(DevDebug)
  include(DakotaDebug)
  include(DakotaDev)
elseif (DevDistro)
  include(DakotaDistro)
  include(DakotaDev)
endif()

# Attempt to detect MPI if not specified
option(DAKOTA_HAVE_MPI "Enable MPI in DAKOTA" OFF)
if(DAKOTA_HAVE_MPI AND NOT DEFINED MPI_INCLUDE_PATH AND NOT DEFINED MPI_LIBRARY)
  include(DakotaHaveMPI)
  DakotaHaveMPI()
endif()

# Now check languages after MPI compilers are processed
enable_language(C)
enable_language(CXX)
enable_language(Fortran)

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
if(NOT DEFINED BLAS_LIBS OR NOT DEFINED LAPACK_LIBS)
  if(NOT DEFINED BLAS_LIBS)
    find_library(BLAS_LIBS blas)
  endif()
  if(NOT DEFINED LAPACK_LIBS)
    find_library(LAPACK_LIBS lapack)
  endif()
  if(NOT BLAS_LIBS OR NOT LAPACK_LIBS)
    # if not a system blas and lapack, then look for a cmake built LAPACK
    # with find_package
    find_package(LAPACK REQUIRED NO_MODULE)
    set(BLAS_LIBS blas)
    set(LAPACK_LIBS lapack)
  endif()
endif()


# Mandate a system or user-provided Boost, including some libraries
# DAKOTA requires Boost 1.37 or newer; PGI may demand 1.40 or newer
option(HAVE_ACRO "Build the ACRO package." ON)
if(HAVE_ACRO)
  # Acro requires the signals library in addition
  find_package(Boost 1.37 REQUIRED COMPONENTS regex signals filesystem system)
else()
  find_package(Boost 1.37 REQUIRED COMPONENTS regex filesystem system)
endif()

include(CTest)

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
      find_library(LIBXMU Xmu)
      include(FindThreads)
      if (Threads_FOUND AND LIBXMU)
        message("  All X_GRAPHICS dependencies have been found;
                2D graphics will be ENABLED in dakota as requested.")
      else()
        message(WARNING "HAVE_X_GRAPHICS requested, but threads not found;
                disabling.")
        set(HAVE_X_GRAPHICS OFF CACHE BOOL
            "X graphics OFF on MOST PLATFORMS" FORCE)
      endif()
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

# Manage option for f90 (default ON)
option(DAKOTA_F90 "Build the f90, bvls_wrapper." ON)
if(DAKOTA_F90)
  FortranCInterface_HEADER(dak_f90_config.h MACRO_NAMESPACE DAK_F90_)
  install(FILES ${Dakota_BINARY_DIR}/dak_f90_config.h DESTINATION include)
endif(DAKOTA_F90)

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

add_subdirectory(examples)

# Top-level install rules from source (binary rules are in subdirs)
install(FILES INSTALL LICENSE COPYRIGHT README DESTINATION .)

install(DIRECTORY examples DESTINATION .
  PATTERN "CMakeLists.txt" EXCLUDE
  PATTERN "Makefile.am" EXCLUDE
  PATTERN ".svn" EXCLUDE
  PATTERN "*.c" EXCLUDE 
  PATTERN "*.f" EXCLUDE 
  )

install(DIRECTORY test DESTINATION .
  PATTERN "CMakeLists.txt" EXCLUDE
  PATTERN "Makefile.*" EXCLUDE
  PATTERN ".svn" EXCLUDE
  PATTERN "*.c" EXCLUDE 
  PATTERN "*.C" EXCLUDE 
  PATTERN "*.f" EXCLUDE 
  )

# CPack Settings
set(CPACK_PACKAGE_VENDOR "Sandia National Laboratories")

set(local_version "${Dakota_VERSION_MAJOR}.${Dakota_VERSION_MINOR}${Dakota_VERSION_APPEND_STABLE}")
set(local_arch "${CMAKE_SYSTEM_NAME}.${CMAKE_SYSTEM_PROCESSOR}")

set(CPACK_PACKAGE_FILE_NAME "dakota-${local_version}.${local_arch}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "dakota-${local_version}.src")
set(CPACK_SOURCE_IGNORE_FILES ".svn")

if (WIN32)
  set(CPACK_GENERATOR ZIP)
  set(CPACK_SOURCE_GENERATOR ZIP)
else()
  set(CPACK_GENERATOR TGZ)
  set(CPACK_SOURCE_GENERATOR TGZ)
endif()

include(CPack)
