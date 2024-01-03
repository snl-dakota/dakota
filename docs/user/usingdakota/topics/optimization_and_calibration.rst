.. _`topic-optimization_and_calibration`:

Optimization and Calibration
============================

Optimization algorithms work to minimize (or maximize) an objective
function, typically calculated by the user simulation code, subject to
constraints on design variables and responses. Available approaches in
Dakota include well-tested, proven gradient-based, derivative-free
local, and global methods for use in science and engineering design
applications. Dakota also offers more advanced algorithms, e.g., to
manage multi-objective optimization or perform surrogate-based
minimization.  This chapter summarizes optimization problem
formulation, standard algorithms available in Dakota (mostly through
included third-party libraries, see :ref:`opt:libraries`), some
advanced capabilities, and offers usage guidelines.

.. _`OptFormulations`:

Optimization Formulations
-------------------------

This section provides a basic introduction to the mathematical
formulation of optimization, problems. The primary goal of this
section is to introduce terms relating to these topics, and is not
intended to be a description of theory or numerical algorithms. For
further details,
consult :cite:p:`Aro89,Gil81,Haf92,Noc99,Van84`.

A general optimization problem is formulated as follows:

.. math::

  {eqnarray}{
  \hbox{minimize:} & & f(\mathbf{x})\nonumber\\
  & & \mathbf{x} \in \Re^{n}\nonumber\\
  \hbox{subject to:} & &
  \mathbf{g}_{L} \leq \mathbf{g(x)} \leq \mathbf{g}_U\nonumber\\
  & & \mathbf{h(x)}=\mathbf{h}_{t}\label{opt:formulations:equation01}\\
  & & \mathbf{a}_{L} \leq \mathbf{A}_i\mathbf{x} \leq
  \mathbf{a}_U\nonumber\\
  & & \mathbf{A}_{e}\mathbf{x}=\mathbf{a}_{t}\nonumber\\
  & & \mathbf{x}_{L} \leq \mathbf{x} \leq \mathbf{x}_U\nonumber
  }

where vector and matrix terms are marked in bold typeface. In this
formulation, :math:`\mathbf{x}=[x_{1},x_{2},\ldots,x_{n}]`  is an
n-dimensional vector of real-valued *design variables* or
*design parameters*. The n-dimensional vectors, :math:`\mathbf{x}_{L}` 
and :math:`\mathbf{x}_U` , are the lower and upper bounds, respectively, on
the design parameters. These bounds define the allowable values for
the elements of :math:`\mathbf{x}` , and the set of all allowable values is
termed the *design space* or the *parameter space*. A
*design point* or a *sample point* is a particular set of
values within the parameter space.

The optimization goal is to minimize the *objective function*,
:math:`f(\mathbf{x})` , while satisfying the constraints. Constraints can be
categorized as either linear or nonlinear and as either inequality or
equality. The *nonlinear inequality constraints}*,
:math:`\mathbf{g(x)}` , are ''2-sided,'' in that they have both lower and
upper bounds, :math:`\mathbf{g}_L`  and :math:`\mathbf{g}_U` , respectively. The
*nonlinear equality constraints*, :math:`\mathbf{h(x)}` , have target
values specified by :math:`\mathbf{h}_{t}` . The linear inequality
constraints create a linear system :math:`\mathbf{A}_i\mathbf{x}` , where
:math:`\mathbf{A}_i`  is the coefficient matrix for the linear system. These
constraints are also 2-sided as they have lower and upper bounds,
:math:`\mathbf{a}_L`  and :math:`\mathbf{a}_U` , respectively. The linear equality
constraints create a linear system :math:`\mathbf{A}_e\mathbf{x}` , where
:math:`\mathbf{A}_e`  is the coefficient matrix for the linear system and
:math:`\mathbf{a}_{t}`  are the target values. The constraints partition the
parameter space into feasible and infeasible regions. A design point
is said to be *feasible* if and only if it satisfies all of the
constraints. Correspondingly, a design point is said to be
*infeasible* if it violates one or more of the constraints.

Many different methods exist to solve the optimization problem given
in :ref:`opt:formulations`, all of which iterate on
:math:`\mathbf{x}`  in some manner. That is, an initial value for each
parameter in :math:`\mathbf{x}`  is chosen, the *response quantities*,
:math:`f(\mathbf{x})` , :math:`\mathbf{g(x)}` , :math:`\mathbf{h(x)}` , are computed, often
by running a simulation, and some algorithm is applied to generate a
new :math:`\mathbf{x}`  that will either reduce the objective function,
reduce the amount of infeasibility, or both. To facilitate a general
presentation of these methods, three criteria will be used in the
following discussion to differentiate them: optimization problem type,
search goal, and search method.

The *optimization problem type* can be characterized both by
the types of constraints present in the problem and by the linearity
or nonlinearity of the objective and constraint functions. For
constraint categorization, a hierarchy of complexity exists for
optimization algorithms, ranging from simple bound constraints,
through linear constraints, to full nonlinear constraints. By the
nature of this increasing complexity, optimization problem
categorizations are inclusive of all constraint types up to a
particular level of complexity. That is, an *unconstrained
problem* has no constraints, a *bound-constrained problem* has
only lower and upper bounds on the design parameters, a
*linearly-constrained problem* has both linear and bound
constraints, and a *nonlinearly-constrained problem* may contain
the full range of nonlinear, linear, and bound constraints. If all of
the linear and nonlinear constraints are equality constraints, then
this is referred to as an *equality-constrained problem*, and if
all of the linear and nonlinear constraints are inequality
constraints, then this is referred to as an
*inequality-constrained problem*. Further categorizations can be
made based on the linearity of the objective and constraint functions.
A problem where the objective function and all constraints are linear
is called a *linear programming (LP) problem*. These types of
problems commonly arise in scheduling, logistics, and resource
allocation applications. Likewise, a problem where at least some of
the objective and constraint functions are nonlinear is called a
*nonlinear programming (NLP) problem*. These NLP problems
predominate in engineering applications and are the primary focus of
Dakota.

The *search goal* refers to the ultimate objective of the
optimization algorithm, i.e., either global or local optimization. In
*global optimization*, the goal is to find the design point that
gives the lowest feasible objective function value over the entire
parameter space. In contrast, in *local optimization*, the goal
is to find a design point that is lowest relative to a ''nearby''
region of the parameter space. In almost all cases, global
optimization will be more computationally expensive than local
optimization. Thus, the user must choose an optimization algorithm
with an appropriate search scope that best fits the problem goals and
the computational budget.

The *search method* refers to the approach taken in the
optimization algorithm to locate a new design point that has a lower
objective function or is more feasible than the current design point.
The search method can be classified as either *gradient-based* or
*nongradient-based*. In a gradient-based algorithm, gradients of
the response functions are computed to find the direction of
improvement. Gradient-based optimization is the search method that
underlies many efficient local optimization methods. However, a
drawback to this approach is that gradients can be computationally
expensive, inaccurate, or even nonexistent. In such situations,
nongradient-based search methods may be useful. There are numerous
approaches to nongradient-based optimization. Some of the more well
known of these include pattern search methods (nongradient-based local
techniques) and genetic algorithms (nongradient-based global
techniques).

Because of the computational cost of running simulation
models, surrogate-based optimization (SBO) methods are often used to
reduce the number of actual simulation runs. In SBO, a surrogate or
approximate model is constructed based on a limited number of
simulation runs. The optimization is then performed on the surrogate
model. Dakota has an extensive framework for managing a variety of
local, multipoint, global, and hierarchical surrogates for use in
optimization. Finally, sometimes there are multiple objectives that
one may want to optimize simultaneously instead of a single scalar
objective.  In this case, one may employ multi-objective methods
that are described in :ref:`opt:additional:multiobjective`.

This overview of optimization approaches underscores that no single
optimization method or algorithm works best for all types of
optimization problems. :ref:`opt:usage` offers guidelines for
choosing a Dakota optimization algorithm best matched to your specific
optimization problem.

.. _`OptFormulationsConstraints`:

Constraint Considerations
^^^^^^^^^^^^^^^^^^^^^^^^^

Dakota's input commands permit the user to specify two-sided nonlinear
inequality constraints of the form :math:`g_{L_{i}} \leq g_{i}(\mathbf{x})
\leq g_{U_{i}}` , as well as nonlinear equality constraints of the form
:math:`h_{j}(\mathbf{x}) = h_{t_{j}}` . Some optimizers (e.g.,
``npsol_``, ``optpp_``, ``soga``, and ``moga``
methods) can handle these constraint forms directly, whereas other
optimizers (e.g., ``asynch_pattern_search``, ``dot_``,
and ``conmin_``, ``mesh_adaptive_search``) require Dakota
to perform an internal conversion of all constraints to one-sided
inequality constraints of the form :math:`g_{i}(\mathbf{x}) \leq 0` . In the
latter case, the two-sided inequality constraints are treated as
:math:`g_{i}(\mathbf{x}) - g_{U_{i}} \leq 0`  and :math:`g_{L_{i}} -
g_{i}(\mathbf{x}) \leq 0`  and the equality constraints are treated as
:math:`h_{j}(\mathbf{x}) - h_{t_{j}} \leq 0`  and :math:`h_{t_{j}} -
h_{j}(\mathbf{x}) \leq 0` . The situation is similar for linear
constraints: ``asynch_pattern_search``, ``npsol_``,
``optpp_``, ``soga``, and ``moga`` methods support
them directly, whereas ``dot_`` and ``conmin_`` methods do
not. For linear inequalities of the form :math:`a_{L_{i}} \leq
\mathbf{a}_{i}^{T}\mathbf{x} \leq a_{U_{i}}`  and linear equalities of
the form :math:`\mathbf{a}_{i}^{T}\mathbf{x} = a_{t_{j}}` , the nonlinear
constraint arrays in ``dot_`` and ``conmin_`` methods are
further augmented to include :math:`\mathbf{a}_{i}^{T}\mathbf{x} - a_{U_{i}}
\leq 0`  and :math:`a_{L_{i}} - \mathbf{a}_{i}^{T}\mathbf{x} \leq 0`  in the
inequality case and :math:`\mathbf{a}_{i}^{T}\mathbf{x} - a_{t_{j}} \leq 0` 
and :math:`a_{t_{j}} - \mathbf{a}_{i}^{T}\mathbf{x} \leq 0`  in the equality
case. Awareness of these constraint augmentation procedures can be
important for understanding the diagnostic data returned from the
``dot_`` and ``conmin_`` methods. Other optimizers fall
somewhere in between.  ``nlpql_`` methods support nonlinear
equality constraints :math:`h_{j}(\mathbf{x}) = 0`  and nonlinear one-sided
inequalities :math:`g_{i}(\mathbf{x}) \geq 0` , but does not natively support
linear constraints. Constraint mappings are used with NLPQL for both
linear and nonlinear cases. Most ``coliny_`` methods now support
two-sided nonlinear inequality constraints and nonlinear constraints
with targets, but do not natively support linear constraints.

When gradient and Hessian information is used in the optimization,
derivative components are most commonly computed with respect to the
active continuous variables, which in this case are the
*continuous design variables*. This differs from parameter study
methods (for which all continuous variables are active) and from
nondeterministic analysis methods (for which the uncertain variables
are active). Refer to Chapter 11 of the User's Manual for additional
information on derivative components and active continuous variables.

.. _`OptMethods`:

Optimizing with Dakota: Choosing a Method
-----------------------------------------

This section summarizes the optimization methods available in
Dakota. We group them according to search method and search goal and
establish their relevance to types of problems. For a summary of this
discussion, see :ref:`opt:usage`.

.. _`OptMethodsGradient`:

Gradient-Based Local Methods
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Gradient-based optimizers are best suited for efficient navigation to
a local minimum in the vicinity of the initial point.  They are not
intended to find global optima in nonconvex design spaces.  For global
optimization methods, see :ref:`opt:methods:gradientfree:global`.
Gradient-based optimization methods are highly efficient, with the
best convergence rates of all of the local optimization methods, and
are the methods of choice when the problem is smooth, unimodal, and
well-behaved. However, these methods can be among the least robust
when a problem exhibits nonsmooth, discontinuous, or multimodal
behavior.  The derivative-free methods described
in :ref:`opt:methods:gradientfree:local` are more appropriate for
problems with these characteristics.

Gradient accuracy is a critical factor for gradient-based optimizers,
as inaccurate derivatives will often lead to failures in the search or
pre-mature termination of the method.  Analytic gradients and Hessians
are ideal but often unavailable.  If analytic gradient and Hessian
information can be provided by an application code, a full Newton
method will achieve quadratic convergence rates near the solution. If
only gradient information is available and the Hessian information is
approximated from an accumulation of gradient data, the superlinear
convergence rates can be obtained.  It is most often the case for
engineering applications, however, that a finite difference method
will be used by the optimization algorithm to estimate gradient
values. Dakota allows the user to select the step size for these
calculations, as well as choose between forward-difference and
central-difference algorithms. The finite difference step size should
be selected as small as possible, to allow for local accuracy and
convergence, but not so small that the steps are ''in the noise.''
This requires an assessment of the local smoothness of the response
functions using, for example, a parameter study method. Central
differencing will generally produce more reliable gradients than
forward differencing but at roughly twice the expense.

Gradient-based methods for nonlinear optimization problems can be
described as iterative processes in which a sequence of subproblems,
usually which involve an approximation to the full nonlinear problem,
are solved until the solution converges to a local optimum of the full
problem.  The optimization methods available in Dakota fall into
several categories, each of which is characterized by the nature of
the subproblems solved at each iteration.
