add_subdirectory(nidr)

#if(NOT USE_SYSTEM_TEUCHOS)
#  add_subdirectory(teuchos)
#endif()

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

if(HAVE_AMPL)
  add_subdirectory(ampl)
endif(HAVE_AMPL)


if(HAVE_ACRO)
  add_subdirectory(acro)
endif(HAVE_ACRO)

if(HAVE_ACRO)
  add_subdirectory(acro)
endif(HAVE_ACRO)

if(HAVE_DDACE)
  add_subdirectory(DDACE)
endif(HAVE_DDACE)

if(HAVE_DOT)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/DOT)
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
  if(CMAKE_SYSTEM_NAME MATCHES Darwin)
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
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/NLPQL)
    add_subdirectory(NLPQL)
  else()
    set(HAVE_NLPQL OFF CACHE BOOL "Build the NLPQL package" FORCE)
  endif()
endif(HAVE_NLPQL)

if(HAVE_NPSOL)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/NPSOL)
    add_subdirectory(NPSOL)
  else()
    set(HAVE_NPSOL OFF CACHE BOOL "Build the NPSOL package" FORCE)
  endif()
endif(HAVE_NPSOL)

if(HAVE_OPTPP)
  add_subdirectory(OPTPP)
endif(HAVE_OPTPP)

if(HAVE_PSUADE)
  add_subdirectory(PSUADE)
endif(HAVE_PSUADE)

