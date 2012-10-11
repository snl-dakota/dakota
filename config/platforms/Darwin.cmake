############################################################################
#
# CMake configuration file for Darwin (MacOSX) platform
#
############################################################################

# ToDo - discuss with DMV regarding ictinus specific Boost out of "Darwin"
# BELONGS in ictinus specific file NOT general Darwin file
# Boost base directory

set(Boost_INCLUDE_DIR
  "/Users/dakota/local/boost.gnu-4.3.2/include"
  CACHE PATH "Always use pre-installed Boost1.45 subset" FORCE)

############################################################################
# BLAS, LAPACK libraries assumed to be installed in /usr/lib on Darwin
# WJB: should I leave commented-out here?  (allow override?)
# (Default (Dakota/CMakeLists.txt): CMake will find_library( blas/lapack )
#set( BLAS_LIBS 
#      "/usr/lib/libblas.dylib"
#      CACHE FILEPATH "Use non-standard BLAS library path" FORCE )
#set( LAPACK_LIBS 
#      "/usr/lib/liblapack.dylib"
#      CACHE FILEPATH "Use non-standard LAPACK library path" FORCE )
############################################################################
# WJB - ASAP:  Enable MPI-enabled builds on Mac
# All Unix platforms build DAKOTA with MPI (OFF (for now))
set( DAKOTA_HAVE_MPI OFF
     CACHE BOOL "Always build with MPI enabled" FORCE)

set(BUILD_STATIC_LIBS OFF CACHE BOOL "Set to ON to build static libraries" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Set to ON to build DSO libraries" FORCE)

