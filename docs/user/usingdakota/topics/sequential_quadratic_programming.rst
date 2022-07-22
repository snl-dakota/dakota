.. _`topic-sequential_quadratic_programming`:

Sequential Quadratic Programming (SQP)
======================================

Sequential Quadratic Programming (SQP) algorithms are a class of mathematical programming problems used to solve nonlinear optimization problems with nonlinera constraints.
These methods are a generalization of Newton's method:  each iteration involves minimizing a quadratic model of the problem.
These subproblems are formulated as minimizing a quadratic approximation of the
Lagrangian subject to linearized constraints.
Only gradient information is required; Hessians are approximated by low-rank updates
defined by the step taken at each iteration.
It is important to note that while the solution found by an SQP method
will respect the constraints, the intermediate iterates may not.
SQP methods available in Dakota are ``dot_sqp``, ``nlpql_sqp``, ``nlssol_sqp``, and
``npsol_sqp``.
The particular implementation in ``nlpql_sqp`` uses a variant with
distributed and non-monotone line search.
Thus, this variant is designed to be more robust in the presence of
inaccurate or noisy gradients common in many engineering applications.