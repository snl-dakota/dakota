############################################################################
#
# CMake configuration file for Native Windows, 32-bit MSVC/ifort platform
#
############################################################################

# BMA: Not sure this is the right place to make these settings...
set( DAKOTA_HAVE_MPI OFF
     CACHE BOOL "MSVC is a serial build" FORCE )

# WJB - ToDo:  move these settings to a proper location

set( Boost_INCLUDE_DIR "C:/Dashboards/Support/boost_1_42"
     CACHE PATH "Boost spec should be in host-specific file" FORCE )

set( Boost_LIBRARY_DIRS "C:/Dashboards/Support/boost_1_42/lib"
     CACHE FILEPATH "Boost spec should be in host-specific file" FORCE )

#set( LAPACK_DIR "C:/Dashboards/Support/lapack-3.4.2/build"
set( LAPACK_DIR "C:/Dashboards/Support/lapack-3.3.1"
     CACHE PATH "LAPACK spec should be in host-specific file" FORCE )

############################################################################

# DO NOT use shared libraries on Windows (yet)
set(BUILD_STATIC_LIBS ON  CACHE BOOL "Build static libs")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "DO NOT Build shared libs!")

set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Use static Boost libs")

