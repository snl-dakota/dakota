##############################################################################
#
# CTest Automated Build Variables for Native Windows MSVC/ifort, 32-bit
#
##############################################################################

set( CTEST_BUILD_NAME "dakota_win_msvs_ifort" )

set( DAKOTA_CMAKE_PLATFORM "win_msvs_ifort.cmake")
set( DAKOTA_CMAKE_BUILD_TYPE "DakotaDistroStatic.cmake")

set( CTEST_DASHBOARD_ROOT 
     $ENV{WORKSPACE} )
set( CTEST_SOURCE_DIRECTORY
     "${CTEST_DASHBOARD_ROOT}/source")

set( CTEST_CMAKE_GENERATOR "Visual Studio 14 2015" )

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

