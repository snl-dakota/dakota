add_subdirectory(nidr)
add_subdirectory(pecos)

if(NOT USE_SYSTEM_TEUCHOS)
  add_subdirectory(teuchos)
endif()

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


# WJB - ToDo: refactor directory hierarchy (dependency-oriented rather than
# logically-oriented) so that DAKOTA and Surfpack can "share" CONMIN
if(HAVE_SURFPACK)
  add_subdirectory(surfpack)
endif(HAVE_SURFPACK)

if(HAVE_AMPL)
  add_subdirectory(ampl)
endif(HAVE_AMPL)


if(HAVE_CONMIN)
  add_subdirectory(CONMIN)
endif(HAVE_CONMIN)

if(HAVE_DDACE)
  add_subdirectory(DDACE)
endif(HAVE_DDACE)

if(HAVE_DOT)
  add_subdirectory(DOT)
endif(HAVE_DOT)

if(HAVE_FSUDACE)
  add_subdirectory(FSUDace)
endif(HAVE_FSUDACE)

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
  add_subdirectory(NLPQL)
endif(HAVE_NLPQL)

if(HAVE_NPSOL)
  add_subdirectory(NPSOL)
endif(HAVE_NPSOL)

if(HAVE_OPTPP)
  add_subdirectory(OPTPP)
endif(HAVE_OPTPP)

if(HAVE_PSUADE)
  add_subdirectory(PSUADE)
endif(HAVE_PSUADE)

