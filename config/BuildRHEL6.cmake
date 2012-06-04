##############################################################################
#
# CTest configuration file for RHEL6 platform
#
##############################################################################

# ****************************************
# ****************************************
# Customize BLAS, LAPACK
# ****************************************

# ****************************************
# Customize compiler/MPI
# ****************************************
set( DAKOTA_HAVE_MPI ON 
     CACHE BOOL "Always build with MPI enabled" FORCE)

set(CMAKE_CXX_COMPILER "$ENV{MPI_BIN}/mpicxx" CACHE FILEPATH
    "Use installed, MPI compiler wrapper" FORCE)

set(MPI_INCLUDE_PATH "$ENV{MPI_INCLUDE}" CACHE FILEPATH
    "Use installed, MPI headers" FORCE)

set(MPI_LIBRARY "$ENV{MPI_LIB}/libmpi_cxx.so" CACHE FILEPATH
    "Use installed, MPI libraries" FORCE)

# ****************************************
# Customize Boost
# ****************************************

# ****************************************
# Customize Trilinos
# ****************************************

# ****************************************
# Customize DAKOTA
# ****************************************

##############################################################################

# ****************************************
# Set Up Automated Build Varialbles
# ****************************************
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

##########################################################################
# Set up Internal CMake paths first. Then call automated build file.
# DO NOT CHANGE!!
##########################################################################
include( ${CTEST_SCRIPT_DIRECTORY}/utilities/DakotaSetupBuildEnv.cmake )
include( common_build )
##########################################################################

