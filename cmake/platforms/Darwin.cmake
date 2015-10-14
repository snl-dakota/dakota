############################################################################
#
# CMake configuration file for Darwin (MacOSX) platform
#
############################################################################

############################################################################
# All Unix platforms build DAKOTA with MPI enabled

set( DAKOTA_HAVE_MPI ON
     CACHE BOOL "Always build with MPI enabled")

############################################################################
# Ensure DEFAULT values for static (OFF) and shared-dylibs (ON)

set(BUILD_STATIC_LIBS OFF CACHE BOOL "Set to ON to build static libs" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Set to ON to build DSO libraries" FORCE)

