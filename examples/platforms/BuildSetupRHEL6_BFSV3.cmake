# Assume a "comparably equipped" RHEL6 COE build host
#   - We compile with the stock gcc-4.4.6 
#   - Install linear algebra RPMS:  blas blas-devel lapack lapack-devel 
#
# Refer to wiki for more info:
#   - https://software.sandia.gov/trac/dakota/wiki/CMakeFAQ
#
# Use a CMakeified version of Boost with Filesystem VERSION 3 default
#   - http://gitorious.org/~denisarnaud/boost/denisarnauds-cmake/trees/cmake-1.48.0
#
##############################################################################
#
# EXAMPLE BuildSetup.cmake file for DAKOTA (GNU-4.4.6/RHEL6/Boost.FS_V3)
#
##############################################################################

set(Boost_NO_SYSTEM_PATHS ON CACHE BOOL
    "Do NOT use system boost for when linking with Boost.Filesytem V3" FORCE)
 
set(BOOST_INCLUDEDIR
    "/Net/dakota/utils/Linux_x86_64/boost_1.48.gnu-4.4.6/include/boost-1.48.0"
    CACHE PATH "CMakeified Boost-1.48" FORCE)

set(BOOST_LIBRARYDIR
    "/Net/dakota/utils/Linux_x86_64/boost_1.48.gnu-4.4.6/lib/boost-1.48.0"
    CACHE PATH "CMakeified Boost-1.48" FORCE)

set(Boost_USE_MULTITHREADED OFF CACHE BOOL
    "Do NOT use -mt variant when linking with Boost.Filesytem V3" FORCE)

set(Boost_USE_STATIC OFF CACHE BOOL
    "Do NOT use -static variant when linking with Boost.Filesytem V3" FORCE)

set(Boost_DEBUG OFF CACHE BOOL
    "Do NOT use -d variant when linking with Boost.Filesytem V3" FORCE)

##############################################################################
# Enforce shared library build for DAKOTA and all of its TPLs
##############################################################################

set(BUILD_SHARED_LIBS ON CACHE BOOL "Set to ON to build DSO libraries" FORCE)
set(BUILD_STATIC_LIBS OFF CACHE BOOL "Save time/space -- NO static libs" FORCE)

# Disable optional X graphics
set(HAVE_X_GRAPHICS OFF CACHE BOOL "Disable dependency on X libraries" FORCE)

