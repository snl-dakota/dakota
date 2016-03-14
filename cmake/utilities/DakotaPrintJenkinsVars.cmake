# Print Jenkins build variables
#
#

function (print_jenkins_vars _build_path )

  if ( EXISTS ${_build_path}/dakota_jenkins.out )
    message("Deleting ${_build_path}/dakota_jenkins.out" ) 
    execute_process( 
      COMMAND ${CMAKE_COMMAND} -E remove dakota_jenkins.out
      WORKING_DIRECTORY ${_build_path} )
  endif()

  # Add header to dakota_system.out
  file( WRITE ${_build_path}/dakota_jenkins.out 
    "****************************************************\n"
    "DAKOTA Jenkins Variables\n"
    "****************************************************\n" )

  # Write Jenkins variables
    # A recent Jenkins update changed BUILD_ID, which used to
    # be the date/time when the build started. This is a 
    # hopefully temporary work-around to get email reports
    # and archiving working again.
    string(TIMESTAMP BUILD_ID "%Y-%m-%d_%H-%M-%S")
    file( APPEND ${_build_path}/dakota_jenkins.out
      "BUILD_NUMBER:           $ENV{BUILD_NUMBER}\n"
      "BUILD_ID:               ${BUILD_ID}\n"
      "JOB_NAME:               $ENV{JOB_NAME}\n"
      "BUILD_TAG:              $ENV{BUILD_TAG}\n"
      "EXECUTOR_NUMBER:        $ENV{EXECUTOR_NUMBER}\n"
      "NODE_NAME:              $ENV{NODE_NAME}\n"
      "NODE_LABELS:            $ENV{NODE_LABELS}\n"
      "WORKSPACE:              $ENV{WORKSPACE}\n"
      "JENKINS_HOME:           $ENV{JENKINS_HOME}\n"
      "JENKINS_URL:            $ENV{JENKINS_URL}\n"
      "BUILD_URL:              $ENV{BUILD_URL}\n"
      "JOB_URL:                $ENV{JOB_URL}\n"
      "\n" )

endfunction()

