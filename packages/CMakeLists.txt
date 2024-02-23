#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2022
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

# All Dakota options for packages have been moved to top-level

# For managing additional packages that packages like pecos and surfpack depend on
include(ManageRelocatablePackage)

# Surfpack requires CONMIN and NCSUOPT before descending
ManageRelocatablePackage(CONMIN external/CONMIN)
ManageRelocatablePackage(NCSUOPT external/NCSUOpt)

add_subdirectory(external/nidr)

if(HAVE_AMPL)
  if (NOT AMPL_INCLUDE_DIRS)
    add_subdirectory(external/ampl)
  endif()
endif(HAVE_AMPL)

if(HAVE_OPTPP)
  if (NOT OPTPP_INCLUDE_DIRS)
    add_subdirectory(external/OPTPP)
  endif()
endif()

# Configure other Pecos dependencies before descending
# Both DAKOTA and PECOS unconditionally depend on Teuchos, and the "shared
# component magic" is managed in the top-level CMakeLists.txt file
include(FftwExternalProject) 
if(HAVE_FFT) 
  ManageRelocatablePackage(DFFTPACK external/dfftpack) 
  FftwExternalProject() 
endif(HAVE_FFT) 
 
ManageRelocatablePackage(LHS external/LHS)
 
ManageRelocatablePackage(SPARSE_GRID external/VPISparseGrid)

add_subdirectory(pecos)

include(ExternalProject)

if(HAVE_QUESO)
  # Queso depends on GSL, so ensure GSL detection and proper lib settings
  # Require user to turn on this GPL package explicitly
  if(NOT DAKOTA_HAVE_GSL)
    message(FATAL_ERROR 
      "HAVE_QUESO requires enabling GPL package GSL via DAKOTA_HAVE_GSL.")
  endif()
  add_subdirectory(external/queso)
endif(HAVE_QUESO)


if(HAVE_ADAPTIVE_SAMPLING AND HAVE_MORSE_SMALE)
  # Extract Dionysus, but no build required
  externalproject_add(dionysus_ext
    # svn co 
    #   https://software.sandia.gov/svn/public/tpl/dionysus/tags/91c35fefb54e
    # mv dionysus-91c35fefb54e.tar.gz dionysus.tar.gz
    # Can't get download to work properly with proxy
    #URL http://hg.mrzv.org/Dionysus/archive/tip.tar.gz
    URL ${CMAKE_CURRENT_SOURCE_DIR}/dionysus.tar.gz
    URL_MD5 ""
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )
endif()

if(HAVE_APPROXNN)
  add_subdirectory(external/approxnn/src)
endif()


if(HAVE_SURFPACK)
  add_subdirectory(surfpack)
endif(HAVE_SURFPACK)

if(HAVE_ACRO)
  # Overrides of default MPI settings; may want to always set off
  # until we're using PEBBL parallel features to minimize configure
  # time errors
  if(DAKOTA_HAVE_MPI)
    # This variable has historically been set when Dakota enables MPI;
    # needs to be revisted
    set(USE_MPI "ON")
  else()
    # COLIN, PEBBL, and SCOLIB have USE_MPI
    set(USE_MPI FALSE CACHE BOOL
      "MPI NOT enabled in DAKOTA so disable in Acro (COLIN, PEBBL, SCOLIB)" 
      FORCE)
    set(UTILIB_USE_MPI FALSE CACHE BOOL
      "MPI NOT enabled in DAKOTA so disable in Acro (Utilib) as well" FORCE)
  endif()
  add_subdirectory(external/acro)
endif(HAVE_ACRO)

if(HAVE_C3)
  add_subdirectory(external/C3)
endif()

if(HAVE_DDACE)
  add_subdirectory(external/DDACE)
endif(HAVE_DDACE)

if(HAVE_DOT)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/local/DOT/dbl_prec/dot1.f)
    add_subdirectory(local/DOT ${CMAKE_CURRENT_BINARY_DIR}/DOT)
  else()
    set(HAVE_DOT OFF CACHE BOOL "Build the DOT package" FORCE)
  endif()
endif(HAVE_DOT)

if(HAVE_DREAM)
  add_subdirectory(external/dream)
endif(HAVE_DREAM)

if(HAVE_FSUDACE)
  add_subdirectory(external/FSUDace)
endif(HAVE_FSUDACE)

if(HAVE_HOPSPACK)
  # Default the GPL package to off in Dakota, but allow user override
  set(HAVE_CDDLIB OFF CACHE BOOL "Enable GPL-licensed package CDDLIB")

  # Override the HOPSPACK default to install the binary in bin/
  set(HOPSPACK_EXEC_DEST "bin")

  # HOPSPACK doesn't install headers and libraries by default
  set(INSTALL_HEADERS ON CACHE BOOL "Install headers")
  set(INSTALL_LIB ON CACHE BOOL "Install libraries")

  # Dakota is not currently using MPI features of HOPSPACK; if we do,
  # we will need to better sync up our CMake MPI probes
  #if(DAKOTA_HAVE_MPI)
  #  # See hopspack/ConfigureMPI.cmake for MPI "help"
  #  set(mpi ON CACHE BOOL "MPI requested so enable in HOPSPACK" FORCE)
  #  message("HOPSPACK caches the variable mpi = ${mpi}")
  #endif(DAKOTA_HAVE_MPI)

  # HOPSPACK expects BLAS symbol ddot to be found in LAPACK libs 
  set(LAPACK_ADD_LIBS ${BLAS_LIBS})
  set(HOPSPACK_TEST_LAPACK_FUNCS FALSE)
  add_subdirectory(external/hopspack)
endif(HAVE_HOPSPACK)

if(HAVE_JEGA)
  add_subdirectory(external/JEGA)
endif(HAVE_JEGA)

if(HAVE_NL2SOL)
  add_subdirectory(external/NL2SOL)
endif(HAVE_NL2SOL)

if(HAVE_NLPQL)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/local/NLPQL/NLPQLB.f)
    add_subdirectory(local/NLPQL ${CMAKE_CURRENT_BINARY_DIR}/NLPQL)
  else()
    set(HAVE_NLPQL OFF CACHE BOOL "Build the NLPQL package" FORCE)
  endif()
endif(HAVE_NLPQL)

if(HAVE_DEMO_TPL)
  add_subdirectory(external/demo_tpl)
endif(HAVE_DEMO_TPL)

if(HAVE_NOMAD)
  add_subdirectory(external/NOMAD)
endif(HAVE_NOMAD)

if(HAVE_NOWPAC)
  if(NOT HAVE_EXTERNAL_NOWPAC)
    set(NOWPAC_EIGEN_INCLUDE_PATH
      "${DAKOTA_EIGEN3_INCLUDE_DIR}")
#      CACHE PATH "Dakota forcing NOWPAC to use Dakota's Eigen")
    add_subdirectory(external/snowpac)
  endif()
endif()

if(HAVE_NPSOL)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/local/NPSOL/npsolsubs.f)
    add_subdirectory(local/NPSOL ${CMAKE_CURRENT_BINARY_DIR}/NPSOL)
  else()
    set(HAVE_NPSOL OFF CACHE BOOL "Build the NPSOL package" FORCE)
  endif()
endif(HAVE_NPSOL)

if(HAVE_PSUADE)
  add_subdirectory(external/PSUADE)
endif(HAVE_PSUADE)

if(HAVE_X_GRAPHICS)
  add_subdirectory(external/sciplot)
endif(HAVE_X_GRAPHICS)


if(HAVE_MUQ)
  #if(WIN32)
  #  message(FATAL_ERROR "MUQ does not currently support Windows.")
  #endif()

  if(NOT DAKOTA_HAVE_HDF5)
    message(FATAL_ERROR
            "HAVE_MUQ requires enabling HDF5 package DAKOTA_HAVE_HDF5.")
  endif()

  # Might need to "muck" with this to avoid MUQ attempting writes to /usr/local
  #message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
  #set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/muq2)
  if(NOT MUQ_EIGEN3_DIR)
    set(MUQ_EIGEN3_DIR ${DAKOTA_EIGEN3_INCLUDE_DIR})
  endif()

  if(NOT MUQ_HDF5_DIR)
    if(HDF5_ROOT)
      set(MUQ_HDF5_DIR ${HDF5_ROOT})
    else()
      set(MUQ_HDF5_DIR ${HDF5_INCLUDE_DIRS}/..)
    endif()
  endif()

  # This also disables MUQ's attempts to acquire TPLs externally
  set(SKIP_MUQ_TPL_CHECKS ON CACHE BOOL "Disable MUQ TPL compile/link checks" FORCE)

  set(MUQ_ENABLEGROUP_DEFAULT OFF CACHE BOOL "Disable MUQ default packages" FORCE)
  set(MUQ_ENABLEGROUP_SAMPLING_ALGORITHM ON CACHE BOOL "Enable MUQ sampling package" FORCE)
  add_subdirectory(external/muq2)
endif()
