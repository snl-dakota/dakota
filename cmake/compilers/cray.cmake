#############################################################################
#
# CMake configuration file for building on Cray machines
#
############################################################################

set( CTEST_BUILD_NAME "${CTEST_BUILD_NAME}_cray" )

set(CMAKE_C_COMPILER cc)
set(CMAKE_CXX_COMPILER CC)
set(CMAKE_Fortran_COMPILER ftn)