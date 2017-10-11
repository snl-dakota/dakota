############################################################################
#
# CMake configuration file for Native Windows, 32-bit MSVS/ifort platform
#
############################################################################

# BMA: Not sure this is the right place to make these settings...
set( DAKOTA_HAVE_MPI OFF
     CACHE BOOL "MSVS is a serial build" FORCE )


############################################################################

# DO NOT use shared libraries on Windows (yet)
set(BUILD_STATIC_LIBS ON  CACHE BOOL "Build static libs")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "DO NOT Build shared libs!")

