include(DakotaCreateDiffFile)

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
  dakota_create_diff_file( ${_build_path} ${dakota_results_log} "dakota_diffs.out" )	

  # create dakota_pdiffs.out
  dakota_create_diff_file( ${_build_path} ${dakota_results_log} "dakota_pdiffs.out" )	

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

  # PASS/FAIL summary for reporting to team.
  # Get the number of fails and total number of tests from the summary line near the end 
  # of the report. The number of passes is the difference.
  file(READ ${_build_path}/${subset_out_filename} unitText)
  string(REGEX MATCH "tests passed, ([0-9]+) tests failed out of ([0-9]+)" unitJunk ${unitText})
  set(failCount ${CMAKE_MATCH_1})
  set(totalCount ${CMAKE_MATCH_2})
  math(EXPR passCount "${totalCount} - ${failCount}")
  file( APPEND ${_build_path}/${unit_test_results_log}
    "\nDashboard/Email Reporting.. \n" )
  file( APPEND ${_build_path}/${unit_test_results_log} "${subset_name} Counts:\n" )
  file( APPEND ${_build_path}/${unit_test_results_log} "PASS: ${passCount}\n" )
  file( APPEND ${_build_path}/${unit_test_results_log} "FAIL: ${failCount}\n" )

endfunction()
