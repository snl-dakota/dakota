# Process DAKOTA test results
#
# Processing DAKOTA test results involves
# - Removing previous dakota_[p]diffs.out and dakota_test_results.log files
# - Creating new dakota_[p]diffs.out results file
# - Adding PASS/FAIL/DIFF results to dakota_test_results.log file
#
# process_dakota_test_results( 
#   <build_path>
# )
#
# Processes dakota test results files relative to the <path> specified.
# Thus, this function removes old and creates new: 
#    ${_build_path}/dakota_test_results.log}  
#    ${_build_path}/test/dakota_[p]diffs.out

# ***********************************************************************
# Helper function
# ***********************************************************************
function(create_diff_file _build_path _results_file _diff_filename )
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

# ***********************************************************************
# Function: process_dakota_test_results
# ***********************************************************************
function( process_dakota_test_results _build_path ) 

  # remove expected files; files are relative to ${_build_path}
  set( dakota_results_log "dakota_test_results.log" )
  list( APPEND removeFiles 
    "test/dakota_diffs.out"
    "test/dakota_pdiffs.out"
    "${dakota_results_log}" )

  foreach( file ${removeFiles} )
    message("Processing ${_build_path}/${file}") 
    if ( EXISTS ${_build_path}/${file} )
      message("Deleting ${_build_path}/${file}") 
      execute_process( 
        COMMAND ${CMAKE_COMMAND} -E remove ${file}
	WORKING_DIRECTORY ${_build_path} )
    endif()
  endforeach()

  file( WRITE ${_build_path}/${dakota_results_log} "Test Results:\n" )

  # create dakota_diffs.out
  create_diff_file( ${_build_path} ${dakota_results_log} "dakota_diffs.out" )	

  # create dakota_pdiffs.out
  create_diff_file( ${_build_path} ${dakota_results_log} "dakota_pdiffs.out" )	

endfunction()

# ***********************************************************************
# Function: process_unit_test_results
# ***********************************************************************
function( process_unit_test_results _build_path ) 

  # remove expected files; files are relative to ${_build_path}
  set( unit_test_results_log "unit_test_results.log" )
  list( APPEND removeFiles
    "${unit_test_results_log}" )

  foreach( file ${removeFiles} )
    message("Processing ${_build_path}/${file}")
    if ( EXISTS ${_build_path}/${file} )
      message("Deleting ${_build_path}/${file}")
      execute_process(
        COMMAND ${CMAKE_COMMAND} -E remove ${file}
        WORKING_DIRECTORY ${_build_path} )
    endif()
  endforeach()

  set( subset_name "UnitTest" )
  set( subset_out_filename "unit_test/${subset_name}.out" )

  configure_file(
    ${_build_path}/${subset_out_filename}
    ${_build_path}/${unit_test_results_log} COPYONLY )

  # PASS/FAIL summary for reporting to team
  file( STRINGS ${_build_path}/${subset_out_filename} unitPass REGEX "Passed " )
  file( STRINGS ${_build_path}/${subset_out_filename} unitFail REGEX "Failed " )
  list( LENGTH unitPass passCount )
  list( LENGTH unitFail failCount )

  file( APPEND ${_build_path}/${unit_test_results_log}
    "\nDashboard/Email Reporting.. \n" )
  file( APPEND ${_build_path}/${unit_test_results_log} "${subset_name} Counts:\n" )
  file( APPEND ${_build_path}/${unit_test_results_log} "PASS: ${passCount}\n" )
  file( APPEND ${_build_path}/${unit_test_results_log} "FAIL: ${failCount}\n" )

endfunction()
