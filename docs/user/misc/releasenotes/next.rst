.. _releasenotes-next:

.. note::

   These are release notes for the development / stable version of
   Dakota, accumulated since the last release.  It will typically
   contain template prompts to be removed when finalizing release
   notes and promoting to a versioned page.

""""""""""""""""""""""""""""""""""""""
Version 6.17 (2022/11/15; pre-release)
""""""""""""""""""""""""""""""""""""""

**Highlight: Integrated User Manual**

Dakota 6.17 includes a beta version of a new Sphinx-based integrated
user manual. It aggregates content from the historical User's, Theory,
and Reference manuals, as well as the Dakota website. Feedback on this
new compendium is invited.

*Enabling / Accessing:* Primarily from https://dakota.sandia.gov, also
in :file:`<dakota_src>/docs/user`.

*Documentation:* Where to learn more

**Highlight: User Support Moves to Github**

External user support is moving to Github Discussions. Discussions offers
many advantages over the dakota-users listserv, including threaded
conversations, advanced formatting via markdown, LaTeX-formatted mathematical
expressions, and more.

We plan to operate dakota-users and the new Discussions forums concurrently
until February 15, 2023, when dakota-users will be shut down.

*Enabling / Accessing:* Create an account on https://github.com/ and visit
Dakota's `Discussions page <https://github.com/orgs/snl-dakota/discussions>`_.


**Improvements by Category**

*Graphical User Interface*

- Experimental Multi-level/Multi-fidelity UQ editor adds support for
  ensemble vs. hierarchical surrogate models and has basic estimator
  performance prediction based on pilot samples.

*Interfaces, Input/Output*

- Allow the experimental Python plugin interface to work correctly
  as a callback underneath a top-level Dakota Python environment.

*Models*

*Optimization Methods*

*UQ Methods*

- MLMF sampling: Fixed bugs and improved debug output when using
  online cost recovery.

- Update (experimental) MUQ package for Bayesian calibration
 
**Miscellaneous Enhancements and Bugfixes**

- Enh: Make build system more tolerant of HDF5/h5py variants when
  registering unit tests. Allow working with h5py 3.0+.

- Enh: Routine testing on RHEL8 operating system with system-default
  compilers and tools.

- Enh: Console output/errors are more reliably and consistently
  redirected when options are given in the input file. (Most
  TPL-generated output will still not appear in them however).

- Enh: Restart file versioning. Restart files now have embedded
  version information and will attempt to detect whether an imported
  file is compatible.

- Bug fix: MLMF UQ methods properly generate tabular output for
  concurrent evaluations

- Bug fix: Dakota should now compile when using a C++17-compliant
  compiler. (Thanks to Heidi Thornquist for reporting and testing.)

**Deprecated and Changed**

- Dakota no longer supports Python 2 for Dakota-provided Python
  utilities such as dprepro or dakota.interfacing, though tools may
  continue to work for some time.

- Migrated to std C++ random number generators and distribution
  utilities in a number of places in Dakota and associate TPLs. This
  favored over rand()/srand() and Boost utilities. Will break
  reproducibility of specific sample sets w.r.t. historical Dakota
  versions, even on the same architecture and toolchain.

- Future (post-6.17) Dakota releases will require a C++14-compliant
  compiler and will only be tested on gcc-8.5.0 and newer (and similar
  vintage) compilers.

- LaTeX is no longer required to build default versions of Dakota
  manuals.

**Compatibility**

- CMake 3.17 or newer is required to compile Dakota

- Documentation tools:

  * Sphinx and associated plugins are required for building the new
    integrated user manual.

  * Java 11 JDK is required for documentation generation and input
    specification maintenance.

  * Doxygen 1.8.20 is required for building the developer manual.

- No changes to minimum C++11 compiler requirement, Boost 1.69, or
  required/optional external TPLs.

**Other Notes and Known Issues**
