# Hard coded list of DLLs to install
# For MSVS 9 (2008), Intel Fortran 11.1

if(CMAKE_CL_64)
  message(WARNING "InstallWinDLLs.cmake: MSVC compiler is 64-bit; only 32-bit DLLs are supported.")
endif()
# Hardwire x86 architecture
set(CMAKE_MSVC_ARCH x86)

# We either install Release libs or (Release AND Debug)
if (NOT DEFINED CMAKE_INSTALL_DEBUG_LIBRARIES)
  string(TOLOWER ${CMAKE_BUILD_TYPE} my_build_type)
  if ( ${my_build_type} MATCHES "debug" )
    set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
  endif() 
endif()

# Code borrowed from CMake's InstallRequiredSystemLibraries.cmake
# We only support MSVC90 for now
#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

  GET_FILENAME_COMPONENT(devenv_dir "${CMAKE_MAKE_PROGRAM}" PATH)
  GET_FILENAME_COMPONENT(base_dir "${devenv_dir}/../.." ABSOLUTE)

  IF(MSVC90)
    # Find the runtime library redistribution directory.
    GET_FILENAME_COMPONENT(msvc_install_dir
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\9.0;InstallDir]" ABSOLUTE)
    GET_FILENAME_COMPONENT(msvc_express_install_dir
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\9.0;InstallDir]" ABSOLUTE)
    FIND_PATH(MSVC90_REDIST_DIR NAMES ${CMAKE_MSVC_ARCH}/Microsoft.VC90.CRT/Microsoft.VC90.CRT.manifest
      PATHS
        "${msvc_install_dir}/../../VC/redist"
        "${msvc_express_install_dir}/../../VC/redist"
        "${base_dir}/VC/redist"
      )
    MARK_AS_ADVANCED(MSVC90_REDIST_DIR)
    SET(MSVC90_CRT_DIR "${MSVC90_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC90.CRT")

    # Install the manifest that allows DLLs to be loaded from the
    # directory containing the executable.
    IF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)
      SET(__install__libs
        "${MSVC90_CRT_DIR}/Microsoft.VC90.CRT.manifest"
        "${MSVC90_CRT_DIR}/msvcm90.dll"
        "${MSVC90_CRT_DIR}/msvcp90.dll"
        "${MSVC90_CRT_DIR}/msvcr90.dll"
        )
    ENDIF(NOT CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY)

    IF(CMAKE_INSTALL_DEBUG_LIBRARIES)
      SET(MSVC90_CRT_DIR
        "${MSVC90_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC90.DebugCRT")
      SET(__install__libs ${__install__libs}
        "${MSVC90_CRT_DIR}/Microsoft.VC90.DebugCRT.manifest"
        "${MSVC90_CRT_DIR}/msvcm90d.dll"
        "${MSVC90_CRT_DIR}/msvcp90d.dll"
        "${MSVC90_CRT_DIR}/msvcr90d.dll"
        )
    ENDIF(CMAKE_INSTALL_DEBUG_LIBRARIES)
  ENDIF(MSVC90)

# Assume ifortvars sourced and append Intel libraries based on environment var
#set(ifort_libpath "c:/Program Files/Intel/Compiler/11.1/065/lib/ia32")
if(DEFINED ENV{IFORT_COMPILER11})
  GET_FILENAME_COMPONENT(ifort_libpath "$ENV{IFORT_COMPILER11}/lib/ia32" ABSOLUTE)
elseif (DEFINED ENV{IFORT_COMPILER12})
  GET_FILENAME_COMPONENT(ifort_libpath "$ENV{IFORT_COMPILER12}/redist/ia32/compiler" ABSOLUTE)
endif()

if(DEFINED ifort_libpath)
  message(STATUS "Installing Intel Fortran libraries from ${ifort_libpath}")
  # Intel release libraries
  list(APPEND __install__libs
    "${ifort_libpath}/libmmd.dll"
    "${ifort_libpath}/libifcoremd.dll"
    "${ifort_libpath}/svml_dispmd.dll"
  )
  # Intel debug libraries
  IF(CMAKE_INSTALL_DEBUG_LIBRARIES)
    list(APPEND  __install__libs
      "${ifort_libpath}/libmmdd.dll"
      "${ifort_libpath}/libifcoremdd.dll"
    )
  ENDIF()
endif()

# Verify libraries exist
  FOREACH(lib
      ${__install__libs}
      )
    IF(EXISTS ${lib})
      SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
        ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} ${lib})
    ELSE(EXISTS ${lib})
      IF(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
        MESSAGE(WARNING "system runtime library file does not exist: '${lib}'")
        # This warning indicates an incomplete Visual Studio installation
        # or a bug somewhere above here in this file.
        # If you would like to avoid this warning, fix the real problem, or
        # set CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS before including
        # this file.
      ENDIF()
    ENDIF(EXISTS ${lib})
  ENDFOREACH(lib)

# Include system runtime libraries in the installation if any are
# specified by CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS.
IF(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
  IF(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP)
    IF(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION)
      IF(WIN32)
        SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION bin)
      ELSE(WIN32)
        SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION lib)
      ENDIF(WIN32)
    ENDIF(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION)
    INSTALL(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
      DESTINATION ${CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION})
  ENDIF(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP)
ENDIF(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)

message(STATUS "Will install system DLLs: ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}")
