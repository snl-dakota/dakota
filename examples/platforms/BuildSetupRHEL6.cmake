# Assume a "comparably equipped" RHEL6 COE build host
#   We compile with the stock gcc-4.4.x 
#   Install linear algebra RPMS:  blas blas-devel lapack lapack-devel 
#   (Optionally) install MPI RPMS:  openmpi openmpi-devel 
#   (Optionally) install graphics RPMS:  lesstif lesstif-devel libX11 libXau libXext libXmu libXmu-devel libXp libXp-devel libXpm libXpm-devel 

##############################################################################
#
# EXAMPLE BuildSetup.cmake file for DAKOTA (GNU-4.4.4/RHEL6)
#
##############################################################################

# CMake 2.8.6 has problems with RHEL6/Boost -- the following is a workaround
set(Boost_NO_BOOST_CMAKE ON CACHE BOOL "Obtain desired behavior on RHEL6" FORCE)

##############################################################################
# Enforce shared library build for DAKOTA and all of its TPLs
##############################################################################

set(BUILD_STATIC_LIBS OFF CACHE BOOL "Set to ON to build static libraries" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Set to ON to build DSO libraries" FORCE)

# Disable optional X graphics
set(HAVE_X_GRAPHICS OFF CACHE BOOL "Disable dependency on X libraries" FORCE)

