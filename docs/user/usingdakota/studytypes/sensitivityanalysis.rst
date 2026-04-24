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

Sensitivity analysis methods can be broadly categorized by their scope
(local vs. global) and by the metrics they produce for quantifying
parameter influence.

.. _`sa:local`:

Local Sensitivity
-----------------

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

.. _`sa:global`:

Global Sensitivity
------------------

Global sensitivity analysis assesses the relative influence of parameters
over the entire input space, either between upper and lower bounds or over
the support of the parameters' probability distributions. Rather than
examining behavior at a single point, global SA characterizes how
parameters affect the response over the range of plausible parameter values.

Global SA addresses questions such as: 
  - What is the general trend of the response over all values of a parameter? 
  - Does the response depend more nonlinearly on one factor than another? 
  - How do parameters interact to influence the response?

Global SA is performed by evaluating the response at well-distributed
points in the input space and analyzing the resulting input/output pairs. 
Dakota primarily focuses on global sensitivity analysis methods.

Various metrics can quantify parameter influence, each with different
interpretations and computational requirements:


.. _`sa:global:corr_coeffs`: 

Correlation coefficients
~~~~~~~~~~~~~~~~~~~~~~~~

Correlation coefficients, computed from sampling
data, measure the strength of linear or monotonic relationships between two 
quantities. Correlations can be computed between input parameters, between
output responses, and between inputs and outputs. They are inexpensive to
compute from existing samples. As such, they are computed by default as part
of the results of a :dakkw:`method-sampling` study. For the purposes of 
sensitivity analysis, this discussion focuses on correlations between inputs
and outputs.

The sample (Pearson) correlation coefficient measures the strength of the 
linear relationship between an input and an output, ignoring all other
inputs. Their efficacy can break down as a sensitivity measure if the relationship 
is nonlinear.

The rank (Spearman) correlation coefficient measures the strength of the 
monotonic relationship between an input and an output, ignoring all other
inputs. For this reason, they are robust to nonlinear monotonic relationships,
but can break down for non-monotonic cases.

Partial correlation coefficients measure correlation between inputs and outputs,
after removing linear effects of other inputs. In this sense, they control for 
other inputs in the computation of correlation between a given input and output.
However, their accuracy can degrade when the sample size is small relative to
the number of inputs (e.g., less than 10-20x the number of inputs).

Correlation coefficients can struggle in the following contexts: 
- non-monotonic, nonlinear input-output relationships
- correlated inputs
- input-output relationships driven by interactions between inputs

.. TNP TODO: add picture of correlation coefficients for different functions?

.. _`sa:global:std_regression_coeffs`:

Standardized regression coefficients
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Standardized regression coefficients, 
computed from sampling data, measure the change in output responses (in 
standard deviation) per standard deviation change in the input. In contrast
to correlation coefficients, they are robust to correlated inputs. 

They are inexpensive to compute from existing samples, where inputs and 
outputs are standardized (i.e., mean=0, std=1), and a multiple linear
regression is performed on the standardized quantities. The standardized
regression coefficients are the coefficients in the linear regression model.

Standardized regresion coefficients can be optionally computed in the results
of a random sampling study. Keyword reference:
:dakkw:`std_regression_coeffs <method-sampling-std_regression_coeffs>`

As with correlation coefficients, the standardized regression coefficients
are not robust to nonlinear or non-monotonic input-output relationships, or
relationships driven by interactions between inputs.

.. _`sa:global:morris`:

Morris one-at-a-time (elementary effects) method
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Morris one-at-a-time (MOAT) method is a global approach to screen 
out unimportant inputs. The algorithm is described in detail in 
:ref:`the PSUADE MOAD documentation <dace:psuade>`.

At a high level, the method creates a distribution of elementary effects for 
a single input by computing finite-difference derivative approximations of the partial derivative of the output with respect to the input at random points in the input domain.

From this sample set, the following metrics are computed:
- **Modified mean** (:math:`\mu^*`): The average absolute value of
  elementary effects. Large values indicate the input has a
  significant effect on the output.
- **Standard deviation** (:math:`\sigma`): The spread of elementary
  effects across the input space. Large values indicate either nonlinear
  effects or interactions with other inputs.

Inputs with high :math:`\mu^*` are influential and should be retained
for subsequent analyses. Inputs with high :math:`\sigma` relative to
:math:`\mu^*` exhibit strong nonlinear behavior or interactions.

The number of model evaluations to compute a size :math:`r` ensemble of 
elementary effects for :math:`d` inputs is :math:`N=r(d+1)`. A common
first choice for :math:`r` is :math:`\sim 10-20`, then increasing as needed
to achieve stability in the computed metrics.

Morris sensitivity metrics can be a useful tool for screening out unimportant
inputs at relatively low computational cost while being robust to nonlinear
input-output relationships and relationships driven by interactions. However, 
they have some downsides: 
- Input-output samples generated by the algorithm are not independent, identically distributed, so are poorly suited for reuse in Monte Carlo-based uncertainty analysis.
- While they can provide an indication of the presence of nonlinearity or interaction effects, they can't distinguish between the two or attribute output variance to specific inputs.

.. _`sa:global:vbd`:

Sobol' Indices/Variance-Based Decomposition
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sobol' indices are variance-based global sensitivity measures derived
from the functional ANOVA decomposition of a model. 

Let :math:`Y = f(X_1, \ldots, X_d)`, where the inputs are independent. 
The ANOVA decomposition represents the function as

.. math::

  Y = f_0 + \sum_{i=1}^d f_i (X_i) + \sum_{i<j}^d f_{ij}(X_i,X_j) + \cdots + f_{1,\ldots,d}(X_1,\ldots,X_d)

where the functional terms of the decomposition are orthogonal and are defined recursively as

.. math::

  f_0 = \mathbb{E}[Y]
  f_i(X_i) = \mathbb{E}[Y \mid X_i] - f_0
  f_{ij}(X_i) = \mathbb{E}[Y \mid X_i,X_j] - f_0 - f_i - f_j
and so on.
Assuming :math:`f` is square integrable, taking variance on both sides gives

.. math::
  
  \mathrm{Var}(Y) = \sum_{i=1}^d V_i + \sum_{i<j}^d V_{ij} + \cdots V_{1\ldots d}

where

.. math::

  V_i = \mathrm{Var}(\mathbb{E}[Y \mid X_i])
  V_{ij} = \mathrm{Var}(\mathbb{E}[Y \mid X_i, X_j ]) - V_i - V_j

and so on.

This variance decomposition shows how the variance in the model output can
be attributed to each input individually, as well as the effects of interactions between inputs. 
By definition, all terms sum to the total variance of the output.

First-order (main effect) Sobol' indices meaure the fraction of total variance in
:math:`Y` that can be attributing to varying :math:`X_i` alone. They are defined
as

.. math::

  S_i = \frac{V_i}{\mathrm{Var}(Y)}.

Higher-order Sobol' indices measure the fraction of variance in :math:`Y` 
that can be attributed to interactions between inputs and are defined as:

.. math::

  S_{ij} = \frac{V_{ij}}{\mathrm{Var}(Y)}.

Total-effect Sobol' indices quantify the total contribution of an input to the output
variance, including all interactions. They are defined as

.. math::

  S_{T_i} = \frac{\sum_{u \ni i} V_u}{\mathrm{Var}(Y)}, 

where the numerator is sum of all terms in the ANOVA decomposition involving 
input :math:`X_i`.

Important notes for interpreting Sobol' indices (assuming inputs are 
statistically independent):
  - All indices are bounded between 0 and 1.
  - Main effects sum to :math:`\leq 1`. If they sum to significantly less than one, interaction effects are significant.
  - If :math:`S_i = S_{T_i}`, there are no interactions.
  - If :math:`S_i` is close to 0, :math:`X_i` may be influential through interaction effects.
  - If :math:`S_{T_i}` is close to zero, :math:`X_i` is not influential. 

Sobol' indices are robust to nonlinear, non-monotonic input-output relationships
and to relationships driven by interactions between inputs.
In contrast to all other previously discussed global sensitivity metrics, 
Sobol' indices provide quantitative attribution of output variations to inputs, 
enabling assessment of the contribution to variance from individual inputs, 
their interactions, etc. 
However, they can be computationally challenging to compute, and their interpretation
breaks down for correlated inputs. 

Dakota provides two main approaches for computing Sobol' indices, a.k.a. variance-based
decomposition (VBD):
  - Polynomial Chaos Expansion (PCE)
  - Sampling-based

The sampling-based methods are activated via the :dakkw:`method-sampling-variance_based_decomp` keyword for sampling methods.
There are two sampling-based algorithms available:
  - Saltelli Pick-and-Freeze
  - Binned

**VBD via Polynomial Chaos Expansion**: 

For smooth response functions, polynomial chaos expansions (PCE) provide
an efficient means of computing Sobol' indices. Once a PCE surrogate is
constructed, Sobol' indices can be computed analytically from the expansion
coefficients at negligible additional cost.

.. literalinclude:: samples/vbd_pce.in
   :language: dakota
   :tab-width: 2
   :caption: VBD via PCE input excerpt
   :name: sa:vbd_pce

PCE-based VBD is particularly attractive when:

- The response is smooth and well-approximated by polynomials
- The PCE can be reused for other analysis tasks such as uncertainty quantification
- Interaction effects are important (interaction effects are represented in the PCE basis, subject to truncation scheme)

PCE can also be constructed from pre-existing sampling data using the
``import_build_points_file`` keyword, enabling VBD analysis without
additional simulation evaluations.

**VBD via Saltelli Pick-and-Freeze**:
The Saltelli sampling method (which we also call pick-and-freeze) employs 
structured sampling to compute the full set of main- and total-effect Sobol' 
indices for all parameters, using :math:`N(d+2)` samples for :math:`d` parameters
for :math:`N` independent samples. While the Saltelli method can be used to 
compute interaction effects, the Dakota implementation does not support this. 

.. literalinclude:: samples/vbd_saltelli.in
   :language: dakota
   :tab-width: 2
   :caption: VBD via Saltelli sampling input excerpt
   :name: sa:vbd_saltelli

**VBD via Binned Approach**: 
The binned method computes main-effect Sobol' indices for all parameters using 
:math:`N` independent samples. 

.. literalinclude:: samples/vbd_binned.in
   :language: dakota
   :tab-width: 2
   :caption: VBD via sampling-based binned method input excerpt
   :name: sa:vbd_binned

.. _`sa:pstudy`:

Parameter Studies
-----------------

Parameter studies systematically explore the parameter space along
specified directions or at specified points. While parameter studies
are described in detail in :ref:`Parameter Study Methods <ps>`, the
centered parameter study is particularly useful for initial sensitivity
screening.

Centered Parameter Study
~~~~~~~~~~~~~~~~~~~~~~~~

The :dakkw:`method-centered_parameter_study`  
varies each parameter along its axis, about a central point. 

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

Note that parameter studies are generally analyzed through visualization
and don't provide sensitivity metric or sensitivity ranking like other
methods described on this page.

.. _`sa:method_summary`:

Method Summary
______________

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

.. 
  TNP TODO: We may want to distinguish recommendations based on the anticipated
  downstream analysis. If it's UQ and your model is anything but stupidly cheap
  I think I would recommend you go straight to doing LHS sampling. Maybe there
  would be justification for doing something like MOAT or centered param if you're
  going to do an optimization after, without a surrogate where it would be better
  to have space-filling samples, since optimization will follow some particular 
  trajector through parameter space anyway, so any initial SA samples will be
  "thrown out"

.. 
  TNP TODO: How many steps do people do? I guess it would be a bit less than the 
  lower limit of LHS recommendation if they do less than 5 steps. I'm 
  still not convinced this is the thing we should be recommending as the 
  first pass since they can be effectively wasted evaluations if you 
  move on to UQ. Change my mind!
**For initial screening** with limited computational budget, start with
a centered parameter study to identify obviously influential or
non-influential parameters. This requires only :math:`2kd + 1` evaluations
for :math:`d` parameters with :math:`k` steps per variable.

.. TNP TODO: Same concern applies here
**For moderate budgets** with potentially nonlinear or non-monotonic
responses, Morris MOAT provides efficient screening with more robustness
than centered parameter studies. The modified mean and standard deviation
metrics help distinguish between linear influence, nonlinear influence,
and interaction effects.

.. TNP TODO: what about sampling-based, just getting things like CCs? This 
   has always been my go-to if a precursor to UQ. 
**For comprehensive analysis** when computational resources permit,
sampling-based methods with variance-based decomposition provide the
most complete picture of parameter importance. For smooth responses,
polynomial chaos expansion offers an efficient route to Sobol indices.

.. TNP TODO: review. I definitely woudln't recommend CCs for noisy responses. 
**Consider model characteristics** when selecting methods:
- Smooth, well-behaved responses: PCE-based VBD is typically most efficient
- Noisy or discontinuous responses: Sampling with correlations or MOAT
- High-dimensional problems: MOAT for screening, then detailed analysis
  on down-selected parameters
- Expensive simulations: Start with centered studies, proceed to MOAT,
  use PCE if smoothness permits

.. _`sa:postprocessing`:

.. TNP TODO: Not sure this section is necessary?
Using Dakota-Generated Data
---------------------------

Dakota produces tabular output files (``dakota_tabular.dat`` by default
or HDF5 format with ``dakota_results.h5``) containing all evaluation
data. These files can be imported into external tools for additional
analysis:

- **Scatterplots**: Visualize input-output relationships to identify
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
- **UQ focuses on responses**: The primary goal is to characterize
  statistical properties of outputs given uncertain inputs

Some methods serve both purposes. For example, Latin hypercube sampling
is commonly used for SA (computing correlations) and UQ (estimating
response moments, PDFs, and CDFs). Polynomial chaos expansions are
often considered a UQ method but efficiently produce Sobol' indices
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
