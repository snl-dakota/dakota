# CMakeLists.txt for DAKOTA packages
option(HAVE_ACRO "Build the ACRO package." ON)
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
option(HAVE_SURFPACK "Build the Surfpack package." ON)

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

  # packages/nidr/../ampl/asl.h:28:51: error: arith.h: No such file
  # WJB - ToDo: nidr dependency on AMPL's asl.h/arith.h is "loose"
  #             could/should be managed better than adding explicit
  #             dependency in the CMakeLists.txt file
  add_dependencies(nidr amplsolver)
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

set(EXTERNAL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/nidr)
execute_process(
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXTERNAL_BINARY_DIR} )

find_program(sh sh)
if(${sh} MATCHES "sh-NOTFOUND")
  message(FATAL_ERROR
          "Cannot build nidr as an external project without sh")
else()
  #build NIDR as an external project
  #externalproject_add(ac_nidr
  #  DOWNLOAD_COMMAND ""
  #  CONFIGURE_COMMAND ${sh} ${CMAKE_CURRENT_SOURCE_DIR}/nidr/configure
  #  BINARY_DIR ${EXTERNAL_BINARY_DIR}
  #  INSTALL_COMMAND ""
  #)
endif() #sh



if(HAVE_SURFPACK)
  add_subdirectory(surfpack)
endif(HAVE_SURFPACK)

if(HAVE_ACRO)
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
  # WJB:  can't explain why hopspack/ConfigureLapack.cmake does NOT work on Mac
  # ToDo: address issue post 5.1 release
  if(CMAKE_SYSTEM_NAME MATCHES Darwin OR
     $ENV{TEST_PLATFORM_VER} MATCHES "suse_x86_64")
    set(LAPACK_ADD_LIBS ${BLAS_LIBS})
  endif()
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
