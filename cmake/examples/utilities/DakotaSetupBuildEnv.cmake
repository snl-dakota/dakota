# DAKOTA directory structure assumption:
# {Dakota-source}
# |- cmake                         (directory)
# |  |- compilers                  (directory)
# |  |- platforms                  (directory)
# |  |- utilities                  (directory)
# |  |- Build<Plaform1>.cmake      (file)
# |  |- Build<Plaform2>.cmake      (file)
# |- local                         (directory)
# |  |- cmake                      (directory)
# |  |  |- compilers               (directory)
# |  |  |- platforms               (directory)
# |  |  |- utilities               (directory)
# |  |  |- Build<Plaform1>.cmake   (file)
# |  |  |- Build<Plaform2>.cmake   (file)
# |  |  |- Build<Plaform2>.sh      (file)

message( "Reading Script: ${CMAKE_CURRENT_LIST_FILE}" )

if ( NOT DAKOTA_PATHS_SET )
  message( "Setting DAKOTA configuration paths..." )

  if ( ${CTEST_SCRIPT_DIRECTORY} MATCHES "/local/cmake" ) 
    set( DAKOTA_LOCAL_CONFIG_DIR ${CTEST_SCRIPT_DIRECTORY} )
    get_filename_component( scriptParent ${CTEST_SCRIPT_DIRECTORY} PATH ) 
    get_filename_component( dakotaSrc ${scriptParent} PATH )
    set( DAKOTA_CONFIG_DIR ${dakotaSrc}/cmake )
  else()
    set( DAKOTA_CONFIG_DIR ${CTEST_SCRIPT_DIRECTORY} )
    get_filename_component( dakotaSrc ${CTEST_SCRIPT_DIRECTORY} PATH )
    set( DAKOTA_LOCAL_CONFIG_DIR ${dakotaSrc}/local/cmake )
  endif()

  foreach( i
      ${DAKOTA_CONFIG_DIR}
      ${DAKOTA_LOCAL_CONFIG_DIR} )
    set( CMAKE_MODULE_PATH
        ${i}
        ${i}/compilers
      	${i}/platforms
      	${i}/utilities
      	${CMAKE_MODULE_PATH}
      	)
  endforeach()

  # We are using SEMS tools. Add this path as well
  set( CMAKE_MODULE_PATH
       ${DAKOTA_CONFIG_DIR}/semsCMake
       ${CMAKE_MODULE_PATH} )

  set( DAKOTA_PATHS_SET ON )
else()
  message( "DAKOTA configuration paths previously set" )
endif()

#message( "  CMAKE_MODULE_PATH: [${CMAKE_MODULE_PATH}]" )
#message("DAKOTA_CONFIG_DIR: ${DAKOTA_CONFIG_DIR}")
#message("DAKOTA_LOCAL_CONFIG_DIR: ${DAKOTA_LOCAL_CONFIG_DIR}")

include( DakotaProcessTestResults RESULT_VARIABLE includeResult )
#message( "1. ${includeResult}" )
include( DakotaCreateCMakeSystemFile RESULT_VARIABLE includeResult )
#message( "2. ${includeResult}" )
include( DakotaAddConfigurationFiles RESULT_VARIABLE includeResult )
#message( "3. ${includeResult}" )
include( CMakeParseArguments RESULT_VARIABLE includeResult )
#message( "4. ${includeResult}" )
