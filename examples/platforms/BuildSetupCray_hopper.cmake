# NOTE:  This file contains hardwired paths - specific to the HPC host, hopper,
#        in the Spring 2012 timeframe!
#        Please DO NOT attempt to use this cache-initialization as-is.
#        Please use your favorite editor to make necessary adjustments.
#
#        Contributed by collaborator at ORNL - see DAKOTA Trac #4166.
#
##############################################################################
#
# EXAMPLE BuildSetup.cmake file for DAKOTA (hopper - GNU with libsci linalg)
#
##############################################################################
#
# Currently loaded modules on hopper (libsci for linalg)
#
#1) modules/3.2.6.6     10) pmi/3.0.1-1.0000.9101.2.26.gem 19) moab/6.1.8
#2) xtpe-network-gemini 11) ugni/2.3-1.0400.4374.4.88.gem  20) altd/1.0
#3) cray-mpich2/5.5.2  12) udreg/2.3.1-1.0400.4264.3.1.gem 21) PrgEnv-gnu/4.0.46
#4) atp/1.5.0          13) cray-libsci/11.1.00             22) curl/7.22.0
#5) xt-asyncpe/5.12    14) gcc/4.6.3                       23) boost/1.47.0
#6) xe-sysroot/4.0.46  15) eswrap/1.0.10                   24) cmake/2.8.7
#7) xpmem/0.1-2.0400.31280.3.1.gem      16) xtpe-mc12      25) python/2.7.1
#8) gni-headers/2.1-1.0400.4351.3.1.gem 17) cray-shmem/5.5.2
#9) dmapp/3.2.1-1.0400.4255.2.159.gem   18) torque/2.5.9

##############################################################################

# -DDAKOTA_HAVE_MPI=ON
set(DAKOTA_HAVE_MPI ON CACHE BOOL "Always build with MPI enabled" FORCE)

# -DCMAKE_CXX_COMPILER=/opt/cray/xt-asyncpe/5.12/bin/CC
set(CMAKE_CXX_COMPILER
    "$ENV{ASYNCPE_DIR}/bin/CC" CACHE FILEPATH
    "Always use installed, MPI compiler wrapper" FORCE)

# -DCMAKE_C_COMPILER=/opt/cray/xt-asyncpe/5.12/bin/cc
set(CMAKE_C_COMPILER
    "$ENV{ASYNCPE_DIR}/bin/cc" CACHE FILEPATH
    "Always use installed, MPI compiler wrapper" FORCE)

# -DCMAKE_Fortran_COMPILER=/opt/cray/xt-asyncpe/5.12/bin/ftn 

##############################################################################
# Setup path variables - specific to hopper
##############################################################################

# -DMPI_INCLUDE_PATH=/opt/cray/mpt/5.5.2/gni/mpich2-gnu/46/include
set(MPI_INCLUDE_PATH
    "$ENV{MPICH_DIR}/include" CACHE FILEPATH
    "Use installed MPI headers" FORCE)

# -DMPI_LIBRARY=/opt/cray/mpt/5.5.2/gni/mpich2-gnu/46/lib
set(MPI_LIBRARY
    "$ENV{MPICH_DIR}/lib/libmpichcxx.a" CACHE FILEPATH
    "Use installed MPI libs" FORCE)

# Boost base directory

# -DBoost_INCLUDE_DIR=/global/common/hopper2/usg/boost/1.47.0/include
set(Boost_INCLUDE_DIR
    /global/common/hopper2/usg/boost/1.47.0/include
    CACHE PATH "Always use pre-installed Boost1.47 available on hopper" FORCE)

# LINALG libraries

# -DBLAS_LIBS=/opt/xt-libsci/11.1.00/GNU/46/mc12/lib/libsci_gnu_mp.a
set(BLAS_LIBS
    /opt/xt-libsci/11.1.00/GNU/46/mc12/lib/libsci_gnu_mp.a
    CACHE PATH "Use pre-installed libsci blas/lapack" FORCE)

# -DLAPACK_LIBS=/opt/xt-libsci/11.1.00/GNU/46/mc12/lib/libsci_gnu_mp.a
set(LAPACK_LIBS
    /opt/xt-libsci/11.1.00/GNU/46/mc12/lib/libsci_gnu_mp.a
    CACHE PATH "Use pre-installed libsci blas/lapack" FORCE)

##############################################################################
# Enforce STATIC library build for DAKOTA and all of its TPLs
##############################################################################

# -DBoost_USE_STATIC_LIBS=ON
set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Use STATIC Boost libs" FORCE)

# -DBUILD_STATIC_LIBS=ON
set(BUILD_STATIC_LIBS ON CACHE BOOL "Build STATIC libraries" FORCE)

# -DBUILD_SHARED_LIBS=OFF
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Set shared libs OFF on hopper" FORCE)

# Disable X graphics

# -DHAVE_X_GRAPHICS=FALSE
set(HAVE_X_GRAPHICS OFF CACHE BOOL "Disable graphics on HPC platform" FORCE)

# -DCMAKE_INSTALL_PREFIX=/global/project/projectdirs/m1503/Dakota

