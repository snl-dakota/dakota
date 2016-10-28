##############################################################################
#
# CTest Automated Build Variables for Cray XE6
#
##############################################################################

set( CTEST_BUILD_NAME "dakota_crayxc30" )

set( DAKOTA_CMAKE_PLATFORM "CrayXC30.cmake")
set( DAKOTA_CMAKE_BUILD_TYPE "DakotaDistro.cmake")

set( DAKOTA_CTEST_PROJECT_TAG "Continuous" )

set( CTEST_BUILD_CONFIGURATION RelWithDebInfo )
set( DAKOTA_CTEST_REGEXP "dakota_*" )
set( DAKOTA_DEBUG ON )

##############################################################################

##########################################################################
# Set up Internal CMake paths first. Then call automated build file.
# DO NOT CHANGE!!
##########################################################################
include( ${CTEST_SCRIPT_DIRECTORY}/utilities/DakotaSetupBuildEnv.cmake )
include( common_build )
##########################################################################

