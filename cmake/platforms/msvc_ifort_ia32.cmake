############################################################################
#
# CMake configuration file for Native Windows, 32-bit MSVC/ifort platform
#
############################################################################

# BMA: Not sure this is the right place to make these settings...
set( DAKOTA_HAVE_MPI OFF
     CACHE BOOL "MSVC is a serial build" FORCE )

# WJB - ToDo:  move these settings to a proper host-specific location

# Path to installed Boost
#set (BOOST_ROOT "C:/sems/boost/1.54.0" CACHE PATH
#     "Use SEMS installed Boost 1.54.0")

# Path to lapack-config.cmake in the installed location
set( LAPACK_DIR "C:/sems/lapack/3.4.2/lib/cmake/lapack-3.4.2"
     CACHE PATH "Use SEMS installed LAPACK 3.4.2" FORCE )

############################################################################

# WJB - ToDo:  come up with a better way ("custom" flags are undesirable)
# WJB - NOTE:  desperate measures here -- hard code path to boost on steinman
#####  "${CMAKE_EXE_LINKER_FLAGS} /FORCE:MULTIPLE -LIBPATH:C:/sems/boost/1.54.0/lib"
set( CMAKE_EXE_LINKER_FLAGS
  "${CMAKE_EXE_LINKER_FLAGS} /FORCE:MULTIPLE"
  CACHE STRING "Linker FORCE flag needed on aday and steinman"
  FORCE )

# DO NOT use shared libraries on Windows (yet)
set(BUILD_STATIC_LIBS ON  CACHE BOOL "Build static libs")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "DO NOT Build shared libs!")

# WB - ToDo:  investigate why Boost STATIC libs are a problem on steinman
#set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Use static Boost libs")

