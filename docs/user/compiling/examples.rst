.. _examples:

"""""""""""""""""""""""""""""""
End-to-end Compilation Examples
"""""""""""""""""""""""""""""""

=====
RHEL9
=====

On RHEL9, Dakota can be built using tools and libraries available from the repos.

1. Determine whether the required repositories are enabled on your system. You
   must have baseos, appstream, and codeready-builder.

   :: 

       dnf repolist


   Enable any that are missing, e.g.:
   
   ::
      
       sudo subscription-manager repos --enable codeready-builder-for-rhel-9-$(arch)-rpms

2. Install required packages:

   ::

       sudo dnf install git make cmake gcc gcc-c++ gcc-gfortran perl python3 blas-devel lapack-devel boost-devel openmpi-devel atlas-devel

3. Obtain Dakota source and unpack it. Alongside the source folder, create a build folder.

4. Change to the build folder and configure. In the command below, ``$DAK_INSTALL`` is the desired installation location and ``$DAK_SOURCE``
   is the path to the unpacked source.

   ::

       cmake -DCMAKE_INSTALL_PREFIX=$DAK_INSTALL
             -DDAKOTA_HAVE_MPI=ON \
             -DMPI_CXX_COMPILER=/usr/lib64/openmpi/bin/mpicxx \
             -DBLAS_LIBS=/usr/lib64/atlas/libsatlas.so \
             -DLAPACK_LIBS=/usr/lib64/atlas/libsatlas.so \
             -DDAKOTA_HAVE_HDF5=ON \
             -DHAVE_MUQ=ON \
             -DDAKOTA_JAVA_SURROGATES=ON \
             -DDAKOTA_PYTHON=ON \
             -DDAKOTA_PYTHON_DIRECT_INTERFACE=ON \
             -DDAKOTA_PYTHON_DIRECT_INTERFACE_NUMPY=ON \
             -DDAKOTA_PYTHON_SURROGATES=ON \
             -DDAKOTA_PYTHON_WRAPPER=ON \
             -DDAKOTA_HAVE_GSL=ON \
             -DHAVE_QUESO=ON \
             ../$DAK_SOURCE
       
5. Build and install:

   ::

       make install

6. Set the paths. In the instructions below, ``$DAK_INSTALL``
   refers to the Dakota installation path you specified for the variable
   ``CMAKE_INSTALL_PREFIX`` during the configure step.

   ::

      export PATH=$DAK_INSTALL/bin:$DAK_INSTALL/share/dakota/test:$PATH
      export PYTHONPATH=$DAK_INSTALL/share/dakota/Python:$PYTHONPATH

7. Test that Dakota is working, eg

   ::

      which dakota
      dakota -v

These should return the path to the dakota executable and a couple of
lines of text indicating the version of dakota built, respectively.

=====
RHEL7
=====

The following are instructions for building Dakota on a Redhat 7 platform
in a Bash shell.

1. Install the recommended version of CMake, and ensure it is in your $PATH.
2. Extract Dakota source archive file. In the example below, Dakota will
   be extracted into the user's home directory, /home/username.

   ::

      cd $HOME
      tar xzvf /path/to/dakota-<release>.src.tar.gz

You will see a new directory,
/home/username/dakota-<release>.<platform>. In the instructions below,
``$DAK_SOURCE`` refers to this directory.

   ::

      export DAK_SRC=$HOME/dakota-<release>.<platform>

3. Create separate build directory, e.g. $HOME/dakota-build. In the
   instructions below, ``$DAK_BUILD`` refers to the directory that you create
   for CMake to configure and build Dakota.

   ::

      mkdir -p /path/to/Dakota/build/directory
      export DAK_BUILD=/path/to/Dakota/build/directory

4. Make a copy of the template BuildDakotaTemplate.cmake to customize a
   CMake Dakota build for your platform. Keep the file in the
   ``$DAK_SOURCE/cmake`` directory to use for subsequent Dakota CMake builds.

   ::

      cp $DAK_SOURCE/cmake/examples/BuildDakotaTemplate.cmake $DAK_SOURCE/cmake/BuildDakotaCustom.cmake

5. Update ``$DAK_SOURCE/cmake/BuildDakotaCustom.cmake`` file to reflect your
   platform configuration. Instructions are provided in that file.

As an example, consider the need to specify a custom location for the
Boost 1.69 library, eg in ``/extra/boost_1_69_0``.  The appropriate lines
to uncomment and modify in the BuildDakotaCustom.cmake file are these::

  set(BOOST_ROOT
      "/extra/boost_1_69_0"
      CACHE PATH "Use non-standard Boost install" FORCE)
  set( Boost_NO_SYSTEM_PATHS TRUE
       CACHE BOOL "Supress search paths other than BOOST_ROOT" FORCE)

Consider also specifying a custom location for the Dakota installation
artifacts, eg::

  set( CMAKE_INSTALL_PREFIX
       "~/temp/dakota-installation"
       CACHE PATH "Path to Dakota installation" )

6. Configure and build Dakota.

   ::

      cd $DAK_BUILD
      cmake -C $DAK_SOURCE/cmake/BuildDakotaCustom.cmake $DAK_SOURCE
      make [-j#]
      make install

7. Set the paths. In the instructions below, ``$DAK_INSTALL``
   refers to the Dakota installation path you specified for the variable
   ``CMAKE_INSTALL_PREFIX`` in your BuildCustom.cmake file.

   ::

      export PATH=$DAK_INSTALL/bin:$DAK_INSTALL/share/dakota/test:$PATH
      export PYTHONPATH=$DAK_INSTALL/share/dakota/Python:$PYTHONPATH

8. Test that Dakota is working, eg

   ::

      which dakota
      dakota -v

These should return the path to the dakota executable and a couple of
lines of text indicating the version of dakota built, respectively.

.. note::

   The following tips apply to all types of builds:

   * ``make [-j#]``: Dakota supports concurrent build processes

   * ``make install`` requires write privileges in ``CMAKE_INSTALL_PREFIX``.

   * Executing cmake without any options will result in an attempt to
     build with as many vendor algorithms and packages as are
     available in the distribution.  See options in
     :ref:`configure-dakota-features` to turn off features.

   * Once make has successfully completed, the generated Dakota
     executables (dakota and dakota_restart_util) will reside in
     :file:`$DAK_BUILD/src`.  If ``make install`` is invoked, copies
     of the executables will be placed in ``CMAKE_INSTALL_PREFIX/bin``
     and copies of the libraries (``libdakota.a``, etc.) are placed in
     ``CMAKE_INSTALL_PREFIX/lib``. You may set
     ``CMAKE_INSTALL_PREFIX`` in ``BuildDakotaCustom.cmake``.


=====
MacOS
=====

A terse example of how to build at the command line using the
:ref:`mixed system Clang plus Homebrew gfortran example
<devenv-mac-clang-homebrew>`

.. code-block::

   cmake -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ -D CMAKE_Fortran_COMPILER=gfortran \
   -D BOOST_ROOT:PATH=/usr/local -D DAKOTA_HAVE_MPI:BOOL=TRUE \
   -D HAVE_QUESO:BOOL=TRUE -D DAKOTA_HAVE_GSL:BOOL=TRUE
   -D DAKOTA_HAVE_HDF5:BOOL=TRUE ../source

   make -j 4

==========
Windows 10
==========

A terse example of how to build using the :ref:`devenv-windows`
example environment. Representative paths to installed libraries are
shown in the configuration commands below.

In a command prompt (alternately can of course make CMake settings via
the GUI or a cache initialization file):

.. code-block::

   cmake.exe -G "Visual Studio 16 2019" -A x64 ^
   -D BLAS_LIBS=C:/local/64bit/lapack/3.10.1/static/lib/BLAS.lib -D LAPACK_LIBS=C:/local/64bit/lapack/3.10.1/static/lib/LAPACK.lib ^
   -D Boost_INCLUDE_DIR=C:/local/64bit/boost_1_71_0 -D Boost_LIBRARY_DIR=C:/local/64bit/boost_1_71_0/lib64-msvc-14.2 ^
   -D BUILD_SHARED_LIBS=FALSE ^
   -DDAKOTA_HAVE_HDF5:BOOL=TRUE -DHDF5_USE_STATIC_LIBRARIES:BOOL=TRUE -DHDF5_ROOT="C:/local/64bit/hdf5.static.nozlib/1.10.8" ^
   -D DAKOTA_PYTHON_DIRECT_INTERFACE:BOOL=TRUE -D DAKOTA_PYTHON_DIRECT_INTERFACE_NUMPY:BOOL=FALSE -D DAKOTA_PYTHON_SURROGATES:BOOL=FALSE ^
   ../source

Then:

#. Open the Visual Studio solution file :file:`Dakota.sln` in Visual Studio

#. Choose a build configuration, e.g., ``Release`` or ``Debug``, using Configuration Manager

#. Build the ``ALL_BUILD`` and optionally ``INSTALL`` or ``PACKAGE`` targets

Alternately the build can be driven with ``nmake``, ``devenv``, or
``cmake --build`` with appropriate arguments.
