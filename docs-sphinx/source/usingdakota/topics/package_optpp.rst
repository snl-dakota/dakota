.. _`topic-package_optpp`:

Package: OPT++
==============

The OPT++ library :cite:p:`MeOlHoWi07` contains primarily
gradient-based nonlinear programming optimizers for unconstrained,
bound-constrained, and nonlinearly constrained minimization:
Polak-Ribiere conjugate gradient (Dakota's ``optpp_cg`` method),
quasi-Newton (Dakota's ``optpp_q_newton`` method), finite difference
Newton (Dakota's ``optpp_fd_newton`` method), and full Newton (Dakota's
``optpp_newton`` method).

The conjugate gradient method is strictly
unconstrained, and each of the Newton-based methods are automatically
bound to the appropriate OPT++ algorithm based on the user constraint
specification (unconstrained, bound-constrained, or
generally-constrained). In the generally-constrained case, the Newton
methods use a nonlinear interior-point approach to manage the
constraints.
The library also contains a direct search algorithm, PDS
(parallel direct search, Dakota's ``optpp_pds`` method), which supports
bound constraints.

*Controls*
 -# ``max_iterations``
 -# ``max_function_evaluations``
 -# ``convergence_tolerance``
 -# ``output``
 -# ``speculative``



*Concurrency*

OPT++'s gradient-based methods are not parallel algorithms and
cannot directly take advantage of concurrent function
evaluations. However, if ``numerical_gradients`` with ``method_source``
``dakota`` is specified, a parallel Dakota configuration can utilize
concurrent evaluations for the finite difference gradient
computations.

*Constraints*

Linear constraint specifications are supported by each of the
Newton methods ( ``optpp_newton``, ``optpp_q_newton``, ``optpp_fd_newton``,
and ``optpp_g_newton``)

``optpp_cg`` must be unconstrained

``optpp_pds`` can be, at most, bound-constrained.