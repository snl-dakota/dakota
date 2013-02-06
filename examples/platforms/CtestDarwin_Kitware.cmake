# This file contains hardwired settings, specific to an SNL MacOSX build host.
# Please DO NOT attempt to use this ctest template/cache-initialization as-is.
# Contributed by collaborator at Kitware - see DAKOTA Trac #4028
# And inspired by "parameterized" ctest file:
#   cmake/2.8.*/share/cmake-2.8/Templates/CTestScript.cmake.in

cmake_minimum_required(VERSION 2.8.4)

set(CTEST_SITE                "$ENV{NODE_NAME}")
set(CTEST_BUILD_NAME          "MacOSX Leopard/GNU 4.3.2 Release")
set(CTEST_NOTES_FILES         "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}")

set(CTEST_DASHBOARD_ROOT      "$ENV{WORKSPACE}")
set(CTEST_SOURCE_DIRECTORY    "${CTEST_DASHBOARD_ROOT}/source")
set(CTEST_BINARY_DIRECTORY    "${CTEST_DASHBOARD_ROOT}/build")

set(CTEST_CMAKE_GENERATOR     "Unix Makefiles")
set(CTEST_BUILD_COMMAND       "make -i -j2" )
set(CTEST_BUILD_CONFIGURATION Release)

CTEST_EMPTY_BINARY_DIRECTORY(${CTEST_BINARY_DIRECTORY})

# Write initial cache.
file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
Boost_INCLUDE_DIR:PATH=/Users/dakota/local/boost.gnu-4.3.2/include
DAKOTA_HAVE_MPI:BOOL=ON
CMAKE_CXX_COMPILER:FILEPATH=$ENV{MPIHOME}/bin/mpicxx
CMAKE_C_COMPILER:FILEPATH=$ENV{MPIHOME}/bin/mpicc
CMAKE_Fortran_COMPILER:FILEPATH=/usr/local/gcc-4.3.2/bin/gfortran
MPI_INCLUDE_PATH:FILEPATH=$ENV{MPIHOME}/include
MPI_LIBRARY:FILEPATH=$ENV{MPIHOME}/lib/libmpi_cxx.dylib
MOTIF_INCLUDE_DIR:PATH=/Users/dakota/local/openmotif-2.2.3/include
MOTIF_LIBRARIES:PATH=/Users/dakota/local/openmotif-2.2.3/lib/libXm.dylib
")

#DAKOTA_PUBLIC_CDASH:BOOL=ON

ctest_start(Nightly)
ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}")
#set(CTEST_DROP_SITE "dakota.sandia.gov")

ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}")
ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}")
ctest_submit()

