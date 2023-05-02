:orphan:

.. _releasenotes-template:

.. note::

   This unlinked page is a template with prompts to consider when
   authoring release notes before promoting to a versioned page.
   Remove :orphan: above if promoting this page.

""""""""""""""""""""""""""""""""""""""
Version 6.18 (2023/05/15; pre-release)
""""""""""""""""""""""""""""""""""""""

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

*Models*

*Optimization Methods*

*UQ Methods*

Tolerance Intervals - Ernesto should add detail.

*MLMF Sampling*

*Sensitivity Analsys*

Standardized Regression Coefficients - Russell should add detail.
 
**Miscellaneous Enhancements and Bugfixes**

- Enh:

- Bug fix: Correlation matrices now receive the correct variable labels
  in studies that include variables from more than one category (e.g. mixture
  of design and aleatory uncertain).

- Bug fix: Standard moments are now written correctly to HDF5 for stochastic
  expansion methods

- Bug fix: No datasets are written to HDF5 for PDFs for zero-variance responses.
  This matches the console output. Previously, empty datasets were written.



**Deprecated and Changed**

**Compatibility**

- Dakota's snapshot of Trilinos is now version 13.4.
- Dakota now requires C++14 to build.
- There are no further changes to TPLs or requirements for
  this release.

**Other Notes and Known Issues**
