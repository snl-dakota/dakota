#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014 Sandia Corporation.
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

cmake_minimum_required(VERSION 3.2)

set( CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  ${CMAKE_MODULE_PATH}
)

# Surrogates requires C++11
include(DakotaCxxOptions)
dakota_cxx_standard()

project(Surrogates LANGUAGES CXX)

# Libraries from Surrogates itself
set(Surrogates_LIBS)
# External third-party libraries installed on the system
set(Surrogates_TPL_LIBS)

# --- Global options ---

option(BUILD_SHARED_LIBS "Build Surrogates with shared libraries?" ON)

if(CMAKE_SYSTEM_NAME MATCHES Darwin)
  set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -undefined dynamic_lookup")
endif()

# --- External TPLs ---

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

if (NOT BUILD_IN_TRILINOS)

  # First probe for system-installed Trilinos, respecting Trilinos_DIR if set
  find_package(Trilinos QUIET)

  if(Trilinos_FOUND)
    if(Trilinos_DIR)
      message(STATUS
	"Surrogates using previously specified Trilinos in ${Trilinos_DIR}")
    else()
      message(STATUS "Surrogates using external Trilinos")
    endif()
  else()

    # If no parent project configured Teuchos, do so, using Teuchos_DIR if set
    if(Teuchos_DIR)
      message(STATUS
	"Surrogates using previously specified Teuchos in ${Teuchos_DIR}")
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/packages/trilinos)

      set(Teuchos_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/packages/trilinos/packages/teuchos)
      set(Trilinos_ENABLE_Teuchos ON CACHE BOOL
        "Surrogates enabling Trilinos Teuchos" FORCE)

      # Map key Surrogates variables to TriBITS variables
      set( TPL_BLAS_LIBRARIES ${BLAS_LIBS} )
      set( TPL_LAPACK_LIBRARIES ${LAPACK_LIBS} )
      # Surrogates doesn't use any Teuchos MPI features; may want to force off
      #set( TPL_ENABLE_MPI ${DAKOTA_HAVE_MPI} )
      # This doesn't do as name implies; setting OFF doesn't generate Config.cmake
      # at all; doesn't just control whether installed!  Want Config.cmake in build
      #        set(Trilinos_ENABLE_INSTALL_CMAKE_CONFIG_FILES OFF CACHE BOOL
      #	  "Surrogates is the top-level package; don't write Trilinos config files")


      message(STATUS "Surrogates setting Teuchos_DIR to ${Teuchos_DIR}")

      # This mirrors the Trilinos release process setting, so Surrogates
      # can work with Trilinos master, when needed.  Specifically it
      # will suppress errors about missing packages.
      set(Trilinos_ENABLE_DEVELOPMENT_MODE OFF CACHE BOOL
	"Surrogates disabling Trilinos development mode")
      # This would be lighter weight, but might result in strong warnings
      ##set(Trilinos_ASSERT_MISSING_PACKAGES OFF CACHE BOOL "Dakota being lazy")

      set(Trilinos_GENERATE_REPO_VERSION_FILE OFF CACHE BOOL
        "Surrogates disabling generation of TrilinosRepoVersion.txt")

      # TODO: Propagate to top-level Dakota configure
      set(Teuchos_ENABLE_COMPLEX TRUE CACHE BOOL
	"Surrogates requires some complex blas wrappers")

	    # Enabling so we generate ROL_config.h
	    set(Trilinos_ENABLE_ROL ON CACHE BOOL "Dakota enabling Trilinos ROL")
	    # Directory containing ROLConfig.cmake at build time (not used for now)
	      ##set(ROL_DIR
        ##  ${CMAKE_CURRENT_BINARY_DIR}/packages/external/trilinos/packages/rol)
	      ##message(STATUS "Dakota setting ROL_DIR to ${ROL_DIR}")

      add_subdirectory(packages/trilinos)

    else()
      message(WARNING
	"Surrogates requires teuchos, but could not find local nor parent copy!")
    endif() # Teuchos_DIR

    # Additional setting to prevent multiple targets with the same name
    set(Trilinos_TARGETS_IMPORTED 1)

    find_package( Teuchos NO_MODULE REQUIRED )

  endif() # Trilinos_DIR

endif() # NOT BUILD_IN_TRILINOS

link_directories(${Teuchos_LIBRARY_DIRS})

# Unconditional dependencies
# (Teuchos is in TPL even if we're building it to avoid export issues
# with Trilinos and Dakota)
list(APPEND Surrogates_TPL_LIBS
  ${Teuchos_LIBRARIES} ${LAPACK_LIBS} ${BLAS_LIBS} ${FLIBS} ${FCLIBS})


#add_subdirectory(util)
add_subdirectory(models)

set(Surrogates_LINK_LIBRARIES "${Surrogates_LIBS}" CACHE INTERNAL
  "Surrogates link libraries")
