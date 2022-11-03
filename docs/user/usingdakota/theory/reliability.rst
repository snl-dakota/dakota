.. _`theory:uq:reliability`:

Reliability Methods
===================

.. _`theory:uq:reliability:local`:

..
   TODO:
   This chapter explores local and global reliability methods in greater
   detail than that provided in the Uncertainty Quantification chapter of
   the User's Manual.

Local Reliability Methods
-------------------------

Local reliability methods include the Mean Value method and the family
of most probable point (MPP) search methods. Each of these methods is
gradient-based, employing local approximations and/or local optimization
methods.

.. _`theory:uq:reliability:local:mv`:

Mean Value
~~~~~~~~~~

The Mean Value method (MV, also known as MVFOSM in
:cite:p:`Hal00`) is the simplest, least-expensive reliability
method because it estimates the response means, response standard
deviations, and all CDF/CCDF response-probability-reliability levels
from a single evaluation of response functions and their gradients at
the uncertain variable means. This approximation can have acceptable
accuracy when the response functions are nearly linear and their
distributions are approximately Gaussian, but can have poor accuracy in
other situations.

The expressions for approximate response mean :math:`\mu_g` and
approximate response variance :math:`\sigma^2_g` are

.. math::

   \begin{aligned}
   \mu_g      & = & g(\mu_{\bf x})  \label{eq:mv_mean1} \\
   \sigma^2_g & = & \sum_i \sum_j Cov(i,j) \frac{dg}{dx_i}(\mu_{\bf x})
                    \frac{dg}{dx_j}(\mu_{\bf x}) \label{eq:mv_std_dev}\end{aligned}

where :math:`{\bf x}` are the uncertain values in the space of the
original uncertain variables (“x-space”), :math:`g({\bf x})` is the
limit state function (the response function for which
probability-response level pairs are needed), and the use of a linear
Taylor series approximation is evident. These two moments are then used
for mappings from response target to approximate reliability level
(:math:`\bar{z} \to \beta`) and from reliability target to approximate
response level (:math:`\bar{\beta} \to z`) using

.. math::

   \begin{aligned}
   \bar{z} \rightarrow \beta: & ~ & 
   \beta_{\rm CDF} = \frac{\mu_g - \bar{z}}{\sigma_g}, ~~~~~
   \beta_{\rm CCDF} = \frac{\bar{z} - \mu_g}{\sigma_g} \label{eq:mv_ria} \\
   \bar{\beta} \rightarrow z: & ~ & 
   z = \mu_g - \sigma_g \bar{\beta}_{\rm CDF}, ~~~~~
   z = \mu_g + \sigma_g \bar{\beta}_{\rm CCDF} \label{eq:mv_pma}\end{aligned}

respectively, where :math:`\beta_{\rm CDF}` and :math:`\beta_{\rm CCDF}`
are the reliability indices corresponding to the cumulative and
complementary cumulative distribution functions (CDF and CCDF),
respectively.

With the introduction of second-order limit state information, MVSOSM
calculates a second-order mean as

.. math::

   \mu_g = g(\mu_{\bf x}) + \frac{1}{2} \sum_i \sum_j Cov(i,j) 
   \frac{d^2g}{dx_i dx_j}(\mu_{\bf x}) \label{eq:mv_mean2}

This is commonly combined with a first-order variance
(Equation `[eq:mv_std_dev] <#eq:mv_std_dev>`__), since second-order
variance involves higher order distribution moments (skewness,
kurtosis) :cite:p:`Hal00` which are often unavailable.

The first-order CDF probability :math:`p(g \le z)`, first-order CCDF
probability :math:`p(g > z)`, :math:`\beta_{\rm CDF}`, and
:math:`\beta_{\rm CCDF}` are related to one another through

.. math::
   :label: eq:beta_cdf_ccdf

   \begin{aligned}
   p(g \le z)  & = & \Phi(-\beta_{\rm CDF})     \label{eq:p_cdf} \\
   p(g > z)    & = & \Phi(-\beta_{\rm CCDF})    \label{eq:p_ccdf} \\
   \beta_{\rm CDF}  & = & -\Phi^{-1}(p(g \le z)) \label{eq:beta_cdf} \\
   \beta_{\rm CCDF} & = & -\Phi^{-1}(p(g > z))   \label{eq:beta_ccdf} \\
   \beta_{\rm CDF}  & = & -\beta_{\rm CCDF}       \label{eq:beta_cdf_ccdf} \\
   p(g \le z)  & = & 1 - p(g > z)             \label{eq:p_cdf_ccdf}\end{aligned}

where :math:`\Phi()` is the standard normal cumulative distribution
function, indicating the introduction of a Gaussian assumption on the
output distributions. A common convention in the literature is to define
:math:`g` in such a way that the CDF probability for a response level
:math:`z` of zero (i.e., :math:`p(g \le 0)`) is the response metric of
interest. Dakota is not restricted to this convention and is designed to
support CDF or CCDF mappings for general response, probability, and
reliability level sequences.

With the Mean Value method, it is possible to obtain importance factors
indicating the relative contribution of the input variables to the
output variance. The importance factors can be viewed as an extension of
linear sensitivity analysis combining deterministic gradient information
with input uncertainty information, *i.e.* input variable standard
deviations. The accuracy of the importance factors is contingent of the
validity of the linear Taylor series approximation used to approximate
the response quantities of interest. The importance factors are
determined as follows for each of :math:`n` random variables:

..
   TODO:  where we require uncorrelated input variables: 

.. math::

   {\rm ImportFactor}_i = \left[ \frac{\sigma_{x_i}}{\sigma_g}
     \frac{dg}{dx_i}(\mu_{\bf x}) \right]^2, ~~~~ i = 1, \dots, n

where it is evident that these importance factors correspond to the
diagonal terms in Eq. `[eq:mv_std_dev] <#eq:mv_std_dev>`__ normalized by
the total response variance. In the case where the input variables are
correlated resulting in off-diagonal terms for the input covariance, we
can also compute a two-way importance factor as


..
   TODO: diagonal terms in Eq.~\ref{eq:mv_std_dev} normalized by the total
   response variance.  %involve an attribution of the total response
   %variance among the set of input variables.

.. math::

   {\rm ImportFactor}_{ij} = 2 \frac{\sigma^2_{x_{ij}}}{\sigma^2_g} 
     \frac{dg}{dx_i}(\mu_{\bf x}) \frac{dg}{dx_j}(\mu_{\bf x}),
     ~~~~ i = 1, \dots, n; ~~~~ j = 1, \dots, i-1

These two-way factors differ from the Sobol’ interaction terms that are
computed in variance-based decomposition (refer to
Section `[theory:uq:expansion:vbd] <#theory:uq:expansion:vbd>`__) due to the
non-orthogonality of the Taylor series basis. Due to this
non-orthogonality, two-way importance factors may be negative, and due
to normalization by the total response variance, the set of importance
factors will always sum to one.

.. _`theory:uq:reliability:local:mpp`:

MPP Search Methods
~~~~~~~~~~~~~~~~~~

All other local reliability methods solve an equality-constrained
nonlinear optimization problem to compute a most probable point (MPP)
and then integrate about this point to compute probabilities. The MPP
search is performed in uncorrelated standard normal space (“u-space”)
since it simplifies the probability integration: the distance of the MPP
from the origin has the meaning of the number of input standard
deviations separating the mean response from a particular response
threshold. The transformation from correlated non-normal distributions
(x-space) to uncorrelated standard normal distributions (u-space) is
denoted as :math:`{\bf u} = T({\bf x})` with the reverse transformation
denoted as :math:`{\bf x} = T^{-1}({\bf u})`. These transformations are
nonlinear in general, and possible approaches include the
Rosenblatt :cite:p:`Ros52`, Nataf :cite:p:`Der86`,
and Box-Cox :cite:p:`Box64` transformations. The nonlinear
transformations may also be linearized, and common approaches for this
include the Rackwitz-Fiessler :cite:p:`Rac78` two-parameter
equivalent normal and the Chen-Lind :cite:p:`Che83` and
Wu-Wirsching :cite:p:`Wu87` three-parameter equivalent
normals. Dakota employs the Nataf nonlinear transformation which is
suitable for the common case when marginal distributions and a
correlation matrix are provided, but full joint distributions are not
known [1]_. This transformation occurs in the following two steps. To
transform between the original correlated x-space variables and
correlated standard normals (“z-space”), a CDF matching condition is
applied for each of the marginal distributions:

.. math:: \Phi(z_i) = F(x_i) \label{eq:trans_zx}

where :math:`F()` is the cumulative distribution function of the
original probability distribution. Then, to transform between correlated
z-space variables and uncorrelated u-space variables, the Cholesky
factor :math:`{\bf L}` of a modified correlation matrix is used:

.. math:: {\bf z} = {\bf L} {\bf u} \label{eq:trans_zu}

where the original correlation matrix for non-normals in x-space has
been modified to represent the corresponding “warped” correlation in
z-space :cite:p:`Der86`.

The forward reliability analysis algorithm of computing CDF/CCDF
probability/reliability levels for specified response levels is called
the reliability index approach (RIA), and the inverse reliability
analysis algorithm of computing response levels for specified CDF/CCDF
probability/reliability levels is called the performance measure
approach (PMA) :cite:p:`Tu99`. The differences between the RIA
and PMA formulations appear in the objective function and equality
constraint formulations used in the MPP searches. For RIA, the MPP
search for achieving the specified response level :math:`\bar{z}` is
formulated as computing the minimum distance in u-space from the origin
to the :math:`\bar{z}` contour of the limit state response function:

.. math::
   :label: eq:ria_opt

   \begin{aligned}
   {\rm minimize}     & {\bf u}^T {\bf u} \nonumber \\
   {\rm subject \ to} & G({\bf u}) = \bar{z}
   \end{aligned}

where :math:`{\bf u}` is a vector centered at the origin in u-space and
:math:`g({\bf x}) \equiv G({\bf u})` by definition. For PMA, the MPP
search for achieving the specified reliability level :math:`\bar{\beta}`
or first-order probability level :math:`\bar{p}` is formulated as
computing the minimum/maximum response function value corresponding to a
prescribed distance from the origin in u-space:

.. math::
   :label: eq:pma_opt

   \begin{aligned}
   {\rm minimize}     & \pm G({\bf u}) \nonumber \\
   {\rm subject \ to} & {\bf u}^T {\bf u} = \bar{\beta}^2
   \end{aligned}

where :math:`\bar{\beta}` is computed from :math:`\bar{p}` using
Eq. `[eq:beta_cdf] <#eq:beta_cdf>`__
or `[eq:beta_ccdf] <#eq:beta_ccdf>`__ in the latter case of a prescribed
first-order probability level. For a specified generalized reliability
level :math:`\bar{\beta^*}` or second-order probability level
:math:`\bar{p}`, the equality constraint is reformulated in terms of the
generalized reliability index:

.. math::

   \begin{aligned}
   {\rm minimize}     & \pm G({\bf u}) \nonumber \\
   {\rm subject \ to} & \beta^*({\bf u}) = \bar{\beta^*} \label{eq:pma2_opt}\end{aligned}

where :math:`\bar{\beta^*}` is computed from :math:`\bar{p}` using
Eq. `[eq:gen_beta] <#eq:gen_beta>`__ (or its CCDF complement) in the
latter case of a prescribed second-order probability level.

In the RIA case, the optimal MPP solution :math:`{\bf u}^*` defines the
reliability index from :math:`\beta = \pm \|{\bf u}^*\|_2`, which in
turn defines the CDF/CCDF probabilities (using
Equations `[eq:p_cdf] <#eq:p_cdf>`__-`[eq:p_ccdf] <#eq:p_ccdf>`__ in the
case of first-order integration). The sign of :math:`\beta` is defined
by

.. math::

   \begin{aligned}
   G({\bf u}^*) > G({\bf 0}): \beta_{\rm CDF} < 0, \beta_{\rm CCDF} > 0 \\
   G({\bf u}^*) < G({\bf 0}): \beta_{\rm CDF} > 0, \beta_{\rm CCDF} < 0\end{aligned}

where :math:`G({\bf 0})` is the median limit state response computed at
the origin in u-space [2]_ (where :math:`\beta_{\rm CDF}` =
:math:`\beta_{\rm CCDF}` = 0 and first-order :math:`p(g \le z)` =
:math:`p(g > z)` = 0.5). In the PMA case, the sign applied to
:math:`G({\bf u})` (equivalent to minimizing or maximizing
:math:`G({\bf u})`) is similarly defined by either :math:`\bar{\beta}`
(for a specified reliability or first-order probability level) or from a
:math:`\bar{\beta}` estimate [3]_ computed from :math:`\bar{\beta^*}`
(for a specified generalized reliability or second-order probability
level)

.. math::

   \begin{aligned}
   \bar{\beta}_{\rm CDF} < 0, \bar{\beta}_{\rm CCDF} > 0: {\rm maximize \ } G({\bf u}) \\
   \bar{\beta}_{\rm CDF} > 0, \bar{\beta}_{\rm CCDF} < 0: {\rm minimize \ } G({\bf u})\end{aligned}

where the limit state at the MPP (:math:`G({\bf u}^*)`) defines the
desired response level result.

.. _`theory:uq:reliability:local:mpp:approx`:

Limit state approximations
^^^^^^^^^^^^^^^^^^^^^^^^^^

There are a variety of algorithmic variations that are available for use
within RIA/PMA reliability analyses. First, one may select among several
different limit state approximations that can be used to reduce
computational expense during the MPP searches. Local, multipoint, and
global approximations of the limit state are possible.
:cite:p:`Eld05` investigated local first-order limit state
approximations, and :cite:p:`Eld06a` investigated local
second-order and multipoint approximations. These techniques include:

#. a single Taylor series per response/reliability/probability level in
   x-space centered at the uncertain variable means. The first-order
   approach is commonly known as the Advanced Mean Value (AMV) method:

   .. math::

      g({\bf x}) \cong g(\mu_{\bf x}) + \nabla_x g(\mu_{\bf x})^T 
      ({\bf x} - \mu_{\bf x}) \label{eq:linear_x_mean}

   and the second-order approach has been named AMV\ :math:`^2`:

   .. math::

      g({\bf x}) \cong g(\mu_{\bf x}) + \nabla_{\bf x} g(\mu_{\bf x})^T 
      ({\bf x} - \mu_{\bf x}) + \frac{1}{2} ({\bf x} - \mu_{\bf x})^T 
      \nabla^2_{\bf x} g(\mu_{\bf x}) ({\bf x} - \mu_{\bf x})
      \label{eq:taylor2_x_mean}

#. same as AMV/AMV\ :math:`^2`, except that the Taylor series is
   expanded in u-space. The first-order option has been termed the
   u-space AMV method:

   .. math::

      G({\bf u}) \cong G(\mu_{\bf u}) + \nabla_u G(\mu_{\bf u})^T 
      ({\bf u} - \mu_{\bf u}) \label{eq:linear_u_mean}

   where :math:`\mu_{\bf u} = T(\mu_{\bf x})` and is nonzero in general,
   and the second-order option has been named the u-space
   AMV\ :math:`^2` method:

   .. math::

      G({\bf u}) \cong G(\mu_{\bf u}) + \nabla_{\bf u} G(\mu_{\bf u})^T 
      ({\bf u} - \mu_{\bf u}) + \frac{1}{2} ({\bf u} - \mu_{\bf u})^T 
      \nabla^2_{\bf u} G(\mu_{\bf u}) ({\bf u} - \mu_{\bf u}) 
      \label{eq:taylor2_u_mean}

#. an initial Taylor series approximation in x-space at the uncertain
   variable means, with iterative expansion updates at each MPP estimate
   (:math:`{\bf x}^*`) until the MPP converges. The first-order option
   is commonly known as AMV+:

   .. math::

      g({\bf x}) \cong g({\bf x}^*) + \nabla_x g({\bf x}^*)^T ({\bf x} - {\bf x}^*)
      \label{eq:linear_x_mpp}

   and the second-order option has been named AMV\ :math:`^2`\ +:

   .. math::

      g({\bf x}) \cong g({\bf x}^*) + \nabla_{\bf x} g({\bf x}^*)^T 
      ({\bf x} - {\bf x}^*) + \frac{1}{2} ({\bf x} - {\bf x}^*)^T 
      \nabla^2_{\bf x} g({\bf x}^*) ({\bf x} - {\bf x}^*) \label{eq:taylor2_x_mpp}

#. same as AMV+/AMV\ :math:`^2`\ +, except that the expansions are
   performed in u-space. The first-order option has been termed the
   u-space AMV+ method.

   .. math::

      G({\bf u}) \cong G({\bf u}^*) + \nabla_u G({\bf u}^*)^T ({\bf u} - {\bf u}^*)
      \label{eq:linear_u_mpp}

   and the second-order option has been named the u-space
   AMV\ :math:`^2`\ + method:

   .. math::

      G({\bf u}) \cong G({\bf u}^*) + \nabla_{\bf u} G({\bf u}^*)^T 
      ({\bf u} - {\bf u}^*) + \frac{1}{2} ({\bf u} - {\bf u}^*)^T 
      \nabla^2_{\bf u} G({\bf u}^*) ({\bf u} - {\bf u}^*) \label{eq:taylor2_u_mpp}

#. a multipoint approximation in x-space. This approach involves a
   Taylor series approximation in intermediate variables where the
   powers used for the intermediate variables are selected to match
   information at the current and previous expansion points. Based on
   the two-point exponential approximation concept (TPEA,
   :cite:p:`Fad90`), the two-point adaptive nonlinearity
   approximation (TANA-3, :cite:p:`Xu98`) approximates the
   limit state as:

   .. math::

      g({\bf x}) \cong g({\bf x}_2) + \sum_{i=1}^n 
      \frac{\partial g}{\partial x_i}({\bf x}_2) \frac{x_{i,2}^{1-p_i}}{p_i} 
      (x_i^{p_i} - x_{i,2}^{p_i}) + \frac{1}{2} \epsilon({\bf x}) \sum_{i=1}^n 
      (x_i^{p_i} - x_{i,2}^{p_i})^2 \label{eq:tana_g}

   where :math:`n` is the number of uncertain variables and:

   .. math::

      \begin{aligned}
      p_i & = & 1 + \ln \left[ \frac{\frac{\partial g}{\partial x_i}({\bf x}_1)}
      {\frac{\partial g}{\partial x_i}({\bf x}_2)} \right] \left/ 
      \ln \left[ \frac{x_{i,1}}{x_{i,2}} \right] \right. \label{eq:tana_pi_x} \\
      \epsilon({\bf x}) & = & \frac{H}{\sum_{i=1}^n (x_i^{p_i} - x_{i,1}^{p_i})^2 + 
      \sum_{i=1}^n (x_i^{p_i} - x_{i,2}^{p_i})^2} \label{eq:tana_eps_x} \\
      H & = & 2 \left[ g({\bf x}_1) - g({\bf x}_2) - \sum_{i=1}^n 
      \frac{\partial g}{\partial x_i}({\bf x}_2) \frac{x_{i,2}^{1-p_i}}{p_i} 
      (x_{i,1}^{p_i} - x_{i,2}^{p_i}) \right] \label{eq:tana_H_x}\end{aligned}

   and :math:`{\bf x}_2` and :math:`{\bf x}_1` are the current and
   previous MPP estimates in x-space, respectively. Prior to the
   availability of two MPP estimates, x-space AMV+ is used.

#. a multipoint approximation in u-space. The u-space TANA-3
   approximates the limit state as:

   .. math::

      G({\bf u}) \cong G({\bf u}_2) + \sum_{i=1}^n 
      \frac{\partial G}{\partial u_i}({\bf u}_2) \frac{u_{i,2}^{1-p_i}}{p_i} 
      (u_i^{p_i} - u_{i,2}^{p_i}) + \frac{1}{2} \epsilon({\bf u}) \sum_{i=1}^n 
      (u_i^{p_i} - u_{i,2}^{p_i})^2 \label{eq:tana_G}

   where:

   .. math::

      \begin{aligned}
      p_i & = & 1 + \ln \left[ \frac{\frac{\partial G}{\partial u_i}({\bf u}_1)}
      {\frac{\partial G}{\partial u_i}({\bf u}_2)} \right] \left/ 
      \ln \left[ \frac{u_{i,1}}{u_{i,2}} \right] \right. \label{eq:tana_pi_u} \\
      \epsilon({\bf u}) & = & \frac{H}{\sum_{i=1}^n (u_i^{p_i} - u_{i,1}^{p_i})^2 + 
      \sum_{i=1}^n (u_i^{p_i} - u_{i,2}^{p_i})^2} \label{eq:tana_eps_u} \\
      H & = & 2 \left[ G({\bf u}_1) - G({\bf u}_2) - \sum_{i=1}^n 
      \frac{\partial G}{\partial u_i}({\bf u}_2) \frac{u_{i,2}^{1-p_i}}{p_i} 
      (u_{i,1}^{p_i} - u_{i,2}^{p_i}) \right] \label{eq:tana_H_u}\end{aligned}

   and :math:`{\bf u}_2` and :math:`{\bf u}_1` are the current and
   previous MPP estimates in u-space, respectively. Prior to the
   availability of two MPP estimates, u-space AMV+ is used.

#. the MPP search on the original response functions without the use of
   any approximations. Combining this option with first-order and
   second-order integration approaches (see next section) results in the
   traditional first-order and second-order reliability methods (FORM
   and SORM).

The Hessian matrices in AMV\ :math:`^2` and AMV\ :math:`^2`\ + may be
available analytically, estimated numerically, or approximated through
quasi-Newton updates. The selection between x-space or u-space for
performing approximations depends on where the approximation will be
more accurate, since this will result in more accurate MPP estimates
(AMV, AMV\ :math:`^2`) or faster convergence (AMV+, AMV\ :math:`^2`\ +,
TANA). Since this relative accuracy depends on the forms of the limit
state :math:`g(x)` and the transformation :math:`T(x)` and is therefore
application dependent in general, Dakota supports both options. A
concern with approximation-based iterative search methods (i.e., AMV+,
AMV\ :math:`^2`\ + and TANA) is the robustness of their convergence to
the MPP. It is possible for the MPP iterates to oscillate or even
diverge. However, to date, this occurrence has been relatively rare, and
Dakota contains checks that monitor for this behavior. Another concern
with TANA is numerical safeguarding (e.g., the possibility of raising
negative :math:`x_i` or :math:`u_i` values to nonintegral :math:`p_i`
exponents in Equations `[eq:tana_g] <#eq:tana_g>`__,
`[eq:tana_eps_x] <#eq:tana_eps_x>`__-`[eq:tana_G] <#eq:tana_G>`__,
and `[eq:tana_eps_u] <#eq:tana_eps_u>`__-`[eq:tana_H_u] <#eq:tana_H_u>`__).
Safeguarding involves offseting negative :math:`x_i` or :math:`u_i` and,
for potential numerical difficulties with the logarithm ratios in
Equations `[eq:tana_pi_x] <#eq:tana_pi_x>`__
and `[eq:tana_pi_u] <#eq:tana_pi_u>`__, reverting to either the linear
(:math:`p_i = 1`) or reciprocal (:math:`p_i = -1`) approximation based
on which approximation has lower error in
:math:`\frac{\partial g}{\partial x_i}({\bf x}_1)` or
:math:`\frac{\partial G}{\partial u_i}({\bf u}_1)`.

.. _`theory:uq:reliability:local:mpp:int`:

Probability integrations
^^^^^^^^^^^^^^^^^^^^^^^^

The second algorithmic variation involves the integration approach for
computing probabilities at the MPP, which can be selected to be
first-order
(Equations `[eq:p_cdf] <#eq:p_cdf>`__-`[eq:p_ccdf] <#eq:p_ccdf>`__) or
second-order integration. Second-order integration involves applying a
curvature correction :cite:p:`Bre84,Hoh88,Hon99`. Breitung
applies a correction based on asymptotic
analysis :cite:p:`Bre84`:

.. math::

   p = \Phi(-\beta_p) \prod_{i=1}^{n-1} \frac{1}{\sqrt{1 + \beta_p \kappa_i}}
   \label{eq:p_2nd_breit}

where :math:`\kappa_i` are the principal curvatures of the limit state
function (the eigenvalues of an orthonormal transformation of
:math:`\nabla^2_{\bf u} G`, taken positive for a convex limit state) and
:math:`\beta_p \ge 0` (a CDF or CCDF probability correction is selected
to obtain the correct sign for :math:`\beta_p`). An alternate correction
in :cite:p:`Hoh88` is consistent in the asymptotic regime
(:math:`\beta_p \to \infty`) but does not collapse to first-order
integration for :math:`\beta_p = 0`:

.. math::

   p = \Phi(-\beta_p) \prod_{i=1}^{n-1} 
   \frac{1}{\sqrt{1 + \psi(-\beta_p) \kappa_i}} \label{eq:p_2nd_hr}

where :math:`\psi() = \frac{\phi()}{\Phi()}` and :math:`\phi()` is the
standard normal density function. :cite:p:`Hon99` applies
further corrections to Equation `[eq:p_2nd_hr] <#eq:p_2nd_hr>`__ based
on point concentration methods. At this time, all three approaches are
available within the code, but the Hohenbichler-Rackwitz correction is
used by default (switching the correction is a compile-time option in
the source code and has not been exposed in the input specification).

.. _`sec:hessian`:

Hessian approximations
^^^^^^^^^^^^^^^^^^^^^^

To use a second-order Taylor series or a second-order integration when
second-order information (:math:`\nabla^2_{\bf x} g`,
:math:`\nabla^2_{\bf u} G`, and/or :math:`\kappa`) is not directly
available, one can estimate the missing information using finite
differences or approximate it through use of quasi-Newton
approximations. These procedures will often be needed to make
second-order approaches practical for engineering applications.

In the finite difference case, numerical Hessians are commonly computed
using either first-order forward differences of gradients using

.. math::

   \nabla^2 g ({\bf x}) \cong 
   \frac{\nabla g ({\bf x} + h {\bf e}_i) - \nabla g ({\bf x})}{h}

to estimate the :math:`i^{th}` Hessian column when gradients are
analytically available, or second-order differences of function values
using

.. math::

   \begin{array}{l}
   \nabla^2 g ({\bf x}) \cong \frac{g({\bf x} + h {\bf e}_i + h {\bf e}_j) - 
   g({\bf x} + h {\bf e}_i - h {\bf e}_j) - 
   g({\bf x} - h {\bf e}_i + h {\bf e}_j) + 
   g({\bf x} - h {\bf e}_i - h {\bf e}_j)}{4h^2}
   \end{array}

to estimate the :math:`ij^{th}` Hessian term when gradients are not
directly available. This approach has the advantage of locally-accurate
Hessians for each point of interest (which can lead to quadratic
convergence rates in discrete Newton methods), but has the disadvantage
that numerically estimating each of the matrix terms can be expensive.

Quasi-Newton approximations, on the other hand, do not reevaluate all of
the second-order information for every point of interest. Rather, they
accumulate approximate curvature information over time using secant
updates. Since they utilize the existing gradient evaluations, they do
not require any additional function evaluations for evaluating the
Hessian terms. The quasi-Newton approximations of interest include the
Broyden-Fletcher-Goldfarb-Shanno (BFGS) update

.. math::

   {\bf B}_{k+1} = {\bf B}_{k} - \frac{{\bf B}_k {\bf s}_k {\bf s}_k^T {\bf B}_k}
   {{\bf s}_k^T {\bf B}_k {\bf s}_k} + 
   \frac{{\bf y}_k {\bf y}_k^T}{{\bf y}_k^T {\bf s}_k} \label{eq:bfgs}

which yields a sequence of symmetric positive definite Hessian
approximations, and the Symmetric Rank 1 (SR1) update

.. math::

   {\bf B}_{k+1} = {\bf B}_{k} + 
   \frac{({\bf y}_k - {\bf B}_k {\bf s}_k)({\bf y}_k - {\bf B}_k {\bf s}_k)^T}
   {({\bf y}_k - {\bf B}_k {\bf s}_k)^T {\bf s}_k} \label{eq:sr1}

which yields a sequence of symmetric, potentially indefinite, Hessian
approximations. :math:`{\bf B}_k` is the :math:`k^{th}` approximation to
the Hessian :math:`\nabla^2 g`,
:math:`{\bf s}_k = {\bf x}_{k+1} - {\bf x}_k` is the step and
:math:`{\bf y}_k = \nabla g_{k+1} - \nabla g_k` is the corresponding
yield in the gradients. The selection of BFGS versus SR1 involves the
importance of retaining positive definiteness in the Hessian
approximations; if the procedure does not require it, then the SR1
update can be more accurate if the true Hessian is not positive
definite. Initial scalings for :math:`{\bf B}_0` and numerical
safeguarding techniques (damped BFGS, update skipping) are described in
:cite:p:`Eld06a`.

Optimization algorithms
^^^^^^^^^^^^^^^^^^^^^^^

The next algorithmic variation involves the optimization algorithm
selection for solving Eqs. :math:numref:`eq:ria_opt`
and :math:numref:`eq:pma_opt`. The Hasofer-Lind Rackwitz-Fissler
(HL-RF) algorithm :cite:p:`Hal00` is a classical approach that
has been broadly applied. It is a Newton-based approach lacking line
search/trust region globalization, and is generally regarded as
computationally efficient but occasionally unreliable. Dakota takes the
approach of employing robust, general-purpose optimization algorithms
with provable convergence properties. In particular, we employ the
sequential quadratic programming (SQP) and nonlinear interior-point
(NIP) optimization algorithms from the NPSOL :cite:p:`Gil86`
and OPT++ :cite:p:`MeOlHoWi07` libraries, respectively.

Warm Starting of MPP Searches
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The final algorithmic variation for local reliability methods involves
the use of warm starting approaches for improving computational
efficiency. :cite:p:`Eld05` describes the acceleration of MPP
searches through warm starting with approximate iteration increment,
with :math:`z/p/\beta` level increment, and with design variable
increment. Warm started data includes the expansion point and associated
response values and the MPP optimizer initial guess. Projections are
used when an increment in :math:`z/p/\beta` level or design variables
occurs. Warm starts were consistently effective in
:cite:p:`Eld05`, with greater effectiveness for smaller
parameter changes, and are used by default in Dakota.

.. _`theory:uq:reliability:global`:

Global Reliability Methods
--------------------------

Local reliability methods, while computationally efficient, have
well-known failure mechanisms. When confronted with a limit state
function that is nonsmooth, local gradient-based optimizers may stall
due to gradient inaccuracy and fail to converge to an MPP. Moreover, if
the limit state is multimodal (multiple MPPs), then a gradient-based
local method can, at best, locate only one local MPP solution. Finally,
a linear (Eqs. `[eq:p_cdf] <#eq:p_cdf>`__–`[eq:p_ccdf] <#eq:p_ccdf>`__)
or parabolic
(Eqs. `[eq:p_2nd_breit] <#eq:p_2nd_breit>`__–`[eq:p_2nd_hr] <#eq:p_2nd_hr>`__)
approximation to the limit state at this MPP may fail to adequately
capture the contour of a highly nonlinear limit state.

..
   TODO:

   For these reasons,
   efficient global reliability analysis (EGRA) is investigated
   in~\cite{bichon_egra_sdm}.
   
   Global reliability methods include the efficient global reliability
   analysis (EGRA) method. Analytical methods of reliability analysis solve a 
   local optimization problem to locate the most probable point of failure (MPP), 
   and then quantify the reliability based on its location and an approximation 
   to the shape of the limit state at this point. Typically, gradient-based 
   solvers are used to solve this optimization problem, which may fail to 
   converge for nonsmooth response functions with unreliable gradients or 
   may converge to only one of several solutions for response functions that 
   possess multiple local optima. In addition to these MPP convergence issues, 
   the evaluated probabilites can be adversely affected by limit state 
   approximations that may be inaccurate. Analysts are then forced
   to revert to sampling methods, which do not rely on MPP convergence or 
   simplifying approximations to the true shape of the limit state. 
   However, employing such methods is impractical when evaluation of the 
   response function is expensive.

A reliability analysis method that is both efficient when applied to
expensive response functions and accurate for a response function of any
arbitrary shape is needed. This section develops such a method based on
efficient global optimization :cite:p:`Jon98` (EGO) to the
search for multiple points on or near the limit state throughout the
random variable space. By locating multiple points on the limit state,
more complex limit states can be accurately modeled, resulting in a more
accurate assessment of the reliability. It should be emphasized here
that these multiple points exist on a single limit state. Because of its
roots in efficient global optimization, this method of reliability
analysis is called efficient global reliability analysis
(EGRA) :cite:p:`Bichon2007`. The following two subsections
describe two capabilities that are incorporated into the EGRA algorithm:
importance sampling and EGO.

.. _`theory:uq:reliability:global:ais`:

Importance Sampling
~~~~~~~~~~~~~~~~~~~

An alternative to MPP search methods is to directly perform the
probability integration numerically by sampling the response function.
Sampling methods do not rely on a simplifying approximation to the shape
of the limit state, so they can be more accurate than FORM and SORM, but
they can also be prohibitively expensive because they generally require
a large number of response function evaluations. Importance sampling
methods reduce this expense by focusing the samples in the important
regions of the uncertain space. They do this by centering the sampling
density function at the MPP rather than at the mean. This ensures the
samples will lie the region of interest, thus increasing the efficiency
of the sampling method. Adaptive importance sampling (AIS) further
improves the efficiency by adaptively updating the sampling density
function. Multimodal adaptive importance
sampling :cite:p:`Dey98,Zou02` is a variation of AIS that
allows for the use of multiple sampling densities making it better
suited for cases where multiple sections of the limit state are highly
probable.

Note that importance sampling methods require that the location of at
least one MPP be known because it is used to center the initial sampling
density. However, current gradient-based, local search methods used in
MPP search may fail to converge or may converge to poor solutions for
highly nonlinear problems, possibly making these methods inapplicable.
As the next section describes, EGO is a global optimization method that
does not depend on the availability of accurate gradient information,
making convergence more reliable for nonsmooth response functions.
Moreover, EGO has the ability to locate multiple failure points, which
would provide multiple starting points and thus a good multimodal
sampling density for the initial steps of multimodal AIS. The resulting
Gaussian process model is accurate in the vicinity of the limit state,
thereby providing an inexpensive surrogate that can be used to provide
response function samples. As will be seen, using EGO to locate multiple
points along the limit state, and then using the resulting Gaussian
process model to provide function evaluations in multimodal AIS for the
probability integration, results in an accurate and efficient
reliability analysis tool.

.. _`theory:uq:reliability:global:ego`:

Efficient Global Optimization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Chapter `[uq:ego] <#uq:ego>`__ is now rewritten to support EGO/Bayesian
optimization theory.

.. _`theory:uq:reliability:global:ego:eff`:

Expected Feasibility Function
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The expected improvement function provides an indication of how much the
true value of the response at a point can be expected to be less than
the current best solution. It therefore makes little sense to apply this
to the forward reliability problem where the goal is not to minimize the
response, but rather to find where it is equal to a specified threshold
value. The expected feasibility function (EFF) is introduced here to
provide an indication of how well the true value of the response is
expected to satisfy the equality constraint
:math:`G({\bf u})\!=\!\bar{z}`. Inspired by the contour estimation work
in :cite:p:`Ran08`, this expectation can be calculated in a
similar fashion as Eq. `[eq:eif_int] <#eq:eif_int>`__ by integrating
over a region in the immediate vicinity of the threshold value
:math:`\bar{z}\pm\epsilon`:

.. math::

   EF\bigl( \hat{G}({\bf u}) \bigr) = 
     \int_{z-\epsilon}^{z+\epsilon} 
       \bigl[ \epsilon - | \bar{z}-G | \bigr] \, \hat{G}({\bf u}) \; dG

where :math:`G` denotes a realization of the distribution
:math:`\hat{G}`, as before. Allowing :math:`z^+` and :math:`z^-` to
denote :math:`\bar{z}\pm\epsilon`, respectively, this integral can be
expressed analytically as:

.. math::

   \begin{aligned}
   EF\bigl( \hat{G}({\bf u}) \bigr) &= \left( \mu_G - \bar{z} \right)
              \left[ 2 \, \Phi\left( \frac{\bar{z} - \mu_G}{\sigma_G} \right) -
                          \Phi\left( \frac{  z^-   - \mu_G}{\sigma_G} \right) -
                          \Phi\left( \frac{  z^+   - \mu_G}{\sigma_G} \right) 
             \right] \notag \\ & \ \ \ \ \ \ \ \ - 
     \sigma_G \left[ 2 \, \phi\left( \frac{\bar{z} - \mu_G}{\sigma_G} \right) \, -
                          \phi\left( \frac{  z^-   - \mu_G}{\sigma_G} \right) \, -
                          \phi\left( \frac{  z^+   - \mu_G}{\sigma_G} \right) 
             \right] \notag \\ & \ \ \ \ \ \ \ \ + \ \ \,
     \epsilon \left[      \Phi\left( \frac{  z^+   - \mu_G}{\sigma_G} \right) -
                          \Phi\left( \frac{  z^-   - \mu_G}{\sigma_G} \right)
             \right] \label{eq:eff}\end{aligned}

where :math:`\epsilon` is proportional to the standard deviation of the
GP predictor (:math:`\epsilon\propto\sigma_G`). In this case,
:math:`z^-`, :math:`z^+`, :math:`\mu_G`, :math:`\sigma_G`, and
:math:`\epsilon` are all functions of the location :math:`{\bf u}`,
while :math:`\bar{z}` is a constant. Note that the EFF provides the same
balance between exploration and exploitation as is captured in the EIF.
Points where the expected value is close to the threshold
(:math:`\mu_G\!\approx\!\bar{z}`) and points with a large uncertainty in
the prediction will have large expected feasibility values.

.. [1]
   If joint distributions are known, then the Rosenblatt transformation
   is preferred.

.. [2]
   It is not necessary to explicitly compute the median response since
   the sign of the inner product
   :math:`\langle {\bf u}^*, \nabla_{\bf u} G \rangle` can be used to
   determine the orientation of the optimal response with respect to the
   median response.

.. [3]
   computed by inverting the second-order probability relationships
   described in Section `1.1.2.2 <#theory:uq:reliability:local:mpp:int>`__ at
   the current :math:`{\bf u}^*` iterate.
