# More widely distributed packages exclude more content
# So smaller number implies less content
set(DAKOTA_DISTRO_snlfull 3)   # Complete checkout
set(DAKOTA_DISTRO_snlsuper 2)  # Less some of local/
set(DAKOTA_DISTRO_snlweb 1)    # Also less DOT
set(DAKOTA_DISTRO_public 0)    # Also less NPSOL, NLPQL

# 0-based faked enum to map integers to strings
set(DAKOTA_DISTRO_enum "public" "snlweb" "snlsuper" "snlfull")

# Transition wrapper to allow distro as integer or string
function(dakota_distro_as_integer _retval_distro_int)
  if(DAKOTA_DISTRO STREQUAL "snlfull" OR DAKOTA_DISTRO STREQUAL "snlsuper" OR
      DAKOTA_DISTRO STREQUAL "snlweb" OR DAKOTA_DISTRO STREQUAL "public")
    set(${_retval_distro_int} "${DAKOTA_DISTRO_${DAKOTA_DISTRO}}" PARENT_SCOPE)
  elseif(DAKOTA_DISTRO GREATER_EQUAL 0 AND DAKOTA_DISTRO LESS_EQUAL 3)
    set(${_retval_distro_int} "${DAKOTA_DISTRO}" PARENT_SCOPE)
  else()
    message(FATAL_ERROR "Invalid DAKOTA_DISTRO = ${DAKOTA_DISTRO}")
  endif()
endfunction()

# Transition wrapper to allow distro as integer or string
function(dakota_distro_as_string _retval_distro_string)
  if(DAKOTA_DISTRO STREQUAL "snlfull" OR DAKOTA_DISTRO STREQUAL "snlsuper" OR
      DAKOTA_DISTRO STREQUAL "snlweb" OR DAKOTA_DISTRO STREQUAL "public")
    set(${_retval_distro_string} "${DAKOTA_DISTRO}" PARENT_SCOPE)
  elseif(DAKOTA_DISTRO GREATER_EQUAL 0 AND DAKOTA_DISTRO LESS_EQUAL 3)
    list(GET DAKOTA_DISTRO_enum ${DAKOTA_DISTRO} _distro_str)
    set(${_retval_distro_string} "${_distro_str}" PARENT_SCOPE)
  else()
    message(FATAL_ERROR "Invalid DAKOTA_DISTRO = ${DAKOTA_DISTRO}")
  endif()
endfunction()


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
    )

  dakota_distro_as_integer(_dakota_distro_integer)

  # Only internal full (developer version) has these things
  if(_dakota_distro_integer LESS ${DAKOTA_DISTRO_snlfull})
    list(APPEND CPACK_SOURCE_IGNORE_FILES "^${Dakota_SOURCE_DIR}/local/")
  endif()

  if(_dakota_distro_integer LESS ${DAKOTA_DISTRO_snlsuper})
    message(STATUS "Removing DOT for less than InternalSupervised build")
    # When building InternalWeb, don't want DOT in the binaries
    # Be aggressive and remove from source tree
    file(REMOVE_RECURSE ${Dakota_SOURCE_DIR}/packages/local/DOT/)
  endif()

  if(_dakota_distro_integer LESS ${DAKOTA_DISTRO_snlweb})
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
  SET(CPACK_SOURCE_INSTALLED_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR};/") 
  LIST(APPEND CPACK_SOURCE_INSTALLED_DIRECTORIES
    "${Dakota_BINARY_DIR}/generated/src/;/src")
  LIST(APPEND CPACK_SOURCE_INSTALLED_DIRECTORIES
    "${Dakota_BINARY_DIR}/generated/VERSION/;/")

  if(DAKOTA_EXAMPLES_CPACK_SOURCE_IGNORE)
    list(APPEND CPACK_SOURCE_IGNORE_FILES ${DAKOTA_EXAMPLES_CPACK_SOURCE_IGNORE})
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
