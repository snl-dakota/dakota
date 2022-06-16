.. _developing-tpls-main:

""""""""""""""""""""""""""""""
Dakota's Third-Party Libraries
""""""""""""""""""""""""""""""

Dakota utilizes the following Sandia-developed optimization, design of experiments, uncertainty quantification, and surrogate modeling libraries:

- DDACE (design and analysis of computer experiments; available under GNU LGPL; contact: Patty Hough); DDACE additionally uses the MARS and OA packages from StatLib (conditions of use)
- HOPSPACK (Hybrid Optimization Parallel Search PACKage, which supplies a robust asynchronous implementation of generating set search; available under GNU LGPL)
- JEGA (multiobjective genetic algorithms; available under GNU LGPL; contact: John Eddy)
- LHS (Latin Hypercube and Monte Carlo sampling for uncertainty quantification; as of June 18, 2009, available under GNU LGPL (formerly GNU GPL); contact: Laura Swiler)
- OPT++ (nonlinear and direct search optimization algorithms; available under GNU LGPL; contact: Patty Hough); OPT++ additionally uses Teuchos (see below).
- PECOS: univariate/multivariate orthogonal/interpolation polynomial basis functions, numerical integration drivers (quadrature, cubature, sparse grids), random variable transformations, stochastic process modeling (available separately under GNU LGPL; contact: Mike Eldred). PECOS additionally uses Boost (see below), DFFTPACK (public domain), FFTW (GNU GPL, optional extension), LHS (see above), and Teuchos (see below).
- SCOLIB (formerly known as COLINY; nongradient optimization algorithms; available under BSD; contact: John Siirola)
- Surfpack: global data fit surrogate models (available under GNU LGPL; contact: Surfpack developers)

...the following Sandia-funded optimization and design of experiments libraries:

- FSUDace (quasi-Monte Carlo and centroidal voronoi tessellation sampling algorithms from Florida State; available under GNU LGPL; contact: Laura Swiler)
- Sparse grid (numerical quadrature rules and Smolyak sparse grids from Virginia Tech; available under GNU LGPL; contact: Mike Eldred)

...and the following external optimization and design of experiments libraries:

- CONMIN (public domain nonlinear programming algorithms; no license required for inclusion in DAKOTA distribution)
- DOT (nonlinear programming algorithms from Vanderplaats Research and Development; optional extension requiring a separate commercial license)
- NL2SOL (public domain nonlinear least squares algorithm; no license required for inclusion in DAKOTA distribution)
- NLPQLP (nonlinear programming algorithms from Klaus Schittkowski; optional extension requiring a separate commercial license)
- NOMAD (Nonlinear Optimization by Mesh Adaptive Direct Search; available under GNU LGPL v 3.0)
- NPSOL/NLSSOL (nonlinear programming algorithms from Stanford Business Software; optional extension requiring a separate commercial license)
- PSUADE (Morris one-at-a-time sampling algorithm from Charles Tong at LLNL CASC; available under GNU LGPL)
- QUESO (Quantification of Uncertainty for Estimation, Simulation and Optimization: libraries for statisical calibration, validation, and uncertainty propagation; with thanks to Ernesto Prudencio and collaborators at UT Austin. Dakota uses the MCMC algorithms from QUESO.)

Dakota also uses the following Sandia-developed utility libraries:

- Teuchos: serial vector/matrix utilities (available under GNU LGPL)
- UTILIB: utility library (available under BSD)
- pyprepro: Template processing tool

...and the following external utility libraries:

- Boost (available under the Boost Software License)
- HDF5 (available under a "BSD style" Open Source license.)
- MPI (parallel distributed-memory communication via message-passing; either the public domain MPICH or hardware-specific MPI versions; no license required)
- SciPlot (2D graphics; available under GNU LGPL)

To the extent possible, all noncommercial libraries will be included in the Dakota tar files available for download. Dakota uses a flexible configuration management system to configure with any desired subset of these available packages. If any of the commercial packages are desired, then these must be licensed separately for source code (preferable) or target platform object libraries (less desirable, but workable with minor configuration modifications). These distributions are then installed in the appropriate Dakota subdirectories prior to building Dakota.

These Dakota-related Sandia-developed packages are also available for download:

- STEPWISE: Fortran program for stepwise regression
