.. _build:

""""""""""""""""""""""""
Build and Install Dakota
""""""""""""""""""""""""

Compiling Dakota typically involves running make or another
system-native build tool.

**Unix-like with make:** To build Dakota on Unix, Linux, Mac, and similar
platforms: ::

  cd $DAK_BUILD
  make -j <N>

where the optional ``-j N`` specifies to build with ``N`` concurrent
processes. When ready to install or generate binary or source
packages: ::

  make -j <N> install

  make package

  make package_source

**Windows with Visual Studio:** To build Dakota on Windows:

- Open ``$DAK_BUILD/Dakota.sln`` in Visual Studio
- Use Configuration Manager to choose Release or Debug build
- Build the ``ALL_BUILD`` (or an individual) target

It's also possible to build on Windows at the command line using the
``devenv`` or ``nmake`` commands. When ready to install or package,
build the corresponding targets in Visual Studio.


**CMake-driven Build:** It's also possible to drive the steps using the
platform-agnostic ``cmake --build`` command, e.g., ::

  cd $DAK_BUILD
  cmake --build .
  cmake --install .
