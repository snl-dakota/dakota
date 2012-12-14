# Create CDash Notes file
#
#

#TODO: make ctest_file a list of files, i.e. ctest_file_list and
#      change second section into a foreach( file ${ctest_file_list} )
#      This allows for a number of files to be added to 

function (create_cmake_system_file _build_path )

  if ( EXISTS ${_build_path}/dakota_system.out )
    message("Deleting ${_build_path}/dakota_system.out" ) 
    execute_process( 
      COMMAND ${CMAKE_COMMAND} -E remove dakota_system.out
      WORKING_DIRECTORY ${_build_path} )
  endif()

  # Add header to dakota_system.out
  file( WRITE ${_build_path}/dakota_system.out 
    "****************************************************\n"
    "DAKOTA CMake System Information\n"
    "****************************************************\n" )

  # Write system information
    file( APPEND ${_build_path}/dakota_system.out
      "System:            ${CMAKE_SYSTEM_NAME}\n"
      "Version:           ${CMAKE_SYSTEM_VERSION}\n"
      "Processor:         ${CMAKE_SYSTEM_PROCESSOR}\n"
      "\n" )

  # Add command line info to dakota_system.out
  if ( EXISTS ${_build_path}/dakota_system.out.tmp )

    # Add header to dakota_system.out
    file( APPEND ${_build_path}/dakota_system.out 
      "****************************************************\n"
      "* ${cmdline_sysinfo_call}\n"
      "****************************************************\n" )

    # Add results of function call
    file( READ ${_build_path}/dakota_system.out.tmp cmdline_sysinfo_results )
    file( APPEND ${_build_path}/dakota_system.out ${cmdline_sysinfo_results} )

#    execute_process( 
#      COMMAND ${CMAKE_COMMAND} -E remove dakota_system.out.tmp
#      WORKING_DIRECTORY ${_build_path} )

  endif()

  # Add CMakeCache.txt to dakota_system.out
  if ( EXISTS ${_build_path}/CMakeCache.txt )

    # Add header to dakota_system.out
    file( APPEND ${_build_path}/dakota_system.out 
      "\n"
      "****************************************************\n"
      "CMakeCache.txt\n"
      "****************************************************\n" )

    # Add file contents
    file( READ ${_build_path}/CMakeCache.txt cache_file )
    file( APPEND ${_build_path}/dakota_system.out ${cache_file} )

  endif()

  # Add cmake_system_info to dakota_system.out
  execute_process(
    COMMAND ${CMAKE_COMMAND} -N --system-information 
    ${_build_path}/dakota_system.out.tmp )

  if ( EXISTS ${_build_path}/dakota_system.out.tmp )

    # Add header to dakota_system.out
    file( APPEND ${_build_path}/dakota_system.out 
      "\n"
      "****************************************************\n"
      "CMake System Information: cmake --system-information\n"
      "****************************************************\n" )

    file( READ ${_build_path}/dakota_system.out.tmp sysinfo )
    file( APPEND ${_build_path}/dakota_system.out ${sysinfo} )

    execute_process( 
      COMMAND ${CMAKE_COMMAND} -E remove dakota_system.out.tmp
      WORKING_DIRECTORY ${_build_path} )

  endif()

endfunction()

