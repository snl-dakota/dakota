############################################################################
#
# CMake configuration file for Cygwin platform
#
############################################################################

############################################################################
# BLAS, LAPACK libraries assumed to be installed in /usr/lib64 on RHEL6
#
#set( BLAS_LIBS 
#      "/usr/lib64"
#      CACHE FILEPATH "Use non-standard BLAS library path" FORCE )
#set( LAPACK_LIBS 
#      "/usr/lib64"
#      CACHE FILEPATH "Use non-standard BLAS library path" FORCE )
############################################################################

# BMA: Not sure this is the right place to make these settings...
set( DAKOTA_HAVE_MPI OFF
     CACHE BOOL "Cygwin is serial build")

# Use shared libraries and don't use static
set(BUILD_STATIC_LIBS TRUE  CACHE BOOL "Build static libs?")
set(BUILD_SHARED_LIBS FALSE CACHE BOOL "Build shared libs?")

