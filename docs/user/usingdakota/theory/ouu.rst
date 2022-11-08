.. _ouu:

Optimization Under Uncertainty (OUU)
====================================

.. _`ouu:rbdo`:

Reliability-Based Design Optimization (RBDO)
--------------------------------------------

Reliability-based design optimization (RBDO) methods are used to perform
design optimization accounting for reliability metrics. The reliability
analysis capabilities described in Section :ref:`Local Reliability Methods <theory:uq:reliability:local>`
provide a
substantial foundation for exploring a variety of gradient-based RBDO
formulations. :cite:p:`Eld05` investigated bi-level,
fully-analytic bi-level, and first-order sequential RBDO approaches
employing underlying first-order reliability assessments.
:cite:p:`Eld06a` investigated fully-analytic bi-level and
second-order sequential RBDO approaches employing underlying
second-order reliability assessments. These methods are overviewed in
the following sections.

.. _`ouu:rbdo:bilev`:

Bi-level RBDO 
~~~~~~~~~~~~~

The simplest and most direct RBDO approach is the bi-level approach in
which a full reliability analysis is performed for every optimization
function evaluation. This involves a nesting of two distinct levels of
optimization within each other, one at the design level and one at the
MPP search level.

Since an RBDO problem will typically specify both the :math:`\bar{z}`
level and the :math:`\bar{p}/\bar{\beta}` level, one can use either the
RIA or the PMA formulation for the UQ portion and then constrain the
result in the design optimization portion. In particular, RIA
reliability analysis maps :math:`\bar{z}` to :math:`p/\beta`, so RIA
RBDO constrains :math:`p/\beta`:

.. math::
   :label: eq:rbdo_ria


     \begin{aligned}
     {\rm minimize\ }     & f \nonumber \\ 
     {\rm subject \ to\   } & \beta \ge \bar{\beta} \nonumber \\
     {\rm or\   }           & p \le \bar{p} 
     \end{aligned}

And PMA reliability analysis maps :math:`\bar{p}/\bar{\beta}` to
:math:`z`, so PMA RBDO constrains :math:`z`:

.. math::
   :label: eq:rbdo_pma

     \begin{aligned}
     {\rm minimize\ }     & f \nonumber \\
     {\rm subject \ to\ } & z \ge \bar{z} 
     \end{aligned}

where :math:`z \ge \bar{z}` is used as the RBDO constraint for a
cumulative failure probability (failure defined as
:math:`z \le \bar{z}`) but :math:`z \le \bar{z}` would be used as the
RBDO constraint for a complementary cumulative failure probability
(failure defined as :math:`z
\ge \bar{z}`). It is worth noting that Dakota is not limited to these
types of inequality-constrained RBDO formulations; rather, they are
convenient examples. Dakota supports general optimization under
uncertainty mappings :cite:p:`Eld02` which allow flexible use
of statistics within multiple objectives, inequality constraints, and
equality constraints.

An important performance enhancement for bi-level methods is the use of
sensitivity analysis to analytically compute the design gradients of
probability, reliability, and response levels. When design variables are
separate from the uncertain variables (i.e., they are not distribution
parameters), then the following first-order expressions may be
used :cite:p:`Hoh86,Kar92,All04`:

.. math::
   :label: eq:deriv_z

   \begin{aligned}
   \nabla_{\bf d} z           & = & \nabla_{\bf d} g  \\
   \end{aligned}

.. math::
   :label: eq:deriv_beta

   \begin{aligned}
   \nabla_{\bf d} \beta_{cdf} & = & \frac{1}{{\parallel \nabla_{\bf u} G 
   \parallel}} \nabla_{\bf d} g  \\
   \end{aligned}  

.. math::
   :label: eq:deriv_p

   \begin{aligned}
   \nabla_{\bf d} p_{cdf}     & = & -\phi(-\beta_{cdf}) \nabla_{\bf d} \beta_{cdf}
   \end{aligned}

where it is evident from Eqs. :math:numref:`eq:beta_cdf_ccdf`
that :math:`\nabla_{\bf d} \beta_{ccdf} = -\nabla_{\bf d} \beta_{cdf}`
and :math:`\nabla_{\bf d} p_{ccdf} = -\nabla_{\bf d} p_{cdf}`. In the
case of second-order integrations, Eq. :math:numref:`eq:deriv_z`
must be expanded to include the curvature correction. For Breitung’s
correction (Eq. :math:numref:`eq:p_2nd_breit`  ),

.. math::
   :label: eq:deriv_p_breit

   \nabla_{\bf d} p_{cdf} = \left[ \Phi(-\beta_p) \sum_{i=1}^{n-1} 
   \left( \frac{-\kappa_i}{2 (1 + \beta_p \kappa_i)^{\frac{3}{2}}}
   \prod_{\stackrel{\scriptstyle j=1}{j \ne i}}^{n-1} 
   \frac{1}{\sqrt{1 + \beta_p \kappa_j}} \right) - 
   \phi(-\beta_p) \prod_{i=1}^{n-1} \frac{1}{\sqrt{1 + \beta_p \kappa_i}} 
   \right] \nabla_{\bf d} \beta_{cdf} 

where :math:`\nabla_{\bf d} \kappa_i` has been neglected and
:math:`\beta_p \ge 0` (see
Section :ref:`Local Reliability Integration <theory:uq:reliability:local:mpp:int>`.
Other approaches assume the curvature correction is nearly independent
of the design variables :cite:p:`Rac02`, which is equivalent
to neglecting the first term in
Eq. :math:numref:`eq:deriv_p_breit` .

To capture second-order probability estimates within an RIA RBDO
formulation using well-behaved :math:`\beta` constraints, a generalized
reliability index can be introduced where, similar to
Eq. :math:numref:`eq:beta_cdf` ,

.. math:: 
   :label: eq:gen_beta

   \beta^*_{cdf} = -\Phi^{-1}(p_{cdf}) 

for second-order :math:`p_{cdf}`. This reliability index is no longer
equivalent to the magnitude of :math:`{\bf u}`, but rather is a
convenience metric for capturing the effect of more accurate probability
estimates. The corresponding generalized reliability index sensitivity,
similar to Eq. :math:numref:`eq:deriv_z`, is

.. math::
   :label: eq:deriv_gen_beta

   \nabla_{\bf d} \beta^*_{cdf} = -\frac{1}{\phi(-\beta^*_{cdf})}
   \nabla_{\bf d} p_{cdf} 

where :math:`\nabla_{\bf d} p_{cdf}` is defined from
Eq. :math:numref:`eq:deriv_p_breit`. Even when
:math:`\nabla_{\bf d} g` is estimated numerically,
Eqs. :math:numref:`eq:deriv_z` - :math:numref:`eq:deriv_gen_beta` 
can be used to avoid numerical differencing across full reliability
analyses.

When the design variables are distribution parameters of the uncertain
variables, :math:`\nabla_{\bf d} g` is expanded with the chain rule and
Eqs.  :math:numref:`eq:deriv_z`
and   :math:numref:`eq:deriv_beta` (2)  become

.. math::
   :label: eq:deriv_z_ds

   \begin{aligned}
   \nabla_{\bf d} z           & = & \nabla_{\bf d} {\bf x} \nabla_{\bf x} g
   \\
   \end{aligned}

.. math::
   :label: eq:deriv_beta_ds

   \begin{aligned}
   \nabla_{\bf d} \beta_{cdf} & = & \frac{1}{{\parallel \nabla_{\bf u} G 
   \parallel}} \nabla_{\bf d} {\bf x} \nabla_{\bf x} g
   \end{aligned}

where the design Jacobian of the transformation
(:math:`\nabla_{\bf d} {\bf x}`) may be obtained analytically for
uncorrelated :math:`{\bf x}` or semi-analytically for correlated
:math:`{\bf x}` (:math:`\nabla_{\bf d} {\bf L}` is evaluated
numerically) by differentiating Eqs. :math:numref:`eq:trans_zx` 
and   :math:numref:`eq:trans_zu` _ with respect to the distribution
parameters.
Eqs. :math:numref:`eq:deriv_z` - :math:numref:`eq:deriv_gen_beta` remain the same as before. For this design variable case, all required
information for the sensitivities is available from the MPP search.

Since
Eqs. :math:numref:`eq:deriv_z` - :math:numref:`eq:deriv_beta_ds` 
are derived using the Karush-Kuhn-Tucker optimality conditions for a
converged MPP, they are appropriate for RBDO using AMV+,
AMV\ :math:`^2`\ +, TANA, FORM, and SORM, but not for RBDO using MVFOSM,
MVSOSM, AMV, or AMV\ :math:`^2`.

.. _`ouu:rbdo:surr`:

Sequential/Surrogate-based RBDO
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An alternative RBDO approach is the sequential approach, in which
additional efficiency is sought through breaking the nested relationship
of the MPP and design searches. The general concept is to iterate
between optimization and uncertainty quantification, updating the
optimization goals based on the most recent probabilistic assessment
results. This update may be based on safety
factors :cite:p:`Wu01` or other
approximations :cite:p:`Du04`.

A particularly effective approach for updating the optimization goals is
to use the :math:`p/\beta/z` sensitivity analysis of
Eqs.  :math:numref:`eq:deriv_z` - :math:numref:`eq:deriv_beta_ds`
in combination with local surrogate models :cite:p:`Zou04`. In
:cite:p:`Eld05` and :cite:p:`Eld06a`, first-order
and second-order Taylor series approximations were employed within a
trust-region model management framework :cite:p:`Giu00` in
order to adaptively manage the extent of the approximations and ensure
convergence of the RBDO process. Surrogate models were used for both the
objective function and the constraints, although the use of constraint
surrogates alone is sufficient to remove the nesting.

In particular, RIA trust-region surrogate-based RBDO employs surrogate
models of :math:`f` and :math:`p/\beta` within a trust region
:math:`\Delta^k` centered at :math:`{\bf d}_c`. For first-order
surrogates:

.. math::
   :label: eq:rbdo_surr1_ria

   \begin{aligned}
     {\rm minimize\ }     & f({\bf d}_c) + \nabla_d f({\bf d}_c)^T
   ({\bf d} - {\bf d}_c) \nonumber \\
     {\rm subject \ to\ } & \beta({\bf d}_c) + \nabla_d \beta({\bf d}_c)^T
   ({\bf d} - {\bf d}_c) \ge \bar{\beta} \nonumber \\
     {\rm or\ }           & p ({\bf d}_c) + \nabla_d p({\bf d}_c)^T 
   ({\bf d} - {\bf d}_c) \le \bar{p} \nonumber \\
   & {\parallel {\bf d} - {\bf d}_c \parallel}_\infty \le \Delta^k
   \end{aligned}

and for second-order surrogates:

.. math::
   :label: eq:rbdo_surr2_ria

   \begin{aligned}
     {\rm minimize\ }     & f({\bf d}_c) + \nabla_{\bf d} f({\bf d}_c)^T
   ({\bf d} - {\bf d}_c)  + \frac{1}{2} ({\bf d} - {\bf d}_c)^T 
   \nabla^2_{\bf d} f({\bf d}_c) ({\bf d} - {\bf d}_c) \nonumber \\
     {\rm subject \ to\ } & \beta({\bf d}_c) + \nabla_{\bf d} \beta({\bf d}_c)^T
   ({\bf d} - {\bf d}_c) + \frac{1}{2} ({\bf d} - {\bf d}_c)^T 
   \nabla^2_{\bf d} \beta({\bf d}_c) ({\bf d} - {\bf d}_c) \ge \bar{\beta}
   \nonumber \\
     {\rm or\ }           & p ({\bf d}_c) + \nabla_{\bf d} p({\bf d}_c)^T 
   ({\bf d} - {\bf d}_c) + \frac{1}{2} ({\bf d} - {\bf d}_c)^T 
   \nabla^2_{\bf d} p({\bf d}_c) ({\bf d} - {\bf d}_c) \le \bar{p} \nonumber \\
   & {\parallel {\bf d} - {\bf d}_c \parallel}_\infty \le \Delta^k
   \end{aligned}

For PMA trust-region surrogate-based RBDO, surrogate models of :math:`f`
and :math:`z` are employed within a trust region :math:`\Delta^k`
centered at :math:`{\bf d}_c`. For first-order surrogates:

.. math::
   :label: eq:rbdo_surr1_pma

   \begin{aligned}
     {\rm minimize\ }     & f({\bf d}_c) + \nabla_d f({\bf d}_c)^T
   ({\bf d} - {\bf d}_c) \nonumber \\
     {\rm subject \ to\ } & z({\bf d}_c) + \nabla_d z({\bf d}_c)^T ({\bf d} - {\bf d}_c) 
   \ge \bar{z} \nonumber \\
   & {\parallel {\bf d} - {\bf d}_c \parallel}_\infty \le \Delta^k
   \end{aligned}

and for second-order surrogates:

.. math::
   :label: eq:rbdo_surr2_pma

   \begin{aligned}
     {\rm minimize\ }     & f({\bf d}_c) + \nabla_{\bf d} f({\bf d}_c)^T
   ({\bf d} - {\bf d}_c) + \frac{1}{2} ({\bf d} - {\bf d}_c)^T 
   \nabla^2_{\bf d} f({\bf d}_c) ({\bf d} - {\bf d}_c) \nonumber \\
     {\rm subject \ to\ } & z({\bf d}_c) + \nabla_{\bf d} z({\bf d}_c)^T ({\bf d} - {\bf d}_c)
    + \frac{1}{2} ({\bf d} - {\bf d}_c)^T \nabla^2_{\bf d} z({\bf d}_c) 
   ({\bf d} - {\bf d}_c) \ge \bar{z} \nonumber \\
   & {\parallel {\bf d} - {\bf d}_c \parallel}_\infty \le \Delta^k
   \end{aligned}

where the sense of the :math:`z` constraint may vary as described
previously. The second-order information in
Eqs. :math:numref:`eq:rbdo_surr2_ria`  and
:math:numref:`eq:rbdo_surr2_pma`  will typically be
approximated with quasi-Newton updates.

.. _`ouu:sebdo`:

Stochastic Expansion-Based Design Optimization (SEBDO)
------------------------------------------------------

.. _`ouu:sebdo:ssa`:

Stochastic Sensitivity Analysis
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Section :ref:`Expansion RVSA <theory:uq:expansion:rvsa>` describes
sensitivity analysis of the polynomial chaos expansion with respect to
the expansion variables. Here we extend this analysis to include
sensitivity analysis of probabilistic moments with respect to
nonprobabilistic (i.e., design or epistemic uncertain) variables.

.. _`ouu:sebdo:ssa:dvsa_rve`:

Local sensitivity analysis: first-order probabilistic expansions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

With the introduction of nonprobabilistic variables
:math:`\boldsymbol{s}` (for example, design variables or epistemic
uncertain variables), a polynomial chaos expansion only over the
probabilistic variables :math:`\boldsymbol{\xi}` has the functional
relationship:

.. math::
   :label: eq:R_alpha_s_psi_xi

   R(\boldsymbol{\xi}, \boldsymbol{s}) \cong \sum_{j=0}^P \alpha_j(\boldsymbol{s}) 
   \Psi_j(\boldsymbol{\xi})

For computing sensitivities of response mean and variance, the
:math:`ij` indices may be dropped from
Eqs.  :math:numref:`eq:mean_pce`  
and  :math:numref:`eq:covar_pce`  , simplifying to

.. math:: 
   :label: eq:var_pce

   \mu(s) ~=~ \alpha_0(s), ~~~~\sigma^2(s) = \sum_{k=1}^P \alpha^2_k(s) \langle \Psi^2_k \rangle 

Sensitivities of Eq. :math:numref:`eq:var_pce`  with respect to the
nonprobabilistic variables are as follows, where independence of
:math:`\boldsymbol{s}` and :math:`\boldsymbol{\xi}` is assumed:

.. math::
   :label: eq:dmuR_ds_xi_pce

   \begin{aligned}
   \frac{d\mu}{ds} &=& \frac{d\alpha_0}{ds} ~~=~~ 
   %\frac{d}{ds} \langle R \rangle ~~=~~ 
   \langle \frac{dR}{ds} \rangle \\
   \end{aligned}

.. math::
   :label: eq:dsigR_ds_xi_pce

   \begin{aligned
   \frac{d\sigma^2}{ds} &=& \sum_{k=1}^P \langle \Psi_k^2 \rangle 
   \frac{d\alpha_k^2}{ds} ~~=~~ 
   2 \sum_{k=1}^P \alpha_k \langle \frac{dR}{ds}, \Psi_k \rangle 
   \end{aligned}

where

.. math::
   :label: eq:dalpha_k_ds

   \frac{d\alpha_k}{ds} = \frac{\langle \frac{dR}{ds}, \Psi_k \rangle}
   {\langle \Psi^2_k \rangle} 

has been used. Due to independence, the coefficients calculated in
Eq.  :math:numref:`eq:dalpha_k_ds`   may be interpreted as either
the derivatives of the expectations or the expectations of the
derivatives, or more precisely, the nonprobabilistic sensitivities of
the chaos coefficients for the response expansion or the chaos
coefficients of an expansion for the nonprobabilistic sensitivities of
the response. The evaluation of integrals involving
:math:`\frac{dR}{ds}` extends the data requirements for the PCE approach
to include response sensitivities at each of the sampled points. The
resulting expansions are valid only for a particular set of
nonprobabilistic variables and must be recalculated each time the
nonprobabilistic variables are modified.

..
   TODO:The evaluation of integrals involving $\frac{dR}{ds}$ extends
   the data requirements for the PCE approach to include response
   sensitivities at each of the sampled points.% for the quadrature,
   sparse grid, sampling, or point collocation coefficient estimation
   approaches.

Similarly for stochastic collocation,

.. math::
   :label: eq:R_r_s_K_xi

   R(\boldsymbol{\xi}, \boldsymbol{s}) \cong \sum_{k=1}^{N_p} r_k(\boldsymbol{s}) 
   \boldsymbol{L}_k(\boldsymbol{\xi}) 

leads to

.. math::
   :label: eq:var_sc

   \begin{aligned}
   \mu(s) &=& \sum_{k=1}^{N_p} r_k(s) w_k, ~~~~\sigma^2(s) ~=~ \sum_{k=1}^{N_p} r^2_k(s) w_k - \mu^2(s)  \\
   \end{aligned}

.. math::
   :label: eq:dmuR_ds_xi_sc

   \begin{aligned}
   \frac{d\mu}{ds} &=& %\frac{d}{ds} \langle R \rangle ~~=~~ 
   %\sum_{k=1}^{N_p} \frac{dr_k}{ds} \langle \boldsymbol{L}_k \rangle ~~=~~ 
   \sum_{k=1}^{N_p} w_k \frac{dr_k}{ds} \\
   \end{aligned}

.. math::
   :label: eq:dsigR_ds_xi_sc

   \begin{aligned}
   \frac{d\sigma^2}{ds} &=& \sum_{k=1}^{N_p} 2 w_k r_k \frac{dr_k}{ds}
   - 2 \mu \frac{d\mu}{ds} 
   ~~=~~ \sum_{k=1}^{N_p} 2 w_k (r_k - \mu) \frac{dr_k}{ds}
   \end{aligned}

..
   TODO: based on differentiation of Eqs.~\ref{eq:mean_sc}-\ref{eq:covar_sc}.

.. _`ouu:sebdo:ssa:dvsa_cve`:

Local sensitivity analysis: zeroth-order combined expansions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Alternatively, a stochastic expansion can be formed over both
:math:`\boldsymbol{\xi}` and :math:`\boldsymbol{s}`. Assuming a bounded
design domain :math:`\boldsymbol{s}_L \le \boldsymbol{s} \le
\boldsymbol{s}_U` (with no implied probability content), a Legendre
chaos basis would be appropriate for each of the dimensions in
:math:`\boldsymbol{s}` within a polynomial chaos expansion.

.. math::
   :label: eq:R_alpha_psi_xi_s

   R(\boldsymbol{\xi}, \boldsymbol{s}) \cong \sum_{j=0}^P \alpha_j 
   \Psi_j(\boldsymbol{\xi}, \boldsymbol{s}) 

In this case, design sensitivities for the mean and variance do not
require response sensitivity data, but this comes at the cost of forming
the PCE over additional dimensions. For this combined variable
expansion, the mean and variance are evaluated by performing the
expectations over only the probabilistic expansion variables, which
eliminates the polynomial dependence on :math:`\boldsymbol{\xi}`,
leaving behind the desired polynomial dependence of the moments on
:math:`\boldsymbol{s}`:

.. math::
   :label: eq:muR_comb_pce

   \begin{aligned}
   \mu_R(\boldsymbol{s}) &=& \sum_{j=0}^P \alpha_j \langle \Psi_j(\boldsymbol{\xi},
   \boldsymbol{s}) \rangle_{\boldsymbol{\xi}}  \\
   \end{aligned}
   
.. math::
   :label: eq:sigR_comb_pce

   \begin{aligned}
   \sigma^2_R(\boldsymbol{s}) &=& \sum_{j=0}^P \sum_{k=0}^P \alpha_j \alpha_k 
   \langle \Psi_j(\boldsymbol{\xi}, \boldsymbol{s}) \Psi_k(\boldsymbol{\xi},
   \boldsymbol{s}) \rangle_{\boldsymbol{\xi}} ~-~ \mu^2_R(\boldsymbol{s})
   \end{aligned}

The remaining polynomials may then be differentiated with respect to
:math:`\boldsymbol{s}`. In this approach, the combined PCE is valid for
the full design variable range
(:math:`\boldsymbol{s}_L \le \boldsymbol{s} \le \boldsymbol{s}_U`) and
does not need to be updated for each change in nonprobabilistic
variables, although adaptive localization techniques (i.e., trust region
model management approaches) can be employed when improved local
accuracy of the sensitivities is required.

..
   TODO: The remaining polynomials may then be differentiated with
   respect to $\boldsymbol{s}$. % as in
   Eqs.~\ref{eq:dR_dxi_pce}-\ref{eq:deriv_prod_pce}.  In this
   approach, the combined PCE is valid for the full design variable
   range ($\boldsymbol{s}_L \le \boldsymbol{s} \le \boldsymbol{s}_U$)
   and does not need to be updated for each change in nonprobabilistic
   variables, although adaptive localization techniques (i.e., trust
   region model management approaches) can be employed when improved
   local accuracy of the sensitivities is required.

   Q: how is TR ratio formed if exact soln can't be evaluated?
   A: if objective is accuracy over a design range, then truth is PCE/SC
      at a single design point!  -->>  Can use first-order corrections based
      on the 2 different SSA approaches!  This is a multifidelity SBO using
      HF = probabilistic expansion, LF = Combined expansion. Should get data reuse.

Similarly for stochastic collocation,

.. math::
   :label: eq:eq:R_r_L_xi_s

   R(\boldsymbol{\xi}, \boldsymbol{s}) \cong \sum_{j=1}^{N_p} r_j 
   \boldsymbol{L}_j(\boldsymbol{\xi}, \boldsymbol{s}) 

leads to

.. math::
   :label: eq:muR_both_sc

   \begin{aligned}
   \mu_R(\boldsymbol{s}) &=& \sum_{j=1}^{N_p} r_j \langle 
   \boldsymbol{L}_j(\boldsymbol{\xi}, \boldsymbol{s}) \rangle_{\boldsymbol{\xi}} 
   \end{aligned}

.. math::
   :label: eq:sigR_both_sc

   \begin{aligned}
   \sigma^2_R(\boldsymbol{s}) &=& \sum_{j=1}^{N_p} \sum_{k=1}^{N_p} r_j r_k 
   \langle \boldsymbol{L}_j(\boldsymbol{\xi}, \boldsymbol{s}) 
   \boldsymbol{L}_k(\boldsymbol{\xi}, \boldsymbol{s}) \rangle_{\boldsymbol{\xi}}
   ~-~ \mu^2_R(\boldsymbol{s}) 
   \end{aligned}

where the remaining polynomials not eliminated by the expectation over
:math:`\boldsymbol{\xi}` are again differentiated with respect to
:math:`\boldsymbol{s}`.

.. _`ouu:sebdo:ssa:io`:

Inputs and outputs
^^^^^^^^^^^^^^^^^^

There are two types of nonprobabilistic variables for which
sensitivities must be calculated: “augmented,” where the
nonprobabilistic variables are separate from and augment the
probabilistic variables, and “inserted,” where the nonprobabilistic
variables define distribution parameters for the probabilistic
variables. Any inserted nonprobabilistic variable sensitivities must be
handled using
Eqs.  :math:numref:`eq:dmuR_ds_xi_pce`  
and
Eqs.  :math:numref:`eq:var_sc`  
where :math:`\frac{dR}{ds}` is calculated as
:math:`\frac{dR}{dx} \frac{dx}{ds}` and :math:`\frac{dx}{ds}` is the
Jacobian of the variable transformation
:math:`{\bf x} = T^{-1}(\boldsymbol{\xi})` with respect to the inserted
nonprobabilistic variables. In addition, parameterized polynomials
(generalized Gauss-Laguerre, Jacobi, and numerically-generated
polynomials) may introduce a :math:`\frac{d\Psi}{ds}` or
:math:`\frac{d\boldsymbol{L}}{ds}` dependence for inserted :math:`s`
that will introduce additional terms in the sensitivity expressions.

..
   TODO:

   While one could artificially augment the dimensionality of
   a combined variable expansion approach with inserted nonprobabilistic
   variables, this is not currently explored in this work.  Thus, any
   
   TO DO: discuss independence of additional nonprobabilistic dimensions:
   > augmented are OK.
   > inserted rely on the fact that expansion variables \xi are _standard_
     random variables.
   Special case: parameterized orthogonal polynomials (gen Laguerre,
   Jacobi) can be differentiated w.r.t. their {alpha,beta}
   distribution parameters.  However, the PCE coefficients are likely
   also fns of {alpha,beta}.  Therefore, the approach above is correct
   conceptually but is missing additional terms resulting from the
   polynomial dependence.  NEED TO VERIFY PCE EXPANSION DERIVATIVES
   FOR PARAMETERIZED POLYNOMIALS!

While moment sensitivities directly enable robust design optimization
and interval estimation formulations which seek to control or bound
response variance, control or bounding of reliability requires
sensitivities of tail statistics. In this work, the sensitivity of
simple moment-based approximations to cumulative distribution function
(CDF) and complementary cumulative distribution function (CCDF) mappings
(Eqs.  :math:numref:`eq:mv_ria`  –  :math:numref:`eq:mv_pma`  ) are
employed for this purpose, such that it is straightforward to form
approximate design sensitivities of reliability index :math:`\beta`
(forward reliability mapping :math:`\bar{z} \rightarrow \beta`) or
response level :math:`z` (inverse reliability mapping
:math:`\bar{\beta} \rightarrow z`) from the moment design sensitivities
and the specified levels :math:`\bar{\beta}` or :math:`\bar{z}`.

..
   TODO:

   From here, approximate design sensitivities of probability levels may
   also be formed given a probability expression (such as $\Phi(-\beta)$)
   for the reliability index.  The current alternative of numerical
   design sensitivities of sampled probability levels would employ fewer
   simplifying approximations, but would also be much more expensive to
   compute accurately and is avoided for now.  Future capabilities for
   analytic probability sensitivities could be based on Pearson/Johnson
   model for analytic response PDFs or 
   sampling sensitivity approaches. % TO DO: cite 
   
   Extending beyond these simple approaches to support probability and
   generalized reliability metrics is a subject of current work~\cite{mao2010}.

.. _`ouu:sebdo:form`:

Optimization Formulations
~~~~~~~~~~~~~~~~~~~~~~~~~

Given the capability to compute analytic statistics of the response
along with design sensitivities of these statistics, Dakota supports
bi-level, sequential, and multifidelity approaches for optimization
under uncertainty (OUU). The latter two approaches apply surrogate
modeling approaches (data fits and multifidelity modeling) to the
uncertainty analysis and then apply trust region model management to the
optimization process.

..
   TODO: for optimization under uncertainty (OUU). %for
   reliability-based design and robust design.

.. _`ouu:sebdo:form:bilev`:

Bi-level SEBDO
^^^^^^^^^^^^^^

The simplest and most direct approach is to employ these analytic
statistics and their design derivatives from
Section :ref:`SEBDO SSA <theory:uq:sebdo:ssa>` directly within an optimization loop.
This approach is known as bi-level OUU, since there is an inner level
uncertainty analysis nested within an outer level optimization.

Consider the common reliability-based design example of a deterministic
objective function with a reliability constraint:

.. math::
   :label: eq:rbdo

   \begin{aligned}
     {\rm minimize\ }     & f \nonumber \\
     {\rm subject \ to\ } & \beta \ge \bar{\beta} 
   \end{aligned}

where :math:`\beta` is computed relative to a prescribed threshold
response value :math:`\bar{z}` (e.g., a failure threshold) and is
constrained by a prescribed reliability level :math:`\bar{\beta}`
(minimum allowable reliability in the design), and is either a CDF or
CCDF index depending on the definition of the failure domain (i.e.,
defined from whether the associated failure probability is cumulative,
:math:`p(g \le
\bar{z})`, or complementary cumulative, :math:`p(g > \bar{z})`).

Another common example is robust design in which the constraint
enforcing a reliability lower-bound has been replaced with a constraint
enforcing a variance upper-bound :math:`\bar{\sigma}^2` (maximum
allowable variance in the design):

.. math::
   :label: eq:rdo

   \begin{aligned}
     {\rm minimize\ }     & f \nonumber \\
     {\rm subject \ to\ } & \sigma^2 \le \bar{\sigma}^2 
   \end{aligned}

Solving these problems using a bi-level approach involves computing
:math:`\beta` and :math:`\frac{d\beta}{d\boldsymbol{s}}` for
Eq.  :math:numref:`eq:rbdo`   or :math:`\sigma^2` and
:math:`\frac{d\sigma^2}{d\boldsymbol{s}}` for Eq.  :math:numref:`eq:rdo`
for each set of design variables :math:`\boldsymbol{s}` passed from the
optimizer. This approach is supported for both probabilistic and
combined expansions using PCE and SC.

.. _`ouu:sebdo:form:surr`:

Sequential/Surrogate-Based SEBDO
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

An alternative OUU approach is the sequential approach, in which
additional efficiency is sought through breaking the nested relationship
of the UQ and optimization loops. The general concept is to iterate
between optimization and uncertainty quantification, updating the
optimization goals based on the most recent uncertainty assessment
results. This approach is common with the reliability methods community,
for which the updating strategy may be based on safety
factors :cite:p:`Wu01` or other
approximations :cite:p:`Du04`.

A particularly effective approach for updating the optimization goals is
to use data fit surrogate models, and in particular, local Taylor series
models allow direct insertion of stochastic sensitivity analysis
capabilities. In Ref. :cite:p:`Eld05`, first-order Taylor
series approximations were explored, and in
Ref. :cite:p:`Eld06a`, second-order Taylor series
approximations are investigated. In both cases, a trust-region model
management framework :cite:p:`Eld06b` is used to adaptively
manage the extent of the approximations and ensure convergence of the
OUU process. Surrogate models are used for both the objective and the
constraint functions, although the use of surrogates is only required
for the functions containing statistical results; deterministic
functions may remain explicit is desired.

In particular, trust-region surrogate-based optimization for
reliability-based design employs surrogate models of :math:`f` and
:math:`\beta` within a trust region :math:`\Delta^k` centered at
:math:`{\bf s}_c`:

.. math::
   :label: eq:rbdo_surr

   \begin{aligned}
     {\rm minimize\ }     & f({\bf s}_c) + \nabla_s f({\bf s}_c)^T
   ({\bf s} - {\bf s}_c) \nonumber \\
     {\rm subject \ to\ } & \beta({\bf s}_c) + \nabla_s \beta({\bf s}_c)^T
   ({\bf s} - {\bf s}_c) \ge \bar{\beta} \\
   & {\parallel {\bf s} - {\bf s}_c \parallel}_\infty \le \Delta^k \nonumber
   \end{aligned}

and trust-region surrogate-based optimization for robust design employs
surrogate models of :math:`f` and :math:`\sigma^2` within a trust region
:math:`\Delta^k` centered at :math:`{\bf s}_c`:

.. math::
   :label: eq:rdo_surr

   \begin{aligned}
     {\rm minimize\ }     & f({\bf s}_c) + \nabla_s f({\bf s}_c)^T
   ({\bf s} - {\bf s}_c) \nonumber \\
     {\rm subject \ to\ } & \sigma^2({\bf s}_c) + \nabla_s \sigma^2({\bf s}_c)^T 
   ({\bf s} - {\bf s}_c) \le \bar{\sigma}^2 \\
   & {\parallel {\bf s} - {\bf s}_c \parallel}_\infty \le \Delta^k \nonumber
   \end{aligned}

Second-order local surrogates may also be employed, where the Hessians
are typically approximated from an accumulation of curvature information
using quasi-Newton updates :cite:p:`Noc99` such as
Broyden-Fletcher-Goldfarb-Shanno (BFGS, Eq.  :math:numref:`eq:bfgs`  or
symmetric rank one (SR1, Eq.  :math:numref:`eq:sr1`  . The sequential
approach is available for probabilistic expansions using PCE and SC.

.. _`ouu:sebdo:form:mf`:

Multifidelity SEBDO
^^^^^^^^^^^^^^^^^^^

The multifidelity OUU approach is another trust-region surrogate-based
approach. Instead of the surrogate UQ model being a simple data fit (in
particular, first-/second-order Taylor series model) of the truth UQ
model results, distinct UQ models of differing fidelity are now
employed. This differing UQ fidelity could stem from the fidelity of the
underlying simulation model, the fidelity of the UQ algorithm, or both.
In this section, we focus on the fidelity of the UQ algorithm. For
reliability methods, this could entail varying fidelity in approximating
assumptions (e.g., Mean Value for low fidelity, SORM for high fidelity),
and for stochastic expansion methods, it could involve differences in
selected levels of :math:`p` and :math:`h` refinement.

..
   TODO:
   Here we will explore multifidelity stochastic models and employ
   first-order additive corrections, where the meaning of multiple
   fidelities is expanded to imply the quality of multiple UQ analyses,
   not necessarily the fidelity of the underlying simulation model.  For
   example, taking an example from the reliability method family, one
   might employ the simple Mean Value method as a ``low fidelity'' UQ
   model and take SORM as a ``high fidelity'' UQ model.  In this case,
   the models do not differ in their ability to span a range of design
   parameters; rather, they differ in their sets of approximating
   assumptions about the characteristics of the response function.

Here, we define UQ fidelity as point-wise accuracy in the design space
and take the high fidelity truth model to be the probabilistic expansion
PCE/SC model, with validity only at a single design point. The low
fidelity model, whose validity over the design space will be adaptively
controlled, will be either the combined expansion PCE/SC model, with
validity over a range of design parameters, or the MVFOSM reliability
method, with validity only at a single design point. The combined
expansion low fidelity approach will span the current trust region of
the design space and will be reconstructed for each new trust region.
Trust region adaptation will ensure that the combined expansion approach
remains sufficiently accurate for design purposes. By taking advantage
of the design space spanning, one can eliminate the cost of multiple low
fidelity UQ analyses within the trust region, with fallback to the
greater accuracy and higher expense of the probabilistic expansion
approach when needed. The MVFOSM low fidelity approximation must be
reformed for each change in design variables, but it only requires a
single evaluation of a response function and its derivative to
approximate the response mean and variance from the input mean and
covariance
(Eqs.  :math:numref:`eq:mv_mean1`   –  :math:numref:`eq:mv_std_dev`  
from which forward/inverse CDF/CCDF reliability mappings can be
generated using
Eqs.  :math:numref:`eq:mv_ria`  –  :math:numref:`eq:mv_pma`  . This is
the least expensive UQ option, but its limited accuracy [1]_ may dictate
the use of small trust regions, resulting in greater iterations to
convergence. The expense of optimizing a combined expansion, on the
other hand, is not significantly less than that of optimizing the high
fidelity UQ model, but its representation of global trends should allow
the use of larger trust regions, resulting in reduced iterations to
convergence. The design derivatives of each of the PCE/SC expansion
models provide the necessary data to correct the low fidelity model to
first-order consistency with the high fidelity model at the center of
each trust region, ensuring convergence of the multifidelity
optimization process to the high fidelity optimum. Design derivatives of
the MVFOSM statistics are currently evaluated numerically using forward
finite differences.

..
   TODO:
   While conceptually different, in the end, this approach is
   similar to the use of a global data fit surrogate-based optimization
   at the top level in combination with the probabilistic expansion PCE/SC
   at the lower level, with the distinction that the multifidelity approach
   embeds the design space spanning within a modified PCE/SC process
   whereas the data fit approach performs the design space spanning
   outside of the UQ (using data from a single unmodified PCE/SC process,
   which may now remain zeroth-order).

Multifidelity optimization for reliability-based design can be
formulated as:

.. math::
   :label: eq:rbdo_mf

   \begin{aligned}
     {\rm minimize\ }     & f({\bf s}) \nonumber \\
     {\rm subject \ to\ } & \hat{\beta_{hi}}({\bf s}) \ge \bar{\beta} \\
   & {\parallel {\bf s} - {\bf s}_c \parallel}_\infty \le \Delta^k \nonumber
   \end{aligned}

and multifidelity optimization for robust design can be formulated as:

.. math::
   :label: eq:rdo_mf

   \begin{aligned}
     {\rm minimize\ }     & f({\bf s}) \nonumber \\
     {\rm subject \ to\ } & \hat{\sigma_{hi}}^2({\bf s}) \le \bar{\sigma}^2 \\
   & {\parallel {\bf s} - {\bf s}_c \parallel}_\infty \le \Delta^k \nonumber
   \end{aligned}

where the deterministic objective function is not approximated and
:math:`\hat{\beta_{hi}}` and :math:`\hat{\sigma_{hi}}^2` are the
approximated high-fidelity UQ results resulting from correction of the
low-fidelity UQ results. In the case of an additive correction function:

.. math::
   :label: eq:corr_lf_beta

   \begin{aligned}
   \hat{\beta_{hi}}({\bf s})    &=& \beta_{lo}({\bf s}) + 
   \alpha_{\beta}({\bf s})   \\
   \end{aligned}
   
.. math::
   :label: eq:corr_lf_sigma

   \begin{aligned}
   \hat{\sigma_{hi}}^2({\bf s}) &=& \sigma_{lo}^2({\bf s}) + 
   \alpha_{\sigma^2}({\bf s}) 
   \end{aligned}

where correction functions :math:`\alpha({\bf s})` enforcing first-order
consistency :cite:p:`Eld04` are typically employed.
Quasi-second-order correction functions :cite:p:`Eld04` can
also be employed, but care must be taken due to the different rates of
curvature accumulation between the low and high fidelity models. In
particular, since the low fidelity model is evaluated more frequently
than the high fidelity model, it accumulates curvature information more
quickly, such that enforcing quasi-second-order consistency with the
high fidelity model can be detrimental in the initial iterations of the
algorithm [2]_. Instead, this consistency should only be enforced when
sufficient high fidelity curvature information has been accumulated
(e.g., after :math:`n` rank one updates).

..
   TODO: where correction functions $\alpha({\bf s})$ enforcing first-order
   %and quasi-second-order 
   consistency~\cite{Eld04} are typically employed.  Quasi-second-order

.. _`ouu:sampling`:

Sampling-based OUU
------------------

Gradient-based OUU can also be performed using random sampling methods.
In this case, the sample-average approximation to the design derivative
of the mean and standard deviation are:

.. math::
   :label: eq:smp_ouu

   \begin{aligned}
     \frac{d\mu}{ds}    &=& \frac{1}{N} \sum_{i=1}^N \frac{dQ}{ds} \\
     \frac{d\sigma}{ds} &=& \left[ \sum_{i=1}^N (Q \frac{dQ}{ds})
       - N \mu \frac{d\mu}{ds} \right] / (\sigma (N-1))\end{aligned}

This enables design sensitivities for mean, standard deviation or
variance (based on ``final_moments`` type), and forward/inverse
reliability index mappings (:math:`\bar{z} \rightarrow \beta`,
:math:`\bar{\beta} \rightarrow z`).

..
   TODO: Multilevel MC ...

.. [1]
   MVFOSM is exact for linear functions with Gaussian inputs, but
   quickly degrades for nonlinear and/or non-Gaussian.

.. [2]
   Analytic and numerical Hessians, when available, are instantaneous
   with no accumulation rate concerns.
