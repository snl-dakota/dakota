# NOTE:  This file contains hardwired paths; please see STEPS at bottom
#        of the file for tips on necessary adjustments for non-SNL platforms
#
# Use Intel/MKL modules
#
# export PATH=/projects/dakota/utils/cmake/current/bin:$PATH:.
# module switch mpi mpi/openmpi-1.4.2_intel-11.1-f064-c064
# module load libraries/intel-mkl-11.1.064
#
##############################################################################
#
# EXAMPLE BuildSetup.cmake file for DAKOTA (Intel - with MKL linalg)
#
##############################################################################

set(DAKOTA_HAVE_MPI ON CACHE BOOL "Always build with MPI enabled" FORCE)

set(CMAKE_CXX_COMPILER "$ENV{MPIHOME}/bin/mpicxx" CACHE FILEPATH
    "Always use installed, MPI compiler wrapper" FORCE)

set(CMAKE_C_COMPILER "$ENV{MPIHOME}/bin/mpicc" CACHE FILEPATH
    "Always use installed, MPI compiler wrapper" FORCE)

##############################################################################
# Setup path variables
##############################################################################

set(MPI_INCLUDE_PATH "$ENV{MPIHOME}/include" CACHE FILEPATH
    "Use installed, MPI headers" FORCE)

set(MPI_LIBRARY
    "/projects/global/x86_64/compilers/intel/intel-11.1-cproc-064/lib/intel64/libimf.a"
   CACHE FILEPATH "Use installed MPI libs" FORCE)

# Boost base directory

set(Boost_INCLUDE_DIR
  "/projects/dakota/utils/boost1.45.intel-11.1/include"
  CACHE PATH "Always use pre-installed Boost1.45 subset" FORCE)

# LINALG libraries

set(BLAS_LIBS "$ENV{BLASLIB}" CACHE STRING
    "Use pre-installed MKL blas/lapack" FORCE)

set(LAPACK_LIBS "$ENV{BLASLIB}" CACHE STRING
    "Use pre-installed MKL blas/lapack" FORCE)

##############################################################################
# Enforce shared library build for DAKOTA and all of its TPLs
##############################################################################

set(BUILD_STATIC_LIBS OFF CACHE BOOL "Set to ON to build static libraries" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Set to OFF to build DSO libraries" FORCE)


##############################################################################
#
# This initial cache is NOT expected to work "as-is" without developer making
# adjustments to paths!  That said, it has been tested and shown to work on
# DAKOTA's primary build host, as well as our MacOSX build host,
# (with minimal modifications since the mount point to 'utils' differs).
#
# STEPS for building with cmake type:
# (1) $ mkdir build; cd build
#
# (2) $ export DAKOTA_SRC=$HOME/Dakota    # for example (path will likely vary)
#
# (3) $ vi $DAKOTA_SRC/examples/platforms/BuildSetupIntelMKL.cmake
#       - verify path to boost subset, ~line 38, or tweak as necessary
#       - verify path to MPI library, ~line 32, or tweak as necessary
#
# (4) $ cmake -C $DAKOTA_SRC/examples/platforms/BuildSetupIntelMKL.cmake $DAKOTA_SRC
#
# (5) $ make -j8 dakota     # number of jobs specified will likely vary
#     NOTE: parallel build MAY FAIL if the 'dakota' target is NOT specified!
#
##############################################################################
#
# THE 5 STEPS ABOVE ASSUME:
# A. cmake (v2.8.4 or greater) is installed.
#
# B. Dakota source tree via SVN checkout or source tarball extraction.
#
# C. Build is performed in a freshly created, out-of-source build directory.
#    (Note STEP 1 above; in-source builds are strongly discouraged with CMake.)
#
# D. On RHEL5, due to problems with the FindBoost.cmake module, Boost libraries
#    MUST be installed a priori.

