# Find the Unix so dependencies of dakota, excluding system libraries, 
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

message( "CMAKE_CURRENT_BINARY_DIR (1): ${CMAKE_CURRENT_BINARY_DIR}" ) 
if ( DAKOTA_JENKINS_BUILD OR DEFINED ENV{WORKSPACE} )
  # By convention, all Dakota, jenkins-driven build jobs use a 'build'
  # subdir for clear separation of source and build trees in the WORKSPACE
  set( CMAKE_CURRENT_BINARY_DIR $ENV{WORKSPACE}/build )
elseif ( NOT CMAKE_CURRENT_BINARY_DIR )
  set( CMAKE_CURRENT_BINARY_DIR $ENV{PWD} )
endif()
message( "CMAKE_CURRENT_BINARY_DIR (2): ${CMAKE_CURRENT_BINARY_DIR}" ) 

# ldd may resolve symlinks, do the same for the build tree location
get_filename_component(resolved_build_dir ${CMAKE_CURRENT_BINARY_DIR} REALPATH)

# Get the shared objects excluding system libraries and anything in
# the build tree (as will be installed to lib/) as a
# semicolon-separated list
execute_process(
  COMMAND ldd "${CMAKE_CURRENT_BINARY_DIR}/src/dakota"
  COMMAND awk "/=>/ {print $3}"
  # Omit linux-vdso.so and ld-linux-x86-64.so for example
  COMMAND grep ".so"
  # Omit libs in the build tree
  COMMAND egrep -v "${CMAKE_CURRENT_BINARY_DIR}/.+.so"
  COMMAND egrep -v "${resolved_build_dir}/.+.so"
  # Omit other system libraries
  COMMAND egrep -v "(^/lib|^/usr/lib)"
  COMMAND tr "\\n" ";"
  OUTPUT_VARIABLE dakota_unix_sos
  )

# Ignore empty list elements:
cmake_policy(PUSH)
cmake_policy(SET CMP0007 OLD)
list(REMOVE_DUPLICATES dakota_unix_sos)
cmake_policy(POP)

## Process each DLL and install
foreach(dakota_dll ${dakota_unix_sos})
  dakota_install_dll("${dakota_dll}")
endforeach()
