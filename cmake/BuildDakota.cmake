##############################################################################
# Set DAKOTA directories
##############################################################################
set( CMAKE_MODULE_PATH
  ${Dakota_SOURCE_DIR}/cmake
  ${Dakota_SOURCE_DIR}/cmake/platforms
  ${Dakota_SOURCE_DIR}/cmake/compilers
  ${CMAKE_MODULE_PATH} )
message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

##############################################################################
# Define platform. Change the following defininion for alternate platform.
# Valid options are:
#   RHEL6      Red Hat Release 6
#   MacOSX     Mac OS X
#   Intel 
#   IntelMKL   Intel with MKL linear algebra libraries
##############################################################################
include( RHEL6 )

##############################################################################
# Define compiler. Valid options are:
#   "gnu"    GNU compilers
#   "intel"  Intel compilers
#
##############################################################################

#                       *******************************
# If you are not using the default compiler set for your platform, uncomment
# the following line and define compiler set from valid options above.
#                       *******************************
#include( gnu )

#                       *******************************
# For parallel builds, uncomment lines below and define appropriate paths.
# If MPI include and lib directories do not have common base directory,
# see note in Dakota/cmake/DakotaHaveMPI.cmake.
#                       *******************************
#set( DAKOTA_HAVE_MPI ON 
#     CACHE BOOL "Always build with MPI enabled" FORCE)
#set( MPI_INCLUDE_PATH
#     "/path/to/MPI/include"
#     CACHE FILEPATH "Use installed MPI headers" FORCE)
#set( MPI_LIBRARY
#     "/path/to/MPI/lib/libxxx.so"
#     CACHE FILEPATH "Use installed MPI library" FORCE)

##############################################################################
# Define Boost directory and library paths
#
# If CMake Boost probe does not work, uncomment the following lines and
# define appropriate paths.
##############################################################################
#set(BOOST_INCLUDEDIR
#  "/path/to/Boost/include"
#  CACHE PATH "Use Boost installed here" FORCE)

#set(BOOST_LIBRARYDIR
#  "/path/to/Boost/lib"
#  CACHE PATH "Use Boost installed here" FORCE)

##############################################################################
# Define Trilinos directory
#
# By default, DAKOTA will use Trilinos-Teuchos in its source tree. If you
# want to use a system Trilinos, uncomment the following lines and
# define appropriate paths.
##############################################################################
#set( Trilinos_DIR
#      "/path/to/Trilinos/install"
#      CACHE FILEPATH "Path to installed Trilinos" FORCE )
