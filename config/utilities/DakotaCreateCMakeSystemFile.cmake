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

  execute_process(
    COMMAND ${CMAKE_COMMAND} -N --system-information 
    ${_build_path}/dakota_system.out )

  list( APPEND CTEST_NOTES_FILES 
    ${_build_path}/dakota_system.out )
  message( "CTEST_NOTES_FILES: ${CTEST_NOTES_FILES}" )

endfunction()

