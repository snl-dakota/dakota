##############################################################################
#
# CTest Automated Build Variables for Native Windows MSVC/ifort, 32-bit
#
##############################################################################

set( CTEST_BUILD_NAME "dakota_win_msvc_ifort" )

set( DAKOTA_CMAKE_PLATFORM "msvc_ifort_ia32.cmake")
set( DAKOTA_CMAKE_BUILD_TYPE "DakotaDistro.cmake")

set( CTEST_DASHBOARD_ROOT 
     $ENV{WORKSPACE} )
set( CTEST_SOURCE_DIRECTORY
     "${CTEST_DASHBOARD_ROOT}/source")

set( CTEST_CMAKE_GENERATOR "NMake Makefiles" )

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

