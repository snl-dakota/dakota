#############################################################################
#
# Common Dashboard CMake Script
#
##############################################################################

#*****************************************************************
# Check for required variables
foreach( v
    CTEST_DASHBOARD_ROOT
    CTEST_BUILD_CONFIGURATION
    DAKOTA_CMAKE_PLATFORM
    DAKOTA_CMAKE_BUILD_TYPE
    )
  if ( NOT ${v} )
    message( FATAL_ERROR "ERROR: ${v} must be defined" )
  endif()
endforeach()

# CTEST_BUILD_NAME is required for all non-Jenkins builds
if ( NOT DAKOTA_JENKINS_BUILD AND NOT CTEST_BUILD_NAME )
  message( FATAL_ERROR "ERROR: CTEST_BUILD_NAME must be defined" )
endif()

#*****************************************************************
# Get|set default CTEST_[SOURCE|BINARY]_DIRECTORY
#   (default: ${CTEST_DASHBOARD_ROOT}/[source|build])

if ( DAKOTA_JENKINS_BUILD )
  set( CTEST_BUILD_NAME "$ENV{JOB_NAME}" )
  set( CTEST_DASHBOARD_ROOT "$ENV{WORKSPACE}" )
  set( CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/build")
  set( CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/source")

else ()
  if ( NOT CTEST_BINARY_DIRECTORY )
    set( CTEST_BINARY_DIRECTORY
    	 "${CTEST_DASHBOARD_ROOT}/build")
  endif()
  if ( NOT CTEST_SOURCE_DIRECTORY )
    set( CTEST_BINARY_DIRECTORY
       	 "${CTEST_DASHBOARD_ROOT}/source" )
  endif()

endif()

#*****************************************************************
# Set CTEST_SITE 
#    (default: hostname)
# and DAKOTA_HOSTFILE
#    (default hostname.cmake)

if ( NOT CTEST_SITE )
  find_program(HOSTNAME hostname)

  #TODO: change to execute_process, verify works in Windows
  exec_program(${HOSTNAME} ARGS -s OUTPUT_VARIABLE hostname)
  string(REGEX REPLACE "[/\\\\+<> #]" "-" hostname "${hostname}")

  set( CTEST_SITE "${hostname}" )
  if ( NOT DAKOTA_CMAKE_HOSTFILE )
    set( DAKOTA_CMAKE_HOSTFILE "${hostname}.cmake" )
  endif()
endif()

#*****************************************************************
# Set DAKOTA_CTEST_PROJET_TAG

if ( NOT DAKOTA_CTEST_PROJET_TAG )
   set( DAKOTA_CTEST_PROJET_TAG "Nightly" )
endif()

#*****************************************************************
# Set CTEST_CMAKE_GENERATOR

if ( NOT CTEST_CMAKE_GENERATOR )
  set( CTEST_CMAKE_GENERATOR "Unix Makefiles" )
endif()

#*****************************************************************
# Set CTEST_BUILD_COMMAND and CTEST_TEST_COMMAND

# First set parallel level if not already set
if ( NOT DAKOTA_CTEST_PARALLEL_LEVEL )
 set( DAKOTA_CTEST_PARALLEL_LEVEL 1 )

  if(EXISTS "/proc/cpuinfo")
    file(STRINGS "/proc/cpuinfo" cpuInfo REGEX "processor" )
    list( LENGTH cpuInfo processorCount )
    if ( processorCount GREATER 1)
       math(EXPR DAKOTA_CTEST_PARALLEL_LEVEL
       		 "${processorCount}*3/4")
    endif()     
  endif()
endif()

if ( NOT CTEST_BUILD_COMMAND )
  set( CTEST_BUILD_COMMAND "make -i -j${DAKOTA_CTEST_PARALLEL_LEVEL}" )
endif()

# ***************************************************************************
# Set CTest Configure command

set(CTEST_CONFIGURE_COMMAND
  "${CMAKE_COMMAND} 
  -DCMAKE_BUILD_TYPE:STRING=${CTEST_BUILD_CONFIGURATION}")
set(CTEST_CONFIGURE_COMMAND
  "${CTEST_CONFIGURE_COMMAND} \"-G${CTEST_CMAKE_GENERATOR}\"")

set_configure_command( ${DAKOTA_CMAKE_PLATFORM} SUBDIR platforms REQUIRED )

if ( DAKOTA_CMAKE_COMPILER ) 
  set_configure_command( ${DAKOTA_CMAKE_COMPILER} SUBDIR compilers )
endif()

set_configure_command( ${DAKOTA_CMAKE_BUILD_TYPE} REQUIRED )

if ( DAKOTA_CMAKE_HOSTFILE )
  set_configure_command( ${DAKOTA_CMAKE_HOSTFILE} SUBDIR platforms )
endif()

set(CTEST_CONFIGURE_COMMAND
  "${CTEST_CONFIGURE_COMMAND} \"${CTEST_SOURCE_DIRECTORY}\"")

#*****************************************************************
# Since this is the filename used by the email notification, this
# cannot be overridden by the user
set( dakotaCtestResultsFile 
     "${CTEST_BINARY_DIRECTORY}/dakota_ctest_results.log" )

##############################################################################
# The work begins...
##############################################################################

#*****************************************************************
# Print summary information.

foreach(v
    CMAKE_MODULE_PATH
    CTEST_SITE
    CTEST_BUILD_NAME
    CTEST_DASHBOARD_ROOT
    CTEST_SOURCE_DIRECTORY
    CTEST_BINARY_DIRECTORY
    CTEST_SCRIPT_DIRECTORY
    CTEST_CMAKE_GENERATOR
    CTEST_BUILD_CONFIGURATION
    CTEST_CONFIGURE_COMMAND
    CTEST_BUILD_COMMAND
    CTEST_TEST_COMMAND
    CTEST_NOTES_FILE
    DAKOTA_CMAKE_PLATFORM
    DAKOTA_CMAKE_COMPILER
    DAKOTA_CMAKE_HOSTFILE
    DAKOTA_CMAKE_BUILD_TYPE
    DAKOTA_CTEST_PROJECT_TAG
    DAKOTA_CTEST_PARALLEL_LEVEL
    DAKOTA_CTEST_REGEXP
    DAKOTA_DEBUG
    DAKOTA_CONFIG_DIR
    DAKOTA_LOCAL_CONFIG_DIR
    dakotaCtestResultsFile
    )
  set(vars "${vars}  ${v}=[${${v}}]\n")
endforeach(v)
if ( DAKOTA_DEBUG )
   file( WRITE ${CTEST_BINARY_DIRECTORY}/dakota_ctest_variables.out ${vars} )
endif()
list( APPEND CTEST_NOTES_FILES 
      ${CTEST_BINARY_DIRECTORY}/dakota_ctest_variables.out )
message("Dashboard script configuration:\n${vars}\n")

# Create a notes file to be submitted to the Dashboard
list( APPEND CTEST_NOTES_FILES ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME} )
create_cmake_system_file( ${CTEST_BINARY_DIRECTORY} )

ctest_start(${DAKOTA_CTEST_PROJECT_TAG})

ctest_configure(RETURN_VALUE ConfigStatus)
message("ctest_configure: cmake return code: ${ConfigStatus}")

# Using file(WRITE...) resets the dakotaCtestResultsFile for each CTest run
file( WRITE ${dakotaCtestResultsFile} "ctest_configure: ${ConfigStatus}\n" )

# Build & test if configuration is successful
if ( ${ConfigStatus} EQUAL 0 )

  ctest_build( RETURN_VALUE BuildStatus
  	       NUMBER_ERRORS NumErr
	       NUMBER_WARNINGS NumWarn)
  message("ctest_build: Build return code: ${BuildStatus}; errors: ${NumErr}, warnings: ${NumWarn}")
  file( APPEND ${dakotaCtestResultsFile} "ctest_build: ${BuildStatus}\n" ) 
  file( APPEND ${dakotaCtestResultsFile} "ctest_build errors: ${NumErr}\n" ) 
  file( APPEND ${dakotaCtestResultsFile} "ctest_build warnings: ${NumWarn}\n" ) 

  # Test if build is successful
  if ( ${BuildStatus} EQUAL 0 )

    if ( DEFINED DAKOTA_CTEST_REGEXP )
      message("DAKOTA_CTESTREGEXP: ${DAKOTA_CTEST_REGEXP}")
      message("DAKOTA_CTEST_PARALLEL_LEVEL: ${DAKOTA_CTEST_PARALLEL_LEVEL} ")	
      ctest_test(
        INCLUDE ${DAKOTA_CTEST_REGEXP}
    	PARALLEL_LEVEL ${DAKOTA_CTEST_PARALLEL_LEVEL}
    	RETURN_VALUE CtestStatus)
    else ()
      message("DAKOTA_CTEST_PARALLEL_LEVEL: ${DAKOTA_CTEST_PARALLEL_LEVEL} ")	
      ctest_test(
        PARALLEL_LEVEL ${DAKOTA_CTEST_PARALLEL_LEVEL}
        RETURN_VALUE CtestStatus)
    endif()
    
    message("ctest_test: Test return code: ${CtestStatus}")
    file( APPEND ${dakotaCtestResultsFile} "ctest_test: ${CtestStatus}\n" ) 

  endif()  # build successful

endif()  # configure successful

if ( DAKOTA_CDASH_SUBMIT )
  ctest_submit(RETURN_VALUE SubmitStatus)
  file( APPEND ${dakotaCtestResultsFile} "ctest_submit: ${SubmitStatus}\n" ) 
endif()

# Post-process dakota test results. Do this even if testing was not
# done to ensure the results from the last build are removed
message("processing test results")
process_dakota_test_results( ${CTEST_BINARY_DIRECTORY} )
message("done processing test results")


