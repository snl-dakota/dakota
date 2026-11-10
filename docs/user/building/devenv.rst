.. _devenv:

"""""""""""""""""""""""""""""
Setup Development Environment
"""""""""""""""""""""""""""""

Dakota has the following toolchain and software package
dependencies. See sections that follow for suggestions on how to
satisfy them.

Dakota regression test baselines are based on the recommended Dakota
RHEL 9 configuration below. Regression tests will typically exhibit
numerical differences when running on other platforms. For developers
needing to reproduce RHEL 9 baselines on other platforms, e.g., for
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
     - >= 3.23
     - Required
   * - C Compiler
     - >= C99
     - Required
   * - C++ Compiler
     - >= C++20
     - Required
   * - Fortran Compiler
     - 77, 90
     - Required
   * - `Python <https://www.python.org/>`_
     - 3.x; >= 3.10 for ``DAKOTA_PYTHON_STUDY``
     - Required; Python 3 only
   * - `Perl <https://www.perl.org/>`_
     - >= 5.16
     - Required
   * - Java JDK
     - = 11
     - Optional; required for ``DAKOTA_JAVA_SURROGATES``
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
   * - `Boost <https://www.boost.org/>`_
     - >= 1.75
     - Required; headers, plus libraries: filesystem, program_options,
       regex, serialization, system
   * - MPI
     - any
     - Optional, e.g., MPICH, OpenMPI
   * - `HDF5 <https://www.hdfgroup.org/>`_
     - >= 1.10.4
     - Optional; required for Dakota structured results output
   * - `h5py <https://www.h5py.org/>`_
     -
     - Optional; required to enable HDF5-based tests (recommended for
       developers)
   * - `GSL <https://www.gnu.org/software/gsl/>`_
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

- If compiling HDF5 from source, C++ and HL support must be enabled
  (``--enable-cxx`` and ``--enable-hl`` options to configure), which are not the defaults when
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
RHEL 9
------

The following packages work for building Dakota on RHEL 9, Rocky Linux 9,
or AlmaLinux 9. This is the platform used for Dakota regression test
baselines.

First enable the EPEL and CodeReady Builder (CRB) repositories::

   dnf install https://dl.fedoraproject.org/pub/epel/epel-release-latest-9.noarch.rpm
   dnf config-manager --set-enabled crb

Then install packages::

   dnf install \
       cmake clang clang-tools-extra gcc-gfortran git make ninja-build patch perl \
       atlas atlas-devel blas blas-devel lapack lapack-devel \
       boost-devel \
       openmpi openmpi-devel \
       hdf5 hdf5-devel \
       gsl gsl-devel \
       python3.12 python3.12-devel python3.12-pip python3.12-setuptools

   # Optional; only needed for DAKOTA_JAVA_SURROGATES
   dnf install java-17-openjdk-devel

   # Optional; only needed for generating C++ code docs
   dnf install doxygen

The ``boost-devel`` package from EPEL satisfies the >= 1.75 requirement.

Python packages for HDF5-based tests are most easily installed in a
virtual environment::

   python3.12 -m venv /path/to/dakota-venv
   source /path/to/dakota-venv/bin/activate
   pip install h5py numpy scipy

For documentation builds, also install Sphinx and required plugins into
the same environment::

   pip install sphinx myst-parser sphinx-rtd-theme sphinxcontrib-bibtex \
       sphinx-copybutton sphinx-inline-tabs pydantic autodoc-pydantic

OpenMPI binaries and libraries are installed under ``/usr/lib64/openmpi``
and require either loading the environment module or setting paths
manually before building or running Dakota::

   module load mpi/openmpi-x86_64

or equivalently::

   export MPI_HOME=/usr/lib64/openmpi
   export PATH=/usr/lib64/openmpi/bin:$PATH

Basic build example::

   source /path/to/dakota-venv/bin/activate
   module load mpi/openmpi-x86_64
   cmake -D CMAKE_C_COMPILER=clang \
         -D CMAKE_CXX_COMPILER=clang++ \
         -D CMAKE_Fortran_COMPILER=gfortran \
         -D DAKOTA_HAVE_MPI=ON \
         -D DAKOTA_HAVE_HDF5=ON \
         /path/to/dakota/source


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
macOS / OS X. Typically the OS-provided Accelerate linear algebra
suffices. These are listed in reverse chronological order, in hopes of
archiving useful information, with the first being the most promising
approach currently.

.. note::

   Mac SIP and other security features can be highly problematic when
   running Dakota tests or even using the Dakota binaries.

.. note::

   If you build your own toolchains, make sure that the TPLs are built
   with the compiler you intend to use for Dakota and not the system
   compiler.


.. _devenv-mac-clang-homebrew:

**Verified on macOS Tahoe (Apple Silicon):** System Clang + GFortran + Homebrew

Uses system-provided clang/clang++, VecLib/Accelerate for linear algebra,
with Homebrew for the remaining dependencies.

1. Follow the instructions on the `Homebrew <https://brew.sh>`_ website to
   install Homebrew.

2. Install packages::

      brew install cmake gcc open-mpi boost@1.85 gsl hdf5 openjdk

   ``gcc`` is installed for GFortran only; C and C++ use Apple Clang.
   ``boost@1.85`` is used rather than the unversioned ``boost`` formula
   to ensure a known-working version.

3. Set ``JAVA_HOME`` for the CMake configure step (only needed for
   ``DAKOTA_JAVA_SURROGATES``)::

      export JAVA_HOME=/opt/homebrew/opt/openjdk

4. Sample basic build (from the build directory)::

      cmake \
        -D CMAKE_Fortran_COMPILER=gfortran \
        -D BOOST_ROOT=/opt/homebrew/Cellar/boost@1.85/1.85.0_3 \
        -D DAKOTA_HAVE_MPI=ON \
        -D DAKOTA_HAVE_HDF5=ON \
        -D HDF5_ROOT=/opt/homebrew/Cellar/hdf5/2.2.0 \
        -D DAKOTA_HAVE_GSL=ON \
        -D GSL_ROOT_DIR=/opt/homebrew/Cellar/gsl/2.8 \
        /path/to/dakota/source

   Adjust the Cellar version numbers to match what Homebrew installed
   (``brew info boost@1.85`` etc. will show the installed path).
   CMake will use system Apple Clang for C and C++ by default.
   Linear algebra is satisfied by the system VecLib/Accelerate framework.


