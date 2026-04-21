:orphan:

.. _releasenotes-template:

.. note::

   This unlinked page is a template with prompts to consider when
   authoring release notes before promoting to a versioned page.
   Remove :orphan: above if promoting this page.

""""""""""""""""""""""""""""""""""""""""
Version 6.24 (2026/05/15); pre-release)
""""""""""""""""""""""""""""""""""""""""

**Highlight: Feature A**

*Enabling / Accessing:* Where available and how to access

*Documentation:* Where to learn more

**Highlight: Sensitivity Analysis Chapter**

*Enabling / Accessing:* Available in the User Manual


**Highlight: JSON-format Input Files**

Dakota now accepts JSON-format input files. The schema is documented
in XXX. JSON support is meant to simplify incorporation of Dakota
into larger workflows, its use in agentic workflows, and and as a
stepping stone toward more capable Python bindings.

The capability is experimental, and we appreciate your bug
reports. To fall back to the old input file reader, run Dakota
with the `-parser legacy` command line argument.

*Enabling / Accessing:* Write a JSON format input file and use the
`-json` command line argument (e.g. `dakota -json dakota_in.json`).

*Documentation*: TODO




**Improvements by Category**

*Interfaces, Input/Output*


*Models*

*Optimization Methods*


*UQ Methods*


*MLMF Sampling*
 
*Sensitivity Analysis*

- The :dakref:`method-sampling-variance_based_decomp-vbd_sampling_method-binned` approach to variance-based decomposition now
  works with discrete variables.

**Miscellaneous Enhancements and Bugfixes**

**Deprecated and Changed**

- Dakota's NIDR (New Input Deck Reader) is being deprecated and replaced with
  a newly written input parser. For this release, NIDR is available as a fallback.
  Use the command line argument `-parser legacy` to use it.

**Compatibility**

- The schema for JSON input files is defined by Pydantic models, which are
  located at `python/dakota/spec/` in the source tree. To update Dakota's
  input grammar, Python >=3.9 and Pydantic >=2.12 are required.

**Other Notes and Known Issues**
