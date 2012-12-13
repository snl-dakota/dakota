# Find the Cygwin DLL dependencies of dakota.exe that live in /bin or
# /lib and install to ${CMAKE_INSTALL_PREFIX}/bin

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

# Get the DLLs only in cygwin\bin or cygwin\lib as a semicolon-separated list
execute_process(
  COMMAND cygcheck.exe ${CMAKE_CURRENT_BINARY_DIR}/dakota.exe 
  COMMAND egrep "cygwin\\\\(bin|lib)\\\\"
  COMMAND tr "\\n" ";"
  OUTPUT_VARIABLE dakota_cygwin_dlls
  )

# Process each DLL and install
foreach(dakota_dll ${dakota_cygwin_dlls})
  # Get a Cygwin unix-style path
  execute_process(
    COMMAND cygpath.exe "${dakota_dll}"
    OUTPUT_VARIABLE dakota_dll
    )
  # Remove leading and trailing whitespace, including newlines
  string(STRIP "${dakota_dll}" dakota_dll)
  dakota_install_dll("${dakota_dll}")
endforeach()

