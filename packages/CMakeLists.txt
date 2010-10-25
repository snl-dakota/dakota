add_subdirectory(nidr)
add_subdirectory(Pecos)

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
#if(HAVE_SURFPACK)
#  add_subdirectory(Surfpack)
#endif(HAVE_SURFPACK)

if(HAVE_AMPL)
  add_subdirectory(ampl)
endif(HAVE_AMPL)

