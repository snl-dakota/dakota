:orphan:

.. _releasenotes-template:

.. note::

   This unlinked page is a template with prompts to consider when
   authoring release notes before promoting to a versioned page.
   Remove :orphan: above if promoting this page.

""""""""""""""""""""""""""""""""""""""""
Version 6.22 (2025/05/15); pre-release)
""""""""""""""""""""""""""""""""""""""""

**Highlight: Feature A**

*Enabling / Accessing:* Where available and how to access

*Documentation:* Where to learn more

For example:

The Dakota GUI has added many significant feature improvements over
the last year.  The Dakota GUI now allows you to seamlessly browse
Dakota's HDF5 database output files, as well as generate sophisticated
graphical plots from HDF5 data with just a few clicks.  HDF5 browsing
and plot generation can also be driven within Dakota GUI by Next-Gen
Workflow, a powerful tool that allows you to construct node-based
workflows for complex tasks.

*Enabling / Accessing:* Dakota GUI ships with Dakota and is available
for Windows, Mac, and RHEL7.

*Documentation:* An enhanced version of the Dakota GUI manual now ships
with the GUI, giving you easy access to a wealth of reference material
for using the GUI.  The 6.11 GUI manual is also available here.


**Improvements by Category**

*Interfaces, Input/Output*

- The pyprepro template processing tool now supports JSON format Dakota parameters files.

*Models*

*Optimization Methods*

- ROL optimizer was updated to release 2.0.

*UQ Methods*

- Support for MUQ's multilevel MCMC algorithm was added.

*MLMF Sampling*

 
**Miscellaneous Enhancements and Bugfixes**

- Enh: Dakota now uses GoogleTest instead of Boost.Test
- Enh: Dakota now uses std::filesystem when available instead of Boost.filesystem
- Enh: The Trilinos snapshot was updated to version 16.1
- Bug fix: Small fixes to JEGA to satisfy recent versions of clang (issues `#178 <https://github.com/snl-dakota/dakota/issues/178>`_ and `#94 <https://github.com/snl-dakota/dakota/issues/94>`_)
- Bug fix: Boost version check fixed (issue `#163 <https://github.com/snl-dakota/dakota/issues/163>`_)
- Bug fix: Broken links in examples (issue `#162 <https://github.com/snl-dakota/dakota/issues/162>`_)

**Deprecated and Changed**

**Compatibility**

- Dakota now requires C++17 and CMake 3.23

**Other Notes and Known Issues**
