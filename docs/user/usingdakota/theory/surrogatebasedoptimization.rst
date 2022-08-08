.. _sblm:

Surrogate-Based Local Minimization
==================================

A generally-constrained nonlinear programming problem takes the form

.. math::

   \begin{aligned}
   {\rm minimize } \hfil & f({\bf x}) \nonumber \\
   {\rm subject\  to } & {\bf g}_l \le {\bf g}({\bf x}) \le {\bf g}_u \nonumber \\
               &               {\bf h}({\bf x}) = {\bf h}_t \nonumber \\
               & {\bf x}_l \le {\bf x} \le {\bf x}_u
   \label{eq:NLP_standard}\end{aligned}

where :math:`{\bf x} \in \Re^n` is the vector of design variables, and
:math:`f`, :math:`{\bf g}`, and :math:`{\bf h}` are the objective
function, nonlinear inequality constraints, and nonlinear equality
constraints, respectively [1]_. Individual nonlinear inequality and
equality constraints are enumerated using :math:`i` and :math:`j`,
respectively (e.g., :math:`g_i` and :math:`h_j`). The corresponding
surrogate-based optimization (SBO) algorithm may be formulated in
several ways and applied to either optimization or least-squares
calibration problems. In all cases, SBO solves a sequence of :math:`k`
approximate optimization subproblems subject to a trust region
constraint :math:`\Delta^k`; however, many different forms of the
surrogate objectives and constraints in the approximate subproblem can
be explored. In particular, the subproblem objective may be a surrogate
of the original objective or a surrogate of a merit function (most
commonly, the Lagrangian or augmented Lagrangian), and the subproblem
constraints may be surrogates of the original constraints, linearized
approximations of the surrogate constraints, or may be omitted entirely.
Each of these combinations is shown in Table `1.1 <#tab:sbo_subprob>`__,
where black indicates an inappropriate combination, gray indicates an
acceptable combination, and blue indicates a common combination.

.. container::
   :name: tab:sbo_subprob

   .. table:: SBO approximate subproblem formulations.

      +------------------------+--------------------+------------+----------------------+
      |                        | Original Objective | Lagrangian | Augmented Lagrangian |
      +========================+====================+============+======================+
      | No constraints         |                    |            | TRAL                 |
      +------------------------+--------------------+------------+----------------------+
      | Linearized constraints |                    | SQP-like   |                      |
      +------------------------+--------------------+------------+----------------------+
      | Original constraints   | Direct surrogate   |            | IPTRSAO              |
      +------------------------+--------------------+------------+----------------------+

Initial approaches to nonlinearly-constrained SBO optimized an
approximate merit function which incorporated the nonlinear
constraints :cite:p:`Rod98,Ale00`:

.. math::

   \begin{aligned}
   {\rm minimize } & {\hat \Phi}^k({\bf x}) \nonumber \\
   {\rm subject\  to } 
       & {\parallel {\bf x} - {\bf x}^k_c \parallel}_\infty \le \Delta^k
   \label{eq:NLP_SBO_TRAL}\end{aligned}

where the surrogate merit function is denoted as
:math:`\hat \Phi({\bf x})`, :math:`{\bf x}_c` is the center point of the
trust region, and the trust region is truncated at the global variable
bounds as needed. The merit function to approximate was typically chosen
to be a standard implementation :cite:p:`Van84,Noc99,Gil81` of
the augmented Lagrangian merit function (see
Eqs. `[eq:aug_lag_merit] <#eq:aug_lag_merit>`__–`[eq:aug_lag_psi] <#eq:aug_lag_psi>`__),
where the surrogate augmented Lagrangian is constructed from individual
surrogate models of the objective and constraints (approximate and
assemble, rather than assemble and approximate). In
Table `1.1 <#tab:sbo_subprob>`__, this corresponds to row 1, column 3,
and is known as the trust-region augmented Lagrangian (TRAL) approach.
While this approach was provably convergent, convergence rates to
constrained minima have been observed to be slowed by the required
updating of Lagrange multipliers and penalty
parameters :cite:p:`Per04a`. Prior to converging these
parameters, SBO iterates did not strictly respect constraint boundaries
and were often infeasible. A subsequent approach
(IPTRSAO :cite:p:`Per04a`) that sought to directly address
this shortcoming added explicit surrogate constraints (row 3, column 3
in Table `1.1 <#tab:sbo_subprob>`__):

.. math::

   \begin{aligned}
   {\rm minimize } & {\hat \Phi}^k({\bf x}) \nonumber \\
   {\rm subject\  to } 
       & {\bf g}_l \le {\bf {\hat g}}^k({\bf x}) \le {\bf g}_u \nonumber \\
       &               {\bf {\hat h}}^k({\bf x}) = {\bf h}_t \nonumber \\
       & {\parallel {\bf x} - {\bf x}^k_c \parallel}_\infty \le \Delta^k \; . 
   \label{eq:NLP_SBO_TRAL2}\end{aligned}

While this approach does address infeasible iterates, it still shares
the feature that the surrogate merit function may reflect inaccurate
relative weightings of the objective and constraints prior to
convergence of the Lagrange multipliers and penalty parameters. That is,
one may benefit from more feasible intermediate iterates, but the
process may still be slow to converge to optimality. The concept of this
approach is similar to that of SQP-like SBO
approaches :cite:p:`Ale00` which use linearized constraints:

.. math::

   \begin{aligned}
   {\rm minimize } & {\hat \Phi}^k({\bf x}) \nonumber \\
   {\rm subject\  to } 
   & {\bf g}_l \le {\bf {\hat g}}^k({\bf x}^k_c) + 
   \nabla {\bf {\hat g}}^k({\bf x}^k_c)^T ({\bf x} - {\bf x}^k_c) \le {\bf g}_u 
   \nonumber \\
   & {\bf {\hat h}}^k({\bf x}^k_c) + \nabla {\bf {\hat h}}^k({\bf x}^k_c)^T 
   ({\bf x} - {\bf x}^k_c) = {\bf h}_t \nonumber \\
   & {\parallel {\bf x} - {\bf x}^k_c \parallel}_\infty \le \Delta^k \; . 
   \label{eq:NLP_SBO_SQP}\end{aligned}

in that the primary concern is minimizing a composite merit function of
the objective and constraints, but under the restriction that the
original problem constraints may not be wildly violated prior to
convergence of Lagrange multiplier estimates. Here, the merit function
selection of the Lagrangian function (row 2, column 2 in
Table `1.1 <#tab:sbo_subprob>`__; see also
Eq. `[eq:lag_merit] <#eq:lag_merit>`__) is most closely related to SQP,
which includes the use of first-order Lagrange multiplier updates
(Eq. `[eq:lls_lambda] <#eq:lls_lambda>`__) that should converge more
rapidly near a constrained minimizer than the zeroth-order updates
(Eqs. `[eq:lambda_psi] <#eq:lambda_psi>`__-`[eq:lambda_h] <#eq:lambda_h>`__)
used for the augmented Lagrangian.

All of these previous constrained SBO approaches involve a recasting of
the approximate subproblem objective and constraints as a function of
the original objective and constraint surrogates. A more direct approach
is to use a formulation of:

.. math::

   \begin{aligned}
   {\rm minimize } & {\hat f}^k({\bf x}) \nonumber \\
   {\rm subject\  to } 
       & {\bf g}_l \le {\bf {\hat g}}^k({\bf x}) \le {\bf g}_u \nonumber \\
       &               {\bf {\hat h}}^k({\bf x}) = {\bf h}_t \nonumber \\
       & {\parallel {\bf x} - {\bf x}^k_c \parallel}_\infty \le \Delta^k 
   \label{eq:NLP_SBO_direct}\end{aligned}

This approach has been termed the direct surrogate approach since it
optimizes surrogates of the original objective and constraints (row 3,
column 1 in Table `1.1 <#tab:sbo_subprob>`__) without any recasting. It
is attractive both from its simplicity and potential for improved
performance, and is the default approach taken in Dakota. Other Dakota
defaults include the use of a filter method for iterate acceptance (see
Section `1.1 <#sbm:sblm_con_iter>`__), an augmented Lagrangian merit
function (see Section `1.2 <#sbm:sblm_con_merit>`__), Lagrangian hard
convergence assessment (see Section `1.3 <#sbm:sblm_con_hard>`__), and
no constraint relaxation (see Section `1.4 <#sbm:sblm_con_relax>`__).

While the formulation of Eq. `[eq:NLP_SBO_TRAL] <#eq:NLP_SBO_TRAL>`__
(and others from row 1 in Table `1.1 <#tab:sbo_subprob>`__) can suffer
from infeasible intermediate iterates and slow convergence to
constrained minima, each of the approximate subproblem formulations with
explicit constraints
(Eqs. `[eq:NLP_SBO_TRAL2] <#eq:NLP_SBO_TRAL2>`__-`[eq:NLP_SBO_direct] <#eq:NLP_SBO_direct>`__,
and others from rows 2-3 in Table `1.1 <#tab:sbo_subprob>`__) can suffer
from the lack of a feasible solution within the current trust region.
Techniques for dealing with this latter challenge involve some form of
constraint relaxation. Homotopy
approaches :cite:p:`Per04a,Per04b` or composite step
approaches such as Byrd-Omojokun :cite:p:`Omo89`,
Celis-Dennis-Tapia :cite:p:`Cel85`, or
MAESTRO :cite:p:`Ale00` may be used for this purpose (see
Section `1.4 <#sbm:sblm_con_relax>`__).

After each of the :math:`k` iterations in the SBO method, the predicted
step is validated by computing :math:`f({\bf x}^k_\ast)`,
:math:`{\bf g}({\bf x}^k_\ast)`, and :math:`{\bf h}({\bf x}^k_\ast)`.
One approach forms the trust region ratio :math:`\rho^k` which measures
the ratio of the actual improvement to the improvement predicted by
optimization on the surrogate model. When optimizing on an approximate
merit function
(Eqs. `[eq:NLP_SBO_TRAL] <#eq:NLP_SBO_TRAL>`__–`[eq:NLP_SBO_SQP] <#eq:NLP_SBO_SQP>`__),
the following ratio is natural to compute

.. math::

   \rho^k = \frac{     \Phi({\bf x}^k_c)      - \Phi({\bf x}^k_\ast)}
             {\hat \Phi({\bf x}^k_c) - \hat \Phi({\bf x}^k_\ast)} \; .
   \label{eq:rho_phi_k}

The formulation in Eq. `[eq:NLP_SBO_direct] <#eq:NLP_SBO_direct>`__ may
also form a merit function for computing the trust region ratio;
however, the omission of this merit function from explicit use in the
approximate optimization cycles can lead to synchronization problems
with the optimizer.

Once computed, the value for :math:`\rho^k` can be used to define the
step acceptance and the next trust region size :math:`\Delta^{k+1}`
using logic similar to that shown in Table `1.2 <#tab:rho_k>`__. Typical
factors for shrinking and expanding are 0.5 and 2.0, respectively, but
these as well as the threshold ratio values are tunable parameters in
the algorithm (see Surrogate-Based Method controls in the Dakota
Reference Manual :cite:p:`RefMan`). In addition, the use of
discrete thresholds is not required, and continuous relationships using
adaptive logic can also be explored :cite:p:`Wuj98a,Wuj98b`.
Iterate acceptance or rejection completes an SBO cycle, and the cycles
are continued until either soft or hard convergence criteria (see
Section `1.3 <#sbm:sblm_con_hard>`__) are satisfied.

.. container::
   :name: tab:rho_k

   .. table:: Sample trust region ratio logic.

      +----------------+----------------+----------------+----------------+
      | Ratio Value    | Surrogate      | Iterate        | Trust Region   |
      |                | Accuracy       | Acceptance     | Sizing         |
      +================+================+================+================+
      | :math:`\rho^k  | poor           | reject step    | shrink         |
      | \le 0`         |                |                |                |
      +----------------+----------------+----------------+----------------+
      | :math:`0 < \r  | marginal       | accept step    | shrink         |
      | ho^k \le 0.25` |                |                |                |
      +----------------+----------------+----------------+----------------+
      | :math:`0.25 <  | moderate       | accept step    | retain         |
      | \rho^k < 0.75` |                |                |                |
      | or             |                |                |                |
      | :math:`        |                |                |                |
      | \rho^k > 1.25` |                |                |                |
      +----------------+----------------+----------------+----------------+
      | :mat           | good           | accept step    | expand         |
      | h:`0.75 \le \r |                |                |                |
      | ho^k \le 1.25` |                |                |                |
      +----------------+----------------+----------------+----------------+

.. _`sbm:sblm_con_iter`:

Iterate acceptance logic
------------------------

TODO: Figure, image, and reference/caption.

.. container:: wrapfigure

   r.3 |image|

When a surrogate optimization is completed and the approximate solution
has been validated, then the decision must be made to either accept or
reject the step. The traditional approach is to base this decision on
the value of the trust region ratio, as outlined previously in
Table `1.2 <#tab:rho_k>`__. An alternate approach is to utilize a filter
method :cite:p:`Fle02`, which does not require penalty
parameters or Lagrange multiplier estimates. The basic idea in a filter
method is to apply the concept of Pareto optimality to the objective
function and constraint violations and only accept an iterate if it is
not dominated by any previous iterate. Mathematically, a new iterate is
not dominated if at least one of the following:

.. math::

   {\rm either~~~} f < f^{(i)} {\rm ~~~or~~~} c < c^{(i)}
   %  if (new_f >= filt_f && new_g >= filt_g)
   %    return false;            // new point is dominated: reject iterate
   %  else if (new_f < filt_f && new_g < filt_g)
   %    rm_list.insert(filt_it); // old pt dominated by new: queue for removal

is true for all :math:`i` in the filter, where :math:`c` is a selected
norm of the constraint violation. This basic description can be
augmented with mild requirements to prevent point accumulation and
assure convergence, known as a slanting
filter :cite:p:`Fle02`. Figure `[fig:filter] <#fig:filter>`__
illustrates the filter concept, where objective values are plotted
against constraint violation for accepted iterates (blue circles) to
define the dominated region (denoted by the gray lines). A filter method
relaxes the common enforcement of monotonicity in constraint violation
reduction and, by allowing more flexibility in acceptable step
generation, often allows the algorithm to be more efficient.

The use of a filter method is compatible with any of the SBO
formulations in
Eqs. `[eq:NLP_SBO_TRAL] <#eq:NLP_SBO_TRAL>`__–`[eq:NLP_SBO_direct] <#eq:NLP_SBO_direct>`__.

.. _`sbm:sblm_con_merit`:

Merit functions
---------------

The merit function :math:`\Phi({\bf x})` used in
Eqs. `[eq:NLP_SBO_TRAL] <#eq:NLP_SBO_TRAL>`__-`[eq:NLP_SBO_SQP] <#eq:NLP_SBO_SQP>`__,\ `[eq:rho_phi_k] <#eq:rho_phi_k>`__
may be selected to be a penalty function, an adaptive penalty function,
a Lagrangian function, or an augmented Lagrangian function. In each of
these cases, the more flexible inequality and equality constraint
formulations with two-sided bounds and targets
(Eqs. `[eq:NLP_standard] <#eq:NLP_standard>`__,\ `[eq:NLP_SBO_TRAL2] <#eq:NLP_SBO_TRAL2>`__-`[eq:NLP_SBO_direct] <#eq:NLP_SBO_direct>`__),
have been converted to a standard form of :math:`{\bf g}({\bf x}) \le 0`
and :math:`{\bf h}({\bf x}) = 0` (in
Eqs. `[eq:penalty_merit] <#eq:penalty_merit>`__,\ `[eq:lag_merit] <#eq:lag_merit>`__-`[eq:lls_lambda] <#eq:lls_lambda>`__).
The active set of inequality constraints is denoted as
:math:`{\bf g}^+`.

The penalty function employed in this paper uses a quadratic penalty
with the penalty schedule linked to SBO iteration number

.. math::

   \begin{aligned}
   \Phi({\bf x}, r_p) & = & f({\bf x})
   %+ \sum_{i=1}^{n_g} r_p (g_i^+({\bf x}))^2
   %+ \sum_{i=1}^{n_h} r_p (h_i^+({\bf x}))^2
   + r_p {\bf g}^+({\bf x})^T {\bf g}^+({\bf x})
   + r_p {\bf h}({\bf x})^T {\bf h}({\bf x}) \label{eq:penalty_merit} \\
   r_p & = & e^{(k + {\rm offset})/10} % static offset = 21 gives r_p ~ 8 for k = 0
   \label{eq:exp_rp}\end{aligned}

The adaptive penalty function is identical in form to
Eq. `[eq:penalty_merit] <#eq:penalty_merit>`__, but adapts :math:`r_p`
using monotonic increases in the iteration offset value in order to
accept any iterate that reduces the constraint violation.

The Lagrangian merit function is

.. math::

   \Phi({\bf x}, \mbox{\boldmath $\lambda$}_g, \mbox{\boldmath
   $\lambda$}_h) = f({\bf x})
   %+ \sum_{i=1}^{n_g} (\lambda_i g_i({\bf x})
   %+ \sum_{i=1}^{n_h} (\lambda_i h_i({\bf x})
   + \mbox{\boldmath $\lambda$}_g^T {\bf g}^+({\bf x})
   + \mbox{\boldmath $\lambda$}_h^T {\bf h}({\bf x}) \label{eq:lag_merit}

for which the Lagrange multiplier estimation is discussed in
Section `1.3 <#sbm:sblm_con_hard>`__. Away from the optimum, it is
possible for the least squares estimates of the Lagrange multipliers for
active constraints to be zero, which equates to omitting the
contribution of an active constraint from the merit function. This is
undesirable for tracking SBO progress, so usage of the Lagrangian merit
function is normally restricted to approximate subproblems and hard
convergence assessments.

The augmented Lagrangian employed in this paper follows the sign
conventions described in :cite:p:`Van84`

.. math::

   \begin{aligned}
   \Phi({\bf x}, \mbox{\boldmath $\lambda$}_{\psi}, \mbox{\boldmath
   $\lambda$}_h, r_p) & = & f({\bf x})
   %+ \sum_{i=1}^{n_g} (\lambda_i g_i({\bf x}) + r_p (g_i^+({\bf x}))^2)
   %+ \sum_{i=1}^{n_h} (\lambda_i h_i({\bf x}) + r_p (h_i^+({\bf x}))^2)
   + \mbox{\boldmath $\lambda$}_{\psi}^T \mbox{\boldmath $\psi$}({\bf x})
   + r_p \mbox{\boldmath $\psi$}({\bf x})^T \mbox{\boldmath $\psi$}({\bf x})
   + \mbox{\boldmath $\lambda$}_h^T {\bf h}({\bf x})
   + r_p {\bf h}({\bf x})^T {\bf h}({\bf x}) \label{eq:aug_lag_merit} \\
   \psi_i & = & \max\left\{g_i, -\frac{\lambda_{\psi_i}}{2r_p}\right\}
   \label{eq:aug_lag_psi}\end{aligned}

where :math:`\psi`\ (**x**) is derived from the elimination of slack
variables for the inequality constraints. In this case, simple
zeroth-order Lagrange multiplier updates may be used:

.. math::

   \begin{aligned}
   \mbox{\boldmath $\lambda$}_{\psi}^{k+1} & = & \mbox{\boldmath
   $\lambda$}_{\psi}^k + 2r_p\mbox{\boldmath $\psi$}({\bf x})
   \label{eq:lambda_psi} \\ 
   \mbox{\boldmath $\lambda$}_h^{k+1} & = & \mbox{\boldmath $\lambda$}_h^k 
   + 2 r_p {\bf h}({\bf x})
   \label{eq:lambda_h}\end{aligned}

The updating of multipliers and penalties is carefully
orchestrated :cite:p:`Con00` to drive reduction in constraint
violation of the iterates. The penalty updates can be more conservative
than in Eq. `[eq:exp_rp] <#eq:exp_rp>`__, often using an infrequent
application of a constant multiplier rather than a fixed exponential
progression.

.. _`sbm:sblm_con_hard`:

Convergence assessment
----------------------

To terminate the SBO process, hard and soft convergence metrics are
monitored. It is preferable for SBO studies to satisfy hard convergence
metrics, but this is not always practical (e.g., when gradients are
unavailable or unreliable). Therefore, simple soft convergence criteria
are also employed which monitor for diminishing returns (relative
improvement in the merit function less than a tolerance for some number
of consecutive iterations).

To assess hard convergence, one calculates the norm of the projected
gradient of a merit function whenever the feasibility tolerance is
satisfied. The best merit function for this purpose is the Lagrangian
merit function from Eq. `[eq:lag_merit] <#eq:lag_merit>`__. This
requires a least squares estimation for the Lagrange multipliers that
best minimize the projected gradient:

.. math::

   \nabla_x \Phi({\bf x}, \mbox{\boldmath $\lambda$}_g, \mbox{\boldmath
   $\lambda$}_h) = \nabla_x f({\bf x})
   %+ \sum_{i=1}^{n_g} (\lambda_i g_i({\bf x})
   %+ \sum_{i=1}^{n_h} (\lambda_i h_i({\bf x})
   + \mbox{\boldmath $\lambda$}_g^T \nabla_x {\bf g}^+({\bf x}) +
   \mbox{\boldmath $\lambda$}_h^T \nabla_x {\bf h}({\bf x})
   \label{eq:lag_merit_grad}

where gradient portions directed into active global variable bounds have
been removed. This can be posed as a linear least squares problem for
the multipliers:

.. math:: {\bf A} \mbox{\boldmath $\lambda$} = -\nabla_x f \label{eq:lls_lambda}

where :math:`{\bf A}` is the matrix of active constraint gradients,
:math:`\mbox{\boldmath $\lambda$}_g` is constrained to be non-negative,
and :math:`\mbox{\boldmath $\lambda$}_h` is unrestricted in sign. To
estimate the multipliers using non-negative and bound-constrained linear
least squares, the NNLS and BVLS routines :cite:p:`Law74` from
NETLIB are used, respectively.

.. _`sbm:sblm_con_relax`:

Constraint relaxation
---------------------

The goal of constraint relaxation is to achieve efficiency through the
balance of feasibility and optimality when the trust region restrictions
prevent the location of feasible solutions to constrained approximate
subproblems
(Eqs. `[eq:NLP_SBO_TRAL2] <#eq:NLP_SBO_TRAL2>`__-`[eq:NLP_SBO_direct] <#eq:NLP_SBO_direct>`__,
and other formulations from rows 2-3 in
Table `1.1 <#tab:sbo_subprob>`__). The SBO algorithm starting from
infeasible points will commonly generate iterates which seek to satisfy
feasibility conditions without regard to objective
reduction :cite:p:`Per04b`.

One approach for achieving this balance is to use *relaxed constraints*
when iterates are infeasible with respect to the surrogate constraints.
We follow Perez, Renaud, and Watson :cite:p:`Per04a`, and use
a *global homotopy* mapping the relaxed constraints and the surrogate
constraints. For formulations in
Eqs. `[eq:NLP_SBO_TRAL2] <#eq:NLP_SBO_TRAL2>`__
and `[eq:NLP_SBO_direct] <#eq:NLP_SBO_direct>`__ (and others from row 3
in Table `1.1 <#tab:sbo_subprob>`__), the relaxed constraints are
defined from

.. math::

   \begin{aligned}
   {\bf {\tilde g}}^k({\bf x}, \tau) &=& {\bf {\hat g}}^k({\bf x}) + 
   (1-\tau){\bf b}_{g} \label{eq:relaxed_ineq}\\
   {\bf {\tilde h}}^k({\bf x}, \tau) &=& {\bf {\hat h}}^k({\bf x}) + 
   (1-\tau){\bf b}_{h} \label{eq:relaxed_eq}\end{aligned}

For Eq. `[eq:NLP_SBO_SQP] <#eq:NLP_SBO_SQP>`__ (and others from row 2 in
Table `1.1 <#tab:sbo_subprob>`__), the original surrogate constraints
:math:`{\bf {\hat g}}^k({\bf x})` and :math:`{\bf {\hat h}}^k({\bf x})`
in
Eqs. `[eq:relaxed_ineq] <#eq:relaxed_ineq>`__-`[eq:relaxed_eq] <#eq:relaxed_eq>`__
are replaced with their linearized forms
(:math:`{\bf {\hat g}}^k({\bf x}^k_c) + 
\nabla {\bf {\hat g}}^k({\bf x}^k_c)^T ({\bf x} - {\bf x}^k_c)` and
:math:`{\bf {\hat h}}^k({\bf x}^k_c) + \nabla {\bf {\hat h}}^k({\bf x}^k_c)^T 
({\bf x} - {\bf x}^k_c)`, respectively). The approximate subproblem is
then reposed using the relaxed constraints as

.. math::

   \begin{aligned}
   {\rm minimize } & {\hat f^k}({\bf x})~~{\rm or}~~{\hat \Phi}^k({\bf x})
   \nonumber \\
   {\rm subject\  to } 
     & {\bf g}_l \le {\bf {\tilde g}}^k({\bf x},\tau^k) \le {\bf g}_u \nonumber \\
     &               {\bf {\tilde h}}^k({\bf x},\tau^k) = {\bf h}_t \nonumber \\
     & {\parallel {\bf x} - {\bf x}^k_c \parallel}_\infty \le \Delta^k
   % & {\bf x}_l \le {\bf x} \le {\bf x}_u \nonumber\\
   %  & 0 \le \tau \le 1 
   \label{eq:NLP_relaxed}\end{aligned}

in place of the corresponding subproblems in
Eqs. `[eq:NLP_SBO_TRAL2] <#eq:NLP_SBO_TRAL2>`__-`[eq:NLP_SBO_direct] <#eq:NLP_SBO_direct>`__.
Alternatively, since the relaxation terms are constants for the
:math:`k^{th}` iteration, it may be more convenient for the
implementation to constrain :math:`{\bf {\hat g}}^k({\bf x})` and
:math:`{\bf {\hat h}}^k({\bf x})` (or their linearized forms) subject to
relaxed bounds and targets (:math:`{\bf {\tilde g}}_l^k`,
:math:`{\bf {\tilde g}}_u^k`, :math:`{\bf {\tilde h}}_t^k`). The
parameter :math:`\tau` is the homotopy parameter controlling the extent
of the relaxation: when :math:`\tau=0`, the constraints are fully
relaxed, and when :math:`\tau=1`, the surrogate constraints are
recovered. The vectors :math:`{\bf b}_{g}, {\bf b}_{h}` are chosen so
that the starting point, :math:`{\bf x}^0`, is feasible with respect to
the fully relaxed constraints:

.. math::

   \begin{aligned}
   &{\bf g}_l \le {\bf {\tilde g}}^0({\bf x}^0, 0) \le {\bf g}_u \\
   &{\bf {\tilde h}}^0({\bf x}^0, 0) =  {\bf h}_t\end{aligned}

At the start of the SBO algorithm, :math:`\tau^0=0` if :math:`{\bf x}^0`
is infeasible with respect to the unrelaxed surrogate constraints;
otherwise :math:`\tau^0=1` (i.e., no constraint relaxation is used). At
the start of the :math:`k^{th}` SBO iteration where
:math:`\tau^{k-1} < 1`, :math:`\tau^k` is determined by solving the
subproblem

.. math::

   \begin{aligned}
   {\rm maximize } & \tau^k \nonumber \\
   {\rm subject\  to } 
     & {\bf g}_l \le {\bf {\tilde g}}^k({\bf x},\tau^k) \le {\bf g}_u \nonumber \\
     &               {\bf {\tilde h}}^k({\bf x},\tau^k) = {\bf h}_t \nonumber \\
     & {\parallel {\bf x} - {\bf x}^k_c \parallel}_\infty \le \Delta^k \nonumber\\
   % & {\bf x}_l \le {\bf x} \le {\bf x}_u \nonumber\\
     & \tau^k \ge 0 \label{eq:tau_max}\end{aligned}

starting at :math:`({\bf x}^{k-1}_*, \tau^{k-1})`, and then adjusted as
follows:

.. math::

   \tau^k = \min\left\{1,\tau^{k-1} + \alpha
   \left(\tau^{k}_{\max}-\tau^{k-1}\right)\right\}

The adjustment parameter :math:`0 < \alpha < 1` is chosen so that that
the feasible region with respect to the relaxed constraints has positive
volume within the trust region. Determining the optimal value for
:math:`\alpha` remains an open question and will be explored in future
work.

After :math:`\tau^k` is determined using this procedure, the problem in
Eq. `[eq:NLP_relaxed] <#eq:NLP_relaxed>`__ is solved for
:math:`{\bf x}^k_\ast`. If the step is accepted, then the value of
:math:`\tau^k` is updated using the current iterate
:math:`{\bf x}^k_\ast` and the validated constraints
:math:`{\bf g}({\bf x}^k_\ast)` and :math:`{\bf h}({\bf x}^k_\ast)`:

.. math::

   \begin{aligned}
   \tau^{k} & = \min\left\{1,\min_i \tau_i , \min_j \tau_j \right\} \\
   \rm{where}~~
   \tau_i & = 1 + \frac{\min \left\{g_i({\bf x}^k_\ast) - g_{l_{i}}, 
   g_{u_{i}} - g_i({\bf x}^k_\ast)\right\}}{b_{g_{i}}} \\ 
   \tau_j & = 1 - \frac{| h_j({\bf x}^k_\ast) - h_{t_{j}} |}{b_{h_{j}}}\end{aligned}

TODO: Figure, image, and reference/caption.

.. container:: wrapfigure

   r.35 |image1|

Figure `[fig:constr_relax] <#fig:constr_relax>`__ illustrates the SBO
algorithm on a two-dimensional problem with one inequality constraint
starting from an infeasible point, :math:`{\bf x}^0`. The minimizer of
the problem is denoted as :math:`{\bf x}^*`. Iterates generated using
the surrogate constraints are shown in red, where feasibility is
achieved first, and then progress is made toward the optimal point. The
iterates generated using the relaxed constraints are shown in blue,
where a balance of satisfying feasibility and optimality has been
achieved, leading to fewer overall SBO iterations.

The behavior illustrated in
Fig. `[fig:constr_relax] <#fig:constr_relax>`__ is an example where
using the relaxed constraints over the surrogate constraints may improve
the overall performance of the SBO algorithm by reducing the number of
iterations performed. This improvement comes at the cost of solving the
minimization subproblem in Eq. `[eq:tau_max] <#eq:tau_max>`__, which can
be significant in some cases (i.e., when the cost of evaluating
:math:`{\bf {\hat g}}^k({\bf x})` and :math:`{\bf {\hat h}}^k({\bf x})`
is not negligible, such as with multifidelity or ROM surrogates). As
shown in the numerical experiments involving the Barnes problem
presented in  :cite:p:`Per04a`, the directions toward
constraint violation reduction and objective function reduction may be
in opposing directions. In such cases, the use of the relaxed
constraints may result in an *increase* in the overall number of SBO
iterations since feasibility must ultimately take precedence.

.. [1]
   Any linear constraints are not approximated and may be added without
   modification to all formulations

.. |image| image:: img/filter.png
.. |image1| image:: img/tau_updates.png