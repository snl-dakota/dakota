##############################################################################
#
# CTest Automated Build Variables for Redhat Enterprise Linux release 5, RHEL5
#
##############################################################################

set( CTEST_BUILD_NAME "dakota_rhel5" )

set( DAKOTA_CMAKE_PLATFORM "RHEL5.cmake")
set( DAKOTA_CMAKE_BUILD_TYPE "DakotaDistro.cmake")

set( DAKOTA_CTEST_PROJECT_TAG "Continuous" )
set( CTEST_BUILD_CONFIGURATION Release )

##############################################################################

##########################################################################
# Set up Internal CMake paths first. Then call automated build file.
# DO NOT CHANGE!!
##########################################################################
include( ${CTEST_SCRIPT_DIRECTORY}/utilities/DakotaSetupBuildEnv.cmake )
include( common_build )
##########################################################################

