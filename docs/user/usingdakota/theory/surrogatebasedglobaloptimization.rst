.. _`uq:ego`:

Efficient Global Optimization
=============================

Efficient Global Optimization (EGO) was developed to facilitate the
unconstrained minimization of expensive implicit response functions. The
method builds an initial Gaussian process model as a global surrogate
for the response function, then intelligently selects additional samples
to be added for inclusion in a new Gaussian process model in subsequent
iterations. The new samples are selected based on how much they are
expected to improve the current best solution to the optimization
problem. When this expected improvement is acceptably small, the
globally optimal solution has been found. The application of this
methodology to equality-constrained reliability analysis is the primary
contribution of EGRA.

Efficient global optimization was originally proposed by Jones et
al. :cite:p:`Jon98` and has been adapted into similar methods
such as sequential kriging optimization (SKO) :cite:p:`Hua06`.
The main difference between SKO and EGO lies within the specific
formulation of what is known as the expected improvement function (EIF),
which is the feature that sets all EGO/SKO-type methods apart from other
global optimization methods. The EIF is used to select the location at
which a new training point should be added to the Gaussian process model
by maximizing the amount of improvement in the objective function that
can be expected by adding that point. A point could be expected to
produce an improvement in the objective function if its predicted value
is better than the current best solution, or if the uncertainty in its
prediction is such that the probability of it producing a better
solution is high. Because the uncertainty is higher in regions of the
design space with fewer observations, this provides a balance between
exploiting areas of the design space that predict good solutions, and
exploring areas where more information is needed.

The general procedure of these EGO-type methods is:

#. Build an initial Gaussian process model of the objective function.

#. Find the point that maximizes the EIF. If the EIF value at this point
   is sufficiently small, stop.

#. Evaluate the objective function at the point where the EIF is
   maximized. Update the Gaussian process model using this new point. Go
   to Step 2.

..
   TODO: Review comments:

   \item Use cross validation to ensure that the kriging model is satisfactory.

   To construct a parallel algorithm, the $n$ best points could be
   selected and evaluated in steps 2 and 3.

The following sections discuss the construction of the Gaussian process
model used, the form of the EIF, and then a description of how that EIF
is modified for application to reliability analysis.

.. _`uq:ego:gpm`:

Gaussian Process Model
----------------------

Gaussian process (GP) models are set apart from other surrogate models
because they provide not just a predicted value at an unsampled point,
but also an estimate of the prediction variance. This variance gives an
indication of the uncertainty in the GP model, which results from the
construction of the covariance function. This function is based on the
idea that when input points are near one another, the correlation
between their corresponding outputs will be high. As a result, the
uncertainty associated with the model’s predictions will be small for
input points which are near the points used to train the model, and will
increase as one moves further from the training points.

It is assumed that the true response function being modeled
:math:`G({\bf u})` can be described by: :cite:p:`Cre91`

.. math:: G({\bf u})={\bf h}({\bf u})^T{\boldsymbol \beta} + Z({\bf u})

where :math:`{\bf h}()` is the trend of the model,
:math:`{\boldsymbol \beta}` is the vector of trend coefficients, and
:math:`Z()` is a stationary Gaussian process with zero mean (and
covariance defined below) that describes the departure of the model from
its underlying trend. The trend of the model can be assumed to be any
function, but taking it to be a constant value has been reported to be
generally sufficient. :cite:p:`Sac89` For the work presented
here, the trend is assumed constant and :math:`{\boldsymbol \beta}` is
taken as simply the mean of the responses at the training points. The
covariance between outputs of the Gaussian process :math:`Z()` at points
:math:`{\bf a}` and :math:`{\bf b}` is defined as:

.. math::
   :label: eq:cov

   Cov \left[ Z({\bf a}),Z({\bf b}) \right] = \sigma_Z^2 R({\bf a},{\bf b})

where :math:`\sigma_Z^2` is the process variance and :math:`R()` is the
correlation function. There are several options for the correlation
function, but the squared-exponential function is
common :cite:p:`Sac89`, and is used here for :math:`R()`:

.. math:: R({\bf a},{\bf b}) = \exp \left[ -\sum_{i=1}^d \theta_i(a_i - b_i)^2 \right]

where :math:`d` represents the dimensionality of the problem (the number
of random variables), and :math:`\theta_i` is a scale parameter that
indicates the correlation between the points within dimension :math:`i`.
A large :math:`\theta_i` is representative of a short correlation
length.

The expected value :math:`\mu_G()` and variance :math:`\sigma_G^2()` of
the GP model prediction at point :math:`{\bf u}` are:

.. math::
   :label: eq:exp

   \begin{aligned}
   \mu_G({\bf u}) &= {\bf h}({\bf u})^T{\boldsymbol \beta} + 
     {\bf r}({\bf u})^T{\bf R}^{-1}({\bf g} - {\bf F}{\boldsymbol \beta})
   \end{aligned}

.. math::
   :label: eq:var

   \begin{aligned}
   \sigma_G^2({\bf u}) &= \sigma_Z^2 - 
     \begin{bmatrix} {\bf h}({\bf u})^T  & 
                     {\bf r}({\bf u})^T  \end{bmatrix}
     \begin{bmatrix} {\bf 0} & {\bf F}^T \\ 
                     {\bf F} & {\bf R}   \end{bmatrix}^{-1}
     \begin{bmatrix} {\bf h}({\bf u})    \\ 
                     {\bf r}({\bf u})    \end{bmatrix}
   \end{aligned}

where :math:`{\bf r}({\bf u})` is a vector containing the covariance
between :math:`{\bf u}` and each of the :math:`n` training points
(defined by Eq. :math:numref:`eq:cov`), :math:`{\bf R}` is an
:math:`n \times n` matrix containing the correlation between each pair
of training points, :math:`{\bf g}` is the vector of response outputs at
each of the training points, and :math:`{\bf F}` is an
:math:`n \times q` matrix with rows :math:`{\bf h}({\bf u}_i)^T` (the
trend function for training point :math:`i` containing :math:`q` terms;
for a constant trend :math:`q\!=\!1`). This form of the variance
accounts for the uncertainty in the trend coefficients
:math:`\boldsymbol \beta`, but assumes that the parameters governing the
covariance function (:math:`\sigma_Z^2` and :math:`\boldsymbol \theta`)
have known values.

The parameters :math:`\sigma_Z^2` and :math:`{\boldsymbol \theta}` are
determined through maximum likelihood estimation. This involves taking
the log of the probability of observing the response values
:math:`{\bf g}` given the covariance matrix :math:`{\bf R}`, which can
be written as: :cite:p:`Sac89`

.. math::
   :label: eq:like

   \log \left[ p({\bf g} | {\bf R}) \right] = 
     -\frac{1}{n} \log \lvert{\bf R}\rvert - \log(\hat{\sigma}_Z^2) 

where :math:`\lvert {\bf R} \rvert` indicates the determinant of
:math:`{\bf R}`, and :math:`\hat{\sigma}_Z^2` is the optimal value of
the variance given an estimate of :math:`\boldsymbol \theta` and is
defined by:

.. math::

   \hat{\sigma}_Z^2 = \frac{1}{n}({\bf g}-{\bf F}{\boldsymbol \beta})^T
     {\bf R}^{-1}({\bf g}-{\bf F}{\boldsymbol \beta})

..
   TODO: Review comment:

   where $\hat{\boldsymbol \beta}$ is the generalized least squares 
   estimate of $\boldsymbol \beta$ from:
   \begin{equation}
   \hat{\boldsymbol \beta} = \left[ {\bf F}^T{\bf R}^{-1}{\bf F} \right]^{-1}
     {\bf F}^T{\bf R}^{-1}{\bf g}
   \end{equation}

Maximizing Eq. :math:numref:`eq:like` gives the maximum likelihood
estimate of :math:`\boldsymbol \theta`, which in turn defines
:math:`\sigma_Z^2`.

.. _`uq:ego:acq`:

Acquisition Functions
---------------------

The acquisition function determines the location of the next sampling
point or refinement points, in the sense that maximizing the acquisition
function yields the next sampling point, as

.. math::
   {\bf u}^* = \underset{\bf u}{\text{argmax}}~a({\bf u}).

.. _`uq:ego:acq:eif`:

Expected Improvement
~~~~~~~~~~~~~~~~~~~~

The expected improvement function is used to select the location at
which a new training point should be added. The EIF is defined as the
expectation that any point in the search space will provide a better
solution than the current best solution based on the expected values and
variances predicted by the GP model. An important feature of the EIF is
that it provides a balance between exploiting areas of the design space
where good solutions have been found, and exploring areas of the design
space where the uncertainty is high. First, recognize that at any point
in the design space, the GP prediction :math:`\hat{G}()` is a Gaussian
distribution:

.. math:: \hat{G}({\bf u}) \sim \mathcal{N}\left( \mu_G({\bf u}), \sigma_G({\bf u}) \right)

where the mean :math:`\mu_G()` and the variance :math:`\sigma_G^2()`
were defined in Eqs. :math:numref:`eq:exp` and :math:numref:`eq:var`,
respectively. The EIF is defined as: :cite:p:`Jon98`

.. math::

   EI\bigl( \hat{G}({\bf u}) \bigr) \equiv 
     E\left[ \max \left( G({\bf u}^*) - \hat{G}({\bf u}),0 \right) \right]

where :math:`G({\bf u}^*)` is the current best solution chosen from
among the true function values at the training points (henceforth
referred to as simply :math:`G^*`). This expectation can then be
computed by integrating over the distribution :math:`\hat{G}({\bf u})`
with :math:`G^*` held constant:

.. math::
   :label: eq:eif_int

   EI\bigl( \hat{G}({\bf u}) \bigr) = 
     \int_{-\infty}^{G^*} \left( G^* - G \right) \, \hat{G}({\bf u}) \; dG  

where :math:`G` is a realization of :math:`\hat{G}`. This integral can
be expressed analytically as: :cite:p:`Jon98`

.. math::
   :label: eq:eif

   EI\bigl( \hat{G}({\bf u}) \bigr) = \left( G^* - \mu_G \right) \,
     \Phi\left( \frac{G^* - \mu_G}{\sigma_G} \right) + \sigma_G \,
     \phi\left( \frac{G^* - \mu_G}{\sigma_G} \right)

where it is understood that :math:`\mu_G` and :math:`\sigma_G` are
functions of :math:`{\bf u}`. Rewritting in a more compact manner and
dropping the subscript :math:`_G`,

.. math::
   :label: eq:eifShort

   a_\text{EI}({\bf u}, \{{\bf u}_i,y_i \}_{i=1}^N,\theta)) = \sigma({\bf u}) \cdot( \gamma({\bf u}) \Phi(\gamma({\bf u}) ) + \phi(\gamma({\bf u})) ),

where
:math:`\gamma({\bf u}) = \frac{G^* - \mu({\bf u})}{\sigma({\bf u})}`.
This equation defines the expected improvement acquisition function for
an unknown :math:`{\bf u}`.

The point at which the EIF is maximized is selected as an additional
training point. With the new training point added, a new GP model is
built and then used to construct another EIF, which is then used to
choose another new training point, and so on, until the value of the EIF
at its maximized point is below some specified tolerance. In
Ref. :cite:p:`Hua06` this maximization is performed using a
Nelder-Mead simplex approach, which is a local optimization method.
Because the EIF is often highly multimodal :cite:p:`Jon98` it
is expected that Nelder-Mead may fail to converge to the true global
optimum. In Ref. :cite:p:`Jon98`, a branch-and-bound technique
for maximizing the EIF is used, but was found to often be too expensive
to run to convergence. In Dakota, an implementation of the DIRECT global
optimization algorithm is used :cite:p:`Gab01`.

It is important to understand how the use of this EIF leads to optimal
solutions. Eq. :math:numref:`eq:eif` indicates how much the objective
function value at :math:`{\bf x}` is expected to be less than the
predicted value at the current best solution. Because the GP model
provides a Gaussian distribution at each predicted point, expectations
can be calculated. Points with good expected values and even a small
variance will have a significant expectation of producing a better
solution (exploitation), but so will points that have relatively poor
expected values and greater variance (exploration).

The application of EGO to reliability analysis, however, is made more
complicated due to the inclusion of equality constraints (see
Eqs. :math:numref:`eq:ria_opt`- :math:numref:`eq:pma_opt`). For
inverse reliability analysis, this extra complication is small. The
response being modeled by the GP is the objective function of the
optimization problem (see Eq. :math:numref:`eq:pma_opt`) and the
deterministic constraint might be handled through the use of a merit
function, thereby allowing EGO to solve this equality-constrained
optimization problem. Here the problem lies in the interpretation of the
constraint for multimodal problems as mentioned previously. In the
forward reliability case, the response function appears in the
constraint rather than the objective. Here, the maximization of the EIF
is inappropriate because feasibility is the main concern. This
application is therefore a significant departure from the original
objective of EGO and requires a new formulation. For this problem, the
expected feasibility function is introduced.

.. _`uq:ego:acq:pi`:

Probability Improvement Acquisition Function
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The probability of improvement (PI) acquisition function is proposed by
:cite:p:`kushner1964new`, using the same argument that the GP
prediction is a Gaussian distribution. Similar to Equation
:math:numref:`eq:eifShort`, the PI acquisition function is given
by

.. math:: a_{\text{PI}}({\bf u}) = \Phi(\gamma({\bf u})).

..
   TODO: where $\gamma({\bf u}) = \frac{G^* - \mu({\bf u})}{\sigma({\bf u})} $. 

Generally speaking, the EI acquisition function performs better than the
PI acquisition function.

.. _`uq:ego:acq:lcb`:

Lower-Confidence Bound Acquisition Function
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Another form of acquisition is lower-confidence bound (LCB), proposed
recently by Srinivas et al.
:cite:p:`srinivas2009gaussian,srinivas2012information`, which
has shown to perform very well. The LCB acquisition function takes the
form of

.. math:: a_{\text{LCB}}({\bf u}) = - \mu({\bf u}) + \kappa \sigma({\bf u}),

where :math:`\kappa` is a hyper-parameter describing the acquisition
exploitation-exploration balance. In many cases in design optimization,
:math:`\kappa = 2` is preferred, but relaxing this :math:`\kappa` as a
function of iterations is also possible, cf. Daniel et al.
:cite:p:`daniel2014active`, as

.. math:: \kappa = \sqrt{\nu \gamma_n},\quad \nu = 1, \quad \gamma_n = 2\log{\left(\frac{N^{d/2 + 2}\pi^2}{3\delta} \right)},

and :math:`d` is the dimensionality of the problem, and
:math:`\delta \in (0,1)` :cite:p:`srinivas2012information`.

Batch-sequential parallel
-------------------------

The batch-sequential parallelization is mainly motivated by exploiting
the computational resource, where multiple sampling points
:math:`{\bf u}` can be queried concurrently on a high-performance
computing platform. The benefit of batch implementation is that the
physical time to converge to the optimal solution is significantly
reduced with a factor of :math:`\sqrt{K}`, where :math:`K` is the batch
size. While there are many flavors of batch-sequential parallelization,
as well as asynchronous parallelization in EGO and Bayesian
optimization, we mainly review the theory of GP-BUCB by Desautels et al.
:cite:p:`desautels2014parallelizing`, GP-UCB-PE by Contal et
al :cite:p:`contal2013parallel`, and pBO-2GP-3B by Tran et al
:cite:p:`tran2019pbo`. The parallelization feature of EGO is
sometimes referred to as lookahead or non-myopic Bayesian optimization
in the literature, especially in the machine learning community.

The approach by Desautels et al.
:cite:p:`desautels2014parallelizing` mainly advocates for the
“hallucination” scheme or heuristic liar, in which the unknown
observation at the currently querying sampling point :math:`{\bf u}^*`
is *temporarily* assumed as the posterior mean :math:`\mu({\bf u}^*)`.
Then, the underlying GP model updates based on this assumption and
locates other points in the same batch, until the batch is filled. After
the whole batch is constructed, it is then queried, and all the
responses are received at once when the batch is completed. Contal et
al. :cite:p:`contal2013parallel` extended from the work of
Desautels et al. :cite:p:`desautels2014parallelizing` and
proved that including pure exploration (i.e. sampling at
:math:`{\bf u}^*` where :math:`\sigma({\bf u})` is maximum) increases
the efficiency. Tran et al. :cite:p:`tran2019pbo` adopted two
aforementioned approaches and extended for known and unknown
constraints.

The asynchronous batch parallel EGO is implemented based on the idea
of further leveraging computational efficiency when the computational
query cost varies widely.  In this scenario, the batch-sequential
parallel EGO finishes one iteration when the last worker of the batch
finishes.  This mechanism makes the other workers, which might have
finished the jobs or simulations earlier, wait for the last worker to
finish, thus creating an unnecessary idle period.  The asynchronous
batch parallel scheme is, therefore, created to accelerate the
optimization process by immediately assigning the next jobs to workers
that have finished earlier jobs, without waiting for each other.  When
workers finish one query, the objective GP is updated, and the next
sampling point is found by maximizing the acquisition function.
Numerical comparison results are shown in one of our previous works
:cite:p:`tran2022aphbo`, across a number of numerical functions and
some engineering simulations as well.
