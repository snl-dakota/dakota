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
set(Dakota_VERSION_MINOR 2)
# Set this to append to the version number
set(Dakota_VERSION_APPEND_STABLE "+")
#set(Dakota_VERSION_APPEND_STABLE "")
set(Dakota_VERSION_STRING "${Dakota_VERSION_MAJOR}.${Dakota_VERSION_MINOR}${Dakota_VERSION_APPEND_STABLE}")


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
if(MSVC_VERSION EQUAL 1400 OR MSVC_VERSION GREATER 1400 OR MSVC10)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE 
    -D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
endif()

include(FortranCInterface)
include(CheckFunctionExists)
include(CheckIncludeFile)

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE )
  message(FATAL_ERROR "In-source builds are not allowed.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

add_definitions("-DHAVE_CONFIG_H")

# This no longer works as Trilinos CMake rejects custom CMAKE_BUILD_TYPE
# Add CMAKE_BUILD_TYPE for DISTRIBUTION to match historical tests / releases
#set(CMAKE_C_FLAGS_DISTRIBUTION "-O2")
#set(CMAKE_CXX_FLAGS_DISTRIBUTION "-O2")
#set(CMAKE_Fortran_FLAGS_DISTRIBUTION "-O2")

option(BUILD_SHARED_LIBS "Build shared libraries?" ON)

# Note: May need this in some Cygwin builds as well
if(CMAKE_SYSTEM_NAME MATCHES Darwin)
  set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -undefined dynamic_lookup")
endif()

# TODO: Can't this be integrated into the following logic?
if(MSVC)
  find_package(LAPACK REQUIRED NO_MODULE)
  set(BLAS_LIBS blas)
  set(LAPACK_LIBS lapack)
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


# GSL check at top-level due to need for settings in BOTH packages
# and src subdirectories
option(DAKOTA_HAVE_GSL "Toggle GSL support, default is disabled" OFF)
if(DAKOTA_HAVE_GSL)
  set(GSL_WITHOUT_CBLAS TRUE)
  find_package(GSL REQUIRED)
endif(DAKOTA_HAVE_GSL)

# Mandate a system or user-provided Boost, including some libraries
# DAKOTA requires Boost 1.37 or newer; PGI may demand 1.40 or newer
set(dakota_boost_components "regex;filesystem;system")

# Acro requires the signals library in addition
option(HAVE_ACRO "Build the ACRO package." ON)
if(HAVE_ACRO)
  list(APPEND dakota_boost_components "signals")
endif()

# Surfpack requires the serialization library in addition, if building
# the standalone executable
# Would like to have serialization be optional, unless building standalone?
option(HAVE_SURFPACK "Build the Surfpack package." ON)
option(SURFPACK_HAVE_BOOST_SERIALIZATION "Use Boost serialization library" ON)
if(HAVE_SURFPACK AND SURFPACK_HAVE_BOOST_SERIALIZATION)
  list(APPEND dakota_boost_components "serialization")
endif()

# Options for adaptive sampling that affect both packages and src
option(HAVE_ADAPTIVE_SAMPLING "Enable Morse-Smale-related sampling" ON)
option(HAVE_MORSE_SMALE 
  "Use Morse-Smale; requires ANN and Dionysus libraries" OFF)

if(WIN32)
  set(Boost_USE_STATIC_LIBS TRUE)
endif()
find_package(Boost 1.37 REQUIRED COMPONENTS "${dakota_boost_components}")


include(CTest)


# Installation destination options 
# (relative to ${CMAKE_INSTALL_PREFIX}, unless absolute)
set(DAKOTA_EXAMPLES_INSTALL . CACHE FILEPATH 
  "Installation destination for DAKOTA examples/ dir")
set(DAKOTA_TEST_INSTALL . CACHE FILEPATH 
  "Installation destination for DAKOTA test/ dir")
set(DAKOTA_TOPFILES_INSTALL . CACHE FILEPATH 
  "Installation destination for DAKOTA top-level files")


# Set the export name for install targets; parent packages likely want
# to override this to the name of their target
set(ExportTarget ${CMAKE_PROJECT_NAME}-targets CACHE STRING 
  "Name for the export target for ${CMAKE_PROJECT_NAME}")

# TODO: Have etphipp remove Trilinos-specific code and instead set in TriKota
# when building inside Trilinos, the path to Teuchos will already be set
if (NOT BUILD_IN_TRILINOS)

  # first probe for system-installed Trilinos
  # this will respect Trilinos_DIR if already set
  find_package(Trilinos QUIET)

  if ( NOT Trilinos_DIR )

    # if no one has configured a local src Teuchos, do so
    # this will respect Teuchos_DIR if already set
    if( NOT Teuchos_DIR )
      if( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/packages/teuchos/cmake/tribits )
        set( Teuchos_DIR
          ${CMAKE_CURRENT_BINARY_DIR}/packages/teuchos/packages/teuchos )
        set( Trilinos_ENABLE_Teuchos ON CACHE BOOL
          "ENABLE TriBITS build of teuchos" FORCE )

        # Map key Dakota variables to TriBITS variables
        set( TPL_ENABLE_MPI ${DAKOTA_HAVE_MPI} )
        set( TPL_BLAS_LIBRARIES ${BLAS_LIBS} )
        set( TPL_LAPACK_LIBRARIES ${LAPACK_LIBS} )

      else()
        # Use "historical" directory hierarchy, before TriBITS
        set( Teuchos_DIR ${CMAKE_CURRENT_BINARY_DIR}/packages/teuchos )
      endif() # tribits source directory EXISTS

      message( "Setting Teuchos_DIR to ${Teuchos_DIR}" )
      add_subdirectory(packages/teuchos)

    else()
      message("in ${CMAKE_CURRENT_BINARY_DIR} Teuchos_DIR is: ${Teuchos_DIR}")
    endif() # NOT Teuchos_DIR

    if( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/TrilinosConfig.cmake )
      # Move the "TriBITS" config file to teuchos subdir (not the BuildRoot)
      file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/TrilinosConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/packages/teuchos/TrilinosConfig.cmake)

      # Additional setting to prevent multiple targets with the same name
      set(Trilinos_TARGETS_IMPORTED 1)
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

# Option to turn off key DAKOTA TPL tests, default OFF
# Needs to go before adding the packages subdirectory
option(DAKOTA_ENABLE_TPL_TESTS "Enable DAKOTA TPL tests?" OFF)
if(DAKOTA_ENABLE_TPL_TESTS)
  set(DDACE_ENABLE_TESTS    ON CACHE BOOL "Enable DDACE tests.")
  set(HOPSPACK_ENABLE_TESTS ON CACHE BOOL "Enable HOPSPACK tests.")
  set(LHS_ENABLE_TESTS      ON CACHE BOOL "Enable LHS tests.")
  set(OPTPP_ENABLE_TESTS    ON CACHE BOOL "Enable OPT++ tests.")
  set(PECOS_ENABLE_TESTS    ON CACHE BOOL "Enable PECOS tests.")
else()
  set(DDACE_ENABLE_TESTS    OFF CACHE BOOL "Enable DDACE tests.")
  set(HOPSPACK_ENABLE_TESTS OFF CACHE BOOL "Enable HOPSPACK tests.")
  set(LHS_ENABLE_TESTS      OFF CACHE BOOL "Enable LHS tests.")
  set(OPTPP_ENABLE_TESTS    OFF CACHE BOOL "Enable OPT++ tests.")
  set(PECOS_ENABLE_TESTS    OFF CACHE BOOL "Enable PECOS tests.")
endif()

add_subdirectory(packages)
add_subdirectory(src)

option(DAKOTA_ENABLE_TESTS "Enable Dakota-specific tests?" ON)
# Build the executables in the test directory even if BUILD_TESTING is OFF
if(DAKOTA_ENABLE_TESTS)
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
install(FILES INSTALL LICENSE COPYRIGHT README 
  DESTINATION ${DAKOTA_TOPFILES_INSTALL})

install(DIRECTORY examples DESTINATION ${DAKOTA_EXAMPLES_INSTALL}
  PATTERN "CMakeLists.txt" EXCLUDE
  PATTERN "Makefile.am" EXCLUDE
  PATTERN ".svn" EXCLUDE
  PATTERN "*.c" EXCLUDE 
  PATTERN "*.f" EXCLUDE 
  )

install(DIRECTORY test DESTINATION ${DAKOTA_TEST_INSTALL}
  PATTERN "CMakeLists.txt" EXCLUDE
  PATTERN "Makefile.*" EXCLUDE
  PATTERN ".svn" EXCLUDE
  PATTERN "*.c" EXCLUDE 
  PATTERN "*.C" EXCLUDE 
  PATTERN "*.f" EXCLUDE 
  )


# More widely distributed packages exclude more content
# So smaller number implies less content
set(DAKOTA_InternalFull 3)       # Complete checkout
set(DAKOTA_InternalSupervised 2) # Less some of local/
set(DAKOTA_InternalWeb 1)        # Also less DOT
set(DAKOTA_ExternalWeb 0)        # Also less NPSOL, NLPQL

# The permission level of the distribution (to be made an option)
# TODO: change default to DAKOTA_ExternalWed?
set(DAKOTA_DISTRO ${DAKOTA_InternalFull})

# CPack Settings
set(CPACK_PACKAGE_VENDOR "Sandia National Laboratories")

set(local_arch "${CMAKE_SYSTEM_NAME}.${CMAKE_SYSTEM_PROCESSOR}")

# TODO: insert distribution type into package names, i.e., instead of 
# ${Dakota_VERSION_STRING} use ${Dakota_VERSION_STRING}.int_web, based on DAKOTA_DISTRO
set(CPACK_PACKAGE_FILE_NAME "dakota-${Dakota_VERSION_STRING}.${local_arch}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "dakota-${Dakota_VERSION_STRING}.src")

# Always omit .svn directories, duplicate TPLs, and GPL packages
# TODO: review whether want to include FFTW
set(CPACK_SOURCE_IGNORE_FILES ".*\\\\.svn/"
    "^${Dakota_SOURCE_DIR}/packages/pecos/packages/teuchos/"
    "^${Dakota_SOURCE_DIR}/packages/pecos/packages/fftw/"
    "^${Dakota_SOURCE_DIR}/packages/surfpack/packages/CONMIN/"
    "^${Dakota_SOURCE_DIR}/packages/surfpack/packages/NCSUOpt/"
)

if(DAKOTA_DISTRO LESS DAKOTA_InternalFull)
  list(APPEND CPACK_SOURCE_IGNORE_FILES
    "^${Dakota_SOURCE_DIR}/local/buildscripts/"
    "^${Dakota_SOURCE_DIR}/local/examples/"
    "^${Dakota_SOURCE_DIR}/local/miscscripts/"
    "^${Dakota_SOURCE_DIR}/local/sqa/"
    "^${Dakota_SOURCE_DIR}/local/www.sandia.gov/"
  )
endif()

if(DAKOTA_DISTRO LESS DAKOTA_InternalSupervised)
  # When building InternalWeb, don't want DOT in the binaries
  # Be aggressive and remove from source tree
  execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory
    ${Dakota_SOURCE_DIR}/packages/DOT/)
  # This is now redundant, leave until verified
  list(APPEND CPACK_SOURCE_IGNORE_FILES "^${Dakota_SOURCE_DIR}/packages/DOT/")
endif()

if(DAKOTA_DISTRO LESS DAKOTA_InternalWeb)
  # When building InternalWeb, don't want NLPQL, NPSOL in the binaries
  # Be aggressive and remove from source tree
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E remove_directory
      "${Dakota_SOURCE_DIR}/packages/NLPQL"
    COMMAND "${CMAKE_COMMAND}" -E remove_directory
      "${Dakota_SOURCE_DIR}/packages/NPSOL"
    )
  # This is now redundant, leave until verified
  list(APPEND CPACK_SOURCE_IGNORE_FILES
    "^${Dakota_SOURCE_DIR}/packages/NPSOL/"
    "^${Dakota_SOURCE_DIR}/packages/NLPQL/"
  )
endif()

if(NOT WIN32)

  # if(APPLE)
  #else if(NOT CYGWIN)
  if(NOT CYGWIN)
    # All unix-like systems EXCEPT cygwin (WJB - ToDo:  MacOSX)
    set(CPACK_GENERATOR TGZ)
    set(CPACK_SOURCE_GENERATOR TGZ)

  else()

    # Workaround for surprising error with Source TGZ creation on Cygwin
    #set(CPACK_GENERATOR CygwinBinary)
    set(CPACK_GENERATOR TGZ)

    #message("BROKEN!  Will NOT create a custom, Cygwin SOURCE archive")
    # Unfortunately, FAILS too -- set(CPACK_SOURCE_GENERATOR CygwinSource)
    # Need to rely on windows builds for the source distro format (zip)

  endif() # NOT CYGWIN

else()

  #message("Debugging CPACK logic -- In the Windows ZIP branch")
  set(CPACK_GENERATOR ZIP)
  set(CPACK_SOURCE_GENERATOR ZIP)

  message("Verify CPACK GENERATOR on windows: ${CPACK_GENERATOR}")

endif() # NOT WIN32

include(CPack)

