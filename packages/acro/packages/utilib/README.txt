
UTILIB 4.1
==========

A C++ software library that provides general, portable utilities for
developing C++ codes.  UTILIB provides utilities for random number
generation, abstract data types (including dense arrays and matrices),
sorting routines, I/O (including parallel I/O), common numerical routines
and for masking system-specific differences in operating systems.

UTILIB has largely been developed to support the Acro optimization solver
libraries.  UTILIB is managed with the Acro optimization repository.


INSTALLATION
============

See the INSTALL.txt file.


LICENSE
=======

See the LICENSE.txt file.


GETTING STARTED
===============

Directories

   bin          - Scripts and executables
   config       - A configuration directory for building with autoconf
   doc          - Documentation and makefiles for applying doxygen
   examples     - Simple examples of UTILIB capabilities
   mvs          - MSVS project files
   site_scons   - A configuration directory for building with SCONS
   src          - The main source directories
   test         - Directories of UTILIB software tests
   utilib       - Include files

Documentation and Bug Tracking

    The UTILIB Trac wiki supports documentation, issue tracking,
    and browsing of the UTILIB subversion repository:

        https://software.sandia.gov/trac/pyutilib

    The utilib/doc directory contains documentation that is included
    with the UTILIB software.

Authors

   See the AUTHORS.txt file.

Project Managers

   William E. Hart, wehart@sandia.gov

Mailing Lists

   acro-developers@software.sandia.gov
   acro-users@software.sandia.gov


THIRD PARTY SOFTARE
===================

UTILIB relies on several packages for performing tests: EXACT (included in the Python Fast package), and CxxTest.  Both of these are included in Acro, and UTILIB is commonly tested within the acro-utilib project.

The UTILIB source code does not rely on any third-party libraries.

