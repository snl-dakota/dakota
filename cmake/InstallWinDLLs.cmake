# Install Dakota's Windows DLL dependencies, including Intel runtime

function(find_lib_path base_path result_var)
    # Define the paths to check
    get_filename_component(path_ia32
      "${base_path}/redist/ia32/compiler" ABSOLUTE)
    get_filename_component(path_intel64
      "${base_path}/redist/intel64/compiler" ABSOLUTE)

    # Check if either directory exists
    if(EXISTS "${path_ia32}")
        set(${result_var} "${path_ia32}" PARENT_SCOPE)
    elseif(EXISTS "${path_intel64}")
        set(${result_var} "${path_intel64}" PARENT_SCOPE)
    else()
        set(${result_var} "${base_path}" PARENT_SCOPE)
    endif()

endfunction()

# Prior to calling the CMake InstallRequiredSystemLibraries, append
# the Intel libraries to CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS.

# Assume ifortvars sourced and append Intel libraries based on environment var
# e.g., set(ifort_libpath "c:/Program Files/Intel/Compiler/11.1/065/lib/ia32")
if (DEFINED ENV{INTEL_DEV_REDIST})
  # This should work for at least Intel 2013 -- 2017
  find_lib_path("$ENV{INTEL_DEV_REDIST}" ifort_libpath)

elseif(DEFINED ENV{IFORT_COMPILER11})
  find_lib_path("$ENV{IFORT_COMPILER11}" ifort_libpath)
elseif (DEFINED ENV{IFORT_COMPILER12})
  find_lib_path("$ENV{IFORT_COMPILER12}" ifort_libpath)
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

