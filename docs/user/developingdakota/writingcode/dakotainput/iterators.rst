.. _developer-iterators-main:

"""""""""
Iterators
"""""""""

Class hierarchy: `Iterator. <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1Iterator.html>`_
`Iterator <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1Iterator.html>`_ implementations may choose to split operations up into run-time
phases as described in :ref:`Understanding Iterator Flow.<developer-understanding-iterator-flow>`

The iterator hierarchy contains a variety of iterative algorithms for optimization, uncertainty quantification, nonlinear least squares, design of experiments, and parameter studies.
The hierarchy is divided into `MetaIterator <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1MetaIterator.html>`_,
`Minimizer <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1Minimizer.html>`_, and
`Analyzer <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1Analyzer.html>`_ branches.

The `MetaIterator <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1MetaIterator.html>`_ classes manage sequencing and collaboration among multiple
methods with support for concurrent iterator parallelism. Methods include:

 - `SeqHybridMetaIterator <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1SeqHybridMetaIterator.html>`_: hybrid minimization using a set of iterators
   employing a corresponding set of models of varying fidelity. The sequential hybrid passes the best solutions from one method in as the starting points of the next method in the sequence.
 - `CollabHybridMetaIterator <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1CollabHybridMetaIterator.html>`_: hybrid minimization employing collaboration and
   sharing of response data among methods during the course if iteration. This class is currently a placeholder.
 - `EmbedHybridMetaIterator <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1EmbedHybridMetaIterator.html>`_: hybrid minimization involving periodic use of a
   local search method for refinement during the iteration of an outer global method. This class is currently a placeholder.
 - `ConcurrentMetaIterator <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1ConcurrentMetaIterator.html>`_: two similar algorithms are available: (1)
   multi-start iteration from several different starting points, and (2) pareto set optimization for several different multi-objective weightings. Employs a single iterator with a single model, but
   runs multiple instances of the iterator concurrently for different settings within the model.

The `Minimizer <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1Minimizer.html>`_ classes address optimization and deterministic calibration and are grouped into:

 - Optimization: `Optimizer <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1Optimizer.html>`_ provides a base class for gradient-based
   (e.g., `CONMINOptimizer <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1CONMINOptimizer.html>`_ and
   `SNLLOptimizer <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1SNLLOptimizer.html>`_) and derivative-free (e.g.,
   `NCSUOptimizer <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1NCSUOptimizer.html>`_,
   `JEGAOptimizer <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1JEGAOptimizer.html>`_) optimization solvers. Most of these are wrappers for third-party libraries
   that implement the optimization algorithms. Classes `APPSEvalMgr <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1APPSEvalMgr.html>`_ and
   `COLINApplication <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1COLINApplication.html>`_ provide the function evaluation interface for
   `APPSOptimizer <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1APPSOptimizer.html>`_ and
   `COLINOptimizer <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1COLINOptimizer.html>`_, respectively.
 - Parameter estimation: `LeastSq <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1LeastSq.html>`_ provides a base class for
   `NL2SOLLeastSq <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1NL2SOLLeastSq.html>`_, a least-squares solver based on NL2SOL,
   `SNLLLeastSq <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1SNLLLeastSq.html>`_, a Gauss-Newton least-squares solver, and
   `NLSSOLLeastSq <https://dakota.sandia.gov//sites/default/files/docs/latest_release/html-dev/classDakota_1_1NLSSOLLeastSq.html>`_, an SQP-based least-squares solver.
 - Surrogate-based minimization (both optimization and nonlinear least squares): SurrBasedMinimizer provides a base class for SurrBasedLocalMinimizer, SurrBasedGlobalMinimizer, and EffGlobalMinimizer. The surrogate-based local and global methods employ a single iterator with any of the available SurrogateModel capabilities (local, multipoint, or global data fits or hierarchical approximations) and perform a sequence of approximate optimizations, each involving build, optimize, and verify steps. The efficient global method, on the other hand, hard-wires a recursion involving Gaussian process surrogate models coupled with the DIRECT global optimizer to maximize an expected improvement function.

The Analyzer classes are grouped into:

- Uncertainty quantification: NonD provides a base class for non-deterministic methods in several categories:

  - Sampling: NonDSampling is further specialized with the NonDLHSSampling class for Latin hypercube and Monte Carlo sampling, and a number of other classes supporting incremental and adaptive sampling such as NonDAdaptImpSampling for multi-modal adaptive importance sampling.
  - Reliability Analysis: NonDReliability is further specialized with local and global methods (NonDLocalReliability and NonDGlobalReliability). NonDPOFDarts implements a computational geometry-based reliability method.
  - Stochastic Expansions: NonDExpansion includes specializations for generalized polynomial chaos (NonDPolynomialChaos) and stochastic collocation (NonDStochCollocation) and is supported by the NonDIntegration helper class, which supplies cubature, tensor-product quadrature and Smolyak sparse grid methods (NonDCubature, NonDQuadrature, and NonDSparseGrid).
  - Bayesian Calibration: NonDCalibration provides a base class for nondeterministic calibration methods with specialization to Bayesian calibration in NonDBayesCalibration, and specific implementations such as NonDQUESOBayesCalibration.
  - NonDInterval provides a base class for epistemic interval-based UQ methods. Three interval analysis approaches are provided: LHS (NonDLHSInterval), efficient global optimization (NonDGlobalInterval), and local optimization (NonDLocalInterval). Each of these three has specializations for single interval and Dempster-Shafer Theory of Evidence approaches.

- Parameter studies and design of experiments: PStudyDACE provides a base class for ParamStudy, which provides capabilities for directed parameter space interrogation, PSUADEDesignCompExp, which provides access to the Morris One-At-a-Time (MOAT) method for parameter screening, and DDACEDesignCompExp and FSUDesignCompExp, which provide for parameter space exploration through design and analysis of computer experiments. NonDLHSSampling from the uncertainty quantification branch also supports design of experiments when in active all variables mode.

- Solution verification studies: Verification provides a base class for RichExtrapVerification (verification via Richardson extrapolation) and other solution verification methods in development.