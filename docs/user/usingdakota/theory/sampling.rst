.. _`sampling-theory-main`:

Sampling Methods
================

This chapter introduces several fundamental concepts related to sampling
methods. In particular, the statistical properties of the Monte Carlo
estimator are discussed (:ref:`uq:sampling:montecarlo`) and
strategies for multilevel and multifidelity sampling are introduced
within this context. Hereafter, multilevel refers to the possibility of
exploiting distinct discretization levels (i.e. space/time resolution)
within a single model form, whereas multifidelity involves the use of
more than one model form. In :ref:`uq:sampling:mfmc`,
we describe the multifidelity Monte Carlo and its single fidelity model version, the control variate Monte Carlo, 
that we align with
multifidelity sampling, and in :ref:`uq:sampling:multilevel`, we
describe the multilevel Monte Carlo algorithm that we align with
multilevel sampling. In :ref:`uq:sampling:mlmf`, we show that
these two approaches can be combined to create multilevel-multifidelity
sampling approaches.

.. _uq:sampling:montecarlo:

Monte Carlo (MC)
----------------

Monte Carlo is a popular algorithm for stochastic simulations due to its
simplicity, flexibility, and the provably convergent behavior that is
independent of the number of input uncertainties. A quantity of interest
:math:`Q: \Xi \rightarrow \mathbb{R}`, represented as a random variable
(RV), can be introduced as a function of a random vector
:math:`\boldsymbol{\xi} \in \Xi \subset \mathbb{R}^d`. The goal of any
MC simulation is computing statistics for :math:`Q`, e.g. the expected
value :math:`\mathbb{E}\left[Q\right]`. The MC estimator
:math:`\hat{Q}_N^{MC}` for :math:`\mathbb{E}\left[Q\right]` is defined
as follows

.. math:: \hat{Q}_N^{MC} = \dfrac{1}{N} \sum_{i=1}^N Q^{(i)},

where :math:`Q^{(i)} = Q(\boldsymbol{\xi}^{(i)})` and :math:`N` is used
to indicate the number of realizations of the model.

The MC estimator is unbiased, i.e., its bias is zero and its convergence to the true
statistics is :math:`\mathcal{O}(N^{-1/2})`. Moreover, since each
set of realizations for :math:`Q` is different, another crucial property of any
estimator is its own variance:

.. math::
   :label: varianceMC

   \mathbb{V}ar\left( \hat{Q}_N^{MC} \right) = \dfrac{\mathbb{V}ar\left(Q\right) }{N}.

Furthermore, it is possible to show, in the limit
:math:`N \rightarrow \infty`, that the error
:math:`\left( \mathbb{E}\left[Q\right] - \hat{Q}_N^{MC} \right) \sim 
\sqrt{\dfrac{\mathbb{V}ar\left(Q\right) }{N}} \mathcal{N}(0,1)`, where
:math:`\mathcal{N}(0,1)` represents a standard normal RV. As a
consequence, it is possible to define a confidence interval for the MC
estimator which has an amplitude proportional to the standard deviation
of the estimator. Indeed, the variance of the estimator plays a
fundamental role in the quality of the numerical results: the reduction
of the estimator variance correspond to an error reduction in the statistics.

.. _uq:sampling:mfmc:

Multifidelity Monte Carlo
---------------------------

A closer inspection of Eq. :math:numref:`varianceMC`
indicates that only an increase in the number of simulations :math:`N`
might reduce the overall variance, since
:math:`\mathbb{V}ar\left({Q}\right)` is an intrinsic property of the
model under analysis. However, more sophisticated techniques have been
proposed to accelerate the convergence of a MC simulation. For instance,
an incomplete list of these techniques can include stratified sampling,
importance sampling, Latin hypercube, deterministic Sobol’ sequences and
control variates (see :cite:p:`OwenMC`). In particular, the control variate approach, is based
on the idea of replacing the RV :math:`Q` with one that has
the same expected value, but with a smaller variance. The goal is to
reduce the numerator in Eq. :math:numref:`varianceMC`,
and hence the value of the estimator variance without requiring a larger
number of simulations. In a practical setting, the control variate makes
use of an auxiliary RV :math:`G=G(\boldsymbol{\xi})` for which
the expected value :math:`\mathbb{E}\left[G\right]` is known. Indeed,
the alternative estimator can be defined as


.. add a label :label: control_variate
.. math::
   :label: control_variate
   
   \hat{Q}_N^{MCCV} =  \hat{Q}_N^{MC} - \beta \left( \hat{G}_N^{MC} - \mathbb{E}\left[G\right] \right), \quad \mathrm{where} \quad \beta \in \mathbb{R}.

The MC control variate estimator :math:`\hat{Q}_N^{MCCV}` is unbiased, but its variance now has a more complex
dependence not only on the :math:`\mathbb{V}ar\left({Q}\right)`, but
also on :math:`\mathbb{V}ar\left(G\right)` and the covariance between
:math:`Q` and :math:`G` since

.. math:: \mathbb{V}ar\left(\hat{Q}_N^{MCCV}\right) = \dfrac{1}{N} \left( \mathbb{V}ar\left( \hat{Q}_N^{MC} \right) + \beta^2 \mathbb{V}ar\left( \hat{G}_N^{MC} \right) - 2\beta \mathrm{Cov}\left(Q,G\right) \right).

The parameter :math:`\beta` can be used to minimize the overall variance
leading to

.. math:: \beta = \dfrac{ \mathrm{Cov}\left(Q,G\right) }{ \mathbb{V}ar\left( G \right) },

for which the estimator variance follows as

.. math:: \mathbb{V}ar\left({\hat{Q}_N^{MCCV}}\right) = \mathbb{V}ar\left({\hat{Q}_N^{MC}}\right)\left( 1-\rho^2 \right).

Therefore, the overall variance of the estimator
:math:`\hat{Q}_N^{MCCV}` is proportional to the variance of the standard
MC estimator :math:`\hat{Q}_N^{MC}` through a factor :math:`1-\rho^2`
where
:math:`\rho = \dfrac{ \mathrm{Cov}\left(Q,G\right) }{\sqrt{\mathbb{V}ar\left(Q\right)\mathbb{V}ar\left(G\right)}}`
is the Pearson correlation coefficient between :math:`Q` and :math:`G`.
Since :math:`0<\rho^2<1`, the variance
:math:`\mathbb{V}ar\left( \hat{Q}_N^{MCCV} \right)` is always less than
the corresponding :math:`\mathbb{V}ar\left({\hat{Q}_N^{MC}}\right)`. The
control variate technique can be seen as a very general approach to
accelerate a MC simulation. The main step is to define a convenient
control variate function which is cheap to evaluate and well correlated
to the target function. For instance, function evaluations obtained
through a different (coarse) resolution may be employed or even coming
from a more crude physical/engineering approximation of the problem. A
viable way of building a well correlated control variate is to rely on a
low-fidelity model (i.e. a crude approximation of the model of interest)
to estimate the control variate using estimated control means (see
:cite:p:`Pasupathy2014,Ng2014` for more details). In this latter case,
clearly the expected value of the low-fidelity model is not known and needs to be computed.

With a slight change in notation, it is possible to write
 
 .. math:: \hat{Q}^{CVMC} = \hat{Q} + \alpha_1 \left( \hat{Q}_1 - \hat{\mu}_1 \right),
 
where :math:`\hat{Q}` represents the MC estimator for the high-fidelity model, :math:`\hat{Q}_1` the MC estimator for the low-fidelity model
and :math:`\hat{\mu}_1` a different approximation for :math:`\mathbb{E}[Q_1]`. If :math:`N` samples are used for approximating :math:`\hat{Q}` and
:math:`\hat{Q}_1` and a total of :math:`r_1 N` samples for the low-fidelity models are available, an optimal solution, which guarantees the best use of the low-fidelity resources,
can be obtained following :cite:p:`Ng2014` as

.. math:: \alpha_1 = -\rho_1 \sqrt{ \frac{ \mathbb{V}ar[Q] }{ \mathbb{V}ar[Q_1] } }
.. math:: r_1 = \sqrt{ \frac{ \mathcal{C} }{ \mathcal{C}_1 } \frac{\rho_1^2}{1-\rho_1^2} },

where :math:`\mathcal{C}` and :math:`\mathcal{C}_1` represent the cost of evaluating the high- and low-fidelity models, respectively and :math:`\rho_1` is the correlation between the two models. This solution leads to the following expression for the estimator variance 

.. math::  \mathbb{V}ar[\hat{Q}^{CVMC}] = \mathbb{V}ar[\hat{Q}] \left( 1 - \frac{r_1-1}{r_1} \rho_1^2 \right),

which shows similarities with the variance of a control variate estimator with the only difference being the term :math:`\frac{r_1-1}{r_1}` that, by multiplying the correlation 
:math:`\rho_1`, effectively penalizes the estimator due to the need for estimating the low-fidelity mean.

Another common case encountered in practice is the availability of more than a low-fidelity model. In this case, the multifidelity Monte Carlo can be extended following
:cite:p:`peherstorfer2016optimal,peherstorfer_survey_2018` as

.. math:: \hat{Q}^{MFMC} = \hat{Q} + \sum_{i=1}^M \alpha_i \left( \hat{Q}_i - \hat{\mu}_i \right),

where :math:`\hat{Q}_i` represents the generic ith low-fidelity model.

The MFMC estimator is still unbiased (similarly to MC) and share similarities with CVMC; indeed one can recover CVMC directly from it. For each low-fidelity model we use :math:`N_i r_i` samples, as in the CVMC case, however for :math:`i \geq 2`, the term :math:`\hat{Q_i}` is approximated with exactly the same samples of the previous model, while each :math:`\hat{\mu}_i` is obtained by adding to this set a number of :math:`(r_i-r_{i-1}) N_i` additional independent samples. Following :cite:p:`peherstorfer2016optimal` the weights can be obtained as

.. math::
   :label: mfmc_alpha

   \alpha_i = - \rho_i \sqrt{ \frac{ \mathbb{V}ar[Q] }{ \mathbb{V}ar[Q_i] } }.

The optimal resource allocation problem is also obtainable in closed-form if, as demonstrated in :cite:p:`peherstorfer2016optimal` the following conditions, for the models' correlations and costs, hold 

.. math:: |\rho_1| > |\rho_2| > \dots > |\rho_M|

.. math:: \frac{\mathcal{C}_{i-1}}{\mathcal{C}_{i}} > \frac{ \rho_{i-1}^2 - \rho_{i}^2 }{ \rho_{i}^2 - \rho_{i+1}^2 },

leading to 

.. math:: r_i = \sqrt{ \frac{\mathcal{C}}{\mathcal{C}_i} \frac{\rho_i^2 - \rho_{i+1}^2}{1-\rho_1^2} }.


.. _uq:sampling:multilevel:

Multilevel Monte Carlo
----------------------

In general engineering applications, the quantity of interest :math:`Q`
is obtained as the result of the numerical solution of a partial partial
differential equation (possibly a system of them). Therefore, the
dependence on the physical
:math:`\mathbf{x} \in \Omega\subset\mathbb{R}^n` and/or temporal
:math:`t \in T\subset\mathbb{R^+}` coordinates should be included, hence
:math:`Q=Q(\mathbf{x}, \boldsymbol{\xi}, t)`. A finite spatial/temporal
resolution is always employed to numerically solve a PDE, implying the
presence of a discretization error in addition to the stochastic error.
The term discretization is applied generically with reference to either
the spatial tessellation, the temporal resolution, or both (commonly,
they are linked). For a generic tessellation with :math:`M`
degrees-of-freedom (DOFs), the PDE solution of :math:`Q` is referred to
as :math:`Q_M`. Since :math:`Q_M \rightarrow Q` for
:math:`M\rightarrow\infty`, then
:math:`\mathbb{E}\left[{Q_M}\right] \rightarrow \mathbb{E}\left[{Q}\right]`
for :math:`M\rightarrow\infty` with a prescribed order of convergence. A
MC estimator in presence of a finite spatial resolution and finite
sampling is

.. math:: \hat{Q}^{MC}_{M,N} = \frac{1}{N} \sum_{i=1}^N Q_M^{(i)}

for which the mean square error (MSE) is

.. math::

   \mathbb{E}\left[ (\hat{Q}^{MC}_{M,N}-\mathbb{E}\left[ Q \right] )^2 \right]
          = N^{-1} \mathbb{V}ar\left({Q_M}\right) + \left( \mathbb{E}\left[{ Q_M-Q }\right] \right)^2,

where the first term represents the variance of the estimator, and the
second term :math:`\left( \mathbb{E}\left[ Q_M-Q \right] \right)^2`
reflects the bias introduced by the (finite) spatial discretization. The
two contributions appear to be independent of each other; accurate MC
estimates can only be obtained by drawing the required :math:`N` number
of simulations of :math:`Q_M( \boldsymbol{\xi} )` at a sufficiently fine
resolution :math:`M`. Since the numerical cost of a PDE is related to
the number of DOFs of the tessellation, the total cost of a MC
simulation for a PDE can easily become intractable for complex
multi-physics applications that are computationally intensive.

Multilevel Monte Carlo for the mean
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

..
   TODO:The multilevel Monte Carlo (MLMC) algorithm has been
   introduced, starting from the control variate idea, for situation
   in which additional
   %(with respect to the stochastic space)
   discretization levels can be defined. The basic idea, borrowed from
   the multigrid approach,

The multilevel Monte Carlo (MLMC) algorithm has been introduced,
starting from the control variate idea, for situation in which
additional discretization levels can be defined. The basic idea,
borrowed from the multigrid approach, is to replace the evaluation of
the statistics of :math:`Q_M` with a sequence of evaluations at coarser
levels. If it is possible to define a sequence of discretization levels
:math:`\left\{ M_\ell: \ell = 0, \dots, L \right\}` with
:math:`M_0 < M_1 < \dots < M_L \stackrel{\mathrm{def}}{=} M`, the
expected value :math:`\mathbb{E}\left[{Q_M}\right]` can be decomposed,
exploiting the linearity of the expected value operator as

.. math:: \mathbb{E}\left[{Q_{M}}\right] = \mathbb{E}\left[{Q_{M_0}}\right] + \sum_{\ell = 1}^L \mathbb{E }\left[ Q_{M_{\ell}} - Q_{M_{\ell-1}} \right].

If the difference function :math:`Y_\ell` is defined according to

.. math::

   Y_\ell = \left\{
    \begin{split}
    Q_{M_0} \quad &\mathrm{if} \quad \ell=0 \\
    Q_{M_{\ell}} - Q_{M_{\ell-1}} \quad &\mathrm{if} \quad 0<\ell\leq L,
    \end{split}
    \right.

the expected value
:math:`\mathbb{E}\left[{Q_M}\right]=\sum_{\ell=0}^{L}{  \mathbb{E}\left[Y_\ell\right]   }`.
A multilevel MC estimator is obtained when a MC estimator is adopted
independently for the evaluation of the expected value of :math:`Y_\ell`
on each level. The resulting multilevel estimator
:math:`\hat{Q}_M^{\mathrm{ML}}` is

.. math::

   \hat{Q}_M^{\mathrm{ML}} = \, \sum_{\ell = 0}^L \hat{Y}_{\ell, N_\ell}^{\mathrm{MC}} 
    = \sum_{\ell = 0}^L \frac{1}{N_\ell} \sum_{i=1}^{N_\ell} Y_\ell^{(i)}.

Since the multilevel estimator is unbiased, the advantage of using this
formulation is in its reduced estimator variance
:math:`\sum_{\ell=0}^{L} N_\ell^{-1} \mathbb{V}ar\left({Y_\ell}\right)`:
since :math:`Q_M \rightarrow Q`, the difference function
:math:`Y_\ell \rightarrow 0` as the level :math:`\ell` increases.
Indeed, the corresponding number of samples :math:`N_\ell` required to
resolve the variance associated with the :math:`\ell`\ th level is
expected to decrease with :math:`\ell`.

The MLMC algorithm can be interpreted as a strategy to optimally
allocate resources. If the total cost of the MLMC algorithm is written
as

.. math::

   \label{EQ: MLMC cost}
   \mathcal{C}(\hat{Q}^{ML}_{M}) = \sum_{\ell=0}^{L} N_\ell \, \mathcal{C}_{\ell},

with :math:`\mathcal{C}_{\ell}` being the cost of the evaluation of
:math:`Y_\ell` (involving either one or two discretization evaluations),
then the following constrained minimization problem can be formulated
where an equality constraint enforces a stochastic error (from MLMC
estimator variance) equal to the residual bias error
(:math:`\varepsilon^2/2`)

.. math::
   :label: mlmc_optimization
   
    f(N_\ell,\lambda) = \sum_{\ell=0}^{L} N_\ell \, \mathcal{C}_{\ell} 
                      + \lambda \left( \sum_{\ell=0}^{L} N_\ell^{-1} \mathbb{V}ar\left({Y_\ell}\right) - \varepsilon^2/2 \right).

using a Lagrange multiplier :math:`\lambda`. This equality constraint
reflects a balance between the two contributions to MSE, reflecting the
goal to not over-resolve one or the other. The result of the
minimization is

.. math::

   \label{EQ: MLMC nl}
   N_{\ell} = \frac{2}{\varepsilon^2} \left[ \, \sum_{k=0}^L \left( \mathbb{V}ar\left(Y_k\right) \mathcal{C}_k \right)^{1/2} \right] 
                  \sqrt{\frac{ \mathbb{V}ar\left({Y_\ell}\right) }{\mathcal{C}_{\ell}}},

defining an optimal sample allocation per discretization level.

MLMC extension to the variance
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Despite the original introduction of the MLMC approach for the
computation of the mean estimator in
:cite:p:`Giles2008,Giles2015`, it is possible to estimate
higher-order moments with a MLMC sampling strategy, as for instance the
variance.

A single level unbiased estimator for the variance of a generic QoI at
the highest level :math:`M_L` of the hierarchy can be written as

.. math::
   :label: variance_est_single_level
   
   \mathbb{V}ar\left[Q_{M_L}\right] \approx \frac{1}{N_{M_L} - 1} \sum_{i=1}^{N_{M_L}} \left( Q_{M_L}^{(i)} - \mathbb{E}\left[Q_L\right] \right)^2.

The multilevel version of
Eq. :eq:`variance_est_single_level`
can be obtained via a telescopic expansion in term of difference of
estimators over subsequent levels. To simplify the notation and for
simplicity of exposure from now on we only indicate the level, *i.e.*
:math:`M_\ell = \ell`.

The expansion is obtained by re-writing
Eq. :eq:`variance_est_single_level`
as

.. math::

   \begin{split}
   \label{eq: variance_est_ML}
    \mathbb{V}ar\left[Q_L\right] &\approx       \frac{1}{N_L - 1} \sum_{i=1}^{N_L} \left( Q_L^{(i)} - \mathbb{E}\left[Q_L\right] \right)^2 \\
                                 &\approx \sum_{\ell=0}^L  \frac{1}{N_\ell - 1} \left( \left( Q_{\ell}^{(i)} - \mathbb{E}\left[Q_{\ell}\right] \right)^2 
                                                                                     - \left( Q_{{\ell-1}}^{(i)} - \mathbb{E}\left[Q_{\ell-1}\right] \right)^2 \right).
   \end{split}

It is important here to note that since the estimators at the levels
:math:`\ell` and :math:`\ell-1` are computed with the same number of
samples both estimators use the factor :math:`1/(N_\ell-1)` to obtain
their unbiased version. Moreover, each estimator is indeed written with
respect to its own mean value, *i.e.* the mean value on its level,
either :math:`\ell` or :math:`\ell-1`. This last requirement leads to
the computation of a local expected value estimator with respect to the
same samples employed for the difference estimator. If we now denote
with :math:`\hat{Q}_{\ell,2}` the sampling estimator for the second
order moment of the QoI :math:`Q_\ell` we can write

.. math::
   :label: variance_est_ML_approximation
   
   \begin{split}
   \mathbb{V}ar\left[Q_L\right] \approx \hat{Q}_{L,2}^{\mathrm{ML}} = \sum_{\ell=0}^L \hat{Q}_{\ell,2} - \hat{Q}_{\ell-1,2},
   \end{split}

where

.. math::
   :label: variance_est_ML_level_terms
   
   \hat{Q}_{\ell,2} = \frac{1}{N_\ell - 1} \sum_{i=1}^{N_\ell} \left( Q_\ell^{(i)} - \hat{Q}_\ell \right)^2
   \quad  \mathrm{and} \quad
   \hat{Q}_{\ell - 1,2} = \frac{1}{N_\ell - 1} \sum_{i=1}^{N_\ell} \left( Q_{\ell - 1}^{(i)} - \hat{Q}_{\ell - 1} \right)^2.

Note that :math:`\hat{Q}_{\ell,2}` and :math:`\hat{Q}_{\ell - 1,2}` are
explicitly sharing the same samples :math:`N_\ell`.

For this estimator we are interested in minimizing its cost while also
prescribing its variance as done for the expected value. This is
accomplished by evaluating the variance of the multilevel variance
estimator :math:`\hat{Q}_{L,2}^{ML}`

.. math::

   \mathbb{V}ar\left[ \hat{Q}_{L,2}^{\mathrm{ML}} \right] = \sum_{\ell=0}^L \mathbb{V}ar\left[ \hat{Q}_{\ell,2} - \hat{Q}_{\ell-1,2} \right]
                                                  = \sum_{\ell=0}^L \mathbb{V}ar\left[ \hat{Q}_{\ell,2} \right] + \mathbb{V}ar\left[\hat{Q}_{\ell-1,2} \right]
                                                  - 2 \mathbb{C}ov\left( \hat{Q}_{\ell,2},\hat{Q}_{\ell-1,2} \right),

where the covariance term is a result of the dependence described
in :eq:`variance_est_ML_level_terms`.

The previous expression can be evaluated once the variance for the
sample estimator of the second order order moment
:math:`\mathbb{V}ar\left[ \hat{Q}_{\ell,2} \right]` and the covariance
term
:math:`\mathbb{C}ov\left( \hat{Q}_{\ell,2},\hat{Q}_{\ell-1,2} \right)`
are known. These terms can be evaluated as:

.. math:: \mathbb{V}ar\left[ \hat{Q}_{\ell,2} \right] \approx \frac{1}{N_\ell} \left( \hat{Q}_{\ell,4} - \frac{N_\ell-3}{N_\ell-1} \left(\hat{Q}_{\ell,2}\right)^2 \right),

where :math:`\hat{Q}_{\ell,4}` denotes the sampling estimator for the
fourth order central moment.

The expression for the covariance term is more involved and can be
written as

.. math::

   \begin{split}
    \mathbb{C}ov\left( \hat{Q}_{\ell,2},\hat{Q}_{\ell-1,2} \right) &\approx \frac{1}{N_\ell} \mathbb{E}\left[ \hat{Q}_{\ell,2},\hat{Q}_{\ell-1,2} \right] \\
                                                                         &+ \frac{1}{N_\ell N_{\ell-1}} \left( \mathbb{E}\left[ Q_\ell Q_{\ell-1} \right]^2
                                                                         - 2  \mathbb{E}\left[ Q_\ell Q_{\ell-1} \right] \mathbb{E}\left[ Q_\ell \right] \mathbb{E}\left[Q_{\ell-1} \right] + \left( \mathbb{E}\left[ Q_\ell \right] \mathbb{E}\left[Q_{\ell-1} \right] \right)^2
                                                                         \right).
   \end{split}

The first term of the previous expression is evaluated by estimating and
combining several sampling moments as

.. math::

   \begin{split}
    \mathbb{E}\left[ \hat{Q}_{\ell,2},\hat{Q}_{\ell-1,2} \right] &= \frac{1}{N_\ell} \left( \mathbb{E}\left[ Q_\ell^2 Q_{\ell-1}^2 \right] \right) - \mathbb{E}\left[ Q_\ell^2 \right] \mathbb{E}\left[Q_{\ell-1}^2 \right] - 2 \mathbb{E}\left[Q_{\ell-1} \right] \mathbb{E}\left[ Q_{\ell}^2 Q_{\ell-1} \right] \\
                                         &+ 2 \mathbb{E}\left[Q_{\ell-1}^2 \right] \mathbb{E}\left[ Q_{\ell}^2 \right]
                                         - 2  \mathbb{E}\left[ Q_{\ell} \right] \mathbb{E}\left[ Q_{\ell} Q_{\ell-1}^2 \right]
                                         + 2 \mathbb{E}\left[ Q_{\ell} \right]^2 \mathbb{E}\left[ Q_{\ell-1}^2 \right] \\
                                         &+ 4 \mathbb{E}\left[ Q_{\ell} \right] \mathbb{E}\left[ Q_{\ell-1} \right] \mathbb{E}\left[ Q_{\ell} Q_{\ell-1} \right]
                                         - 4 \mathbb{E}\left[ Q_{\ell} \right]^2 \mathbb{E}\left[ Q_{\ell-1} \right]^2.
   \end{split}

It is important to note here that the previous expression can be
computed only if several sampling estimators for product of the QoIs at
levels :math:`\ell` and :math:`\ell-1` are available. These quantities
are not required in the standard MLMC implementation for the mean and
therefore for the estimation of the variance more data need to be stored
to assemble the quantities on each level.

An optimization problem, similar to the one formulated for the mean in
the previous section, can be written in the case of variance

.. math::
   :label: mlmc_optimization_var
   
   \begin{split}
   \min\limits_{N_\ell} \sum_{\ell=0}^L \mathcal{C}_{\ell} N_\ell \quad \mathrm{s.t.} \quad \mathbb{V}ar\left[ \hat{Q}_{L,2}^{\mathrm{ML}} \right] = \varepsilon^2/2.
   % 
   % 
   %  f(N_\ell,\lambda) = \sum_{\ell=0}^{L} N_\ell \, \mathcal{C}_{\ell} 
   %                    + \lambda \left( \sum_{\ell=0}^{L} N_\ell^{-1} \mathbb{V}ar\left({Y_\ell}\right) - \varepsilon^2/2 \right). 
   \end{split}

This optimization problem can be solved in two different ways, namely an
analytical approximation and by solving a non-linear optimization
problem. The analytical approximation follows the approach described in
:cite:p:`Pisaroni2017` and introduces a helper variable

.. math:: \hat{V}_{2, \ell} := \mathbb{V}ar\left[ \hat{Q}_{\ell,2} \right] \cdot N_{\ell}.

Next, the following constrained minimization problem is formulated

.. math::
   :label: mlmc_var_optimization_nobile
   
    f(N_\ell,\lambda) = \sum_{\ell=0}^{L} N_\ell \, \mathcal{C}_{\ell} 
                      + \lambda \left( \sum_{\ell=0}^{L} N_\ell^{-1} \hat{V}_{2, \ell} - \varepsilon^2/2 \right),

and a closed form solution is obtained

.. math::
   :label: MLMC_nl_var_nobile
   
   N_{\ell} = \frac{2}{\varepsilon^2} \left[ \, \sum_{k=0}^L \left( \hat{V}_{2, k} \mathcal{C}_k \right)^{1/2} \right] 
                  \sqrt{\frac{ \hat{V}_{2, \ell} }{\mathcal{C}_{\ell}}},

similarly as for the expected value in :eq:`mlmc_optimization`.

..
   TODO: Note here, that higher order terms of $N_\ell$ in
   $\mathbb{V}ar\left[ \hat{Q}_{\ell,2} \right]$ are not considered in
   the optimization.

The second approach uses numerical optimization directly on the
non-linear optimization
problem :eq:`mlmc_optimization_var` to
find an optimal sample allocation. Dakota uses OPTPP as the default
optimizer and switches to NPSOL if it is available.

Both approaches for finding the optimal sample allocation when
allocating for the variance are currently implemented in Dakota. The
analytical solution is employed by default while the optimization is
enabled using a keyword. We refer to the reference manual for a
discussion of the keywords to select these different options.

MLMC extension to the standard deviation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The extension of MLMC for the standard deviation is slightly more
complicated by the presence of the square root, which prevents a
straightforward expansion over levels.

One possible way of obtaining a biased estimator for the standard
deviation is

.. math:: \hat{\sigma}_L^{ML} = \sqrt{ \sum_{\ell=0}^L \hat{Q}_{\ell,2} - \hat{Q}_{\ell - 1,2} }.

To estimate the variance of the standard deviation estimator, it is
possible to leverage the result, derived in the previous section for the
variance, and write the variance of the standard deviation as a function
of the variance and its estimator variance. If we can estimate the
variance :math:`\hat{Q}_{L,2}` and its estimator variance
:math:`\mathbb{V}ar\left[ \hat{Q}_{L,2} \right]`, the variance for the
standard deviation :math:`\hat{\sigma}_L^{ML}` can be approximated as

.. math:: \mathbb{V}ar\left[ \hat{\sigma}_L^{ML} \right] \approx \frac{1}{4 \hat{Q}_{L,2}} \mathbb{V}ar\left[ \hat{Q}_{L,2} \right].

Similarly to the variance case, a numerical optimization problem can be
solved to obtain the sample allocation for the estimator of the standard
deviation given a prescribed accuracy target.

MLMC extension to the scalarization function
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Often, especially in the context of optimization, it is necessary to
estimate statistics of a metric defined as a linear combination of
mean and standard deviation of a QoI. A classical reliability measure
:math:`c^{ML}[Q]` can be defined, for the quantity :math:`Q`, starting
from multilevel (ML) statistics, as

.. math::

   c_L^{ML}[Q] = \hat{Q}_{L}^{ML}  + \alpha \hat{\sigma}_L^{ML}.

To obtain the sample allocation, in the MLMC context, it is necessary
to evaluate the variance of :math:`c_L^{ML}[Q]`, which can be written as

.. math::

   \mathbb{V}ar\left[ c_L^{ML}[Q] \right] = \mathbb{V}ar\left[ \hat{Q}_{L}^{ML} \right] + \alpha^2 \mathbb{V}ar\left[ \hat{\sigma}_L^{ML} \right] 
   + 2 \alpha \mathbb{C}ov\left[ \hat{Q}_{L}^{ML}, \hat{\sigma}_L^{ML} \right].

This expression requires, in addition to the already available terms
:math:`\mathbb{V}ar\left[ \hat{Q}_{L}^{ML} \right]` and
:math:`\mathbb{V}ar\left[ \hat{\sigma}_L^{ML} \right]`, also the
covariance term :math:`\mathbb{C}ov\left[ \hat{Q}_{L}^{ML},
\hat{\sigma}_L^{ML} \right]`. This latter term can be written knowing
that shared samples are only present on the same level

.. math::

   \begin{split}
    \mathbb{C}ov\left[ \hat{Q}_{L}^{ML}, \hat{\sigma}_L^{ML} \right] &= \mathbb{C}ov\left[ \sum_{\ell=0}^{L} \hat{Q}_{\ell} - \hat{Q}_{\ell-1}, \sum_{\ell=0}^{L} \hat{\sigma}_{\ell} - \hat{\sigma}_{\ell-1} \right] \\
                                                                     &= \sum_{\ell=0}^{L} \mathbb{C}ov\left[ \hat{Q}_{\ell} - \hat{Q}_{\ell-1}, \hat{\sigma}_{\ell} - \hat{\sigma}_{\ell-1} \right],
   \end{split}

which leads to the need for evaluating the following four
contributions

.. math::

   \mathbb{C}ov\left[ \hat{Q}_{\ell} - \hat{Q}_{\ell-1}, \hat{\sigma}_{\ell} - \hat{\sigma}_{\ell-1} \right] =
   \mathbb{C}ov\left[ \hat{Q}_{\ell} , \hat{\sigma}_{\ell} \right] - \mathbb{C}ov\left[ \hat{Q}_{\ell} , \hat{\sigma}_{\ell-1} \right]
   - \mathbb{C}ov\left[ \hat{Q}_{\ell-1}, \hat{\sigma}_{\ell} \right] + \mathbb{C}ov\left[ \hat{Q}_{\ell-1}, \hat{\sigma}_{\ell-1} \right].

In Dakota, we adopt the following approximation, for two arbitrary
levels :math:`\ell` and
:math:`\kappa \in \left\{ \ell-1, \ell, \ell+1 \right\}`

.. math::

   \rho\left[ \hat{Q}_{\ell}, \hat{\sigma}_{\kappa} \right] \approx \rho\left[ \hat{Q}_{\ell}, \hat{Q}_{\kappa,2} \right]

(we indicate with :math:`\hat{Q}_{\kappa,2}` the second central moment
for :math:`Q` at the level :math:`\kappa`), which corresponds to
assuming that the correlation between expected value and variance is a
good approximation of the correlation between the expected value and
the standard deviation. This assumption is particularly convenient
because it is possible to obtain in closed form the covariance between
expected value and variance and, therefore, we can adopt the following
approximation

.. math::

   \begin{split}
    \frac{ \mathbb{C}ov\left[ \hat{Q}_{\ell}, \hat{\sigma}_{\kappa} \right]}{\sqrt{ \mathbb{V}ar\left[ \hat{Q}_{\ell} \right] \mathbb{V}ar\left[ \hat{\sigma}_{\kappa} \right]} } 
    \approx \frac{\mathbb{C}ov\left[ \hat{Q}_{\ell}, \hat{Q}_{\kappa,2} \right]}{\sqrt{ \mathbb{V}ar\left[ \hat{Q}_{\ell}\right] \mathbb{V}ar\left[ \hat{Q}_{\kappa,2}\right] }} \\
    %
    \mathbb{C}ov\left[ \hat{Q}_{\ell}, \hat{\sigma}_{\kappa} \right] 
    \approx \mathbb{C}ov\left[ \hat{Q}_{\ell}, \hat{Q}_{\kappa,2} \right] \frac{\sqrt{\mathbb{V}ar\left[ \hat{\sigma}_{\kappa} \right]}}{\sqrt{  \mathbb{V}ar\left[ \hat{Q}_{\kappa,2}\right] }}.
   \end{split}

Finally, we can derive the term
:math:`\mathbb{C}ov\left[ \hat{Q}_{\ell}, \hat{Q}_{\kappa,2} \right]`
for all possible cases

.. math::

   \mathbb{C}ov\left[ \hat{Q}_{\ell}, \hat{Q}_{\kappa,2} \right] = 
   \begin{cases}
      \frac{1}{N_\ell} \left( \mathbb{E}\left[ Q_\ell Q_{\kappa}^2 \right] 
                            - \mathbb{E}\left[ Q_\ell \right] \mathbb{E}\left[ Q_{\kappa}^2 \right] 
                            - 2 \mathbb{E}\left[ Q_{\kappa} \right] \mathbb{E}\left[ Q_\ell Q_{\kappa} \right]
                            + 2 \mathbb{E}\left[ Q_\ell \right] \mathbb{E}\left[ Q_\kappa^2 \right]
                            \right),& \text{if } \kappa \neq \ell \\
      \frac{\hat{Q}_{\ell,3}}{N_\ell},              & \text{if }  \kappa = \ell.
  \end{cases}

..
   TODO:

   In this case, in order to obtain the variance of $c^{ML}[Q]$ it is necessary to employ an additional approximation:
   \begin{equation}
   \begin{split}
    \mathbb{V}ar\left[ c^{ML}[Q] \right] &= \mathbb{V}ar\left[ \hat{Q}_{L}^{ML} \right] + \alpha^2 \mathbb{V}ar\left[ \hat{\sigma}_L^{ML} \right] 
                                         + 2 \alpha \mathbb{C}ov\left[ \hat{Q}_{L}^{ML}, \hat{\sigma}_L^{ML} \right] \\
                                         &= \mathbb{V}ar\left[ \hat{Q}_{L}^{ML} \right] + \alpha^2 \mathbb{V}ar\left[ \hat{\sigma}_L^{ML} \right] 
                                         + 2 \alpha \rho\left[\hat{Q},\hat{\sigma}\right] \sqrt{ \mathbb{V}ar\left[ \hat{Q}_{L}^{ML} \right] }  \sqrt{ \mathbb{V}ar\left[ \hat{\sigma}_L^{ML} \right] } \\
                                         &\leq \mathbb{V}ar\left[ \hat{Q}_{L}^{ML} \right] + \alpha^2 \mathbb{V}ar\left[ \hat{\sigma}_L^{ML} \right] 
                                         + 2 |\alpha| \sqrt{ \mathbb{V}ar\left[ \hat{Q}_{L}^{ML} \right] }  \sqrt{ \mathbb{V}ar\left[ \hat{\sigma}_L^{ML} \right] },
   \end{split}
   \end{equation}
   
   which permits to bound the maximum value for the variance (assuming a very conservative approximation for the correlation between the estimators for the mean and the standard deviation, \textit{i.e.} $\left|\rho\left[\hat{Q},\hat{\sigma}\right]\right|=1$).

   All terms in the previous expression can be written as a function of the quantities derived in the previous sections, and, therefore, even for this case the allocation problem can be solved by resorting to a numerical optimization given a prescribed target.

Even for this case, the sample allocation problem can be solved by
resorting to a numerical optimization given a prescribed target.


.. _uq:sampling:mlmf:

A multilevel-multifidelity approach
-----------------------------------

The MLMC approach described in :ref:`uq:sampling:multilevel` can
be related to a recursive control variate technique in that it
seeks to reduce the variance of the target function in order to limit
the sampling at high resolution. In addition, the difference function
:math:`Y_\ell` for each level can itself be the target of an additional
control variate (refer to :ref:`uq:sampling:mfmc`). A
practical scenario is when not only different resolution levels are
available (multilevel part), but also a cheaper computational model can
be used (multifidelity part). The combined approach is a
multilevel-multifidelity algorithm :cite:p:`Fairbanks2017,Nobile2015,GeraciCTR`, and in particular, a
multilevel-control variate Monte Carlo sampling approach.

.. _uq:sampling:mlmf:Ycorr:

:math:`Y_l` correlations
~~~~~~~~~~~~~~~~~~~~~~~~

If the target QoI can be generated from both a high-fidelity (HF) model
and a cheaper, possibly biased low-fidelity (LF) model, it is possible
to write the following estimator

.. math::
   :label: MLMF estimator
   
   \mathbb{E}\left[Q_M^{\mathrm{HF}}\right] = \sum_{l=0}^{L_{\mathrm{HF}}} \mathbb{E}\left[Y^{\mathrm{HF}}_{\ell}\right] 
                                             \approx \sum_{l=0}^{L_{\mathrm{HF}}} \hat{Y}^{\mathrm{HF}}_{\ell} = \sum_{l=0}^{L_{\mathrm{HF}}} Y^{{\mathrm{HF}},\star}_{\ell},

where

.. math:: Y^{{\mathrm{HF}},\star}_{\ell} = Y^{\mathrm{HF}}_{\ell} + \alpha_\ell \left( \hat{Y}^{\mathrm{LF}}_{\ell} - \mathbb{E}\left[{Y^{\mathrm{LF}}_{\ell}}\right] \right).

The estimator :math:`Y^{\mathrm{HF},\star}_{\ell}` is unbiased with
respect to :math:`\hat{Y}^{\mathrm{HF}}_{\ell}`, hence with respect to
the true value :math:`\mathbb{E}\left[Y^{\mathrm{HF}}_{\ell}\right]`.
The control variate is obtained by means of the LF model realizations
for which the expected value can be computed in two different ways:
:math:`\hat{Y}^{\mathrm{LF}}_{\ell}` and
:math:`\mathbb{E}\left[Y^{\mathrm{LF}}_{\ell}\right]`. A MC estimator is
employed for each term but the estimation of
:math:`\mathbb{E}\left[Y^{\mathrm{LF}}_{\ell}\right]` is more resolved
than :math:`\hat{Y}^{\mathrm{LF}}_{\ell}`. For
:math:`\hat{Y}^{\mathrm{LF}}_{\ell}`, we choose the number of LF
realizations to be equal to the number of HF realizations,
:math:`N_{\ell}^{\mathrm{HF}}`. For the more resolved
:math:`\mathbb{E}\left[Y^{\mathrm{LF}}_{\ell}\right]`, we augment with
an additional and independent set of realizations
:math:`\Delta_{\ell}^{\mathrm{LF}}`, hence
:math:`N_{\ell}^{\mathrm{LF}} = N_{\ell}^{\mathrm{HF}} + \Delta_{\ell}^{\mathrm{LF}}`.
The set :math:`\Delta_{\ell}^{\mathrm{LF}}` is written, for convenience,
as proportional to :math:`N_{\ell}^{\mathrm{HF}}` by means of a
parameter :math:`r_{\ell} \in \mathbb{R}^+_0`

.. math::

   N_{\ell}^{\mathrm{LF}} = N_{\ell}^{\mathrm{HF}} + \Delta_{\ell}^{\mathrm{LF}} = N_{\ell}^{\mathrm{HF}} + r_{\ell} N_{\ell}^{\mathrm{HF}} 
                           = N_{\ell}^{\mathrm{HF}} (1 + r_{\ell}).

The set of samples :math:`\Delta_{\ell}^{\mathrm{LF}}` is independent of
:math:`N_{\ell}^{\mathrm{HF}}`, therefore the variance of the estimator
can be written as (for further details see
:cite:p:`GeraciCTR`)

.. math::
   :label: MLMF mean
   
   \begin{split}
   \mathbb{V}ar\left(\hat{Q}_M^{MLMF}\right) &= \sum_{l=0}^{L_{\mathrm{HF}}} \left( \dfrac{1}{N_{\ell}^{\mathrm{HF}}} \mathbb{V}ar\left(Y^{\mathrm{HF}}_{\ell}\right) 
                                             + \dfrac{\alpha_\ell^2 r_\ell}{(1+r_\ell) N_{\ell}^{\mathrm{HF}}} \mathbb{V}ar\left(Y^{\mathrm{HF}}_{\ell}\right) \right. \\
                 &+  \left. 2 \dfrac{\alpha_\ell r_\ell^2}{(1+r_\ell) N_{\ell}^{\mathrm{HF}}} \rho_\ell \sqrt{ \mathbb{V}ar\left(Y^{\mathrm{HF}}_{\ell}\right) 
                                                                                                         \mathbb{V}ar\left(Y^{\mathrm{LF}}_{\ell}\right) } \right),
   \end{split}

The Pearson’s correlation coefficient between the HF and LF models is
indicated by :math:`\rho_\ell` in the previous equations. Assuming the
vector :math:`r_\ell` as a parameter, the variance is minimized per
level, mimicking the standard control variate approach, and thus
obtaining the optimal coefficient as
:math:`\alpha_\ell = -\rho_\ell \sqrt{ \dfrac{ \mathbb{V}ar\left( Y^{\mathrm{HF}}_{\ell} \right) }{ \mathbb{V}ar\left( Y^{\mathrm{LF}}_{\ell}  \right)     }}`.
By making use of the optimal coefficient :math:`\alpha_\ell`, it is
possible to show that the variance
:math:`\mathbb{V}ar\left(Y^{\mathrm{HF},\star}_{\ell}\right)` is
proportional to the variance
:math:`\mathbb{V}ar\left(Y^{\mathrm{HF}}_{\ell}\right)` through a factor
:math:`\Lambda_{\ell}(r_\ell)`, which is an explicit function of the
ratio :math:`r_\ell`:

.. math::
   :label: MLMF variance
   
   \begin{split}
    \mathbb{V}ar\left(\hat{Q}_M^{MLMF}\right) &= \sum_{l=0}^{L_{\mathrm{HF}}} \dfrac{1}{N_{\ell}^{\mathrm{HF}}} \mathbb{V}ar\left(Y^{\mathrm{HF}}_{\ell}\right)
    \Lambda_{\ell}(r_\ell) \quad \mathrm{where} \\
    \Lambda_{\ell}(r_\ell) &= \left( 1 - \dfrac{r_\ell}{1+r_\ell}\rho_\ell^2 \right).
   \end{split}

Note that :math:`\Lambda_{\ell}(r_\ell)` represents a penalty with
respect to the classical control variate approach presented in :ref:`uq:sampling:mfmc`, which stems from the need to
evaluate the unknown function
:math:`\mathbb{E}\left[Y^{\mathrm{LF}}_{\ell}\right]`. However, the
ratio :math:`r_\ell/(r_\ell+1)` is dependent on the additional number of
LF evaluations :math:`\Delta_{\ell}^{\mathrm{LF}}`, hence it is fair to
assume that it can be made very close to unity by choosing an affordably
large :math:`r_\ell`, i.e.,
:math:`\Delta_{\ell}^{\mathrm{LF}} >> N_{\ell}^{\mathrm{HF}}`.

The optimal sample allocation is determined taking into account the
relative cost between the HF and LF models and their correlation (per
level). In particular the optimization problem introduced in
Eq. :eq:`mlmc_optimization` is replaced by

.. math::

   \mathrm{argmin}_{N_{\ell}^{\mathrm{HF}}, r_\ell}(\mathcal{L}), \quad \mathrm{where} \quad \mathcal{L} = \sum_{\ell=0}^{L_{\mathrm{HF}}} N_{\ell}^{\mathrm{HF}} \mathcal{C}_{\ell}^{\mathrm{eq}} +
                    \lambda \left( \sum_{\ell=0}^{L_{\mathrm{HF}}} \dfrac{1}{N_{\ell}^{\mathrm{HF}}}\mathbb{V}ar\left( Y^{\mathrm{HF}}_{\ell}\right) \Lambda_{\ell}(r_\ell) - \varepsilon^2/2 \right),

where the optimal allocation is obtained as well as the optimal ratio
:math:`r_\ell`. The cost per level includes now the sum of the HF and LF
realization cost, therefore it can be expressed as
:math:`\mathcal{C}_{\ell}^{\mathrm{eq}} = \mathcal{C}_{\ell}^{\mathrm{HF}} + \mathcal{C}_{\ell}^{\mathrm{LF}} (1+r_\ell)`.

If the cost ratio between the HF and LF model is
:math:`w_{\ell} =  \mathcal{C}_{\ell}^{\mathrm{HF}} / \mathcal{C}_{\ell}^{\mathrm{LF}}`
then the optimal ratio is

.. math:: r_\ell^{\star} = -1 + \sqrt{ \dfrac{\rho_\ell^2}{1-\rho_\ell^2} w_{\ell}},

and the optimal allocation is

.. math::

   \begin{split}
     N_{\ell}^{\mathrm{HF},\star} &= \frac{2}{\varepsilon^2} \!\! \left[ \, \sum_{k=0}^{L_{\mathrm{HF}}} 
           \left( \dfrac{ \mathbb{V}ar\left(  Y_k^{ \mathrm{HF} } \right) \mathcal{C}_{k}^{\mathrm{HF}}}{1-\rho_\ell^2} \right)^{1/2} \Lambda_{k}(r_k^{\star}) \right] 
                  \sqrt{ \left( 1 - \rho_\ell^2 \right) \frac{ \mathbb{V}ar\left(Y^{\mathrm{HF}}_{\ell}\right) }{\mathcal{C}_{\ell}^{\mathrm{HF}}}}.
   \end{split}

It is clear that the efficiency of the algorithm is related not only to
the efficiency of the LF model, i.e. how fast a simulation runs with
respect to the HF model, but also to the correlation between the LF and
HF model.

.. _uq:sampling:mlmf:Qcorr:

:math:`Q_l` correlations
~~~~~~~~~~~~~~~~~~~~~~~~

A potential refinement of the previous approach :cite:p:`geraci_multifidelity_2017` consists in exploiting
the QoI on each pair of levels, :math:`\ell` and :math:`\ell-1`, to
build a more correlated LF function. For instance, it is possible to use

.. math:: \mathring{Y}^{\mathrm{LF}}_{\ell} =  \gamma_\ell Q_\ell^{\mathrm{LF}} - Q_{\ell-1}^{\mathrm{LF}}

and maximize the correlation between :math:`Y_\ell^{\mathrm{HF}}` and
:math:`\mathring{Y}^{\mathrm{LF}}_{\ell}` through the coefficient
:math:`\gamma_\ell`.

Formally the two formulations are completely equivalent if
:math:`Y_\ell^{\mathrm{LF}}` is replaced with
:math:`\mathring{Y}^{\mathrm{LF}}_{\ell}` in
Eq. :eq:`MLMF estimator` and they can be
linked through the two ratios

.. math::

   \begin{split}
    \theta_{\ell} &= \dfrac{  \mathrm{Cov}\left(  Y^{\mathrm{HF}}_{\ell},\mathring{Y}^{\mathrm{LF}}_{\ell} \right)   }
                           {  \mathrm{Cov}\left( Y^{\mathrm{HF}}_{\ell},Y^{\mathrm{LF}}_{\ell} \right)  } \\
    \quad \tau_{\ell}  &= \dfrac{  \mathbb{V}ar\left(  \mathring{Y}^{\mathrm{LF}}_{\ell} \right)  }{ \mathbb{V}ar\left( Y^{\mathrm{LF}}_{\ell} \right) },
    \end{split}

obtaining the following variance for the estimator

.. math::

   \mathbb{V}ar\left(\hat{Q}_M^{MLMF} \right) = \dfrac{1}{N_{\ell}^{\mathrm{HF}}} \mathbb{V}ar\left( Y^{\mathrm{HF}}_{\ell} \right) 
    \left( 1 - \dfrac{r_\ell}{1+r_\ell} \rho_\ell^2 \dfrac{\theta_\ell^2}{\tau_\ell} \right).

Therefore, a way to increase the variance reduction is to maximize the
ratio :math:`\dfrac{\theta_\ell^2}{\tau_\ell}` with respect to the
parameter :math:`\gamma_\ell`. It is possible to solve analytically this
maximization problem obtaining

.. math::

   \gamma_\ell^\star= \dfrac{ \mathrm{Cov}\left(  Y^{\mathrm{HF}}_{\ell},Q_{\ell-1}^{\mathrm{LF}} \right) \mathrm{Cov}\left( Q_{\ell}^{\mathrm{LF}},Q_{\ell-1}^{\mathrm{LF}} \right) 
                      - \mathbb{V}ar\left(Q_{\ell-1}^{\mathrm{LF}}\right) \mathrm{Cov}\left(  Y^{\mathrm{HF}}_{\ell},Q_{\ell}^{\mathrm{LF}} \right) }
               { \mathbb{V}ar\left(Q_{\ell}^{\mathrm{LF}}\right) \mathrm{Cov}\left( Y^{\mathrm{HF}}_{\ell},Q_{\ell-1}^{\mathrm{LF}} \right) 
               - \mathrm{Cov}\left( Y^{\mathrm{HF}}_{\ell},Q_{\ell}^{\mathrm{LF}} \right) \mathrm{Cov}\left( Q_{\ell}^{\mathrm{LF}},Q_{\ell-1}^{\mathrm{LF}} \right) }.

..
   TODO: to which correspond the optimal ratio
   $\dfrac{\theta_\ell^2}{\tau_\ell} =
   \dfrac{\theta_\ell^2}{\tau_\ell} (\gamma_\ell^\star )$.

The resulting optimal allocation of samples across levels and model
forms is given by

.. math::

   \begin{split}
     r_\ell^{\star} &= -1 + \sqrt{ \dfrac{\rho_l^2 \dfrac{\theta_\ell^2}{\tau_\ell} }{1-\rho_\ell^2 \dfrac{\theta_\ell^2}{\tau_\ell}} w_{\ell}}, \quad \mathrm{where} \quad w_{\ell} 
                  =  \mathcal{C}_{\ell}^{\mathrm{HF}} / \mathcal{C}_{\ell}^{\mathrm{LF}}\\
     \Lambda_{\ell} &= 1 - \rho_\ell^2 \dfrac{\theta_\ell^2}{\tau_\ell} \dfrac{r_\ell^{\star}}{1+r_\ell^{\star}}\\
     N_{\ell}^{\mathrm{HF},\star} &= \frac{2}{\varepsilon^2} \!\! \left[ \, \sum_{k=0}^{ L_{\mathrm{HF}} } 
          \left( \dfrac{ \mathbb{V}ar\left(Y_k^{ \mathrm{HF} } \right) \mathcal{C}_{k}^{\mathrm{HF}}}{1-\rho_\ell^2 \dfrac{\theta_\ell^2}{\tau_\ell}} \right)^{1/2} \Lambda_{k}(r_k^{\star})\right] 
                  \sqrt{ \left( 1 - \rho_\ell^2 \dfrac{\theta_\ell^2}{\tau_\ell} \right) \frac{ \mathbb{V}ar\left( Y^{\mathrm{HF}}_{\ell} \right) }{\mathcal{C}_{\ell}^{\mathrm{HF}}}}
    \end{split}
