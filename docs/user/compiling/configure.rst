.. _configure:

""""""""""""""""""""
Configure with CMake
""""""""""""""""""""

Dakota uses CMake to manage building and testing the complex set of
codes and libraries that make up Dakota, including setting options and
locations of external tools and libraries. The `CMake Documentation
<https://cmake.org/documentation/>`_ offers reference documentation
and tutorials.

CMake uses two separate directories. In these instructions, the source
directory is referred to as ``DAK_SRC``; the build directory,
``DAK_BUILD``. The source directory contains the Dakota source code
and the cmake files. The user creates the build directory and builds
from that directory. It is initially empty; it will be populated with
the Dakota executable, libraries, packages and examples.

.. note::

   Dakota only supports out-of-source builds, not building in-place in
   the source tree. (The build directory can be outside the source
   tree, e.g., peer to it, or in a sub-folder of the source tree,
   e.g., ``DAK_SRC/build``).

Dakota and CMake itself support numerous configuration settings. There
are several ways to specify options to CMake, including at the command
line, with a CMake cache initialization script, and through one of the
CMake GUIs.

.. attention::

   CMake persists configuration information (including detected system
   properties and software) in a file
   :file:`$DAK_BUILD/CMakeCache.txt` and associated
   files/directories. When revising your Dakota CMake settings, it is
   helpful to remove these files or delete the entire build directory
   contents. This avoids inadvertently using previous settings.

.. attention::

   When using the ``CMAKE_INSTALL_PREFIX`` CMake setting, configuration
   information can be stored there as well, and it is good practice
   to remove the installation directory contents when revising CMake
   settings.

===============================
Configuring at the Command Line
===============================

**Simple Command Line Example**::

   mkdir build 
   cd build
   
   # Configure the build tree.
   cmake \
     -D CMAKE_INSTALL_PREFIX=/home/myFavoriteUser/dakota \
     -D DAKOTA_HAVE_MPI:BOOL=TRUE \
     -D CMAKE_CXX_COMPILER:FILEPATH=/usr/bin/mpicxx \
     -D MPI_LIBRARY:FILEPATH=/usr/lib/libmpi_cxx.so \
     $DAK_SRC

In this example the following CMake cache variables are defined:

- ``CMAKE_INSTALL_PREFIX``: where to install the Dakota executables,
  libraries and examples

- ``DAKOTA_HAVE_MPI:BOOL=TRUE``: build with MPI to allow for parallel
  processing

- ``CMAKE_CXX_COMPILER:FILEPATH``: location of MPI compiler

- ``MPI_LIBRARY:FILEPATH``: location of MPI library


===============================
Configuring with a CMake Script
===============================

Dakota build settings can be written in a CMake script file (referred
to as build file here). The directory :file:`$DAK_SRC/cmake/examples` contains
a collection of build files. There are historical platform-specific
examples such as BuildRHEL7.cmake, BuildDarwin.cmake, and
Build_win_msvc_ifort.cmake. The build file BuildDakotaTemplate.cmake
is a template that contains the most commonly used Dakota CMake
variables.

Some common settings in a build file include:

- specify the folder where the Dakota binaries will be installed
- build without MPI
- build with MPI and tell cmake where to find the relevant MPI files
- turn on or off Dakota features

**Simple Build File Example**

See :ref:`sec:using_dak_cmake_template` for a description of the
variables and instructions for use. Once you have customized this
script for your platform, run the CMake script as follows::

   mkdir build 
   cd build
   cp ../cmake/BuildDakotaTemplate.cmake BuildDakota.cmake

   # Configure the build tree
   cmake -C BuildDakota.cmake $DAK_SRC

You can iteratively make changes to :file:`BuildDakota.cmake` for your
platform (see link above). Once your script correctly builds Dakota,
save it in a more permanent location for later reuse.

.. _`sec:using_dak_cmake_template`:

-------------------------------
Using BuildDakotaTemplate.cmake
-------------------------------

The CMake variables in file
:file:`$DAK_SRC/cmake/examples/BuildDakotaTemplate.cmake` represent
the minimum set of variables necessary to:

- find all prerequisite third party libraries (TPLs)
- configure compiler and MPI options
- set Dakota install path

Typically you would uncomment CMake variables *only* for values you
need to change for your platform. Once you edit variables as needed,
run CMake with the script file.

**Linear Algebra:** Set BLAS, LAPACK library paths if in non-standard
locations. ::

  set( BLAS_LIBS "/usr/lib64"
    CACHE FILEPATH "Use non-standard BLAS library path" FORCE )
  set( LAPACK_LIBS "/usr/lib64"
    CACHE FILEPATH "Use non-standard BLAS library path" FORCE )

**Compiler Options:** Uncomment and replace <flag> with actual
compiler flag, e.g. ``-xxe4.2``. ::

  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} <flag>" 
    CACHE STRING "C Flags my platform" ) 
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} <flag>" 
    CACHE STRING "CXX Flags for my platform" ) 
  set( CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} <flag>" 
    CACHE STRING "Fortran Flags for my platform" )

**MPI options:** Recommended practice is to set DAKOTA_HAVE_MPI and
set MPI_CXX_COMPILER to a compiler wrapper. ::

  set( DAKOTA_HAVE_MPI ON
     CACHE BOOL "Build with MPI enabled" FORCE)
  set( MPI_CXX_COMPILER "path/to/mpicxx"
     CACHE FILEPATH "Use MPI compiler wrapper" FORCE)

**Boost:** You will need to set the following variables iff CMake
cannot find your installed version of Boost or if you have a custom
Boost install location. ::

  set(BOOST_ROOT
    "path/to/custom/Boost/install/directory"
    CACHE PATH "Use non-standard Boost install" FORCE)
  set( Boost_NO_SYSTEM_PATHS TRUE
     CACHE BOOL "Supress search paths other than BOOST_ROOT" FORCE)

**Trilinos:** You will need to set the following variable if you
have a custom Trilinos install location. If not set, the
Dakota-shipped Trilinos will be built instead. ::

  set( Trilinos_DIR
    "path/to/Trilinos/install"
    CACHE PATH "Path to installed Trilinos" FORCE )

**Customize DAKOTA:** ::

  set( CMAKE_INSTALL_PREFIX
    "/path/to/Dakota/installation"
    CACHE PATH "Path to Dakota installation" )


=============================
Top-level Build/Debug Options
=============================

**Compilers:** To prevent CMake from identifying mixed toolchains
during system introspection, it is good practice to explicitly set
``CMAKE_C_COMPILER``, ``CMAKE_CXX_COMPILER``, and
``CMAKE_Fortran_COMPILER``.

**Optimization and Debugging Symbols:** The simplest way to set the
optimization level and debug symbol inclusion for your builds is with
``CMAKE_BUILD_TYPE``. For example set one of the following: ::

  CMAKE_BUILD_TYPE=Debug              # defaults to -g
  CMAKE_BUILD_TYPE=Release            # defaults to -O3 -DNDEBUG
  CMAKE_BUILD_TYPE=RelWithDebInfo     # defaults to -O2 -g

You may also manually set the relevant flags at configure time and
these will supplement any set by the CMAKE_BUILD_TYPE configurations
shown immediately above. For example: ::

  CMAKE_C_FLAGS=-O2
  CMAKE_CXX_FLAGS=-O2
  CMAKE_Fortran_FLAGS=-O2

**Shared vs. Static libraries:** By default, Dakota builds and links
external shared libraries but can also build static libraries if you
need to link against static libraries without position-independent
code, e.g,. an installed libblas.a or libmpi.a. ::

  # Build static libraries ONLY
  BUILD_SHARED_LIBS:BOOL=FALSE

**Developer Convenience Macros:** These shortcuts are designed to help
routine development:

- ``-C cmake/DakotaDev.cmake``: enable MPI, docs, and specification
  maintenance.

- ``-D DevDebugStatic:BOOL=ON`` turns on most common developer
  options, with debug build type, static libs, and bounds
  checking. For a shared debug build, instead set ``DevDebugShared``.

- ``-D DevDistro:BOOL=ON`` turns on most common developer options and
  distribution build type, shared libs, no bounds checking (equivalent
  to ``-C cmake/DakotaDistro.cmake -C cmake/DakotaDev.cmake``).

.. note::

   When combining these with additional options or configuration
   files, take care with their ordering to get the desired behavior.

**Verbose Make:** To see detailed compile/link commands generated by
CMake: ``make VERBOSE=1`` (on Unix platforms), or set
``CMAKE_VERBOSE_MAKEFILE:BOOL=TRUE`` when configuring.

==================
External Libraries
==================

This section discusses the most common Dakota options for finding
external libraries.

**Message Passing Interface (MPI)**

Dakota uses the standard `CMake FindMPI
<https://cmake.org/cmake/help/latest/module/FindMPI.html>`_ to find
and configure MPI. The minimum for compiling Dakota with MPI is to
make sure appropriate MPI binaries and libraries are on necessary
PATHs and by setting ``DAKOTA_HAVE_MPI:BOOL=TRUE``.

The recommended practice is to also specify a C++ compiler wrapper
(and all specific compilers as discussed above to avoid mixed
toolchains): ::

  DAKOTA_HAVE_MPI:BOOL=TRUE
  MPI_CXX_COMPILER:FILEPATH="/path/to/mpicxx"

By default, the MPI compiler wrapper will be used by CMake to find
MPI, but not actually used for compilation. Rather the detected
defines, includes, and libraries will be added to relevant MPI-enabled
targets in the build.

If a compiler wrapper isn't available, or specific MPI includes and
libraries are needed, specify appropriate values for
``MPI_CXX_LIBRARIES`` and ``MPI_CXX_INCLUDE_PATH``. This will
circumvent any autodetection. If you want to force Dakota to use the
MPI wrappers and *not* apply the detected libraries and headers, set
::

  DAKOTA_HAVE_MPI:BOOL=TRUE
  CMAKE_CXX_COMPILER:FILEPATH="/path/to/mpicxx"
  MPI_CXX_COMPILER:FILEPATH="/path/to/mpicxx"
  DAKOTA_MPI_WRAPPER_ONLY:BOOL=TRUE

**Boost C++ Libraries**

Dakota uses the standard `CMake FindBoost
<https://cmake.org/cmake/help/latest/module/FindBoost.html>`_; see its
documentation for the most up to date options. To identify a
non-system default Boost installation, set
``BOOST_ROOT:PATH=/path/to/boost/1.69``, where the specified path
contains the ``include/`` and ``lib/`` directories.  Alternately, for
split installations, set ``BOOST_INCLUDEDIR`` and
``BOOST_LIBRARYDIR``.

If you are using a non-system installed Boost, or your system has
multiple Boost installations, it is often critical to set the
following to avoid configuring Daktoa with a mix of headers from one
Boost distribution and libraries from another: ::

  # Avoid using BoostConfig.cmake if found on system
  Boost_NO_BOOST_CMAKE:BOOL=TRUE
  
  # Avoid using Boost from system location if found
  Boost_NO_SYSTEM_PATHS:BOOL=TRUE

**Linear Algebra**

Variables ``BLAS_LIBS=path/to/libblas.*`` and
``LAPACK_LIBS=path/to/liblapack.*`` can be used to identify Fortran
linear algebra libraries.  On some platforms these can be conveniently
set to ``$ENV{BLAS_LIBS}`` and ``$ENV{LAPACK_LIBS}`` in a CMake cache
initialization file.

Note: Not all Dakota's TPLs treat BLAS and LAPACK with the same cache
variables. To use specific versions of these libraries, the following
is likely necessary to handle HOPSPACK LAPACK dependency on BLAS:
``LAPACK_ADD_LIBS=${BLAS_LIBS}``.

**HDF5 (Hierarchical Data Format)**

Dakota :ref:`HDF5 results output <environment-results_output-hdf5>`
requires the HDF5 C++ libraries.  Building Dakota with HDF5 support
requires HDF5 version 1.10.4 or higher. Parallel HDF5 is not needed at
this time, but will be in future Dakota releases. In addition, the
Python ``h5py`` module is needed to run some of the HDF5-related tests
and for the HDF5 examples.

To enable HDF5: ::

  DAKOTA_HAVE_HDF5:BOOL=ON

In addition, the variable ``HDF5_ROOT`` should typically be set to the
top-level HDF5 directory (i.e. that contains the include and lib
folders), cf `CMake's FindHDF5
<https://cmake.org/cmake/help/latest/module/FindHDF5.html>`_

**External Trilinos**

To compile Dakota against an externally installed Trilinos, set
``Trilinos_DIR`` to the directory in an install tree containing
``TrilinosConfig.cmake``, e.g.,
:file:`/usr/local/trilinos/lib/cmake/Trilinos/`, which contains
``TrilinosConfig.cmake`` (similarly ``Teuchos_DIR`` for location of
``TeuchosConfig.cmake``).

Set the variable ``DAKOTA_NO_FIND_TRILINOS:BOOL=TRUE`` to disallow
Dakota from attempting to find the Trilinos package. This can be
useful when building Dakota as part of a large CMake project that
includes Trilinos.

===============
Dakota Features
===============

See :file:`$DAK_SRC/cmake/DakotaOptions.cmake` for the most up-to-date
common options.

**Documentation**

Building Dakota documentation requires appropriate versions of
Java, Perl, Doxygen and Sphinx and only works on Unix platforms: set
``ENABLE_DAKOTA_DOCS:BOOL=TRUE``

..
  TODO? DAKOTA_DOCS_DEV

**Testing**

The following options affect tests:

.. list-table:: Dakota Testing Options
   :header-rows: 1

   * - CMake Option
     - Default
     - Notes
   * - DAKOTA_ENABLE_TESTS
     - ON
     - Whether to enable Dakota system-level tests
   * - DAKOTA_EXPAND_SUBTESTS
     - OFF
     - Whether to register one CTest test per system-level subtest
   * - DAKOTA_ENABLE_TPL_TESTS
     - OFF
     - Whether to enable the following TPL tests
   * - DDACE_ENABLE_TESTS
     - OFF
     -
   * - HOPSPACK_ENABLE_TESTS
     - OFF
     -
   * - LHS_ENABLE_TESTS
     - OFF
     -
   * - OPTPP_ENABLE_TESTS
     - OFF
     -
   * - PECOS_ENABLE_TESTS
     - OFF
     -
   * - ENABLE_TESTS
     - OFF
     - Whether to enable tests in Acro and Utilib

**Specification maintenance**

Dakota specification (input syntax) maintenance mode is used by developers when
modifying Dakota XML input specification.  It generates ``nidrgen``
and ``Dt`` binaries in :file:`packages/nidr` and when
``dakota.input.nspec`` changes, will use them to update relevant
generated files in :file:`$DAK_SRC/src` since currently they need to
be checked in. To enable specification maintenance, use the option
``ENABLE_SPEC_MAINT:BOOL=TRUE``

**QUESO for Bayesian Calibration**

Compiling Dakota's QUESO sub-package requires:

- A Unix-like platform (not supported on Windows)

- GSL 1.15 or newer, which means that **resulting Dakota binaries will
  include GPL licensed components**

Environment: If the program ``gsl-config`` is not on your PATH, you'll
either need to put it there, or set the environment variable export
``GSL_CONFIG=/usr/local/gsl/1.15/bin/gsl-config`` or similar. You may
also need to ``export BOOST_ROOT=/usr/local/boost/1.69.0`` or similar
to make sure that QUESO and Dakota are built with the same Boost.

The following CMake settings are necessary to enable QUESO: ::

  HAVE_QUESO:BOOL=ON
  DAKOTA_HAVE_GSL:BOOL=ON

  # Optionally, to disable GPMSA:
  HAVE_QUESO_GPMSA:BOOL=OFF

  # Optionally, specify a CBLAS to avoid numerical differences induced by using GSL's CBLAS, e.g., on RHEL6:
  DAKOTA_CBLAS_LIBS:FILEPATH=/usr/lib64/atlas/libcblas.so.3


**Dakota Components**

.. list-table:: Dakota Core Component Options
   :header-rows: 1

   * - CMake Option
     - Default
     - Notes
   * - DAKOTA_MODULE_UTIL
     - ON
     - Dakota utility module (can be built on its own)
   * - DAKOTA_MODULE_SURROGATES
     - ON
     - Dakota surrogates module (can be built on its own)
   * - DAKOTA_MODULE_DAKOTA
     - ON
     - Balance of Dakota core capability
   * - DAKOTA_EXAMPLES_INSTALL
     - ``share/dakota``
     - Install destination for ``examples/``, relative to ``CMAKE_INSTALL_PREFIX``
   * - DAKOTA_TEST_INSTALL
     - ``share/dakota``
     - Install destination for ``test/``, relative to ``CMAKE_INSTALL_PREFIX``
   * - DAKOTA_EXAMPLES_INSTALL
     - ``share/dakota``
     - Install destination for top-level files like ``README``, relative to ``CMAKE_INSTALL_PREFIX``


.. list-table:: Dakota Python Options
   :header-rows: 1

   * - CMake Option
     - Default
     - Notes
   * - DAKOTA_PYTHON
     - ON
     - Basic Dakota python usage requiring interpreter only
   * - DAKOTA_PYTHON_DIRECT_INTERFACE
     - OFF
     - Python direct function evaluation interface
   * - DAKOTA_PYTHON_DIRECT_INTERFACE_NUMPY
     - ON
     - Enable NumPy for direct Python interface
   * - DAKOTA_PYTHON_WRAPPER
     - OFF
     - Top-level Dakota Python interface (``dakota.environment`` wrapper)
   * - DAKOTA_PYTHON_SURROGATES
     - OFF
     - Python interface to surrogates modules


**Dakota Sub-packages (TPLs)**

This table lists the most common CMake options for enabling or
disabling specific Dakota sub-packages. Note that Dakota builds may
fail when disabling many of these packages, even if not
marked as required, because various combinations of enable/disables aren't
routinely tested.

.. csv-table:: Dakota Package Options
   :header: "CMake Option", Default, Notes

   HAVE_ACRO, 	    ON, 	 
   HAVE_AMPL, 	    ON,  "May need to be OFF if compiling with C99 support"
   HAVE_C3,         OFF,
   HAVE_CONMIN,     ON,
   HAVE_DDACE, 	    ON,
   HAVE_DEMO_TPL,   OFF,
   HAVE_DFFTPACK,   ON,
   HAVE_DOT, 	    ON,  "Turns OFF if packages/DOT missing"
   HAVE_DREAM, 	    ON,
   HAVE_FFTW, 	    OFF, "Off due to GPL-license"
   HAVE_FSUDACE,    ON,
   HAVE_HOPSPACK,   ON,
   HAVE_JEGA, 	    ON,
   HAVE_LHS, 	    ON,
   HAVE_NCSUOPT,    ON,
   HAVE_NL2SOL,     ON,
   HAVE_NLPQL, 	    ON,  "Turns OFF if packages/NLPQL missing"
   HAVE_NOMAD, 	    ON,
   HAVE_NOWPAC,     OFF,
   HAVE_NPSOL, 	    ON,  "Turns OFF if packages/NPSOL missing"
   HAVE_OPTPP, 	    ON,
   HAVE_PECOS, 	    ON,  "Currently required"
   HAVE_PSUADE,     ON,
   HAVE_QUESO, 	    OFF, "Off due to dependence on GPL-licensed GSL"
   HAVE_QUESO_GPMSA,OFF,
   DAKOTA_HAVE_GSL, OFF, "Required when ``HAVE_QUESO=ON``"
   HAVE_ROL,        ON,
   HAVE_SPARSE_GRID,ON,
   HAVE_SURFPACK,   ON,  "Currently required"


=============
Common Issues
=============

**Errors running CMake:** If you encounter errors running CMake, first
inspect the console output/error. The files ``CMakeOutput.log`` and
``CMakeError.log`` contained in ``$DAK_BUILD/CMakeFiles`` are
per-probe output and rarely help.

**Mixing incompatible compilers:** Be careful that incompatible compilers
aren't selected automatically, e.g., mixing g77 with gfortran or mixing
compiler vendors.  We often see a specific gfortran get combined
with system /usr/bin/cc due to CMake probes.  Best practice is to
specify which C, C++, and Fortran compilers you want to use, e.g.
explictly specify ``CMAKE_C_COMPILER``, ``CMAKE_CXX_COMPILER``, and
``CMAKE_Fortran_COMPILER``.

**Mixing MPI with incompatible compilers**. Solution: explicitly
specify ``MPI_CXX_COMPILER``, as well as the standard compilers shown
above.

**Boost detection or link issues:** Mixing a system and
custom-installed Boost.  Solution: see the guidance on specifying
Boost above.

**TPLs found in CMAKE_INSTALL_PREFIX:** Dakota finds packages, e.g.,
Trilinos, in an old install tree and mis-configured the current
build. Solution: remove the install tree prior to build or try setting
CMake find options, e.g., ``CMAKE_FIND_USE_INSTALL_PREFIX:BOOL=FALSE``
from https://cmake.org/cmake/help/latest/command/find_package.html.
