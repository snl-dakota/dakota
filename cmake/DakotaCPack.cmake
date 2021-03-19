# Initialize basic CPack settings unconditionally
macro(dakota_cpack_initialize)
  # Initial CPack Settings.  Done here as affects configuration in packages/
  # (DOT, NPSOL, NLPQL)
  set(CPACK_PACKAGE_VENDOR "Sandia National Laboratories")

  set(local_arch "${CMAKE_SYSTEM_NAME}.${CMAKE_SYSTEM_PROCESSOR}")
  if(WIN32)
    # CMAKE_SYSTEM_PROCESSOR might be AMD64 on Windows which might cause confusion
    # consider adopting more CMake standard package names
    if(CMAKE_VS_PLATFORM_NAME)
      if(CMAKE_VS_PLATFORM_NAME STREQUAL "Win32")
        set(local_arch "${CMAKE_SYSTEM_NAME}.x86")
      else()
        set(local_arch "${CMAKE_SYSTEM_NAME}.${CMAKE_VS_PLATFORM_NAME}")
      endif()
    endif()
  endif()

  # TODO: insert distribution type into package names, i.e., instead of 
  # ${Dakota_VERSION_STRING} use ${Dakota_VERSION_STRING}.int_web, based on DAKOTA_DISTRO
  set(CPACK_PACKAGE_FILE_NAME "dakota-${Dakota_VERSION_STRING}.${local_arch}")
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "dakota-${Dakota_VERSION_STRING}.src")
  set(CPACK_PACKAGE_VERSION_MAJOR "${Dakota_VERSION_MAJOR}")
  set(CPACK_PACKAGE_VERSION_MINOR "${Dakota_VERSION_MINOR}")
  set(CPACK_PACKAGE_VERSION_PATCH "${Dakota_VERSION_PATCH}")
  set(CPACK_DAKOTA_BINARY_DIR "${Dakota_BINARY_DIR}")

endmacro()


# Prune files not allowed in some variants of Dakota; depends on
# DAKOTA_DISTRO and related variables
macro(dakota_cpack_prune_distro)

  # Always omit .git directories, local scripts, SNL and duplicate
  # TPLs, and GPL packages FFTW and CDDLIB
  set(CPACK_SOURCE_IGNORE_FILES
    ".*\\\\.git/" ".*\\\\.git.*"
    "^${Dakota_SOURCE_DIR}/packages/external/fftw/"
    "^${Dakota_SOURCE_DIR}/packages/external/hopspack/src/src-citizens/citizen-gss/cddlib/"
    "^${Dakota_SOURCE_DIR}/dakota-examples"
    )

  # Only internal full (developer version) has these things
  if(DAKOTA_DISTRO LESS ${DAKOTA_InternalFull})
    list(APPEND CPACK_SOURCE_IGNORE_FILES "^${Dakota_SOURCE_DIR}/local/")
  endif()

  if(DAKOTA_DISTRO LESS ${DAKOTA_InternalSupervised})
    message(STATUS "Removing DOT for less than InternalSupervised build")
    # When building InternalWeb, don't want DOT in the binaries
    # Be aggressive and remove from source tree
    file(REMOVE_RECURSE ${Dakota_SOURCE_DIR}/packages/local/DOT/)
  endif()

  if(DAKOTA_DISTRO LESS ${DAKOTA_InternalWeb})
    message(STATUS "Removing NLPQL, NPSOL for less than InternalWeb build")
    # When building ExternalWeb, don't want NLPQL, NPSOL in the binaries
    # Be aggressive and remove from source tree
    file(REMOVE_RECURSE
      "${Dakota_SOURCE_DIR}/packages/local/NLPQL"
      "${Dakota_SOURCE_DIR}/packages/local/NPSOL")
  endif()

endmacro()


# Finalize CPack settings: installed source directories and package types
macro(dakota_cpack_finalize)

  # Create Add generated files to source package
  if(DAKOTA_GIT_checkout)
    SET(CPACK_SOURCE_INSTALLED_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR};/") 
    LIST(APPEND CPACK_SOURCE_INSTALLED_DIRECTORIES
      "${Dakota_BINARY_DIR}/generated/src/;/src")
    LIST(APPEND CPACK_SOURCE_INSTALLED_DIRECTORIES
      "${Dakota_BINARY_DIR}/generated/VERSION/;/")
  endif()

  if(WIN32)
    set(CPACK_GENERATOR ZIP)
    set(CPACK_SOURCE_GENERATOR ZIP)
  elseif(CYGWIN)
    # The Cygwin-specific generators require additional information to
    # create packages that work with Cygwin's package manager (we suspect)
    #set(CPACK_GENERATOR CygwinBinary)
    #set(CPACK_SOURCE_GENERATOR CygwinSource)
    set(CPACK_GENERATOR ZIP)
    set(CPACK_SOURCE_GENERATOR ZIP)
  else()
    # All unix-like systems EXCEPT cygwin (WJB - ToDo:  MacOSX-specific packages)
    set(CPACK_GENERATOR TGZ)
    set(CPACK_SOURCE_GENERATOR TGZ)
  endif()

endmacro()
