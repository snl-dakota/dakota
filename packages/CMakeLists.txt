# CMakeLists.txt for DAKOTA packages
option(HAVE_AMPL "Build the AMPL package." ON)
option(HAVE_CONMIN "Build the CONMIN package." ON)
option(HAVE_DDACE "Build the DDACE package." ON)
option(HAVE_DOT "Build the DOT package." ON)
option(HAVE_DREAM "Build the DREAM package." OFF)
option(HAVE_FSUDACE "Build the FSUDACE package." ON)
option(HAVE_HOPSPACK "Build the hopspack package." ON)
option(HAVE_JEGA "Build the JEGA package." ON)
option(HAVE_NCSUOPT "Build the NCSUOPT package." ON)
option(HAVE_NL2SOL "Build the NL2SOL package." ON)
option(HAVE_NLPQL "Build the NLPQL package." ON)
option(HAVE_NOMAD "Build the NOMAD package." ON)
option(HAVE_NPSOL "Build the NPSOL package." ON)
option(HAVE_OPTPP "Build the OPTPP package." ON)
option(HAVE_PECOS "Build the Pecos package." ON)
option(HAVE_PLUGIN "Enable plugins to the dakota executable." ON)
option(HAVE_PSUADE "Build the PSUADE package." ON)
option(HAVE_QUESO "Build the QUESO package." OFF)
option(HAVE_QUESO_GPMSA "Use the QUESO GPMSA package when QUESO enabled." ON)
option(HAVE_QUESO_GRVY "Use optional QUESO GRVY package; not recommended." OFF)

if(HAVE_CONMIN)
  if ( NOT CONMIN_DIR )
    set( CONMIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/CONMIN )
    add_subdirectory(CONMIN)
  endif()
endif(HAVE_CONMIN)

add_subdirectory(nidr)

if(HAVE_AMPL)
  if (NOT AMPL_INCLUDE_DIRS)
    add_subdirectory(ampl)
  endif()
endif(HAVE_AMPL)

if(HAVE_OPTPP)
  if (NOT OPTPP_INCLUDE_DIRS)
    add_subdirectory(OPTPP)
  endif()
endif()

# Both DAKOTA and PECOS unconditionally depend on Teuchos, and the "shared
# component magic" is managed in the top-level CMakeLists.txt file

add_subdirectory(pecos)

include(ExternalProject)

if(HAVE_QUESO)

  # Prerequisites for building QUESO
  find_program(SH sh)
  if(${SH} MATCHES "sh-NOTFOUND")
    message(FATAL_ERROR "Cannot build QUESO as an external project without sh")
  endif()

  # Queso depends on GSL, so ensure GSL detection and proper lib settings
  # Require user to turn on this GPL package explicitly
  if(NOT DAKOTA_HAVE_GSL)
    message(FATAL_ERROR 
      "HAVE_QUESO requires enabling GPL package GSL via DAKOTA_HAVE_GSL.")
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

  # Optionally configure and build GRVY. As of queso-0.46.0, GRVY is optional
  if (HAVE_QUESO_GRVY)

    set(GRVY_EXT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/grvy_ext)
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E make_directory ${GRVY_EXT_BINARY_DIR} )
      
    set(grvy_config_cmd
      ${SH} ${CMAKE_CURRENT_BINARY_DIR}/grvy_ext-prefix/src/grvy_ext/configure
      "F77=${CMAKE_Fortran_COMPILER}" "FC=${CMAKE_Fortran_COMPILER}"
      "--without-hdf5" "--with-boost=${Boost_INCLUDE_DIR}/.."
      "--prefix" "${GRVY_EXT_BINARY_DIR}"
      )
    externalproject_add(grvy_ext
      URL ${CMAKE_CURRENT_SOURCE_DIR}/queso/grvy-0.32.0.tar.gz
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND "${grvy_config_cmd}"
      )

    #add_library(grvy UNKNOWN IMPORTED GLOBAL)
    #set_target_properties(grvy PROPERTIES 
    #  IMPORTED_LOCATION ${Dakota_BINARY_DIR}/packages/grvy_ext/lib)

  endif()

  # Configure and build QUESO
  set(QUESO_EXT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/queso_ext)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E make_directory ${QUESO_EXT_BINARY_DIR} )

  set(queso_config_cmd
    ${SH} ${CMAKE_CURRENT_BINARY_DIR}/queso_ext-prefix/src/queso_ext/configure
    "CC=${CMAKE_C_COMPILER}" "CXX=${CMAKE_CXX_COMPILER}"
    "F77=${CMAKE_Fortran_COMPILER}" "FC=${CMAKE_Fortran_COMPILER}"
    "--without-hdf5" "--without-glpk"
    "--with-boost=${Boost_INCLUDE_DIR}/.."
    "LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH}:${Boost_LIBRARY_DIRS}:${GSL_LIBRARY_DIRS}"
    "CXXFLAGS=${CMAKE_CXX_FLAGS}"
    "--prefix" "${QUESO_EXT_BINARY_DIR}"
    )
  if (HAVE_QUESO_GRVY)
    set(queso_config_cmd ${queso_config_cmd} 
      "--with-grvy=${GRVY_EXT_BINARY_DIR}")
  endif()

  externalproject_add(queso_ext
    URL ${CMAKE_CURRENT_SOURCE_DIR}/queso/queso-0.46.0.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND "${queso_config_cmd}"
    )

  #add_library(queso UNKNOWN IMPORTED GLOBAL)
  #set_target_properties(queso PROPERTIES 
  #  IMPORTED_LOCATION ${Dakota_BINARY_DIR}/packages/queso_ext/lib)

  if (HAVE_QUESO_GRVY)
    add_dependencies(queso_ext grvy_ext)
  endif()
  
  # Install QUESO (and possibly GRVY) libs
  install(SCRIPT "${Dakota_SOURCE_DIR}/cmake/InstallQuesoLibs.cmake")

endif(HAVE_QUESO)


if(HAVE_ADAPTIVE_SAMPLING AND HAVE_MORSE_SMALE)
  #svn co https://software.sandia.gov/svn/public/tpl/ann/trunk ann
  add_subdirectory(ann/src)

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


if(HAVE_SURFPACK)
  add_subdirectory(surfpack)
endif(HAVE_SURFPACK)

if(HAVE_ACRO)
  if(NOT DAKOTA_HAVE_MPI)
    set(ACRO_USING_MPI OFF CACHE BOOL
        "MPI NOT enabled in DAKOTA so disable in acro as well" FORCE)
    set(UTILIB_USE_MPI OFF CACHE BOOL
        "MPI NOT enabled in DAKOTA so disable in utilib as well" FORCE)
  endif()
  add_subdirectory(acro)
endif(HAVE_ACRO)

if(HAVE_DDACE)
  add_subdirectory(DDACE)
endif(HAVE_DDACE)

if(HAVE_DOT)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/DOT/dbl_prec/dot1.f)
    add_subdirectory(DOT)
  else()
    set(HAVE_DOT OFF CACHE BOOL "Build the DOT package" FORCE)
  endif()
endif(HAVE_DOT)

if(HAVE_DREAM)
  add_subdirectory(dream)
endif(HAVE_DREAM)


if(HAVE_FSUDACE)
  add_subdirectory(FSUDace)
endif(HAVE_FSUDACE)

if(HAVE_HOPSPACK)
  if(DAKOTA_HAVE_MPI)
    # See hopspack/ConfigureMPI.cmake for MPI "help"
    set(mpi ON CACHE BOOL "MPI requested so enable in HOPSPACK" FORCE)
    message("HOPSPACK caches the variable mpi = ${mpi}")
  endif(DAKOTA_HAVE_MPI)

  # HOPSPACK expects BLAS symbol ddot to be found in LAPACK libs 
  set(LAPACK_ADD_LIBS ${BLAS_LIBS})
  set(HOPSPACK_TEST_LAPACK_FUNCS FALSE)
  add_subdirectory(hopspack)
endif(HAVE_HOPSPACK)

if(HAVE_JEGA)
  add_subdirectory(JEGA)
endif(HAVE_JEGA)

if(HAVE_NCSUOPT)
  add_subdirectory(NCSUOpt)
endif(HAVE_NCSUOPT)

if(HAVE_NL2SOL)
  add_subdirectory(NL2SOL)
endif(HAVE_NL2SOL)

if(HAVE_NLPQL)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/NLPQL/NLPQLB.f)
    add_subdirectory(NLPQL)
  else()
    set(HAVE_NLPQL OFF CACHE BOOL "Build the NLPQL package" FORCE)
  endif()
endif(HAVE_NLPQL)

if(HAVE_NOMAD)
  add_subdirectory(NOMAD)
endif(HAVE_NOMAD)

if(HAVE_NPSOL)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/NPSOL/npsolsubs.f)
    add_subdirectory(NPSOL)
  else()
    set(HAVE_NPSOL OFF CACHE BOOL "Build the NPSOL package" FORCE)
  endif()
endif(HAVE_NPSOL)

if(HAVE_PSUADE)
  add_subdirectory(PSUADE)
endif(HAVE_PSUADE)

#if(DAKOTA_RESULTS_DB)
#  add_subdirectory(yaml-cpp)
#endif()

if(HAVE_X_GRAPHICS)
  add_subdirectory(motif)
endif(HAVE_X_GRAPHICS)
