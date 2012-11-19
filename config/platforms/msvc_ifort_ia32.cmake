############################################################################
#
# CMake configuration file for Native Windows, 32-bit MSVC/ifort platform
#
############################################################################

# BMA: Not sure this is the right place to make these settings...
set( DAKOTA_HAVE_MPI OFF
     CACHE BOOL "MSVC is a serial build" FORCE )

# DO NOT use shared libraries
set(BUILD_STATIC_LIBS ON  CACHE BOOL "Build static libs")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "DO NOT Build shared libs!")

set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Use static Boost libs")
#-DLAPACK_DIR=C:/Dashboards/support/lapack-3.3.1 

