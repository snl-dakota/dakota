.. _examples:

"""""""""""""""""""""""""""""""
End-to-end Compilation Examples
"""""""""""""""""""""""""""""""

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
$DAK_SRC refers to this directory.

   ::

      export DAK_SRC=$HOME/dakota-<release>.<platform>

3. Create separate build directory, e.g. $HOME/dakota-build. In the
   instructions below, $DAK_BUILD refers to the directory that you create
   for CMake to configure and build Dakota.

   ::

      mkdir -p /path/to/Dakota/build/directory
      export DAK_BUILD=/path/to/Dakota/build/directory

4. Make a copy of the template BuildDakotaTemplate.cmake to customize a
   CMake Dakota build for your platform. Keep the file in the
   $DAK_SRC/cmake directory to use for subsequent Dakota CMake builds.

   ::

      cp $DAK_SRC/cmake/examples/BuildDakotaTemplate.cmake $DAK_SRC/cmake/BuildDakotaCustom.cmake

5. Update $DAK_SRC/cmake/BuildDakotaCustom.cmake file to reflect your
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
      cmake -C $DAK_SRC/cmake/BuildDakotaCustom.cmake $DAK_SRC
      make [-j#]
      make install

7. Set paths and library paths. In the instructions below, $DAK_INSTALL
   refers to the Dakota installation path you specified for the variable
   CMAKE_INSTALL_PREFIX in your BuildCustom.cmake file.

   ::

      export PATH=$DAK_INSTALL/bin:$DAK_INSTALL/test:$PATH
      export LD_LIBRARY_PATH=$DAK_INSTALL/lib:$DAK_INSTALL/bin:$LD_LIBRARY_PATH

8. Test that Dakota is working, eg

   ::

      which dakota
      dakota -v

These should return the path to the dakota executable and a couple of
lines of text indicating the version of dakota built, respectively.

.. note::

   The following tips apply to all types of builds:

   * 'make [-j#]': Dakota supports concurrent build processes

   * 'make install' requires write privileges in CMAKE_INSTALL_PREFIX.

   * Executing cmake without any options will result in an attempt to
     build with as many vendor algorithms and packages as are available in
     the distribution.  The Dakota website describes how to turn off
     features.

   * Once make has successfully completed, the generated Dakota
     executables (dakota and dakota_restart_util) will reside
     in $DAK_BUILD/src.  If 'make install' is invoked, copies of
     the   executables will be placed in CMAKE_INSTALL_PREFIX/bin
     and copies of the libraries (libdakota.a, etc.) are placed in
     CMAKE_INSTALL_PREFIX/lib. You may set CMAKE_INSTALL_PREFIX in
     BuildDakotaCustom.cmake.

