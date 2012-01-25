##############################################################################
#
# EXAMPLE BuildSetup.cmake file for a DAKOTA Package (MacOSX, GNU compilers)
#
##############################################################################
# Setup path variables
##############################################################################

# Boost base directory

set(Boost_INCLUDE_DIR
  "/Users/dakota/local/boost.gnu-4.3.2/include"
  CACHE PATH "Always use pre-installed Boost1.45 subset" FORCE)

##############################################################################
# Enforce shared library build for DAKOTA and all of its TPLs
##############################################################################

set(BUILD_STATIC_LIBS OFF CACHE BOOL "Set to ON to build static libraries" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Set to ON to build DSO libraries" FORCE)

##############################################################################
#
# This initial cache is NOT expected to work "as-is" without developer making
# adjustments to paths!  That said, it has been tested and shown to work on
# DAKOTA's primary build host, as well as our MacOSX build host
# (with minimal modifications since the mount point to 'utils' differs).
#
# STEPS for building with cmake type:
# (1) $ mkdir build; cd build
#
# (2) $ export DAKOTA_SRC=$HOME/Dakota    # for example (path will likely vary)
#
# (3) $ vi $DAKOTA_SRC/config/BuildSetup.cmake  # or your BFF editor if not 'vi'
#       - verify path to boost subset (headers, signals, regex), ~line 12,
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
#
# B. Dakota source tree via SVN checkout or source tarball extraction.
#
# C. Build is performed in a freshly created, out-of-source build directory.
#    (Note STEP 1 above; in-source builds are strongly discouraged with CMake.)
#
