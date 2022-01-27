.. _`uq:sampling`:

Sampling Methods
================

This chapter introduces several fundamental concepts related to sampling
methods. In particular, the statistical properties of the Monte Carlo
estimator are discussed (§\ `1.1 <#uq:sampling:montecarlo>`__) and
strategies for multilevel and multifidelity sampling are introduced
within this context. Hereafter, multilevel refers to the possibility of
exploiting distinct discretization levels (i.e. space/time resolution)
within a single model form, whereas multifidelity involves the use of
more than one model form. In §\ `1.2 <#uq:sampling:controlvariate>`__,
we describe the control variate Monte Carlo algorithm that we align with
multifidelity sampling, and in §\ `1.3 <#uq:sampling:multilevel>`__, we
describe the multilevel Monte Carlo algorithm that we align with
multilevel sampling. In §\ `1.4 <#uq:sampling:mlmf>`__, we show that
these two approaches can be combined to create multilevel-multifidelity
sampling approaches.

.. _`uq:sampling:montecarlo`:

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

For large :math:`N` it is trivial to demonstrate that the MC estimator
is unbiased, i.e., its bias is zero and its convergence to the true
statistics is :math:`\mathcal{O}(N^{-1/2})`. Moreover, since each
sequence of realizations is different, another crucial property of any
estimator is its own variance:

.. math::

   \label{EQ: variance MC}
    \mathbb{V}ar\left( \hat{Q}_N^{MC} \right)  = \dfrac{1}{N^2} \sum_{i=1}^{N} \mathbb{V}ar\left( Q \right) 
               = \dfrac{\mathbb{V}ar\left(Q\right) }{N}.

Furthermore, it is possible to show, in the limit
:math:`N \rightarrow \infty`, that the error
:math:`\left( \mathbb{E}\left[Q\right] - \hat{Q}_N^{MC} \right) \sim 
\sqrt{\dfrac{\mathbb{V}ar\left(Q\right) }{N}} \mathcal{N}(0,1)`, where
:math:`\mathcal{N}(0,1)` represents a standard normal RV. As a
consequence, it is possible to define a confidence interval for the MC
estimator which has an amplitude proportional to the standard deviation
of the estimator. Indeed, the variance of the estimator plays a
fundamental role in the quality of the numerical results: the reduction
of the variance (for a fixed computational cost) is a very effective way
of improving the quality of the MC prediction.

.. _`uq:sampling:controlvariate`:

Control variate Monte Carlo
---------------------------

A closer inspection of Eq. `[EQ: variance MC] <#EQ: variance MC>`__
indicates that only an increase in the number of simulations :math:`N`
might reduce the overall variance, since
:math:`\mathbb{V}ar\left({Q}\right)` is an intrinsic property of the
model under analysis. However, more sophisticated techniques have been
proposed to accelerate the convergence of a MC simulation. For instance,
an incomplete list of these techniques can include stratified sampling,
importance sampling, Latin hypercube, deterministic Sobol’ sequences and
control variates. In particular, the control variate approach, is based
on the idea of replacing the RV :math:`Q` with a different one which has
the same expected value, but with a smaller variance. The goal is to
reduce the numerator in Eq. `[EQ: variance MC] <#EQ: variance MC>`__,
and hence the value of the estimator variance without requiring a larger
number of simulations. In a practical setting, the control variate makes
use of an auxiliary functional :math:`G=G(\boldsymbol{\xi})` for which
the expected value :math:`\mathbb{E}\left[G\right]` is known. Indeed,
the alternative estimator can be defined as

.. math::

   \label{EQ: control variate}
   \hat{Q}_N^{MCCV} =  \hat{Q}_N^{MC} - \beta \left( \hat{G}_N^{MC} - \mathbb{E}\left[G\right] \right).

The MC control variate estimator :math:`\hat{Q}_N^{MCCV}` is unbiased
(irrespective of the value of the parameter
:math:`\beta \in \mathbb{R}`), but its variance now has a more complex
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
:raw-latex:`\cite{Pasupathy2014}`). This technique has been introduced
in the context of optimization under uncertainty in
:raw-latex:`\cite{Ng2014}`.

.. _`uq:sampling:multilevel`:

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

   \label{EQ:mlmc_optimization}
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
:raw-latex:`\cite{Giles2008,Giles2015}`, it is possible to estimate
higher-order moments with a MLMC sampling strategy, as for instance the
variance.

A single level unbiased estimator for the variance of a generic QoI at
the highest level :math:`M_L` of the hierarchy can be written as

.. math::

   \label{eq: variance_est_single_level}
    \mathbb{V}ar\left[Q_{M_L}\right] \approx \frac{1}{N_{M_L} - 1} \sum_{i=1}^{N_{M_L}} \left( Q_{M_L}^{(i)} - \mathbb{E}\left[Q_L\right] \right)^2.

The multilevel version of
Eq. `[eq: variance_est_single_level] <#eq: variance_est_single_level>`__
can be obtained via a telescopic expansion in term of difference of
estimators over subsequent levels. To simplify the notation and for
simplicity of exposure from now on we only indicate the level, *i.e.*
:math:`M_\ell = \ell`.

The expansion is obtained by re-writing
Eq. `[eq: variance_est_single_level] <#eq: variance_est_single_level>`__
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

   \begin{split}
   \label{eq: variance_est_ML_approximation}
    \mathbb{V}ar\left[Q_L\right] \approx \hat{Q}_{L,2}^{\mathrm{ML}} = \sum_{\ell=0}^L \hat{Q}_{\ell,2} - \hat{Q}_{\ell-1,2},
   \end{split}

where

.. math::

   \label{eq: variance_est_ML_level_terms}
    \hat{Q}_{\ell,2} = \frac{1}{N_\ell - 1} \sum_{i=1}^{N_\ell} \left( Q_\ell^{(i)} - \hat{Q}_\ell \right)^2
   \text{\quad  and \quad}
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
in `[eq: variance_est_ML_level_terms] <#eq: variance_est_ML_level_terms>`__.

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

   \label{EQ:mlmc_optimization_var}
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
:raw-latex:`\cite{Pisaroni2017}` and introduces a helper variable

.. math:: \hat{V}_{2, \ell} := \mathbb{V}ar\left[ \hat{Q}_{\ell,2} \right] \cdot N_{\ell}.

Next, the following constrained minimization problem is formulated

.. math::

   \label{EQ:mlmc_var_optimization_nobile}
    f(N_\ell,\lambda) = \sum_{\ell=0}^{L} N_\ell \, \mathcal{C}_{\ell} 
                      + \lambda \left( \sum_{\ell=0}^{L} N_\ell^{-1} \hat{V}_{2, \ell} - \varepsilon^2/2 \right),

and a closed form solution is obtained

.. math::

   \label{EQ: MLMC_nl_var_nobile}
   N_{\ell} = \frac{2}{\varepsilon^2} \left[ \, \sum_{k=0}^L \left( \hat{V}_{2, k} \mathcal{C}_k \right)^{1/2} \right] 
                  \sqrt{\frac{ \hat{V}_{2, \ell} }{\mathcal{C}_{\ell}}},

similarly as for the expected value
in `[EQ:mlmc_optimization] <#EQ:mlmc_optimization>`__.

The second approach uses numerical optimization directly on the
non-linear optimization
problem `[EQ:mlmc_optimization_var] <#EQ:mlmc_optimization_var>`__ to
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
estimate statistics of a metric defined as a linear combination of mean
and standard deviation of a QoI. A classical reliability measure can be
estimated, starting from ML statistics, as

.. math:: c^{ML}[Q] = \hat{Q}_{L}^{ML}  + \alpha \hat{\sigma}_L^{ML}.

In this case, in order to obtain the variance of :math:`c^{ML}[Q]` it is
necessary to employ an additional approximation:

.. math::

   \begin{split}
    \mathbb{V}ar\left[ c^{ML}[Q] \right] &= \mathbb{V}ar\left[ \hat{Q}_{L}^{ML} \right] + \alpha^2 \mathbb{V}ar\left[ \hat{\sigma}_L^{ML} \right] 
                                         + 2 \alpha \mathbb{C}ov\left[ \hat{Q}_{L}^{ML}, \hat{\sigma}_L^{ML} \right] \\
                                         &= \mathbb{V}ar\left[ \hat{Q}_{L}^{ML} \right] + \alpha^2 \mathbb{V}ar\left[ \hat{\sigma}_L^{ML} \right] 
                                         + 2 \alpha \rho\left[\hat{Q},\hat{\sigma}\right] \sqrt{ \mathbb{V}ar\left[ \hat{Q}_{L}^{ML} \right] }  \sqrt{ \mathbb{V}ar\left[ \hat{\sigma}_L^{ML} \right] } \\
                                         &\leq \mathbb{V}ar\left[ \hat{Q}_{L}^{ML} \right] + \alpha^2 \mathbb{V}ar\left[ \hat{\sigma}_L^{ML} \right] 
                                         + 2 |\alpha| \sqrt{ \mathbb{V}ar\left[ \hat{Q}_{L}^{ML} \right] }  \sqrt{ \mathbb{V}ar\left[ \hat{\sigma}_L^{ML} \right] },
   \end{split}

which permits to bound the maximum value for the variance (assuming a
very conservative approximation for the correlation between the
estimators for the mean and the standard deviation, *i.e.*
:math:`\left|\rho\left[\hat{Q},\hat{\sigma}\right]\right|=1`).

All terms in the previous expression can be written as a function of the
quantities derived in the previous sections, and, therefore, even for
this case the allocation problem can be solved by resorting to a
numerical optimization given a prescribed target.

.. _`uq:sampling:mlmf`:

A multilevel-multifidelity approach
-----------------------------------

The MLMC approach described in §\ `1.3 <#uq:sampling:multilevel>`__ can
be considered to be a recursive control variate technique in that it
seeks to reduce the variance of the target function in order to limit
the sampling at high resolution. In addition, the difference function
:math:`Y_\ell` for each level can itself be the target of an additional
control variate (refer to §\ `1.2 <#uq:sampling:controlvariate>`__). A
practical scenario is when not only different resolution levels are
available (multilevel part), but also a cheaper computational model can
be used (multifidelity part). The combined approach is a
multilevel-multifidelity algorithm, and in particular, a
multilevel-control variate Monte Carlo sampling approach.

.. _`uq:sampling:mlmf:Ycorr`:

:math:`Y_l` correlations
~~~~~~~~~~~~~~~~~~~~~~~~

If the target QoI can be generated from both a high-fidelity (HF) model
and a cheaper, possibly biased low-fidelity (LF) model, it is possible
to write the following estimator

.. math::

   \label{EQ: MLMF estimator}
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
:raw-latex:`\cite{GeraciCTR}`)

.. math::

   \label{EQ: MLMF mean}
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
:math:`\alpha_\ell = -\rho_\ell \sqrt{ \dfrac{ \mathbb{V}ar\left( Y^{\mathrm{HF}}_{\ell} \right) } 
                                { \mathbb{V}ar\left( Y^{\mathrm{LF}}_{\ell}  \right)     }}`.
By making use of the optimal coefficient :math:`\alpha_\ell`, it is
possible to show that the variance
:math:`\mathbb{V}ar\left(Y^{\mathrm{HF},\star}_{\ell}\right)` is
proportional to the variance
:math:`\mathbb{V}ar\left(Y^{\mathrm{HF}}_{\ell}\right)` through a factor
:math:`\Lambda_{\ell}(r_\ell)`, which is an explicit function of the
ratio :math:`r_\ell`:

.. math::

   \label{EQ: MLMF variance}
   \begin{split}
    \mathbb{V}ar\left(\hat{Q}_M^{MLMF}\right) &= \sum_{l=0}^{L_{\mathrm{HF}}} \dfrac{1}{N_{\ell}^{\mathrm{HF}}} \mathbb{V}ar\left(Y^{\mathrm{HF}}_{\ell}\right)
    \Lambda_{\ell}(r_\ell) \quad \mathrm{where} \\
    \Lambda_{\ell}(r_\ell) &= \left( 1 - \dfrac{r_\ell}{1+r_\ell}\rho_\ell^2 \right).
   \end{split}

Note that :math:`\Lambda_{\ell}(r_\ell)` represents a penalty with
respect to the classical control variate approach presented in
§\ `1.2 <#uq:sampling:controlvariate>`__, which stems from the need to
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
Eq. `[EQ:mlmc_optimization] <#EQ:mlmc_optimization>`__ is replaced by

.. math::

   \operatornamewithlimits{argmin}\limits_{N_{\ell}^{\mathrm{HF}}, r_\ell}(\mathcal{L}), \quad \mathrm{where} \quad \mathcal{L} = \sum_{\ell=0}^{L_{\mathrm{HF}}} N_{\ell}^{\mathrm{HF}} \mathcal{C}_{\ell}^{\mathrm{eq}} +
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

.. _`uq:sampling:mlmf:Ycorr`:

:math:`Q_l` correlations
~~~~~~~~~~~~~~~~~~~~~~~~

A potential refinement of the previous approach consists in exploiting
the QoI on each pair of levels, :math:`\ell` and :math:`\ell-1`, to
build a more correlated LF function. For instance, it is possible to use

.. math:: \mathring{Y}^{\mathrm{LF}}_{\ell} =  \gamma_\ell Q_\ell^{\mathrm{LF}} - Q_{\ell-1}^{\mathrm{LF}}

and maximize the correlation between :math:`Y_\ell^{\mathrm{HF}}` and
:math:`\mathring{Y}^{\mathrm{LF}}_{\ell}` through the coefficient
:math:`\gamma_\ell`.

Formally the two formulations are completely equivalent if
:math:`Y_\ell^{\mathrm{LF}}` is replaced with
:math:`\mathring{Y}^{\mathrm{LF}}_{\ell}` in
Equation `[EQ: MLMF estimator] <#EQ: MLMF estimator>`__ and they can be
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
