##############################################################################
# EXAMPLE of a BuildSetup.cmake file for a DAKOTA Package
# 
# NOT expected to work "as-is" without developer making adjustments to paths!
# (however, tested and shown to work on DAKOTA's primary build host, leoni,
# as well as our MacOSX build host, ictinus)
#
# STEPS for building with cmake type:
# (1) $ mkdir build; cd build
#
# (2) $ export DAKOTA_SRC=$HOME/Dakota    # for example (path will likely vary)
#
# (3) $ vi $DAKOTA_SRC/config/BuildSetup.cmake  # or your BFF editor if not 'vi'
#       - verify paths to boost and teuchos, ~line 52, or tweak as necessary
#
# (4) $ $CMAKEHOME/bin/cmake -C $DAKOTA_SRC/config/BuildSetup.cmake $DAKOTA_SRC
#
# (5) $ make -j8 dakota     # number of jobs specified will likely vary
#     NOTE: parallel build WILL FAIL if the 'dakota' target is NOT specified!
#
##############################################################################
#
# THE 5 STEPS ABOVE ASSUME:
# A. cmake (v2.8 or greater) is installed.
#    (e.g. leoni:/Net/dakota/utils/Linux_x86_64/cmake/current/bin)
#
# B. Dakota source tree via SVN checkout.
#
# C. Build is performed in a freshly created, out-of-source build directory.
#    (in-source builds are strongly discouraged with CMake)
#
# D. Since building in an EMPTY build directory, CMake MUST be able to locate
#    non-optional, third-party libraries (BINARY installations!).
#    i.  BLAS/LAPACK are typically found in a system install location
#    ii. Teuchos MUST BE BUILT/INSTALLED prior to invoking CMake
#        (*TIPS on how-to build Teuchos from distribution are provided at the
#         bottom of this file)
#
#
##############################################################################
# Setup path variables
##############################################################################

# Boost base directory

set(Boost_INCLUDE_DIR "$ENV{DAKOTA_SRC}/packages/boost"
    CACHE PATH "Always use DAKOTA supplied Boost1.40 header subset" FORCE)

# Teuchos spec (WJB: see ASSUME above and "Trilinos TIPS" at bottom of file)
# WJB - ToDo: add detail regarding MacOSX/DYLD_LIBRARY_PATH (ictinus:/Users/dakota/local/trilinos10.2.1/gnu-4.3.2_install)

set(USE_SYSTEM_TEUCHOS ON CACHE BOOL "Always use installed, Teuchos DSO" FORCE)

# Path to "custom", local build of Trilinos: shared-objects, Teuchos ONLY!
set(Trilinos_DIR
    "/Net/dakota/utils/Linux_x86_64/trilinos10.2.1/gnu-4.1.2_nompi_install"
    CACHE PATH "Base directory for Teuchos installation" FORCE)


##############################################################################

set( BUILD_STATIC_LIBS OFF CACHE BOOL "Set to ON to build static libraries" FORCE )
set( BUILD_SHARED_LIBS ON CACHE BOOL "Set to ON to build DSO libraries" FORCE )



##############################################################################
##############################################################################
##############################################################################

# IF YOU ALREADY HAVE A TRILINOS INSTALLATION, please ignore everything from
# here down!
#
# Trilinos TIPS for building/installing Teuchos-ONLY library from a full
# trilinos distribution <http://trilinos.sandia.gov/download/trilinos-10.2.html>
#
# 1. Create a directory for Trilinos source tarball extraction
#    mkdir trilinos10.2.1
#
# 2. Extract the tarball
#    tar xzf trilinos-10.2.1-Source.tar.gz
#
# 3. mkdir a directory for the teuchos build (can also act as an install dir)
#    mkdir gnu-4.1.2_nompi_install
# 
# 4. Cd into the build_dir and build with cmake
#    (the cmake line is lengthy, so I typically create a "do-configure" script)
#    cd gnu-4.1.2_nompi_install
#
#    cmake \
#      -D CMAKE_BUILD_TYPE:STRING=NONE \
#      -D CMAKE_CXX_FLAGS:STRING="-O2 -Wall -funroll-loops" \
#      -D CMAKE_C_FLAGS:STRING="-O2 -Wall -funroll-loops" \
#      -D CMAKE_Fortran_FLAGS:STRING="-O2 -funroll-loops" \
#      -D BUILD_SHARED_LIBS:BOOL=ON \
#      -D TPL_ENABLE_MPI:BOOL=OFF \
#      -D Trilinos_ENABLE_TESTS:BOOL=OFF \
#      -D Trilinos_ENABLE_DEFAULT_PACKAGES:BOOL=OFF \
#      -D Trilinos_ENABLE_ALL_OPTIONAL_PACKAGES:BOOL=OFF \
#      -D Trilinos_ENABLE_Teuchos:BOOL=ON \
#      -D Trilinos_ENABLE_EXPLICIT_INSTANTIATION:BOOL=ON \
#      -D CMAKE_INSTALL_PREFIX:PATH=/Net/dakota/utils/Linux_x86_64/trilinos10.2.1/gnu-4.1.2_nompi_install \
#      /Net/dakota/utils/Linux_x86_64/trilinos10.2.1/trilinos-10.2.1-Source
#
# 5. Build and install teuchos only
#    make; make install

##############################################################################

# NOTE: WJB performed the above steps on leoni in a DAKOTA-wide, project shared
#   directory:  /Net/dakota/utils/Linux_x86_64/trilinos10.2.1
#
# A copy of the only difficult step in the process (step 4 above) is in the 
# following subdir/file:  gnu-4.1.2_nompi_install/do-configure

