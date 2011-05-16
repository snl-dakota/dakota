##############################################################################
#
# EXAMPLE of a BuildSetup.cmake file for a DAKOTA Package
#
##############################################################################
# Setup path variables
##############################################################################

# Boost base directory

set(Boost_INCLUDE_DIR
  "/Net/dakota/utils/Linux_x86_64/boost_signals1.45.gnu-4.3.2/include"
  CACHE PATH "Always use pre-installed Boost1.45 SIGNALS subset" FORCE)

#set(Boost_LIBRARY_DIRS
#  "/Net/dakota/utils/Linux_x86_64/boost_signals1.45.gnu-4.3.2/lib"
#  CACHE PATH "Always use pre-installed Boost1.45 SIGNALS subset" FORCE)

set(USE_SYSTEM_TEUCHOS ON CACHE BOOL "Always use installed, Teuchos DSO" FORCE)

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
#
# This initial cache is NOT expected to work "as-is" without developer making
# adjustments to paths!  That said, it has been tested and shown to work on
# DAKOTA's primary build host, leoni, as well as our MacOSX build host,
# ictinus (with minimal modifications since the mount point to 'utils' differs).
#
# STEPS for building with cmake type:
# (1) $ mkdir build; cd build
#
# (2) $ export DAKOTA_SRC=$HOME/Dakota    # for example (path will likely vary)
#
# (3) $ vi $DAKOTA_SRC/config/BuildSetup.cmake  # or your BFF editor if not 'vi'
#       - verify path to boost subset (headers + signals), ~line 12,
#         or tweak as necessary
#
# (4) $ $CMAKEHOME/bin/cmake -C $DAKOTA_SRC/config/BuildSetup.cmake $DAKOTA_SRC
#
# (5) $ make -j8 dakota     # number of jobs specified will likely vary
#     NOTE: parallel build MAY FAIL if the 'dakota' target is NOT specified!
#
##############################################################################
#
# THE 5 STEPS ABOVE ASSUME:
# A. cmake (v2.8.4 or greater) is installed.
#    (e.g. leoni:/Net/dakota/utils/Linux_x86_64/cmake/current/bin)
#
# B. Dakota source tree via SVN checkout or source tarball extraction.
#
# C. Build is performed in a freshly created, out-of-source build directory.
#    (Note STEP 1 above; in-source builds are strongly discouraged with CMake.)
#
# D. On RHEL5, due to problems with the FindBoost.cmake module, Boost.signals
#    MUST be installed a priori.
#
# E. On RHEL5 and RHEL6, due to problems with the FindLapack.cmake module,
#    symbolic links to shared-object library versions MUST be created a priori.
#    (NOTE:  This was also a requirement with Autotools builds!)
#      cd /usr/lib64
#      sudo ln -s libblas.so.3.2.1 ./libblas.so
#      sudo ln -s liblapack.so.3.2.1 ./liblapack.so

