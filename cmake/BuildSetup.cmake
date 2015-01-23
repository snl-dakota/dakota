##############################################################################
#
# EXAMPLE BuildSetup.cmake file for DAKOTA - RHEL5 with Boost-1.49
#
##############################################################################
# Setup path variables
##############################################################################

# Boost directories

set(BOOST_INCLUDEDIR
  "/Net/dakota/utils/Linux_x86_64/boost_1.49.gnu-4.1.2/include/boost-1.49.0"
  CACHE PATH "Always use pre-installed Boost1.49 subset" FORCE)

set(BOOST_LIBRARYDIR
  "/Net/dakota/utils/Linux_x86_64/boost_1.49.gnu-4.1.2/lib/boost-1.49.0"
  CACHE PATH "Always use pre-installed Boost1.49 subset -- BFSV3! " FORCE)

# Path to "custom", local build of Trilinos: shared-objects, Teuchos ONLY!
set(Trilinos_DIR
  "/Net/dakota/utils/Linux_x86_64/trilinos-10.6.0/gnu-4.3.2_nompi_install"
  CACHE PATH "Base directory for Teuchos installation" FORCE)


##############################################################################
# Enforce shared library build for DAKOTA and all of its TPLs
##############################################################################

set(BUILD_STATIC_LIBS OFF CACHE BOOL "Set to ON to build static libraries" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Set to ON to build DSO libraries" FORCE)

##############################################################################

set(DAKOTA_HAVE_MPI ON CACHE BOOL "Always build with MPI enabled" FORCE)

# Optionally set compiler wrapper
set(MPI_CXX_COMPILER
  "/Net/dakota/utils/Linux_x86_64/openmpi-1.4.1.gcc-4.3.2/bin/mpicxx" CACHE
  FILEPATH "Use custom mpicxx" FORCE)

# Or optionally set include AND libraries to skip autodetection
#set(MPI_CXX_INCLUDE_PATH
#  "/Net/dakota/utils/Linux_x86_64/openmpi-1.4.1.gcc-4.3.2/include" CACHE
#  FILEPATH "Use installed, MPI headers" FORCE)
#set(MPI_CXX_LIBRARIES
#  "/Net/dakota/utils/Linux_x86_64/openmpi-1.4.1.gcc-4.3.2/lib/libmpi_cxx.so 
#  CACHE FILEPATH "Use installed, MPI headers" FORCE)

##############################################################################
#
# This initial cache is NOT expected to work "as-is" without developer making
# adjustments to paths!  That said, it has been tested and shown to work on
# DAKOTA's primary RHEL5 build host, as well as our MacOSX build host
# (with minimal modifications since the mount point to 'utils' differs).
#
# STEPS for building with cmake type:
# (1) $ mkdir build; cd build
#
# (2) $ export DAKOTA_SRC=$HOME/Dakota    # for example (path will likely vary)
#
# (3) $ vi $DAKOTA_SRC/config/BuildSetup.cmake  # or your BFF editor if not 'vi'
#       - verify path to boost subset, ~lines 12 and 16, or tweak as necessary
#       - adjust paths to trilinos and MPI (~lines 23 and 39, respectively)
#
# (4) $ cmake -C $DAKOTA_SRC/config/BuildSetup.cmake $DAKOTA_SRC
#
# (5) $ make -j8 dakota     # number of jobs specified will likely vary
#     NOTE: parallel build MAY FAIL if the 'dakota' target is NOT specified!
#
##############################################################################
#
# THE 5 STEPS ABOVE ASSUME:
# A. cmake (v2.8.5 or greater) is installed.
#
# B. Build is performed in a freshly created, out-of-source build directory.
#    (Note STEP 1 above; in-source builds are strongly discouraged with CMake.)
#
# C. On RHEL5, due to problems with the FindBoost.cmake module, Boost Libraries
#    MUST be installed a priori.
#
# D. On RHEL5 and RHEL6, due to problems with the FindLapack.cmake module,
#    symbolic links to shared-object library versions MUST be created a priori.
#    (NOTE:  This was also a requirement with Autotools builds!)
#      cd /usr/lib64
#      sudo ln -s libblas.so.3.2.1 ./libblas.so
#      sudo ln -s liblapack.so.3.2.1 ./liblapack.so

