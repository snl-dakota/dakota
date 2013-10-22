# This file contains hardwired settings, specific to an SNL Win7 build host.
# Please DO NOT attempt to use this ctest template/cache-initialization as-is.
# Contributed by collaborator at Kitware - see DAKOTA Trac #4028
# And inspired by "parameterized" ctest file:
#   cmake/2.8.*/share/cmake-2.8/Templates/CTestScript.cmake.in

cmake_minimum_required(VERSION 2.8.4)

set(CTEST_SITE                "$ENV{NODE_NAME}")
#set(CTEST_BUILD_NAME          "Win32-cl_v15.00.x-ifort_11.1-Release")
set(CTEST_BUILD_NAME          "Win32-MSVS_10-ifort_11.1-Release")
set(CTEST_NOTES_FILES         "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}")

set(CTEST_DASHBOARD_ROOT      "$ENV{WORKSPACE}")

set(CTEST_SOURCE_DIRECTORY    "${CTEST_DASHBOARD_ROOT}/source")
set(CTEST_BINARY_DIRECTORY    "${CTEST_DASHBOARD_ROOT}/build")

#set(CTEST_CMAKE_GENERATOR     "NMake Makefiles")
set(CTEST_CMAKE_GENERATOR     "Visual Studio 10")
#set(CTEST_BUILD_COMMAND       "nmake -i " )
set(CTEST_BUILD_CONFIGURATION Release)

CTEST_EMPTY_BINARY_DIRECTORY(${CTEST_BINARY_DIRECTORY})

# Write initial cache.
file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
Boost_INCLUDE_DIR:PATH=C:/sems/boost/1.54.0/include/boost-1_54
Boost_LIBRARY_DIRS:FILEPATH=C:/sems/boost/1.54.0/lib
Boost_USE_STATIC_LIBS:BOOL=ON
LAPACK_DIR:PATH=C:/sems/lapack-3.4.2
CMAKE_BUILD_TYPE:STRING=Release
BUILD_STATIC_LIBS:BOOL=ON
BUILD_SHARED_LIBS:BOOL=OFF
")


#ctest_start(Nightly)
ctest_start(Experimental)
ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}")
# Public posts of builds/tests to benefit external developers?
#set(CTEST_DROP_SITE "dakota.sandia.gov")

ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}")
ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}")
ctest_submit()

