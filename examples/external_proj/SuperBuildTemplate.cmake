##############################################################################
#
# Template CMake Configuration File.
#
##############################################################################
# The following CMake variables represent the minimum set of variables that
# are required to allow Dakota and Trilinos to build as external_projects and
#   * find all prerequisite third party libraries (TPLs)
#   * configure compiler and MPI options
#   * set Dakota install path
#
# Instructions:
# 1. Read Dakota/INSTALL - Source Quick Start to use this template file.
#
# 2. Uncomment CMake variables below ONLY for values you need to change for
#    your platform. Edit variables as needed.
#
#    For example, if you are using a custom install of Boost, installed in
#    /home/me/usr/boost, uncomment both CMake Boost variables  and edit
#    paths:
#       set(BOOST_ROOT
#           "/home/me/usr/boost"
#           CACHE PATH "Use non-standard Boost install" FORCE)
#       set( Boost_NO_SYSTEM_PATHS TRUE
#            CACHE BOOL "Suppress search paths other than BOOST_ROOT" FORCE)
#
#    Save file and exit.
#
# 3. Run CMake (NOTE the atypical "top" source dir)
#      $ cmake $DAK_SRC/external_proj
#
##############################################################################

##############################################################################
# Set BLAS, LAPACK library paths ONLY if in non-standard locations
##############################################################################
#set( BLAS_LIBS 
#      "/usr/lib64"
#      CACHE FILEPATH "Use non-standard BLAS library path" FORCE )
#set( LAPACK_LIBS 
#      "/usr/lib64"
#      CACHE FILEPATH "Use non-standard LAPACK library path" FORCE )

##############################################################################
# Set additional compiler options
# Uncomment and replace <flag> with actual compiler flag, e.g. -xxe4.2
##############################################################################
#set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} <flag>" 
#     CACHE STRING "C Flags my platform" )
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} <flag>"
#     CACHE STRING "CXX Flags for my platform" )
#set( CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} <flag>"
#     CACHE STRING "Fortran Flags for my platform" )

##############################################################################
# Set MPI options
# Recommended practice is to set DAKOTA_HAVE_MPI and set MPI_CXX_COMPILER 
# to a compiler wrapper.
##############################################################################
#set( DAKOTA_HAVE_MPI ON
#     CACHE BOOL "Build with MPI enabled" FORCE)
#set( MPI_CXX_COMPILER "path/to/mpicxx"
#     CACHE FILEPATH "Use MPI compiler wrapper" FORCE)

##############################################################################
# Set Boost path if CMake cannot find your installed version of Boost or
# if you have a custom Boost install location.
##############################################################################
#set(BOOST_ROOT
#    "path/to/custom/Boost/install/directory"
#    CACHE PATH "Use non-standard Boost install" FORCE)
#set( Boost_NO_SYSTEM_PATHS TRUE
#     CACHE BOOL "Suppress search paths other than BOOST_ROOT" FORCE)

##############################################################################
# Customize Trilinos
# e.g. ensure BLAS is consistently "mapped" to vars near the top of this file
##############################################################################
#set( BLAS_LIBRARY_NAMES
#     ""
#     CACHE PATH "See trilinos build docs for BLAS_LIBRARY_NAMES usage" )

##############################################################################
# Customize DAKOTA
##############################################################################
#set( CMAKE_INSTALL_PREFIX
#     "/path/to/Dakota/installation"
#     CACHE PATH "Path to Dakota installation" )

