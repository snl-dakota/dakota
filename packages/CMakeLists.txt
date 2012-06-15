# CMakeLists.txt for DAKOTA packages
option(HAVE_AMPL "Build the AMPL package." ON)
option(HAVE_CONMIN "Build the CONMIN package." ON)
option(HAVE_DDACE "Build the DDACE package." ON)
option(HAVE_DOT "Build the DOT package." ON)
option(HAVE_FSUDACE "Build the FSUDACE package." ON)
option(HAVE_HOPSPACK "Build the hopspack package." ON)
option(HAVE_JEGA "Build the JEGA package." ON)
option(HAVE_NCSUOPT "Build the NCSUOPT package." ON)
option(HAVE_NL2SOL "Build the NL2SOL package." ON)
option(HAVE_NLPQL "Build the NLPQL package." ON)
option(HAVE_NPSOL "Build the NPSOL package." ON)
option(HAVE_OPTPP "Build the OPTPP package." ON)
option(HAVE_PECOS "Build the Pecos package." ON)
option(HAVE_PLUGIN "Enable plugins to the dakota executable." ON)
option(HAVE_PSUADE "Build the PSUADE package." ON)
option(HAVE_QUESO "Build the QUESO package." OFF)

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


find_program(SH sh)
if(${SH} MATCHES "sh-NOTFOUND")
  message(FATAL_ERROR
          "Cannot build queso as an external project without sh")
else()
  if(HAVE_QUESO)
    ############################################################################
    # GRVY
    #
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
      URL ${CMAKE_CURRENT_SOURCE_DIR}/queso_tarballs/grvy-0.30.0.tgz
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND "${grvy_config_cmd}"
    )

    ############################################################################
    # Queso
    #
    set(QUESO_EXT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/queso_ext)
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E make_directory ${QUESO_EXT_BINARY_DIR} )

    set(queso_config_cmd
      ${SH} ${CMAKE_CURRENT_BINARY_DIR}/queso_ext-prefix/src/queso_ext/configure
      "F77=${CMAKE_Fortran_COMPILER}" "FC=${CMAKE_Fortran_COMPILER}"
      "--without-hdf5" "--without-glpk" "--with-grvy=${GRVY_EXT_BINARY_DIR}"
      "--with-boost=${Boost_INCLUDE_DIR}/.."
      "--with-trilinos=${Trilinos_DIR}"
      #"LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH}:${Trilinos_DIR}/lib:/Net/dakota/utils/Linux_x86_64/gsl-1.14.gcc-4.3.2/lib"
      "LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH}:${Trilinos_DIR}/lib"
      "CXXFLAGS=${CMAKE_CXX_FLAGS} -Wl,-lepetra"
      "--prefix" "${QUESO_EXT_BINARY_DIR}"
    )
    externalproject_add(queso_ext
      URL ${CMAKE_CURRENT_SOURCE_DIR}/queso_tarballs/queso-0.45.0.tar.gz
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND "${queso_config_cmd}"
    )

    add_dependencies(queso_ext grvy_ext)
  endif(HAVE_QUESO)
endif() #sh



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

if(HAVE_X_GRAPHICS)
  add_subdirectory(motif)
endif(HAVE_X_GRAPHICS)
