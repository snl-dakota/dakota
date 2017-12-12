# Install Dakota's Windows DLL dependencies, including Intel runtime

# Prior to calling the CMake InstallRequiredSystemLibraries, append
# the Intel libraries to CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS.

# Assume ifortvars sourced and append Intel libraries based on environment var
# e.g., set(ifort_libpath "c:/Program Files/Intel/Compiler/11.1/065/lib/ia32")
if (DEFINED ENV{INTEL_DEV_REDIST})
  # This should work for at least Intel 2013 -- 2017
  get_filename_component(ifort_libpath
    "$ENV{INTEL_DEV_REDIST}/redist/ia32/compiler" ABSOLUTE)
elseif(DEFINED ENV{IFORT_COMPILER11})
  get_filename_component(ifort_libpath 
    "$ENV{IFORT_COMPILER11}/lib/ia32" ABSOLUTE)
elseif (DEFINED ENV{IFORT_COMPILER12})
  get_filename_component(ifort_libpath
    "$ENV{IFORT_COMPILER12}/redist/ia32/compiler" ABSOLUTE)
endif()

if(DEFINED ifort_libpath)
  message(STATUS "Installing Intel Fortran libraries from ${ifort_libpath}")
  # Intel release libraries
  list(APPEND __intel__libs
    "${ifort_libpath}/libmmd.dll"
    "${ifort_libpath}/libifcoremd.dll"
    "${ifort_libpath}/svml_dispmd.dll"
  )
  # Intel debug libraries
  IF(CMAKE_INSTALL_DEBUG_LIBRARIES)
    list(APPEND  __intel__libs
      "${ifort_libpath}/libmmdd.dll"
      "${ifort_libpath}/libifcoremdd.dll"
    )
  ENDIF()

  foreach(lib
      ${__intel__libs}
      )
    if(EXISTS ${lib})
      set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
        ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} ${lib})
    else()
      if(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
        message(WARNING "system runtime library file does not exist: '${lib}'")
        # This warning indicates an incomplete Watcom installation
        # or a bug somewhere above here in this file.
        # If you would like to avoid this warning, fix the real problem, or
        # set CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS before including
        # this file.
      endif()
    endif()
  endforeach()

endif()

# The CMake module takes care of MSVS runtime libraries
include(InstallRequiredSystemLibraries)

message(STATUS "Installing system DLLs: ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}")
