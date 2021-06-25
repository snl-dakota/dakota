# ***********************************************************************
# Helper function
# ***********************************************************************
function(dakota_create_diff_file _build_path _results_file _diff_filename )
#
# Helper function to create dakota_diffs.out and dakota_pdiffs.out and
# to log results in ${_build_path}/dakota_test_results.log. Assumes these
# file have been deleted by calling function process_dakota_test_results.

  set( diff_file_path "${_build_path}/test" )
  set( diff_file "${diff_file_path}/${_diff_filename}" )
  set( results_file "${_build_path}/${_results_file}" )

  message ("diff_file: ${diff_file}" )
  message ("results_file: ${results_file}" )

  # create list of files named ${_diff_filename} in any subdirectory
  # of ${_build_path}/test
  file( GLOB diff_file_list "${diff_file_path}/*/${_diff_filename}" ) 
  #message ("diff_file_list: [${diff_file_list}]" )

  # sort the diff_file_list
  list( SORT diff_file_list )

  # append the content of each file in diff_file_list to ${diff_file}
  foreach( file ${diff_file_list} )
    message( "Processing ${file}" )
    file( READ ${file} diff_file_info )
    file( APPEND ${diff_file} "${diff_file_info}\n" ) 
  endforeach()

  # count #PASS/FAIL/DIFF
  if ( EXISTS ${diff_file} )
    message("diff_file ${diff_file} exists") 
    file( STRINGS ${diff_file} serialPass REGEX "PASS" )
    file( STRINGS ${diff_file} serialFail REGEX "FAIL" )
    file( STRINGS ${diff_file} serialDiff REGEX "DIFF" )
    list( LENGTH serialPass passCount )
    list( LENGTH serialFail failCount )
    list( LENGTH serialDiff diffCount )

    # print statistics to ${results_file}
    if ( ${_diff_filename} STREQUAL "dakota_diffs.out" )
      set( testType "serial" )
    else()
      set( testType "parallel" )
    endif()

    file( APPEND ${results_file} "${testType} PASS: ${passCount}\n" ) 
    file( APPEND ${results_file} "${testType} FAIL: ${failCount}\n" ) 
    file( APPEND ${results_file} "${testType} DIFF: ${diffCount}\n" ) 
  else()
    message("diff_file ${diff_file} does not exist") 
  endif()

endfunction()
