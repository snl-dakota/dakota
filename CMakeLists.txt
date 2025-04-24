# _______________________________________________________________________
#
#    Dakota: Explore and predict with confidence.
#    Copyright 2014-2025
#    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#    This software is distributed under the GNU Lesser General Public License.
#    For more information, see the README file in the top Dakota directory.
# _______________________________________________________________________
#

# Dakota itself requires this CMake version; some TPLs may require even newer
cmake_minimum_required(VERSION 3.23.0)

set(CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/semsCMake
  ${CMAKE_MODULE_PATH}
  )

# We delay enabling languages until developer compiler flags are set,
# even though we no longer force the compiler to the MPI wrapper
project("Dakota" NONE)

message(STATUS "Configuring Dakota with CMake ${CMAKE_VERSION}")

# Avoid downstream errors from packages that DNE; likely missing submodules
if(NOT IS_DIRECTORY "${Dakota_SOURCE_DIR}/packages/pecos")
  message(FATAL_ERROR "Required source sub-directory "
    "${Dakota_SOURCE_DIR}/packages/pecos does not exist; "
    "if using a Git repository, are the submodules checked out?"
    )
elseif(NOT IS_DIRECTORY "${Dakota_SOURCE_DIR}/packages/external/nidr")
  message(FATAL_ERROR "Required source sub-directory "
    "${Dakota_SOURCE_DIR}/packages/external/nidr does not exist; "
    "if using a Git repository, are the submodules checked out?"
    )
endif()

# Dakota requires C++11
include(DakotaCxxOptions)
dakota_cxx_standard()

# Init code coverage if requested 
include(DakotaGcov)
dakota_gcov_init()

# Developer convenience options first so take precedence over default options
if(DevDebugShared OR DevDebugStatic OR DevDebug)  # DevDebug is deprecated
  include(DakotaDebug)
  include(DakotaDev)
elseif (DevDistro)
  include(DakotaDistro)
  include(DakotaDev)
endif()

# Publish all top-level and Dakota package options
include(DakotaOptions)

# -----
# Dakota version and optional tags
# -----
include(DakotaVersion)
# Define convenience variables used here and in subdirs, such as
#   Dakota_VERSION_{PAIR, TRIPLE, STRING, SRC}
include(DakotaVersionUtils)
dakota_version_set_helper_vars()


# Installation destination options
# (relative to ${CMAKE_INSTALL_PREFIX}, unless absolute)
set(DAKOTA_EXAMPLES_INSTALL "share/dakota" CACHE STRING
  "Installation destination for DAKOTA examples/ dir")
set(DAKOTA_TEST_INSTALL "share/dakota" CACHE STRING
  "Installation destination for DAKOTA test/ dir")
set(DAKOTA_TOPFILES_INSTALL "share/dakota" CACHE STRING
  "Installation destination for DAKOTA top-level files")


# Location for generated files (setup early as needed below)
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory
  "${Dakota_BINARY_DIR}/generated")
# TODO: Consistent generated file locations and directory creation.  For now:
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory
  "${Dakota_BINARY_DIR}/generated/src")
execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory
  "${Dakota_BINARY_DIR}/generated/VERSION")

# Conditionally include Git revision in version info and create VERSION file.
# Sets several convenience variables
# NOTE: depends on presence of generated source directories
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/VERSION)
  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/DakotaBuildInfo.cpp)
    message(FATAL_ERROR "VERSION file exists but no corresponding
                         src/DakotaBuildInfo.cpp exists.")
  endif()
  set(DAKOTA_VERSION_FOUND TRUE)
else()
  dakota_version_from_git()
  dakota_create_version_file()
endif()


include(DakotaCPack)

# The permission level of the distribution (to be made an option)
# TODO: change default to DAKOTA_ExternalWeb?
# BMA TODO: Publish this option in options directory
if(NOT DEFINED DAKOTA_DISTRO)
  set(DAKOTA_DISTRO "snlfull" CACHE STRING "Dakota Distribution Type")
endif()

# Initial CPack settings and source exclusions
# Depends on DAKOTA_DISTRO for file filtering
dakota_cpack_initialize()

# Prune local and non-distributable TPLs based on DAKOTA_DISTRO
dakota_cpack_prune_distro()

# Default RPATH settings
#
# These settings must take into consideration the presence of shared
# libraries in both bin/ and /lib, and applications in bin/ (formerly
# test/, which is now managed in share/dakota/test; see override in
# test/CMakeLists.txt).
#
#
# $ORIGIN (@executable_path on Mac) is replaced at runtime by the path
# of the executable (application or .so).
#
# Add:
#   $ORIGIN so executables can find shared libs in their own folder
#   $ORIGIN/../lib so that executables in bin and test can find shared libs in the lib folder
#   $ORIGIN/../bin so that exectuables in lib and test can find shared libs in the bin folder.
if(APPLE)
  set(CMAKE_MACOSX_RPATH TRUE CACHE BOOL "Add @rpath to library install names")
  set(CMAKE_INSTALL_RPATH "@executable_path;@executable_path/../lib;@executable_path/../bin"
    CACHE STRING "Set the RPATH in Dakota executables and libraries")
elseif(UNIX)
   set(CMAKE_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib:\$ORIGIN/../lib64:\$ORIGIN/../bin"
     CACHE STRING "Set the RPATH in Dakota executables and libraries")
endif()

# Copy library installation script for Darwin so that it can be found by CPack
if(APPLE)
   file(COPY cmake/install_macos_libs.py DESTINATION ${CMAKE_BINARY_DIR})
endif() 

# Now check languages after compiler flags and options are set
enable_language(C)
enable_language(CXX)
enable_language(Fortran)

# wait to call this until after enabling compilers because the compiler vendor and
# version must be known
dakota_platform_specific_compile_options()

# Ensure that GCC is being used if DAKOTA_GCOV was requested. 
if(DAKOTA_GCOV AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   message(FATAL_ERROR "DAKOTA_GCOV requires building with GCC")
endif()


if(MSVC_VERSION EQUAL 1400 OR MSVC_VERSION GREATER 1400 OR MSVC10)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE 
    -D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
endif()

# Perl is required for examples, docs, and system tests
find_package(Perl REQUIRED)

# Python is optionally required by Dakota, Teuchos, and Acro; probe
# for components here at top-level:
include(DakotaFindPython)
dakota_find_python()

# Conditionally find Java JDK needed for input spec, docs, Java API
include(DakotaFindJava)
dakota_find_java()

# Locate MPI compiler and library settings (must be after languages enabled)
# The DakotaFindMPI() function checks whether the option is enabled
include(DakotaMPI)
DakotaFindMPI()


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

# BMA TODO: Don't want this to affect new surrogates package
add_definitions("-DHAVE_CONFIG_H")

# Note: May need this in some Cygwin builds as well
if(CMAKE_SYSTEM_NAME MATCHES Darwin)
  set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -undefined dynamic_lookup")
endif()

# set compiler flag to prevent issues in boost::serialization for c++17.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_STANDARD GREATER 11)
    set(CMAKE_CXX_FLAGS "-fno-new-ttp-matching ${CMAKE_CXX_FLAGS}"
        CACHE STRING "fix boost::serialization" FORCE)
  endif()
endif()

include(DakotaFindSystemTPLs)
# Unconditionally find BLAS/LAPACK or equivalent
dakota_find_linalg()
# Unconditionally find Eigen3
dakota_find_eigen3()
# Conditionally find GSL; needed by both packages/ (QUESO) and src/
dakota_find_gsl()
# Conditionally find HDF5; needed by both src/ and test/
dakota_find_hdf5()
# Unconditionally find Boost
dakota_find_boost()


include(CTest)


# Set the export name for install targets; parent packages likely want
# to override this to the name of their target
set(ExportTarget DakotaTargets CACHE STRING 
  "Export set name for ${CMAKE_PROJECT_NAME}")

# Unconditionally find Trilinos/Teuchos, optionally ROL
include(DakotaFindTrilinos)
dakota_find_trilinos()

# Conditionally find X-Windows graphics deps
include(DakotaXGraphics)
dakota_find_x_graphics()

if(DAKOTA_F90)
  FortranCInterface_HEADER(dak_f90_config.h MACRO_NAMESPACE DAK_F90_)
  install(FILES ${Dakota_BINARY_DIR}/dak_f90_config.h DESTINATION include)
endif(DAKOTA_F90)

if(HAVE_ROL)
  add_definitions("-DHAVE_ROL")
endif(HAVE_ROL)

# Set up googletest. It must be done early so that
# the targets can be used to add unit tests
dakota_find_googletest()

# -----
# Configuration of Dakota components
# -----

if(DAKOTA_MODULE_UTIL)
  add_subdirectory(src/util)
endif()

# --- New surrogates module
# BMA TODO: Finer granularity in packages to build Trilinos, Eigen
# first, then optionally Pecos, Surfpack, surrogates.

if(DAKOTA_MODULE_SURROGATES) 
  add_subdirectory(src/surrogates)
  add_definitions("-DHAVE_DAKOTA_SURROGATES")
endif()

if(DAKOTA_MODULE_DAKOTA)

  add_subdirectory(packages)

  add_subdirectory(src)
  # Build the executables in the test directory even if BUILD_TESTING is OFF
  if(DAKOTA_ENABLE_TESTS)
    add_subdirectory(test)
  endif()

  if(ENABLE_DAKOTA_DOCS)
    add_subdirectory(docs)
  endif()

  # Legacy examples
  add_subdirectory(examples)
  # New examples library, taking care to guard against empty directory
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dakota-examples/CMakeLists.txt")
    add_subdirectory(dakota-examples)
  endif()

  add_subdirectory(interfaces)

  # Install helper scripts to bin/:
  # NOTE: This directory manages dynamic library manipulation at install time,
  # as the install script must be called after src/ and test/ are installed.
  add_subdirectory("scripts")

endif()  # DAKOTA_MODULE_DAKOTA


# -----
# Install / Package
# -----

# Top-level install rules from source (binary rules are in subdirs)
install(FILES INSTALL LICENSE COPYRIGHT README
  DESTINATION ${DAKOTA_TOPFILES_INSTALL}
  )

install(DIRECTORY examples DESTINATION ${DAKOTA_EXAMPLES_INSTALL}
  USE_SOURCE_PERMISSIONS
  PATTERN "CMakeLists.txt" EXCLUDE
  PATTERN "*.c" EXCLUDE 
  PATTERN "*.f" EXCLUDE 
  )

install(DIRECTORY test DESTINATION ${DAKOTA_TEST_INSTALL}
  PATTERN "CMakeLists.txt" EXCLUDE
  PATTERN "*.c" EXCLUDE 
  PATTERN "*.cpp" EXCLUDE 
  PATTERN "*.f" EXCLUDE 
  )

if(DAKOTA_VERSION_FOUND)
   install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/VERSION DESTINATION
    ${DAKOTA_TOPFILES_INSTALL})
endif()
 
# Final CPack settings
dakota_cpack_finalize()
include(CPack)
