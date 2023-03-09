.. _devenv:

"""""""""""""""""""""""""""""
Setup Development Environment
"""""""""""""""""""""""""""""

Dakota has the following toolchain and software package
dependencies. See sections that follow for suggestions on how to
satisfy them.

Dakota regression test baselines are based on the recommended Dakota
RHEL 7 configuration below. Regression tests will typically exhibit
numerical differences when running on other platforms. For developers
needing to reproduce RHEL 7 baselines on other platforms, e.g., for
pre-push code verification, a Docker-based build/test process is also
described in :ref:`devenvironment-main`.

.. note::

   Dakota developers at SNL should augment this information with
   site-specific documentation.


==========
Toolchains
==========

.. list-table:: Dakota Build Toolchains
   :header-rows: 1

   * - Tool
     - Version
     - Notes
   * - `CMake <https://cmake.org/>`_
     - >= 3.17
     - Required
   * - C Compiler
     - >= C99
     - Required
   * - C++ Compiler
     - >= C++11
     - Required
   * - Fortran Compiler
     - 77, 90
     - Required
   * - `Python <https://www.python.org/>`_
     - >= 2.7 or >= 3.6
     - Required; 3.6 or newer recommended 
   * - `Perl <https://www.perl.org/>`_
     - >= 5.16
     - Required
   * - Java JDK
     - = 11
     - Optional; required for dakota.xml maintenance, generating user docs
   * - Doxygen
     - = 1.8.20
     - Optional; required for generating C++ code docs
   * - Sphinx
     - >= 5.0
     - Optional; required for generating user docs. Requires
       sub-packages , myst-parser, sphinx-rtd-theme,
       sphinxcontrib-bibtex

Notes on toolchains:

- Dakota is most commonly built with GCC, Intel, and Visual C++
  compilers, and to a lesser extent with Cray, PGI, IBM, and
  Clang. Recent versions of these compilers are not covered by
  automated testing so using really new ones may require patches or
  disabling some Dakota packages (often Acro and/or AMPL).

- Typically the compilers should be from the same toolchain and
  version, i.e., don't mix Intel C++ with GCC Fortran unless you
  understand the consequences.

- C99 is not supported by Dakota's AMPL package.

- Refer to :ref:`Writing Dakota Documentation <writing_dakota_docs>` for detailed instructions on
  installing Sphinx into your preferred Python environment.

========
Packages
========

.. list-table:: Core Dakota Packages
   :header-rows: 1

   * - Package
     - Version
     - Notes
   * - Linear Algebra
     - any
     - Required, e.g., BLAS/LAPACK, MKL, Accelerate
   * - `Boost <https://www.hdfgroup.org/>`_
     - >= 1.69
     - Required; headers, plus libraries: filesystem, program_options,
       regex, serialization, system
   * - MPI
     - any
     - Optional, e.g., MPICH, OpenMPI
   * - `HDF5 <https://www.hdfgroup.org/>`_
     - = 1.10.4 (Dakota may work with 1.10.x for x > 4; untested)
     - Optional; required for Dakota structured results output
   * - `h5py <https://www.h5py.org/>`_
     -
     - Optional; required to enable HDF5-based tests (recommended for
       developers)
   * - `GSL <https://www.hdfgroup.org/>`_
     - >= 1.15
     - Optional; required for QUESO Bayesian calibration, **induces
       GPL dependency**
   * - `Trilinos <https://trilinos.github.io/>`_ (Teuchos and ROL)
     - >= 13.0
     - Optional, as Dakota includes a snapshot in
       :file:`packages/external/trilinos`


**Notes on Packages:**

Linear Algebra:

- If installing with a package manager, consider including the user
  runtime as well as development packages, e.g., ``blas``,
  ``blas-devel``, ``lapack``, ``lapack-devel``, to facilitate both
  dynamic and static linking.

- Some systems do not provide canonically named versions of these
  libraries, so it may be necessary to symbolically link, e.g.,
  ``libblas.so -> libblas.so.3``. Otherwise you may be able to specify
  full paths to the desired libraries using the CMake variables
  ``BLAS_LIBS``, ``LAPACK_LIBS``

Boost:

- Make sure the CMake version you're using is newer than the Boost
  version to properly find Boost.

- If compiling Boost from source, it may be necessary to compile Boost
  with C++11, e.g., by setting ``-std=c++11`` or similar.

MPI:

- The MPI installation must be compatible with the compiler toolchain.

HDF5:

- If compiling HDF5 from source, C++ support must be enabled
  (``--enable-cxx`` option to configure), which is not the default when
  building it from source.

Trilinos:

- Dakota's Trilinos snapshot suffices for most users. Those who need
  Dakota to be compatible with a specific Trilinos installation, e.g.,
  when linking Dakota as a library with a Trilinos-dependent
  application, should provide their own Trilinos.

- If compiling Trilinos from source, enable at least the Teuchos and
  ROL packages.

- Dakota likely cannot be compiled against Kokkos-enabled Trilinos.

- Compatibility:

  * Dakota 6.10--6.12 require Trilinos dated May 2019 or newer, or
    otherwise disabling deprecated code in Teuchos due to removal of
    ``ostream::operator<<``.

  * Dakota 6.13 and newer require Trilinos 13.0 or newer.


=======================
Platform-specific Hints
=======================

For Linux platforms in general, it's easiest to satisfy Dakota's
dependencies using OS package managers, supplementing with compiling
from source as needed.

------
RHEL 7
------

On RHEL 7 (most recently 7.8), we satisfied Dakota's dependencies with
the following which are all from either the standard package or EPEL (``yum
install epel-release``) repositories and installed with yum install.

Core development tools: GCC 4.8.5, Java 11, Python 3.6.8 (alternately
2.7.5 via the ``python`` package), Perl 5.16.3:

.. code-block::

   yum install git cmake3 gcc gcc-c++ gcc-gfortran java-11-openjdk-devel python3 perl


Libraries/runtimes: Boost 1.69, OpenMPI 1.10.3, GSL 1.15.

.. code-block::

   yum install blas blas-devel lapack lapack-devel boost169 boost169-devel openmpi openmpi-devel gsl gsl-devel

Enabling HDF5 requires compiling and installing HDF5 1.10.4 and
optionally an h5py that uses it. (Dakota may work with, but hasn't
been verified with, newer HDF5 1.10.x)

If building documentation, see above for Python packages as well.

.. code-block::
 
   yum install doxygen texlive-collection-latexrecommended

When configuring use ``cmake3`` and specify Boost include/library
directories to point to, e.g., ``/usr/include/boost169``.


------
RHEL 8
------

The following list of packages work for a trial Dakota build on CentOS
or Rocky Linux 8 distro.

Base Packages resulting in GCC 8.5.0, Python 3.6.8, CMake 3.20.2, and
OpenMPI 4.1.1::

   # yum or dnf install the following packages...
   git make cmake gcc gcc-c++ gcc-gfortran python3 perl blas lapack openmpi openmpi-devel
    
   # Omit boost as will install 1.66
   ## boost boost-devel
    
   # GSL only needed if enabling Dakota's QUESO library
   gsl gsl-devel
    
   # NOTE: Java should only be needed if maintaining Dakota or building documentation
   java-11-openjdk-devel
    
   # For Python features (optional)
   python3-h5py python3-numpy
    
   # Enable the PowerTools repo via editing /etc/yum.repos.d/CentOS-PowerTools.repo and install
   blas-devel lapack-devel

EPEL Packages add Boost 1.69 and HDF 1.10.5::

   epel-release
    
   # EPEL, PowerTools for Boost to /usr/include/boost169
   boost169 boost169-devel
    
   # Optional; also needs powertools for libsz and libaec deps
   hdf5 hdf5-devel: 1.10.5
    
   # Optional python linked library:
   python36-devel

Basic build example::

   cmake -D BOOST_INCLUDEDIR:PATH=/usr/include/boost169 -DBOOST_LIBRARYDIR:PATH=/usr/lib64/boost169 ../source


------
Ubuntu
------

An incomplete, but perhaps helpful, list of packages for Ubuntu 20.04
LTS, starting from a minimal desktop installation:

::

   apt-get install gcc g++ gfortran cmake libboost-all-dev libblas-dev liblapack-dev libopenmpi-dev openmpi-bin gsl-bin libgsl-dev python perl libhdf5-dev


An incomplete, but perhaps helpful, list of packages for Ubuntu 18.04
LTS:

::

   apt-get install gcc g++ gfortran cmake libboost-all-dev libblas-dev liblapack-dev libopenmpi-dev openmpi-bin gsl-bin libgsl-dev python perl


Other possibly helpful package names:

::

   doxygen texlive-latex-base openjdk-11-jre-headless 


.. _devenv-mac:

-------
Mac OS
-------

A number of approaches can be used to satisfy Dakota's dependencies on
Mac OS / OS X. Typically the OS-provided Accelerate linear algebra
suffices. These are listed in reverse chronological order, in hopes of
archiving useful information, with the first being the most promising
approach currently.

.. note::

   Mac SIP and other security features can be highly problematic when
   running Dakota tests or even using the Dakota binaries.

.. note::

   If you build your own toolchains, make sure that the TPLs are built
   with the compiler you intend to use for Dakota and not the system
   compiler. It may be necessary to explicitly enable C++11 when
   compiling Boost, particularly with GCC compilers.


.. _devenv-mac-clang-homebrew:

**Verified on MacOS 11 (Big Sur):** System Clang + GFortran

Uses system-provided clang/clang++, perl, python, VecLib/Accelerate,
with Homebrew GFortran. Demonstrated on Intel MacBook Pro (2020)

1. Follow the instructions on the homebrew website to install the
   homebrew package manager.

2. Install packages:

   ::

      brew install cmake gcc open-mpi boost gsl hdf5 openjdk@11

3. Sample basic build:

   ::

      cmake -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ -D CMAKE_Fortran_COMPILER=gfortran -D BOOST_ROOT:PATH=/usr/local -D DAKOTA_HAVE_MPI:BOOL=TRUE -D HAVE_QUESO:BOOL=TRUE -D DAKOTA_HAVE_GSL:BOOL=TRUE -D DAKOTA_HAVE_HDF5:BOOL=TRUE ../source



**Verified August 2020 (probably on OS X 10.15 Catalina):**

Follow the instructions on the homebrew website to install the
homebrew package manager.

- Install the desired version of gcc from homebrew::

     brew install gcc@7

Boost:

- Download a source package for Boost 1.69 and unpack it someplace.
- Follow the instructions on the Boost website to build Boost.Build.
- Create ~/user-config.jam with the following in it. (Switch gcc-7 to the desired version, if different)::

     using darwin : : gcc-7 : <cxxflags>"-std=c++11" ;

- At the top of the boost source directory, invoke b2 with the following options, making sure to replace/set ``$INSTALL_PATH`` and ``$BUILD_PATH`` to something convenient::

     b2 link=static --without-python define=_GLIBCXX_USE_CXX11_ABI=1 --prefix=$INSTALL_PATH --build-dir=$BUILD_PATH install


HDF5::

   export CXX=<homebrew C++ compiler>
   export CC=<homebrew C compiler>
   export CXXFLAGS='-std=c++11 -D_GLIBCXX_USE_CXX11_ABI=1'
   ./configure --enable-cxx --prefix=<install location>
   make install


.. note::

   When building Dakota with C++11, CMake
   ``-D_GLIBCXX_USE_CXX11_ABI=1`` by default. You can see this by
   running `make VERBOSE=1`. This define is `explained here
   <https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_dual_abi.html>`_. The
   above configuration produces a compatible Boost build.

   Alternately, one might be able to explicitly disable this by
   adding ``-D_GLIBCXX_USE_CXX11_ABI=0`` to ``CMAKE_CXX_FLAGS``; CMake
   is smart enough to pass that option to the compiler only once, with
   it set to 0. Which of these is the best approach, building with the
   define turned on or off, is currently unclear. Things just need to be
   consistent.

.. attention::

   The following details for OS X 12.12 are outdated but serve an archival
   role for providing hints for building on Mac OS X more generally.

**Verified on OS X 10.12:** All homebrew approach. 

1. Follow the instructions on the homebrew website to install the
   homebrew package manager.

2. Install gcc@5, which includes all needed compilers, with the
   command

   ::

      brew install gcc@5

3. Install CMake with the command

   ::

      brew install cmake

4. Install Boost 1.55. It is necessary to instruct brew to build Boost
   from source using the gcc-5 compiler.

   ::

      brew install --build-from-source --cc=gcc-5 boost@1.55

5. Optional (needed for message-passing parallelism): Install OpenMPI
   1.6, building it from source, too.

   ::

      brew install --build-from-source --cc=gcc-5 open-mpi@1.6

6. Optional (needed to use bayes_calibration queso): GSL 1.16 from
   source.

   ::
   
      brew install --build-from-source --cc=gcc-5 gsl@1.16


**Verified on OS X 10.9 -- 10.12:** CMake from DMG, XCode, GCC
compilers from http://hpc.sourceforge.net. Use them to build Boost,
OpenMPI, and GSL from source.


.. _devenv-windows:

-------
Windows
-------

We regularly compile 64-bit Dakota binaries on Windows 10 64-bit with
the following tools:

- CMake: install using Windows installer from http://cmake.org
- Microsoft Visual Studio Community 2019, update 11 (version 16.11)
- Intel Fortran: Intel Parallel Studio XE 2020, update 4 to work with MSVS
- Python 3.10.6 
- Strawberry Perl 5.32.1.1
- Eclipse Temurin JDK 11 (LTS) HotSpot

And packages:

- BLAS/LAPACK: reference LAPACK 3.10.1 built statically with
  MSVS/Intel Fortran, e.g., ``cmake -G "Visual Studio 16 2019" -A
  x64 -D BUILD_SHARED_LIBS=FALSE``
- Boost 1.71.0 binary package for MSVC 14.2
- HDF5 1.10.8 built statically with MSVS
