# Find the Darwin dylib dependencies of dakota, excluding system libraries, 
# and install to ${CMAKE_INSTALL_PREFIX}/bin

# NOTE: This script will only work for make install from top of build tree
# TODO: Review string quoting conventions and test with spaces in filename

# Function to install a single Dakota dll dependency
# (used by multiple platforms)
function(dakota_install_dll dakota_dll)
  if (EXISTS "${dakota_dll}")
    get_filename_component(dll_filename "${dakota_dll}" NAME)
    message("-- Installing: ${CMAKE_INSTALL_PREFIX}/bin/${dll_filename}")
    execute_process(
      COMMAND 
        ${CMAKE_COMMAND} -E copy "${dakota_dll}" "${CMAKE_INSTALL_PREFIX}/bin" 
      )
  else()
    message(WARNING "Install couldn't find dynamic dependency ${dakota_dll}")
  endif()
endfunction()


if ( DAKOTA_JENKINS_BUILD OR DEFINED ENV{WORKSPACE} )
  # By convention, all Dakota, jenkins-driven build jobs use a 'build'
  # subdir for clear separation of source and build trees in the WORKSPACE
  set( CMAKE_CURRENT_BINARY_DIR $ENV{WORKSPACE}/build )
elseif ( NOT CMAKE_CURRENT_BINARY_DIR )
  set( CMAKE_CURRENT_BINARY_DIR $ENV{PWD} )
endif()

message( "CMAKE_SHARED_LIBRARY_SUFFIX: ${CMAKE_SHARED_LIBRARY_SUFFIX}" )
#message( "... If NOT .dylib, then CMake cache is not respected" )

# otool may resolve symlinks, do the same for the build tree location
get_filename_component(resolved_build_dir ${CMAKE_CURRENT_BINARY_DIR} REALPATH)

# Get the dylibs excluding system libraries as a semicolon-separated list
execute_process(
  COMMAND otool -L "${CMAKE_CURRENT_BINARY_DIR}/src/dakota"
  # Omit the header and get the library only
  COMMAND awk "FNR > 1 {print $1}"
  # Omit system libraries
  COMMAND egrep -v "(^/System|^/usr/lib|^/usr/X11)"
  COMMAND tr "\\n" ";"
  OUTPUT_VARIABLE dakota_darwin_dylibs
  )

# Probe the CMakeCache.txt for location of the known Boost dynlib dependency
# The FindBoost.cmake probe was updated in 2.8.11 (or thereabouts) to cache
# the variable Boost_LIBRARY_DIR:PATH instead of Boost_LIBRARY_DIRS:FILEPATH.
# Check both.

file( STRINGS ${CMAKE_CURRENT_BINARY_DIR}/CMakeCache.txt
      Boost_LIBRARY_DIRS_PAIR REGEX "^Boost_LIBRARY_DIRS:FILEPATH=(.*)$" )
if( "${Boost_LIBRARY_DIRS_PAIR}" STREQUAL "")
  file( STRINGS ${CMAKE_CURRENT_BINARY_DIR}/CMakeCache.txt
        Boost_LIBRARY_DIRS_PAIR REGEX "^Boost_LIBRARY_DIR:PATH=(.*)$" )
endif()

string( REGEX REPLACE "^Boost_LIBRARY_DIR.+=(.*)$" "\\1"
        Cached_Boost_LIBRARY_DIRS "${Boost_LIBRARY_DIRS_PAIR}" )

#message("Boost rpath=${Cached_Boost_LIBRARY_DIRS}")

# Modify dakota_darwin_dylibs for "special case" of Boost
#   otool DOES NOT return absolute path to Boost libs, so workaround the issue

set(dakota_boost_dylibs "")

# Ignore empty list elements:
cmake_policy(PUSH)
cmake_policy(SET CMP0007 OLD)

foreach(pri_lib ${dakota_darwin_dylibs})
  string(REGEX REPLACE "^libboost_(.*)$"
    "${Cached_Boost_LIBRARY_DIRS}/libboost_\\1"
    boost_dylib_fullpath "${pri_lib}")

  if( ${pri_lib} MATCHES libboost_ )
    # REMOVE boost entries if NOT absolute path
    list(REMOVE_ITEM dakota_darwin_dylibs ${pri_lib})
    list(APPEND dakota_boost_dylibs ${boost_dylib_fullpath})
  endif()
endforeach()

# Get the secondary dylibs of the dylibs
foreach(pri_lib ${dakota_darwin_dylibs})
  execute_process(
    COMMAND otool -L "${pri_lib}"
    COMMAND awk "FNR > 1 {print $1}"
    # Omit system libraries
    COMMAND egrep -v "(^/System|^/usr/lib|^/usr/X11)"
    COMMAND tr "\\n" ";"
    OUTPUT_VARIABLE dakota_secondary_dylibs
    )
  list(APPEND dakota_darwin_dylibs ${dakota_secondary_dylibs})
endforeach()

# Make a second pass over the list to prepend paths to boost libs that were
# discovered while looking for dakota_secondary_dylibs. Any duplicates
# will be removed below.
foreach(pri_lib ${dakota_darwin_dylibs})
  string(REGEX REPLACE "^libboost_(.*)$"
    "${Cached_Boost_LIBRARY_DIRS}/libboost_\\1"
    boost_dylib_fullpath "${pri_lib}")

  if( ${pri_lib} MATCHES libboost_ )
    # REMOVE boost entries if NOT absolute path
    list(REMOVE_ITEM dakota_darwin_dylibs ${pri_lib})
    list(APPEND dakota_boost_dylibs ${boost_dylib_fullpath})
  endif()
endforeach()

# otool finished proccessing dylibs -
# OK to "re-insert" Boost dylibs into the list (ABSOLUTE PATH!)

#message("Boost dylibs=${dakota_boost_dylibs}")
list(APPEND dakota_darwin_dylibs ${dakota_boost_dylibs})

list(REMOVE_DUPLICATES dakota_darwin_dylibs)
cmake_policy(POP)

# Process each DLL and install, excluding anything in the build tree
foreach(dakota_dll ${dakota_darwin_dylibs})
  string(REGEX REPLACE "^${CMAKE_CURRENT_BINARY_DIR}(.*)$"
    "dak_omit/\\1" omit_btree_dll "${dakota_dll}")
  string(REGEX REPLACE "^${resolved_build_dir}(.*)$"
    "dak_omit/\\1" omit_resolved_btree_dll "${dakota_dll}")

  if( ${omit_btree_dll} MATCHES dak_omit )
    #message("-- EXCLUDE: ${omit_btree_dll} - OK, already installed in lib")
    message("-- EXCLUDE: ${dakota_dll} - OK, already installed in lib")
  elseif( ${omit_resolved_btree_dll} MATCHES dak_omit )
    message("-- EXCLUDE: ${dakota_dll} - OK, already installed in lib")
  else()
    dakota_install_dll("${dakota_dll}")
  endif()
endforeach()

