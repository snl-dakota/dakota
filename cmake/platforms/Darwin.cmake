############################################################################
#
# CMake configuration file for Darwin (MacOSX) platform
#
############################################################################

#set(Boost_INCLUDE_DIR
#  "/Users/dakota/local/boost.gnu-4.3.2/include"
#  CACHE PATH "Always use pre-installed Boost1.45 subset" FORCE)

############################################################################
# All Unix platforms build DAKOTA with MPI enabled

set( DAKOTA_HAVE_MPI ON
     CACHE BOOL "Always build with MPI enabled" FORCE)

############################################################################
# Ensure DEFAULT values for static (OFF) and shared-dylibs (ON)

set(BUILD_STATIC_LIBS OFF CACHE BOOL "Set to ON to build static libs" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Set to ON to build DSO libraries" FORCE)

