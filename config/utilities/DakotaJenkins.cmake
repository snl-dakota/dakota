#############################################################################
#
# Jenkins CMake Variables
#
##############################################################################

set( CTEST_BUILD_NAME "$ENV{JOB_NAME}" )
set( CTEST_DASHBOARD_ROOT "$ENV{WORKSPACE}" )
set( CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/build")
set( CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/source")

set( DAKOTA_DO_PACK ON )

