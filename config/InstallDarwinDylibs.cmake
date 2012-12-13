# Find the Darwin dylib dependencies of dakota, excluding system libraries, 
# and install to ${CMAKE_INSTALL_PREFIX}/bin

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


# Get the dylibs excluding system libraries as a semicolon-separated list
execute_process(
  COMMAND otool -L "${CMAKE_CURRENT_BINARY_DIR}/dakota"
  COMMAND awk "FNR > 1 {print $1}"
  COMMAND egrep -v "(^/System|^/usr/lib|^/usr/X11)"
  COMMAND tr "\\n" ";"
  OUTPUT_VARIABLE dakota_darwin_dylibs
  )

# Get the secondary dylibs of the dylibs
foreach(pri_lib ${dakota_darwin_dylibs})
  execute_process(
    COMMAND otool -L "${pri_lib}"
    COMMAND awk "FNR > 1 {print $1}"
    COMMAND egrep -v "(^/System|^/usr/lib|^/usr/X11)"
    COMMAND tr "\\n" ";"
    OUTPUT_VARIABLE dakota_secondary_dylibs
    )
  list(APPEND dakota_darwin_dylibs ${dakota_secondary_dylibs})
endforeach()

# Ignore empty list elements:
cmake_policy(PUSH)
cmake_policy(SET CMP0007 OLD)
list(REMOVE_DUPLICATES dakota_darwin_dylibs)
cmake_policy(POP)

# Process each DLL and install
foreach(dakota_dll ${dakota_darwin_dylibs})
  dakota_install_dll("${dakota_dll}")
endforeach()
