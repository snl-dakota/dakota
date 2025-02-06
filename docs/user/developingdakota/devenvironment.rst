.. _devenvironment-main:

""""""""""""""""""""""""""""""""""
Setup Your Development Environment
""""""""""""""""""""""""""""""""""

RHEL 8 is the recommended Dakota development environment, as Dakota
tests are baselined (primarily) to stock RHEL8 packages. SNL
developers should see team documentation for access to RHEL 8
resources
.
See :ref:`devenv` for pointers on setting up a development environment
on various platforms, and overall information on :ref:`compiling-main`.

A typical developer CMake configuration of Dakota, e.g., on RHEL 8
includes the use of the ``DevDistro`` convenience option which enables
developer compiler options, documentation, and specification
maintenance and some typical features::

   cmake \
   -D DevDistro:BOOL=ON -D DAKOTA_HAVE_HDF5:BOOL=TRUE \
   -D HAVE_QUESO:BOOL=ON -D DAKOTA_HAVE_GSL:BOOL=ON" \
   -D DAKOTA_PYTHON:BOOL=TRUE -D DAKOTA_PYTHON_DIRECT_INTERFACE:BOOL=TRUE -D DAKOTA_PYTHON_DIRECT_INTERFACE_NUMPY:BOOL=TRUE -D DAKOTA_PYTHON_SURROGATES:BOOL=TRUE -DDAKOTA_PYTHON_WRAPPER:BOOL=TRUE \
   ../source

Typically on stock RHEL 8, the following are also necessary::

   cmake3 \
   <options from above>
   -D JAVA_HOME:PATH=/usr/lib/jvm/java-11 \
   -D Boost_NO_BOOST_CMAKE:BOOL=TRUE -D Boost_NO_SYSTEM_PATHS:BOOL=TRUE -D BOOST_INCLUDEDIR:PATH=/usr/include/boost169 -DBOOST_LIBRARYDIR:PATH=/usr/lib64/boost169
   ../source

