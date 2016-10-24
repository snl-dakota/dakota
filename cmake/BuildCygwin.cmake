##############################################################################
#
# CTest Automated Build Variables for RHEL6
#
##############################################################################

set( CTEST_BUILD_NAME "dakota_cygwin_gnu_serial" )

set( DAKOTA_CMAKE_PLATFORM "Cygwin.cmake")
set( DAKOTA_CMAKE_BUILD_TYPE "DakotaDistro.cmake")

set( CTEST_DASHBOARD_ROOT 
     $ENV{WORKSPACE} )
set( CTEST_SOURCE_DIRECTORY
     "${CTEST_DASHBOARD_ROOT}/dakota")

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

