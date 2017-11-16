# CMakeLists.txt for DAKOTA packages
option(HAVE_AMPL "Build the AMPL package." ON)
option(HAVE_CONMIN "Build the CONMIN package." ON)
option(HAVE_DDACE "Build the DDACE package." ON)
option(HAVE_DOT "Build the DOT package." ON)
option(HAVE_DREAM "Build the DREAM package." ON)
option(HAVE_FSUDACE "Build the FSUDACE package." ON)
option(HAVE_HOPSPACK "Build the hopspack package." ON)
option(HAVE_JEGA "Build the JEGA package." ON)
option(HAVE_NCSUOPT "Build the NCSUOPT package." ON)
option(HAVE_NL2SOL "Build the NL2SOL package." ON)
option(HAVE_NLPQL "Build the NLPQL package." ON)
option(HAVE_NOMAD "Build the NOMAD package." ON)
option(HAVE_NOWPAC "Build the NOWPAC package." OFF)
option(HAVE_NPSOL "Build the NPSOL package." ON)
option(HAVE_OPTPP "Build the OPTPP package." ON)
option(HAVE_PECOS "Build the Pecos package." ON)
option(HAVE_PLUGIN "Enable plugins to the dakota executable." ON)
option(HAVE_PSUADE "Build the PSUADE package." ON)
option(HAVE_SURFPACK "Build the Surfpack package." ON)
# Packages formerly underneath Pecos
option(HAVE_FFT "Use either the fftw or dfftpack packages" ON)
option(HAVE_DFFTPACK "Build the dfftpack package.  This OPTION has no effect if
  HAVE_FFT is OFF" ON)
option(HAVE_FFTW "Find and use an installed fftw package. This OPTION has no
  effect if HAVE_FFT is OFF" OFF)

option(HAVE_LHS "Build the LHS package." ON)
option(HAVE_SPARSE_GRID "Build the VPISparseGrid package." ON)

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

#add_subdirectory(nowpac)

include(ExternalProject)

if(HAVE_QUESO)

  # Queso depends on GSL, so ensure GSL detection and proper lib settings
  # Require user to turn on this GPL package explicitly
  if(NOT DAKOTA_HAVE_GSL)
    message(FATAL_ERROR 
      "HAVE_QUESO requires enabling GPL package GSL via DAKOTA_HAVE_GSL.")
  endif()

  if(DAKOTA_QUESO_CMAKE)
    add_subdirectory(external/queso)
  else()

  # Prerequisites for building QUESO
  find_program(SH sh)
  if(${SH} MATCHES "sh-NOTFOUND")
    message(FATAL_ERROR "Cannot build QUESO as an external project without sh")
  endif()

  # Use gsl-config to get GSL settings
  if(DEFINED GSL_CONFIG_EXECUTABLE)
    set(GSL_CONFIG "${GSL_CONFIG_EXECUTABLE}")
  else()
    find_program(GSL_CONFIG gsl-config)
    if(${GSL_CONFIG} MATCHES "gsl-config-NOTFOUND")
      if(DEFINED ENV{GSL_CONFIG})
	set(GSL_CONFIG "$ENV{GSL_CONFIG}")
      else()
	message(FATAL_ERROR "QUESO requires gsl-config on PATH or pointed to by"
	  " environment variable GSL_CONFIG.")
      endif()
    endif()
  endif()

  exec_program("${GSL_CONFIG}" ARGS --prefix OUTPUT_VARIABLE Q_GSL_PREFIX)
  message(STATUS "QUESO support -- GSL prefix: ${Q_GSL_PREFIX}")
  # Set GSL_PREFIX for QUESO m4
  set($ENV{GSL_PREFIX} ${Q_GSL_PREFIX})
  # GSL_LIBRARY_DIRS is set by find_package(GSL) at top-level
  message(STATUS "QUESO support -- GSL libdir: ${GSL_LIBRARY_DIRS}")

  # This may not be necessary for system-installed Boost, so warn only
  if(NOT DEFINED ENV{BOOST_ROOT})
    message(WARNING "QUESO typically requires environment variable BOOST_ROOT,"
      " but not set.")
  endif()

  # Configure, build, and install QUESO in packages/queso_ext.  Build
  # is in queso_ext/src; Install is to queso_ext/{bin,examples,include,lib}
  set(queso_config_cmd
    ${SH} ${CMAKE_CURRENT_SOURCE_DIR}/external/queso/configure
    "CC=${CMAKE_C_COMPILER}" "CXX=${CMAKE_CXX_COMPILER}"
    "F77=${CMAKE_Fortran_COMPILER}" "FC=${CMAKE_Fortran_COMPILER}"
    "--without-hdf5" "--without-glpk"
    "--with-boost=${Boost_INCLUDE_DIR}/.."
#    "--disable-shared"
    "LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH}:${Boost_LIBRARY_DIRS}:${GSL_LIBRARY_DIRS}"
    "CXXFLAGS=${CMAKE_CXX_FLAGS}"
    "--prefix=${CMAKE_CURRENT_BINARY_DIR}/queso_ext"
    )

  externalproject_add(queso_ext
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/queso_ext"
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/queso"
    CONFIGURE_COMMAND "${queso_config_cmd}"
    )

  #add_library(queso UNKNOWN IMPORTED GLOBAL)
  #set_target_properties(queso PROPERTIES 
  #  IMPORTED_LOCATION ${Dakota_BINARY_DIR}/packages/queso_ext/lib)

  # Install QUESO (and possibly GRVY) libs
  install(SCRIPT "${Dakota_SOURCE_DIR}/cmake/InstallQuesoLibs.cmake")

  endif()  # DAKOTA_QUESO_CMAKE

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

if(HAVE_DDACE)
  add_subdirectory(external/DDACE)
endif(HAVE_DDACE)

if(HAVE_DOT)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../local/packages/DOT/dbl_prec/dot1.f)
    add_subdirectory(../local/packages/DOT ${CMAKE_CURRENT_BINARY_DIR}/DOT)
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
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../local/packages/NLPQL/NLPQLB.f)
    add_subdirectory(../local/packages/NLPQL ${CMAKE_CURRENT_BINARY_DIR}/NLPQL)
  else()
    set(HAVE_NLPQL OFF CACHE BOOL "Build the NLPQL package" FORCE)
  endif()
endif(HAVE_NLPQL)

if(HAVE_NOMAD)
  add_subdirectory(external/NOMAD)
endif(HAVE_NOMAD)

if(HAVE_NPSOL)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../local/packages/NPSOL/npsolsubs.f)
    add_subdirectory(../local/packages/NPSOL ${CMAKE_CURRENT_BINARY_DIR}/NPSOL)
  else()
    set(HAVE_NPSOL OFF CACHE BOOL "Build the NPSOL package" FORCE)
  endif()
endif(HAVE_NPSOL)

if(HAVE_PSUADE)
  add_subdirectory(external/PSUADE)
endif(HAVE_PSUADE)

#if(DAKOTA_RESULTS_DB)
#  add_subdirectory(yaml-cpp)
#endif()

if(HAVE_X_GRAPHICS)
  add_subdirectory(external/sciplot)
endif(HAVE_X_GRAPHICS)
