##############################################################################
#
# CTest Automated Build Variables for RHEL6
#
##############################################################################

set( CTEST_BUILD_NAME "dakota_rhel6_gnu_ompi" )

set( DAKOTA_CMAKE_PLATFORM "RHEL6.cmake")
set( DAKOTA_CMAKE_BUILD_TYPE "DakotaDistro.cmake")

set( CTEST_DASHBOARD_ROOT 
     "/home/dmvigi/dakota-devel" )
set( CTEST_SOURCE_DIRECTORY
     "${CTEST_DASHBOARD_ROOT}/dakota")

set( DAKOTA_CTEST_PROJECT_TAG "Experimental" )
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

