.. _sa:

Sensitivity Analysis
====================

.. _`sa:overview`:

Overview
--------

Sensitivity analysis (SA) reveals the extent to which simulation outputs
depend on each simulation input. The primary goal is to identify the most
important input variables and their interactions, enabling analysts to
focus resources on the parameters that matter most. This page summarizes
SA concepts and terminology, the practical process for conducting SA studies,
:ref:`available methods in Dakota <sa:methods>`, and offers
:ref:`usage guidelines <sa:usage>`.

Sensitivity analysis serves several key purposes in computational modeling:

- **Screening and ranking**: Identify the most influential variables to
  down-select for further uncertainty quantification or optimization studies.
- **Resource allocation**: Focus data gathering, model development, code
  development, and uncertainty characterization efforts on the most
  impactful parameters.
- **Model understanding**: Identify key model characteristics such as
  smoothness, nonlinear trends, and robustness, while developing intuition
  about model behavior.
- **Quality assurance**: SA can reveal code and model issues as a side
  effect of systematic parameter exploration.
- **Surrogate construction**: Data generated during SA studies can be
  repurposed to construct surrogate models for subsequent analyses.

.. _`sa:concepts`:

Key Concepts
------------

Sensitivity analysis methods can be broadly categorized by their scope
(local vs. global) and by the metrics they produce for quantifying
parameter influence.

.. _`sa:concepts:local`:

Local Sensitivity
~~~~~~~~~~~~~~~~~

Local sensitivity analysis examines how the response changes with
respect to small perturbations around a single point in parameter space.
The classic measure of local sensitivity is the partial derivative of
the response with respect to each parameter:

.. math::

   \frac{\partial f}{\partial x_i} \bigg|_{x=x_0}

Local sensitivity provides information about the slope of the response
surface at the nominal point :math:`x_0`. This can be useful for
understanding instantaneous rates of change, but may not capture the
full picture if the response is highly nonlinear or if the parameter
ranges of interest are large relative to the local curvature.

.. _`sa:concepts:global`:

Global Sensitivity
~~~~~~~~~~~~~~~~~~

Global sensitivity analysis assesses the relative influence of parameters
over the entire input space, typically defined as a hyper-rectangle
bounded by lower and upper limits on each parameter. Rather than
examining behavior at a single point, global SA characterizes how
parameters affect the response across all possible values within
their specified ranges.

Global SA addresses questions such as: What is the general trend of
the response over all values of a parameter? Does the response depend
more nonlinearly on one factor than another? How do parameters interact
to influence the response?

Global SA is performed by evaluating the response at well-distributed
points in the input space (a design of computer experiments) and
analyzing the resulting input/output pairs. Dakota primarily focuses
on global sensitivity analysis methods.

.. _`sa:concepts:measures`:

Measures of Sensitivity
~~~~~~~~~~~~~~~~~~~~~~~

Various metrics can quantify parameter influence, each with different
interpretations and computational requirements:

**Correlation coefficients**: Simple (Pearson) or rank (Spearman)
correlations between input parameters and output responses, computed
from sampling data. These measure linear or monotonic relationships,
respectively, and are inexpensive to compute from existing samples.

**Elementary effects (Morris method)**: The modified mean and standard
deviation of elementary effects, computed by varying one parameter at
a time across a discretized input space. The modified mean indicates
overall influence while the standard deviation indicates nonlinearity
or interactions with other parameters.

**Sobol indices**: Variance-based decomposition metrics that partition
output variance into contributions from individual parameters (main
effects) and parameter interactions. Main effect indices :math:`S_i`
measure the effect of varying parameter :math:`x_i` alone, while total
effect indices :math:`T_i` include interactions with other parameters.

**Importance factors**: Local sensitivity derivatives normalized by
parameter and response standard deviations, providing a dimensionless
measure of relative influence at a given point.

.. _`sa:process`:

A Practical Process for Sensitivity Analysis
--------------------------------------------

The following systematic process helps ensure effective and efficient
sensitivity analysis:

1. **Define objectives**: Identify the key model responses (quantities
   of interest) and clarify your follow-on analysis goals. Will you use
   SA results to down-select variables for optimization? To guide
   uncertainty characterization efforts? Understanding the end goal
   helps select appropriate SA methods.

2. **Identify candidate parameters**: Enumerate potentially important
   input parameters, often through expert opinion or phenomena
   identification and ranking tables (PIRT). Include parameters that
   likely influence the response or might be involved in subsequent
   studies.

3. **Specify parameter ranges**: Pose plausible bounds (for global SA)
   or step sizes (for local SA) for each parameter. For non-physical
   parameters like model coefficients, ranges may be based on prior
   experience, literature values, or engineering judgment.

4. **Configure Dakota input**: Set up the Dakota input file with
   appropriate variable specifications and interface to the simulation.

5. **Perform initial screening**: Start with a centered parameter study
   to examine univariate trends and identify any obviously influential
   or non-influential parameters.

6. **Conduct detailed SA**: Based on simulation cost and known model
   characteristics, perform additional SA studies using sampling,
   Morris MOAT, or variance-based decomposition methods.

7. **Post-process and interpret**: Use Dakota-generated tabular data
   with third-party tools (Excel, MATLAB, R, Python, JMP) to generate
   scatter plots, perform regression analysis, or compute additional
   significance metrics.

.. _`sa:methods`:

Sensitivity Analysis Methods in Dakota
--------------------------------------

Dakota provides several methods for sensitivity analysis, ranging from
simple parameter studies to sophisticated variance-based decomposition
techniques.

.. _`sa:methods:pstudy`:

Parameter Studies
~~~~~~~~~~~~~~~~~

Parameter studies systematically explore the parameter space along
specified directions or at specified points. While parameter studies
are described in detail in :ref:`Parameter Study Methods <ps>`, the
centered parameter study is particularly useful for initial sensitivity
screening.

.. _`sa:methods:pstudy:centered`:

Centered Parameter Study
^^^^^^^^^^^^^^^^^^^^^^^^

The :dakkw:`method-centered_parameter_study` varies each parameter
individually around a central point, providing univariate sensitivity
information. This is often the first step in SA, as it quickly reveals
which parameters have obvious effects on the response.

The centered parameter study requires specifying:

- ``initial_point``: The central point around which parameters are varied
- ``steps_per_variable``: Number of steps to take in each direction for
  each parameter
- ``step_vector``: Step size for each parameter

.. literalinclude:: samples/centered_pstudy.in
   :language: dakota
   :tab-width: 2
   :caption: Centered parameter study input excerpt
   :name: sa:centered_pstudy

The resulting data shows how the response changes as each parameter
is varied while holding others constant. Large response variations
indicate potentially influential parameters, while flat profiles
suggest parameters may be candidates for fixing at nominal values.

.. _`sa:methods:sampling`:

Sampling Methods
~~~~~~~~~~~~~~~~

Random sampling methods, particularly Latin hypercube sampling (LHS),
are workhorses for global sensitivity analysis. By evaluating the
simulation at points distributed throughout the input space, sampling
methods can estimate correlation coefficients that quantify the
strength of input-output relationships.

.. _`sa:methods:sampling:lhs`:

Latin Hypercube Sampling
^^^^^^^^^^^^^^^^^^^^^^^^

Latin hypercube sampling provides better coverage of the parameter
space than simple random sampling with the same number of samples.
For sensitivity analysis, configure LHS using the
:dakkw:`method-sampling` method with :dakkw:`method-sampling-sample_type`
set to ``lhs``:

.. literalinclude:: samples/lhs_sa.in
   :language: dakota
   :tab-width: 2
   :caption: LHS for sensitivity analysis input excerpt
   :name: sa:lhs_sa

Key considerations for LHS-based sensitivity analysis:

- **Sample size**: More samples generally yield more reliable correlation
  estimates. A common rule of thumb is 10 samples per variable at minimum,
  with 100+ samples preferred for stable estimates.
- **Seed specification**: Setting a ``seed`` value ensures reproducibility
  and allows comparison across related studies.
- **Incremental sampling**: The ``refinement_samples`` keyword enables
  incremental LHS, where additional samples can be added while
  maintaining the Latin hypercube property of the combined sample set.

Dakota computes simple and partial correlation coefficients between
all inputs and outputs, reporting these in the console output. Large
absolute correlation values indicate strong linear relationships
between parameters and responses.

.. _`sa:methods:sampling:interpretation`:

Interpreting Correlation Results
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When interpreting sampling-based sensitivity results:

- Large correlation coefficients indicate important factors, but factors
  with small correlations may still be significant if relationships are
  nonlinear.
- The assumed parameter ranges (bounds) strongly influence results.
  Different bounds can lead to different conclusions about relative
  importance.
- Scatter plots of input-output pairs help visualize relationships and
  can reveal nonlinear trends that correlations may miss.
- For nonlinear or non-monotonic relationships, rank correlations
  (Spearman) may be more informative than simple correlations (Pearson).

.. _`sa:methods:moat`:

Morris One-at-a-Time (MOAT)
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Morris one-at-a-time (MOAT) method, available through
:dakkw:`method-psuade_moat`, is a screening method that efficiently
identifies influential parameters even with limited computational
budgets. MOAT computes elementary effects by varying one parameter
at a time across a discretized grid.

.. literalinclude:: samples/moat.in
   :language: dakota
   :tab-width: 2
   :caption: Morris MOAT input excerpt
   :name: sa:moat

The MOAT method computes two key metrics for each parameter:

- **Modified mean** (:math:`\mu^*`): The average absolute value of
  elementary effects. Large values indicate the parameter has a
  significant effect on the response.
- **Standard deviation** (:math:`\sigma`): The spread of elementary
  effects across the input space. Large values indicate either nonlinear
  effects or interactions with other parameters.

Parameters with high :math:`\mu^*` are influential and should be retained
for subsequent analyses. Parameters with high :math:`\sigma` relative to
:math:`\mu^*` exhibit strong nonlinear behavior or interactions.

Key MOAT configuration options:

- ``partitions``: Number of grid partitions for each parameter
  (levels = partitions + 1)
- ``samples``: Number of Morris trajectories (must be a multiple of
  number of parameters + 1)

.. _`sa:methods:vbd`:

Variance-Based Decomposition
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Variance-based decomposition (VBD), also known as Sobol analysis,
decomposes the variance of the response into contributions from
individual parameters and their interactions. This provides the most
complete picture of parameter importance but can be computationally
expensive.

VBD assumes the response can be decomposed as:

.. math::

   f(x) = f_0 + \sum_i f_i(x_i) + \sum_{i<j} f_{ij}(x_i, x_j) + \cdots

The main effect Sobol index :math:`S_i` measures the fraction of output
variance due to parameter :math:`x_i` alone:

.. math::

   S_i = \frac{V[E[f|x_i]]}{V[f]}

The total effect index :math:`T_i` includes both main effects and all
interactions involving :math:`x_i`:

.. math::

   T_i = 1 - \frac{V[E[f|x_{\sim i}]]}{V[f]}

where :math:`x_{\sim i}` denotes all parameters except :math:`x_i`.

Dakota provides three approaches for computing variance-based decomposition:

.. _`sa:methods:vbd:sampling`:

VBD via Sampling
^^^^^^^^^^^^^^^^

The :dakkw:`method-sampling-variance_based_decomp` keyword enables
variance-based decomposition using sampling methods. Two algorithms
are available:

- ``pick_and_freeze``: The classical Sobol algorithm requiring
  :math:`N(d+2)` samples for :math:`d` parameters, providing both main
  and total effect indices.
- ``binned``: A more economical approach that estimates main effects
  only from a single LHS sample set.

.. literalinclude:: samples/vbd_sampling.in
   :language: dakota
   :tab-width: 2
   :caption: VBD via sampling input excerpt
   :name: sa:vbd_sampling

.. _`sa:methods:vbd:pce`:

VBD via Polynomial Chaos Expansion
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For smooth response functions, polynomial chaos expansions (PCE) provide
an efficient means of computing Sobol indices. Once a PCE surrogate is
constructed, Sobol indices can be computed analytically from the expansion
coefficients at negligible additional cost.

.. literalinclude:: samples/vbd_pce.in
   :language: dakota
   :tab-width: 2
   :caption: VBD via PCE input excerpt
   :name: sa:vbd_pce

PCE-based VBD is particularly attractive when:

- The response is smooth and well-approximated by polynomials
- Multiple outputs need to be analyzed (the PCE can be reused)
- Interaction effects are important (full decomposition is available)

PCE can also be constructed from pre-existing sampling data using the
``import_build_points_file`` keyword, enabling VBD analysis without
additional simulation evaluations.

.. _`sa:methods:summary`:

Method Summary
~~~~~~~~~~~~~~

The following table summarizes Dakota's sensitivity analysis methods
and the metrics they provide:

.. list-table:: Dakota Sensitivity Analysis Methods
   :header-rows: 1
   :widths: 20 30 15 15 15 15

   * - Category
     - Dakota Method
     - Univariate Trends
     - Correlations
     - Morris Metrics
     - Sobol Indices
   * - Parameter Studies
     - ``centered_parameter_study``
     - P
     -
     -
     -
   * - 
     - ``multidim_parameter_study``
     -
     - P
     -
     - P
   * - Sampling
     - ``sampling``
     - P
     - D
     -
     -
   * - 
     - ``sampling`` with ``variance_based_decomp``
     - P
     - D
     -
     - D
   * - Morris
     - ``psuade_moat``
     -
     -
     - D
     -
   * - Stochastic Expansions
     - ``polynomial_chaos``
     -
     -
     -
     - D

D: Dakota-generated; P: Post-processing required with third-party tools

.. _`sa:usage`:

Usage Guidelines
----------------

Selecting an appropriate sensitivity analysis method depends on several
factors including computational budget, model characteristics, and
analysis objectives.

**For initial screening** with limited computational budget, start with
a centered parameter study to identify obviously influential or
non-influential parameters. This requires only :math:`2kN + 1` evaluations
for :math:`N` parameters with :math:`k` steps per variable.

**For moderate budgets** with potentially nonlinear or non-monotonic
responses, Morris MOAT provides efficient screening with more robustness
than centered parameter studies. The modified mean and standard deviation
metrics help distinguish between linear influence, nonlinear influence,
and interaction effects.

**For comprehensive analysis** when computational resources permit,
sampling-based methods with variance-based decomposition provide the
most complete picture of parameter importance. For smooth responses,
polynomial chaos expansion offers an efficient route to Sobol indices.

**Consider model characteristics** when selecting methods:

- Smooth, well-behaved responses: PCE-based VBD is typically most efficient
- Noisy or discontinuous responses: Sampling with correlations or MOAT
- High-dimensional problems: MOAT for screening, then detailed analysis
  on down-selected parameters
- Expensive simulations: Start with centered studies, proceed to MOAT,
  use PCE if smoothness permits

.. _`sa:postprocessing`:

Using Dakota-Generated Data
---------------------------

Dakota produces tabular output files (``dakota_tabular.dat`` by default
or HDF5 format with ``dakota_results.h5``) containing all evaluation
data. These files can be imported into external tools for additional
analysis:

- **Scatter plots**: Visualize input-output relationships to identify
  trends missed by correlation coefficients
- **Regression analysis**: Perform stepwise or best-subsets regression
  to identify significant parameters
- **Statistical tests**: Apply formal significance tests available in
  statistical packages
- **Visualization**: Create publication-quality figures using MATLAB,
  Python (matplotlib), R, or similar tools

Common post-processing workflows include:

1. Import tabular data into analysis software
2. Generate scatter plots for all input-output pairs
3. Identify nonlinear trends or interactions
4. Perform formal regression or ANOVA analysis
5. Use results to prune variables and design follow-on studies

.. _`sa:vs_uq`:

Relationship to Uncertainty Quantification
------------------------------------------

Sensitivity analysis and uncertainty quantification (UQ) are closely
related but serve different primary purposes:

- **SA focuses on parameters**: The primary goal is to rank parameter
  importance and understand how parameters influence responses
  (sometimes called inverse UQ)
- **UQ focuses on responses**: The primary goal is to characterize
  statistical properties of outputs given uncertain inputs

Some methods serve both purposes. For example, Latin hypercube sampling
is commonly used for SA (computing correlations) and UQ (estimating
response moments, PDFs, and CDFs). Polynomial chaos expansions are
often considered a UQ method but efficiently produce Sobol indices
for parameter ranking.

A typical workflow combines SA and UQ: first use SA to identify the
most important parameters, then perform detailed UQ analysis on the
reduced parameter set.

.. _`sa:references`:

References
----------

For more detailed treatments of sensitivity analysis theory and methods,
consult:

.. bibliography::
   :filter: False

   Saltelli08
   Helton00
   Oakley04

Additional information is available in the Dakota User's Manual sections on:

- :ref:`Parameter Study Capabilities <ps>`
- :ref:`Design of Experiments Capabilities <dace>`
- :ref:`Uncertainty Quantification Capabilities <uq>`

Corresponding keyword reference pages provide detailed information on
method options and settings.

Video Resources
---------------

+----------------------+-----------------+----------------+
| Title                | Link            | Resources      |
+======================+=================+================+
| Sensitivity Analysis | |Training|_     | `Slides`__ /   |
|                      |                 | `Exercises`__  |
+----------------------+-----------------+----------------+

.. __: https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_SensitivityAnalysis.pdf
__ https://dakota.sandia.gov/sites/default/files/training/sensitivity_analysis.zip

.. |Training| image:: img/SensitivityAnalysisTrainingTeaser.png
                :alt: Sensitivity Analysis
.. _Training: https://digitalops.sandia.gov/Mediasite/Play/PLACEHOLDER
