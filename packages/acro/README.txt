Acro 2.0: Project acro-dakota
=============================

Acro integrates a rich variety of optimization libraries and solvers
that have been developed for large-scale engineering and scientific
applications. Acro was developed to facilitate the design, development,
integration and support of optimization software libraries. Thus, Acro
includes both individual optimization solvers as well as optimization
frameworks that provide abstract interfaces for flexible interoperability
of solver components. Furthermore, many solvers included in Acro can
exploit parallel computing resources to solve optimization problems
more quickly.

See the Acro home page for further details about the capabilities that
are integrated into Acro:  http://software.sandia.gov/Acro

INSTALLATION
============

See the doc/INSTALL.txt file.


LICENSE
=======

See the doc/LICENSE.txt file.


GETTING STARTED
===============

Directories

   admin        - Administrative scripts
   bin          - Scripts and executables
   bootstrap    - Config/makefile fragments shared between Acro projects
   config       - A configuration directory for building with autoconf
   doc          - Documentation and makefiles for applying doxygen
   lib          - Installation directory for Acro libraries
   mvs          - MSVS project files
   packages     - The main Acro source
   test         - A directory for configuration and build logfiles.
   tpl          - Third-party libraries bundled with Acro.
   useraps      - Top-level examples of Acro functionality


Documentation and Bug Tracking

    The Acro Trac wiki supports documentation, issue tracking,
    and browsing of the Acro subversion repository:

        https://software.sandia.gov/trac/acro

Authors

   See the https://software.sandia.gov/trac/acro Trac pages

Project Managers

   William E. Hart, wehart@sandia.gov

Mailing Lists

   acro-developers@software.sandia.gov
   acro-users@software.sandia.gov


THIRD PARTY SOFTARE
===================

Acro relies on a variety of third-party software libraries that are bundled with the software.  Additionall, Acro can integrate the following external libraries:

 * mpich - MPI communication
 * cplex - Linear programming

