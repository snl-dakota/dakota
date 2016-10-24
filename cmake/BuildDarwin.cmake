##############################################################################
#
# CTest Automated Build Variables for Darwin
#
##############################################################################

set( CTEST_BUILD_NAME "dakota_mac" )

set( DAKOTA_CMAKE_PLATFORM "Darwin.cmake")
set( DAKOTA_CMAKE_BUILD_TYPE "DakotaDistro.cmake")

#********* MUST SET CORRECTLY!!! *********** #
# TODO: comment or test and error
#set( CTEST_DASHBOARD_ROOT 
#     "$ENV{HOME}/dakota-devel" )
#set( CTEST_SOURCE_DIRECTORY
#     "${CTEST_DASHBOARD_ROOT}/dakota")
#******************************************* #

set( DAKOTA_CTEST_PROJECT_TAG "Continuous" )
set( CTEST_BUILD_CONFIGURATION Release )

set( DAKOTA_CTEST_REGEXP "dakota_*" )

##############################################################################

##########################################################################
# Set up Internal CMake paths first. Then call automated build file.
# DO NOT CHANGE!!
##########################################################################
include( ${CTEST_SCRIPT_DIRECTORY}/utilities/DakotaSetupBuildEnv.cmake )
include( common_build )
##########################################################################

