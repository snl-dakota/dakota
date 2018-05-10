# 1. Find the Darwin dylib dependencies of all dakota executables and libraries, 
# excluding system and project libraries, and install to ${CMAKE_INSTALL_PREFIX}/bin. 
# 2. Modify the "install names" of these libraries to strip their full paths 
# (where necessary) and prepend them with @rpath, so that they can be located 
# by the dynamic loader.
# 3. Edit all executables and libraries in both bin/ and lib/ to refer to the libraries 
# by their install names.

# NOTE: This script will only work for make install from top of build tree
# TODO: Review string quoting conventions and test with spaces in filename

# Function to install a single Dakota dll dependency
# (used by multiple platforms)
function(dakota_install_dll dakota_dll)
  if (EXISTS "${dakota_dll}")
    get_filename_component(dll_filename "${dakota_dll}" NAME)
    message("-- Installing: ${CMAKE_INSTALL_PREFIX}/bin/${dll_filename}")
    file(COPY "${dakota_dll}" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" FILE_PERMISSIONS
      OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE) 
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

# Get all dylib dependencies excluding system libraries and Dakota project 
# libraries as a semicolon-separated list. This a terrible solution that requires
# this file command to be updated every time a new installation location that contains
# bianries is added.
set(dakota_darwin_dylibs "")
file(GLOB bin_lib_list "${CMAKE_INSTALL_PREFIX}/bin/*" 
                       "${CMAKE_INSTALL_PREFIX}/lib/*"
                       "${CMAKE_INSTALL_PREFIX}/share/dakota/test/*"
                       "${CMAKE_INSTALL_PREFIX}/share/dakota/examples/hopspack/1-var-bnds-only/*"
                       "${CMAKE_INSTALL_PREFIX}/share/dakota/examples/hopspack/2-linear-constraints/*"
                       "${CMAKE_INSTALL_PREFIX}/share/dakota/examples/hopspack/3-degen-linear-constraints/*"
                       "${CMAKE_INSTALL_PREFIX}/share/dakota/examples/hopspack/4-nonlinear-constraints/*"
                       "${CMAKE_INSTALL_PREFIX}/share/dakota/examples/hopspack/5-multi-start/*"
                       "${CMAKE_INSTALL_PREFIX}/share/dakota/examples/script_interfaces/generic/*")

foreach(loader ${bin_lib_list})
  # skip directories and static libs
  if(NOT IS_DIRECTORY "${loader}" AND NOT loader MATCHES "\\.a$")
    execute_process( COMMAND otool -L ${loader}
                     COMMAND awk "FNR > 1 {print $1}"
                     # Omit system libraries and Dakota project libraries
                     COMMAND egrep -v "(^/System|^/usr/lib|^/usr/X11|@rpath)"
                     COMMAND tr "\\n" ";"
                     RESULT_VARIABLE retcode
                     OUTPUT_VARIABLE deps
                     ERROR_QUIET)
    # recode will be nonzero if otool failed, i.e. the file wasn't a
    # mach-o formatted file (compiled executable or lib).
    if( retcode EQUAL 0 )
      list(APPEND dakota_darwin_dylibs ${deps})      
    endif()
  endif()
endforeach()

# Probe the CMakeCache.txt for location of the known Boost dynlib dependency.
# This method is very fragile because the the FindBoost.cmake probe is frequently 
# updated, and the relevant cache variable changes from version to version. That
# is the most likely cause of the FATAL_ERROR below. Consult the docs in the header
# of the FindBoost.cmake probe for this verion of CMake to determine which variable
# to use, and update Dakota's top-level CMakeLists.txt accordingly.
 
file( STRINGS ${CMAKE_CURRENT_BINARY_DIR}/CMakeCache.txt
      Boost_LIBRARY_DIRS_PAIR REGEX "^DAKOTA_Boost_LIB_DIR:PATH=(.*)$" )
if( "${Boost_LIBRARY_DIRS_PAIR}" STREQUAL "")
    message(FATAL_ERROR "Unable to determine Boost library directory!")
endif()

string( REGEX REPLACE "^DAKOTA_Boost_LIB_DIR:PATH=(.*)$" "\\1"
        Cached_Boost_LIBRARY_DIRS "${Boost_LIBRARY_DIRS_PAIR}" )

# message("Boost rpath=${Cached_Boost_LIBRARY_DIRS}")

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

# Get the secondary dylibs of the dylibs.
# TODO: Is secondary sufficient? Consider enclosing all of this 
# logic into a loop and iterating until dakota_darwin_dylibs stops 
# changing.
foreach(pri_lib ${dakota_darwin_dylibs})
  execute_process(
    COMMAND otool -L "${pri_lib}"
    COMMAND awk "FNR > 1 {print $1}"
    # Omit system libraries
    COMMAND egrep -v "(^/System|^/usr/lib|^/usr/X11|@rpath)"
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

# Process each DLL and install
foreach(dakota_dll ${dakota_darwin_dylibs})
  dakota_install_dll("${dakota_dll}")
endforeach()

# Prepend @rpath to the install names of dylibs in ${CMAKE_INSTALL_PREFIX}/bin
foreach(bin_dll ${dakota_darwin_dylibs})
  get_filename_component(dll_filename "${bin_dll}" NAME)
  set(dll_full "${CMAKE_INSTALL_PREFIX}/bin/${dll_filename}") 
  if (EXISTS "${dll_full}")
    execute_process(
      COMMAND 
        install_name_tool -id @rpath/${dll_filename} ${dll_full}  
      )
  endif()
endforeach()

# Update the libraries and executables in ${CMAKE_INSTALL_PREFIX}/bin and 
# ${CMAKE_INSTALL_PREFIX}/lib to refer to libs by their new install names.
file(GLOB bin_lib_list "${CMAKE_INSTALL_PREFIX}/bin/*" 
                       "${CMAKE_INSTALL_PREFIX}/lib/*"
                       "${CMAKE_INSTALL_PREFIX}/share/dakota/test/*"
                       "${CMAKE_INSTALL_PREFIX}/share/dakota/examples/hopspack/1-var-bnds-only/*"
                       "${CMAKE_INSTALL_PREFIX}/share/dakota/examples/hopspack/2-linear-constraints/*"
                       "${CMAKE_INSTALL_PREFIX}/share/dakota/examples/hopspack/3-degen-linear-constraints/*"
                       "${CMAKE_INSTALL_PREFIX}/share/dakota/examples/hopspack/4-nonlinear-constraints/*"
                       "${CMAKE_INSTALL_PREFIX}/share/dakota/examples/hopspack/5-multi-start/*"
                       "${CMAKE_INSTALL_PREFIX}/share/dakota/examples/script_interfaces/generic/*")

foreach(loader ${bin_lib_list})
  # skip directories and static libs
  if(NOT IS_DIRECTORY "${loader}" AND NOT loader MATCHES "\\.a$")
    execute_process( COMMAND otool -L ${loader}
                     COMMAND awk "FNR > 1 {print $1}"
                     # Omit system libraries and Dakota project libraries
                     COMMAND egrep -v "(^/System|^/usr/lib|^/usr/X11|@rpath)"
                     COMMAND tr "\\n" ";"
                     RESULT_VARIABLE retcode
                     OUTPUT_VARIABLE deps
                     ERROR_QUIET)
    # recode will be nonzero if otool failed, i.e. the file wasn't a
    # mach-o formatted executable or lib.
    if( retcode EQUAL 0 )
       # loop over the dependencies of $loader and change their install names
       foreach(old_dep ${deps})
         get_filename_component(dll_name "${old_dep}" NAME)
         execute_process( COMMAND install_name_tool -change ${old_dep} 
                                      "@rpath/${dll_name}" ${loader}
                          RESULT_VARIABLE retcode
                          ERROR_QUIET )
         if(NOT retcode EQUAL 0)
           message("ERROR: Attempt to modify install name of ${old_dep} in ${loader} failed.")
         endif()
       endforeach()
    endif()
  endif()
endforeach()

