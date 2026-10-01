.. _library-api:

Library APIs
============

Dakota's C++ and Python library APIs construct and run studies without first
writing a complete Dakota input file.  They use the same configuration grammar
as JSON input, but replace pointer-based relationships between Dakota blocks
with references to constructed objects.

The APIs are public and supported, but are still evolving.  The methods and
models available through a particular installation depend on the capabilities
enabled when Dakota was built.

.. toctree::
   :maxdepth: 1

   libraryapi/construction
   libraryapi/tutorial
   libraryapi/python-reference

The generated C++ reference is part of the Dakota Developers Manual.  The
guide and tutorial here describe the public construction model; the C++
reference documents the exact overloads available in a build.

