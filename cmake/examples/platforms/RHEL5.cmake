############################################################################
#
# CMake configuration file for RHEL5 platform
#
############################################################################

############################################################################
# BLAS, LAPACK libraries assumed to be installed in /usr/lib64 on RHEL5
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

