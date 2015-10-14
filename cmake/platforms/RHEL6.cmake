############################################################################
#
# CMake configuration file for RHEL6 platform
#
############################################################################


############################################################################
# CMake 2.8.6 has problems with RHEL6/Boost -- the following is a workaround
#
set( Boost_NO_BOOST_CMAKE ON 
     CACHE BOOL "Obtain desired behavior on RHEL6" FORCE)
#
# Boost_NO_BOOST_CMAKE
#     Do not do a find_package call in config mode before searching
#     for a regular boost install. This will avoid finding boost-cmake
#     installs. Defaults to OFF.                 [Since CMake 2.8.6]
############################################################################

set( Boost_NO_SYSTEM_PATHS ON
     CACHE BOOL "DO NOT fallback to Boost-1.41 installed on RHEL6" FORCE)

############################################################################
# BLAS, LAPACK libraries assumed to be installed in /usr/lib64 on RHEL6
# (Default (dakota/CMakeLists.txt): CMake will find_library( blas/lapack )
#set( BLAS_LIBS 
#      "/usr/lib64"
#      CACHE FILEPATH "Use non-standard BLAS library path" FORCE )
#set( LAPACK_LIBS 
#      "/usr/lib64"
#      CACHE FILEPATH "Use non-standard BLAS library path" FORCE )
############################################################################
# All Unix platforms build DAKOTA with MPI
set( DAKOTA_HAVE_MPI ON
     CACHE BOOL "Always build with MPI enabled")

