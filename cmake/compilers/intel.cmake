############################################################################
#
# CMake configuration file for Intel compiler
#
###########################################################################
#############################################################################
#
# CMake configuration file for Intel compiler
#
############################################################################

set( CTEST_BUILD_NAME "${CTEST_BUILD_NAME}_intel" )

set( CMAKE_C_COMPILER icc )
set( CMAKE_CXX_COMPILER icpc )
set( CMAKE_Fortran_COMPILER ifort )
