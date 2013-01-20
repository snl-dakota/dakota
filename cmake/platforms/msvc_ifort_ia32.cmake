############################################################################
#
# CMake configuration file for Native Windows, 32-bit MSVC/ifort platform
#
############################################################################

# DO NOT use shared libraries on Windows (yet)
set(BUILD_STATIC_LIBS ON  CACHE BOOL "Build static libs")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "DO NOT Build shared libs!")

set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Use static Boost libs")

