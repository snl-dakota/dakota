# Build the FFTW from source as an external project with
# configure/make (heterogeneous build)
include(ExternalProject)

function(FftwExternalProject)

  if(HAVE_FFTW)

    message(STATUS "Configuring FFTW as ExternalProject")

    # Set source and binary locations so we can find includes/libs
    set(FFTW_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fftw CACHE PATH 
      "Location of FFTW external project source")
    set(FFTW_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/fftw CACHE PATH 
      "Location of FFTW external project binaries") 

    execute_process(
      COMMAND ${CMAKE_COMMAND} -E make_directory ${FFTW_BINARY_DIR}
    )
    find_program(sh sh)
    if(${sh} MATCHES "sh-NOTFOUND")
      message(FATAL_ERROR
        "Cannot build fftw as an external project without sh")
    else()

      if(BUILD_SHARED_LIBS)
	set(config_flags "--enable-shared")
      endif()

      # build FFTW as an external project
      externalproject_add(fftw
        DOWNLOAD_COMMAND ""
        CONFIGURE_COMMAND ${sh} "${FFTW_SOURCE_DIR}/configure" ${config_flags}
        BINARY_DIR "${FFTW_BINARY_DIR}"
        INSTALL_COMMAND ""
      )
    endif() #sh
  endif(HAVE_FFTW)

endfunction()
