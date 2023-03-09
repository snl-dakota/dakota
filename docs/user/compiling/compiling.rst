.. _compiling-main:

Compiling Dakota
================

For more experienced users, Dakota can be built from source
code. Consider this option if:

- You want to customize Dakota with additional packages, e.g., enable
  QUESO or an experimental feature

- You are installing to a platform not explcitly listed in the binary
  executable distribution list (including Linux other than
  RHEL-compatible)

- You have previously licensed DOT, NPSOL, and/or NLPQL and want to
  add these packages to the Dakota build.

Compiling Dakota from source requires:

- A :ref:`development environment <devenv>` that has Dakota's
  dependencies.

- :ref:`Obtaining the source code <getsource>` via download or Git.

- :ref:`Configuring with CMake <configure>` to (1) set Dakota options
  as well as external software locations and options, and (2) generate
  a development environment-specific build tree.

- Building Dakota, i.e. running the development environment's :ref:`build
  process <build>` to compile and link the Dakota executable, e.g.,
  using Unix ``make``, Windows Visual Studio, or ``nmake``.

- :ref:`Running and assessing tests <test>`.


**Quickstart Build**

This simple example shows the Dakota build process (configure,
compile, install) on a Unix-like platform with mostly default
options::

   mkdir build 
   cd build 

   # configure the build tree 
   cmake -D CMAKE_INSTALL_PREFIX=/path/to/dakota-installation /path/to/dakota-source 

   # compile/link, specifying the number of processors 
   make -j 4 

   # run a minimal set of Dakota acceptance tests (these should all pass) 
   ctest -j 4 -L Accept

   # install to the specified prefix
   make install 


The remaining pages in this section describe the compilation process
and options in more detail.

.. toctree::
   :maxdepth: 1
   
   devenv
   getsource
   configure
   build
   test
   examples
