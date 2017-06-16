#############################################################################
#
# Common Dashboard CMake Script
#
##############################################################################

#*****************************************************************
# Check for required variables
foreach( v
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
message( "CTEST_BUILD_NAME defined, value = ${CTEST_BUILD_NAME}" ) 

# CTEST_DASHBOARD_ROOT is required for all non-Jenkins builds
if ( NOT DAKOTA_JENKINS_BUILD AND NOT CTEST_DASHBOARD_ROOT )
  message( FATAL_ERROR "ERROR: CTEST_DASHBORAD_ROOT must be defined" )
endif()
message( "CTEST_DASHBOARD_ROOT defined, value = ${CTEST_DASHBOARD_ROOT}" ) 

#*****************************************************************
# Error checking on required variables
#*****************************************************************
# Get|set default CTEST_[SOURCE|BINARY]_DIRECTORY
#   (default: ${CTEST_DASHBOARD_ROOT}/[source|build])

if ( DAKOTA_JENKINS_BUILD )
  include( DakotaJenkins ) 
else ()
  if ( NOT CTEST_BINARY_DIRECTORY )
    set( CTEST_BINARY_DIRECTORY
    	 "${CTEST_DASHBOARD_ROOT}/build")
  endif()
  if ( NOT CTEST_SOURCE_DIRECTORY )
    set( CTEST_SOURCE_DIRECTORY
       	 "${CTEST_DASHBOARD_ROOT}/source" )
  endif()

endif()

if ( NOT DAKOTA_TEST_SBATCH )
  set( DAKOTA_TEST_SBATCH OFF )
endif()

if ( NOT DAKOTA_TEST_MSUB )
  set( DAKOTA_TEST_MSUB OFF )
endif()

if ( NOT DEFINED DAKOTA_DO_UNIT )
  set( DAKOTA_DO_UNIT ON )
endif()

if ( NOT DEFINED DAKOTA_DO_TEST )
  set( DAKOTA_DO_TEST ON )
endif()

if ( NOT DEFINED DAKOTA_DO_PACK )
  set( DAKOTA_DO_PACK ON )
endif()

#*****************************************************************
# Set CTEST_SITE and DAKOTA_CMAKE_HOSTFILE 
#    (default: hostname and hostname.cmake )

if ( NOT CTEST_SITE )
  if ( DEFINED ENV{SNLSYSTEM} )
    set( hostname "$ENV{SNLSYSTEM}" )

  elseif( DEFINED ENV{NODE_NAME} )
    set( hostname "$ENV{NODE_NAME}" )
    string( REGEX REPLACE "~dakota_" "" hostname "${hostname}" )

  else()
    # fall-through to previous implementation (does NOT rely on ENV hashing)

    find_program( HOSTNAME hostname )

    # TODO: change to execute_process, verify works in Windows
    #exec_program(${HOSTNAME} OUTPUT_VARIABLE hostname)
    #string(REGEX REPLACE "[/\\\\+<> #]" "-" hostname "${hostname}")
    execute_process( COMMAND ${HOSTNAME} OUTPUT_VARIABLE hostname )
    string( REGEX REPLACE "([^.]+).*" "\\1" hostname "${hostname}" )
  endif()

  set( CTEST_SITE "${hostname}" )

  if ( NOT DAKOTA_CMAKE_HOSTFILE )
    set( DAKOTA_CMAKE_HOSTFILE "${hostname}.cmake" )
  endif()
endif()

#*****************************************************************
# Set DAKOTA_CMAKE_EXTRA_ARGS
# Assumes extra args are in format
# -D CMAKE_ARG1=value1 -D CMAKE_ARG2=value2

set( DAKOTA_CMAKE_EXTRA_ARGS "$ENV{DAKOTA_CMAKE_EXTRA_ARGS}" )
message( "DAKOTA_CMAKE_EXTRA_ARGS = ${DAKOTA_CMAKE_EXTRA_ARGS}" )

#*****************************************************************
# Set DAKOTA_CTEST_PROJECT_TAG

if ( NOT DAKOTA_CTEST_PROJECT_TAG )
   set( DAKOTA_CTEST_PROJECT_TAG "Nightly" )
endif()

#*****************************************************************
# Set CTEST_CMAKE_GENERATOR

if ( NOT CTEST_CMAKE_GENERATOR )
  set( CTEST_CMAKE_GENERATOR "Unix Makefiles" )
endif()
message( "CTEST_CMAKE_GENERATOR = ${CTEST_CMAKE_GENERATOR}" ) 

#*****************************************************************
# Set CTEST_BUILD_COMMAND and CTEST_TEST_COMMAND

# Set parallel level for building (MAKE) and testing (CTEST) 
# if not already set
if ( NOT DAKOTA_MAKE_PARALLEL_LEVEL )

 # Linux
  if(EXISTS "/proc/cpuinfo")
    file(STRINGS "/proc/cpuinfo" cpuInfo REGEX "processor" )
    list( LENGTH cpuInfo processorCount )

  # Mac
  elseif(APPLE)
    set(sysctl_cmd sysctl)
    execute_process(COMMAND ${sysctl_cmd} -n hw.ncpu 
                      OUTPUT_VARIABLE processorCount)

  # Windows
  elseif(WIN32)
    set(processorCount "$ENV{NUMBER_OF_PROCESSORS}")

  # platform unknown
  else()
    set(processorCount 1)
  endif()

  # Increase make parallel level if possible
  if ( processorCount GREATER 1)
    math(EXPR DAKOTA_MAKE_PARALLEL_LEVEL
        "${processorCount}*3/4")
  else()
    set( DAKOTA_MAKE_PARALLEL_LEVEL 1 )
  endif()

endif()

if ( NOT DAKOTA_CTEST_PARALLEL_LEVEL )
  set( DAKOTA_CTEST_PARALLEL_LEVEL ${DAKOTA_MAKE_PARALLEL_LEVEL} )
endif()

if ( NOT CTEST_BUILD_COMMAND )
  set( CTEST_BUILD_COMMAND "make -j${DAKOTA_MAKE_PARALLEL_LEVEL}" )
endif()

# ***************************************************************************
# Set CTest Configure command

set(CTEST_CONFIGURE_COMMAND
  "${CMAKE_COMMAND} 
  -DCMAKE_BUILD_TYPE:STRING=${CTEST_BUILD_CONFIGURATION}")

if ( DAKOTA_CMAKE_EXTRA_ARGS )
  set( CTEST_CONFIGURE_COMMAND
       "${CTEST_CONFIGURE_COMMAND} ${DAKOTA_CMAKE_EXTRA_ARGS}")
endif()

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
# cannot be overridden by the user. Change filename here.
set( dakotaCtestResultsFile 
     "${CTEST_BINARY_DIRECTORY}/dakota_ctest_results.log" )

##############################################################################
# The work begins...
##############################################################################

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

  if ( DAKOTA_DO_UNIT AND ${BuildStatus} EQUAL 0 )
    include( "${CTEST_SOURCE_DIRECTORY}/CTestConfig.cmake" )

    set( unit_test_subset ${CTEST_PROJECT_SUBPROJECTS} )
    set_property( GLOBAL PROPERTY SubProject ${unit_test_subset} )
    set_property( GLOBAL PROPERTY Label ${unit_test_subset} )
    message( "ctest_test: Verifying subset tests by LABEL: ${unit_test_subset}\n" )

    execute_process( COMMAND ${CMAKE_CTEST_COMMAND}
      -L ${unit_test_subset}
      -O ${CTEST_BINARY_DIRECTORY}/unit_test/${unit_test_subset}.out
      WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}
      RESULT_VARIABLE CtestStatus
      )

    file( APPEND ${dakotaCtestResultsFile} "ctest unit_test failures: ${CtestStatus}\n" ) 

    if ( DAKOTA_CDASH_SUBMIT )
      ctest_submit(PARTS Build Test RETURN_VALUE SubmitStatus)
    endif()  # DAKOTA_CDASH_SUBMIT

  endif()  # DAKOTA_DO_UNIT

  # Perform tests if requested and build is successful
  if ( DAKOTA_DO_TEST AND ${BuildStatus} EQUAL 0 )

    # default tests run are dakota_*
    if ( NOT DEFINED DAKOTA_CTEST_REGEXP )
      set( DAKOTA_CTEST_REGEXP "dakota_*" )
    endif()
    message("DAKOTA_CTESTREGEXP: ${DAKOTA_CTEST_REGEXP}")
    message("DAKOTA_CTEST_PARALLEL_LEVEL: ${DAKOTA_CTEST_PARALLEL_LEVEL} ")	

    # Remove results files before testing commences to avoid stale reporting;
    # process_dakota_test_results would remove some of these, but be aggressive
    file(GLOB diffs2delete
     ${CTEST_BINARY_DIRECTORY}/test/dakota_*/dakota_diffs.out 
     ${CTEST_BINARY_DIRECTORY}/test/pdakota_*/dakota_pdiffs.out
     )
    file(REMOVE ${diffs2delete}
      ${CTEST_BINARY_DIRECTORY}/test/dakota_diffs.out 
      ${CTEST_BINARY_DIRECTORY}/test/dakota_pdiffs.out
      ${CTEST_BINARY_DIRECTORY}/test/dakota_results.log
      )

    if (DAKOTA_TEST_SBATCH)
      # Generate files needed to submit tests to queue
      configure_file(
	${CTEST_SOURCE_DIRECTORY}/local/cmake/utilities/dakota_sbatch.sh.in
	${CTEST_BINARY_DIRECTORY}/dakota_sbatch.sh @ONLY)
      configure_file(
	${CTEST_SOURCE_DIRECTORY}/local/cmake/utilities/dakota_tests.sbatch.in
	${CTEST_BINARY_DIRECTORY}/dakota_tests.sbatch @ONLY)
      configure_file(
	${CTEST_SOURCE_DIRECTORY}/local/cmake/utilities/dakota_tests.cmake.in
	${CTEST_BINARY_DIRECTORY}/dakota_tests.cmake)
      # TODO: Propagate the exit code
      message("Submitting tests to batch system.")
      message("CTEST_BINARY_DIRECTORY: ${CTEST_BINARY_DIRECTORY}")
      execute_process(COMMAND ${CTEST_BINARY_DIRECTORY}/dakota_sbatch.sh 
	WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}
	RESULT_VARIABLE CtestStatus)
    elseif (DAKOTA_TEST_MSUB)
      # Generate files needed to submit tests to queue
      configure_file(
	${CTEST_SOURCE_DIRECTORY}/local/cmake/utilities/dakota_msub.sh.in
	${CTEST_BINARY_DIRECTORY}/dakota_msub.sh @ONLY)
      configure_file(
	${CTEST_SOURCE_DIRECTORY}/local/cmake/utilities/dakota_tests.msub.in
	${CTEST_BINARY_DIRECTORY}/dakota_tests.msub @ONLY)
      configure_file(
	${CTEST_SOURCE_DIRECTORY}/local/cmake/utilities/dakota_tests.cmake.in
	${CTEST_BINARY_DIRECTORY}/dakota_tests.cmake)
      # TODO: Propagate the exit code
      message("Submitting tests to batch system.")
      execute_process(COMMAND ${CTEST_BINARY_DIRECTORY}/dakota_msub.sh 
	WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}
	RESULT_VARIABLE CtestStatus)
    else()
      ctest_test(
	INCLUDE ${DAKOTA_CTEST_REGEXP}
	PARALLEL_LEVEL ${DAKOTA_CTEST_PARALLEL_LEVEL}
	RETURN_VALUE CtestStatus 
	)
    endif()
    
    message("ctest_test: Test return code: ${CtestStatus}")
    file( APPEND ${dakotaCtestResultsFile} "ctest_test: ${CtestStatus}\n" ) 

  endif()  # perform tests

endif()  # configure successful

if ( DAKOTA_CDASH_SUBMIT )
  ctest_submit(RETURN_VALUE SubmitStatus)
  file( APPEND ${dakotaCtestResultsFile} "ctest_submit: ${SubmitStatus}\n" ) 
endif()

# Post-process unit and dakota test results. Do this even if testing was not
# done to ensure the results from the last build are removed
message("processing test results")
process_unit_test_results( ${CTEST_BINARY_DIRECTORY} )
process_dakota_test_results( ${CTEST_BINARY_DIRECTORY} )
message("done processing test results")

# Enable packing if requested, regardless of status
# (occasionally, need to download the resulting package and test manually)
if ( DAKOTA_DO_PACK )
  if ( EXISTS ${CTEST_BINARY_DIRECTORY}/CPackConfig.cmake )
    #ctest_build(TARGET package APPEND) - WJB: too bad no ctest_package function
    execute_process(COMMAND ${CMAKE_CPACK_COMMAND}
      WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}
      )
  endif() # CPackConfig.cmake exists

  # WJB- ToDo: Assess the need for another variable, DAKOTA_DO_SOURCE_PACK
  #            PROBABLY is needed since only need to create source tarball once

  
  if ( EXISTS ${CTEST_BINARY_DIRECTORY}/CPackSourceConfig.cmake )
    execute_process(COMMAND ${CMAKE_CPACK_COMMAND}
      --config ${CTEST_BINARY_DIRECTORY}/CPackSourceConfig.cmake
      WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}
      )
  endif() # CPackSourceConfig.cmake exists

  # Create the documentation pkg (only if ENABLE_DAKOTA_DOCS is TRUE in cache)

  file( STRINGS ${CTEST_BINARY_DIRECTORY}/CMakeCache.txt
        ENABLE_DAKOTA_DOCS_KV_PAIR REGEX "^ENABLE_DAKOTA_DOCS:BOOL=(.*)$" )
  string( REGEX REPLACE "^ENABLE_DAKOTA_DOCS:BOOL=(.*)$" "\\1"
          DAKOTA_BUILD_DOCS "${ENABLE_DAKOTA_DOCS_KV_PAIR}" )

  if ( DAKOTA_BUILD_DOCS )
    #message( "ctest_build - Building: ${CMAKE_COMMAND} --target package_docs" )
    execute_process( COMMAND ${CMAKE_COMMAND} --build ${CTEST_BINARY_DIRECTORY}
                     --target package_docs )
  endif() # DAKOTA_BUILD_DOCS
endif() # DAKOTA_DO_PACK


##############################################################################
# Print all data
##############################################################################

#*****************************************************************
# FILE: dakota_jenkins.out
if ( DAKOTA_JENKINS_BUILD )
  include( DakotaPrintJenkinsVars )
  print_jenkins_vars( ${CTEST_BINARY_DIRECTORY} )
endif()

#*****************************************************************
# FILE: dakota_system.out
create_cmake_system_file( ${CTEST_BINARY_DIRECTORY} )

#*****************************************************************
# FILE: dakota_ctest_variables.out

# Set TAG
if ( EXISTS ${CTEST_BINARY_DIRECTORY}/Testing/TAG )
  file( STRINGS ${CTEST_BINARY_DIRECTORY}/Testing/TAG taginfo 
    LIMIT_COUNT 1 )
  set( DAKOTA_CTEST_TAG ${taginfo} )
endif()

# Set CTEST_NOTES_FILES
# Create empty file for next step. This file will be overwritten with
# next commands
file( WRITE ${CTEST_BINARY_DIRECTORY}/dakota_ctest_variables.out "" )

foreach(v
    dakota_ctest_results.log
    dakota_test_results.log
    dakota_ctest_variables.out
    dakota_jenkins.out
    dakota_system.out
)
  if ( EXISTS ${CTEST_BINARY_DIRECTORY}/${v} )
    list( APPEND CTEST_NOTES_FILES ${CTEST_BINARY_DIRECTORY}/${v} )
  endif()
endforeach()

# Print all variables to dakota_ctest_variables.out
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
    CTEST_NOTES_FILES
    DAKOTA_CDASH_SUBMIT
    DAKOTA_CMAKE_PLATFORM
    DAKOTA_CMAKE_COMPILER
    DAKOTA_CMAKE_HOSTFILE
    DAKOTA_CMAKE_BUILD_TYPE
    DAKOTA_CTEST_PROJECT_TAG
    DAKOTA_CTEST_PARALLEL_LEVEL
    DAKOTA_MAKE_PARALLEL_LEVEL
    DAKOTA_CTEST_REGEXP
    DAKOTA_CTEST_TAG
    DAKOTA_DO_TEST
    DAKOTA_TEST_SBATCH
    DAKOTA_DO_PACK
    DAKOTA_BUILD_DOCS
    DAKOTA_CONFIG_DIR
    DAKOTA_LOCAL_CONFIG_DIR
    dakotaCtestResultsFile
    )
  set(vars "${vars}  ${v}=[${${v}}]\n")
endforeach(v)

file( WRITE ${CTEST_BINARY_DIRECTORY}/dakota_ctest_variables.out ${vars} )
message("Dashboard script configuration:\n${vars}\n")

