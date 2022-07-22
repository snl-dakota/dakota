.. _releasenotes-30beta:

"""""""""""""""""""""""""""""
Version 3.0 beta (2001/11/29)
"""""""""""""""""""""""""""""

**General:**

- ANSI C++ compliant: DAKOTA has been updated to the ANSI C++ standard. Vector/list templates from the commercial Rogue Wave Tools.h++ library have been replaced with the Standard Template Library (STL) provided as part of the ANSI C++ standard.
- GNU GPL open source release: DAKOTA is now available for download worldwide.
- New documentation: Users, Reference, and Developers Manuals now available online from http://dakota-new.sandia.gov/documentation.html.

**New strategies:**

- Optimization under uncertainty (OUU): for robust and reliability-based design in the presence of parameter uncertainty. OUU may be executed in simple nested iteration mode, or it may employ surrogates at the optimization level, the uncertainty quantification level, or both.
- Pareto-set optimization: generates a trade-off surface for multiobjective optimization problems. The Pareto-set strategy supports concurrent-iterator parallelism for large-scale multilevel parallel studies.
- Multi-start optimization: runs local optimizations from different starting points for locating multiple minima. The multi-start strategy supports concurrent-iterator parallelism for large-scale multilevel parallel studies.

**New iterative methods:**

- The CONMIN library for gradient-based constrained minimization has been added to provide an freely available alternative to the commercial DOT and NPSOL libraries.
- Extensions to the DAKOTA/UQ uncertainty quantification library: stochastic finite element techniques using polynomial chaos expansions have been added in joint work with Johns Hopkins University.

**New features:**

- New tabular data output capability for postprocessing of DAKOTA results with external post-processing tools such as Matlab and Tecplot.
- Graphics ported to SGI IRIX and PC LINUX platforms and extended for OUU, multilevel hybrid, Pareto-set, and multi-start strategies.
