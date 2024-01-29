.. _nls:

Nonlinear Least Squares
=======================

..
   TODO: discuss calibration overall, then NLS

.. _`nls:overview`:

Overview
--------

Any Dakota optimization algorithm can be applied to calibration problems
arising in parameter estimation, system identification, and
test/analysis reconciliation. However, nonlinear least-squares methods
are optimization algorithms that exploit the special structure of a sum
of the squares objective function :cite:p:`Gil81`.

To exploit the problem structure, more granularity is needed in the
response data than is required for a typical optimization problem. That
is, rather than using the sum-of-squares objective function and its
gradient, least-squares iterators require each term used in the
sum-of-squares formulation along with its gradient. This means that the
:math:`m` functions in the Dakota response data set consist of the
individual least-squares terms along with any nonlinear inequality and
equality constraints. These individual terms are often called
*residuals* when they denote differences of observed quantities from
values computed by the model whose parameters are being estimated.

The enhanced granularity needed for nonlinear least-squares algorithms
allows for simplified computation of an approximate Hessian matrix. In
Gauss-Newton-based methods for example, the true Hessian matrix is
approximated by neglecting terms in which residuals multiply Hessians
(matrices of second partial derivatives) of residuals, under the
assumption that the residuals tend towards zero at the solution. As a
result, residual function value and gradient information (first-order
information) is sufficient to define the value, gradient, and
approximate Hessian of the sum-of-squares objective function
(second-order information). See :ref:`formulations <nls:formulations>` for
additional details on this approximation.

In practice, least-squares solvers will tend to be significantly more
efficient than general-purpose optimization algorithms when the Hessian
approximation is a good one, e.g., when the residuals tend towards zero
at the solution. Specifically, they can exhibit the quadratic
convergence rates of full Newton methods, even though only first-order
information is used. Gauss-Newton-based least-squares solvers may
experience difficulty when the residuals at the solution are
significant. Dakota has three solvers customized to take advantage of
the sum of squared residuals structure in this problem formulation.
Least squares solvers may experience difficulty when the residuals at
the solution are significant, although experience has shown that
Dakota’s NL2SOL method can handle some problems that are highly
nonlinear and have nonzero residuals at the solution.

.. _`nls:formulations`:

Nonlinear Least Squares Fomulations
-----------------------------------

Specialized least squares solution algorithms can exploit the structure
of a sum of the squares objective function for problems of the form:

.. math::

   \begin{aligned}
     \hbox{minimize:} & & f(\mathbf{x}) =
     \sum_{i=1}^{n}[T_i(\mathbf{x})]^2\nonumber\\
     & & \mathbf{x} \in \Re^{p}\nonumber\\
     \hbox{subject to:} & &
     \mathbf{g}_L \leq \mathbf{g(x)} \leq \mathbf{g}_U\nonumber\\
     & & \mathbf{h(x)}=\mathbf{h}_{t}\label{nls:equation02}\\
     & & \mathbf{a}_L \leq \mathbf{A}_i\mathbf{x} \leq
     \mathbf{a}_U\nonumber\\
     & & \mathbf{A}_e\mathbf{x}=\mathbf{a}_{t}\nonumber\\
     & & \mathbf{x}_L \leq \mathbf{x} \leq \mathbf{x}_U\nonumber\end{aligned}

where :math:`f(\mathbf{x})` is the objective function to be minimized
and :math:`T_i(\mathbf{x})` is the i\ :math:`^{\mathrm{th}}` least
squares term. The bound, linear, and nonlinear constraints are the same
as described previously for  :math:numref:`optimformulation`, Optimization Formulations.
Specialized least squares algorithms are generally based on the
Gauss-Newton approximation. When differentiating :math:`f(\mathbf{x})`
twice, terms of :math:`T_i(\mathbf{x})T''_i(\mathbf{x})` and
:math:`[T'_i(\mathbf{x})]^{2}` result. By assuming that the former term
tends toward zero near the solution since :math:`T_i(\mathbf{x})` tends
toward zero, then the Hessian matrix of second derivatives of
:math:`f(\mathbf{x})` can be approximated using only first derivatives
of :math:`T_i(\mathbf{x})`. As a result, Gauss-Newton algorithms exhibit
quadratic convergence rates near the solution for those cases when the
Hessian approximation is accurate, i.e. the residuals tend towards zero
at the solution. Thus, by exploiting the structure of the problem, the
second order convergence characteristics of a full Newton algorithm can
be obtained using only first order information from the least squares
terms.

A common example for :math:`T_i(\mathbf{x})` might be the difference
between experimental data and model predictions for a response quantity
at a particular location and/or time step, i.e.:

.. math::

   T_i(\mathbf{x}) = R_i(\mathbf{x})-\bar{R_i}
     \label{nls:equation03}

where :math:`R_i(\mathbf{x})` is the response quantity predicted by the
model and :math:`\bar{R_i}` is the corresponding experimental data.
In this case, :math:`\mathbf{x}` would have the meaning of model
parameters which are not precisely known and are being calibrated to
match available data. This class of problem is known by the terms
parameter estimation, system identification, model calibration,
test/analysis reconciliation, etc.

Nonlinear Least Squares with Dakota
-----------------------------------

In order to specify a least-squares problem, the responses section of
the Dakota input should be configured using ``calibration_terms`` (as
opposed to ``objective_functions`` as for optimization). The calibration
terms refer to the residuals (differences between the simulation model
and the data). Note that Dakota expects the residuals, not the squared
residuals, and offers options for instead returning the simulation
output to Dakota together with a separate ``calibration_data`` file,
from which residuals will be calculated. Any linear or nonlinear
constraints are handled in an identical way to that of optimization (see
Section Optimization Formulations  :math:numref:`optimformulation` ; note that neither
Gauss-Newton nor NLSSOL require any constraint augmentation and NL2SOL
supports neither linear nor nonlinear constraints). Gradients of the
least-squares terms and nonlinear constraints are required and should be
specified using either ``numerical_gradients``, ``analytic_gradients``,
or ``mixed_gradients``. Since explicit second derivatives are not used
by the least-squares methods, the ``no_hessians`` specification should
be used. Dakota’s scaling options, described in
Section Optimization with User-specified or Automatic Scaling, :numref:`opt:additional:scaling:example:figure01` can be
used on least-squares problems, using the ``calibration_term_scales``
keyword to scale least-squares residuals, if desired.

.. _`nls:solution`:

Solution Techniques
-------------------

Nonlinear least-squares problems can be solved using the Gauss-Newton
algorithm, which leverages the full Newton method from OPT++, the NLSSOL
algorithm, which is closely related to NPSOL, or the NL2SOL algorithm,
which uses a secant-based algorithm. Details for each are provided
below.

.. _`nls:solution:gauss`:

Gauss-Newton
~~~~~~~~~~~~

Dakota’s Gauss-Newton algorithm consists of combining an implementation
of the Gauss-Newton Hessian approximation (see
Section  :ref:`Nonlinear Least Squares Fomulations <nls:formulations>`) with full Newton optimization
algorithms from the OPT++ package :cite:p:`MeOlHoWi07` (see
Section  :ref:`Methods for Constrained Problems <opt:methods:gradient:constrained>`).
The exact objective function value, exact objective function gradient,
and the approximate objective function Hessian are defined from the
least squares term values and gradients and are passed to the
full-Newton optimizer from the OPT++ software package. As for all of the
Newton-based optimization algorithms in OPT++, unconstrained,
bound-constrained, and generally-constrained problems are supported.
However, for the generally-constrained case, a derivative order mismatch
exists in that the nonlinear interior point full Newton algorithm will
require second-order information for the nonlinear constraints whereas
the Gauss-Newton approximation only requires first order information for
the least squares terms. License: LGPL.

This approach can be selected using the ``optpp_g_newton`` method
specification. An example specification follows:

::

       method,
             optpp_g_newton
               max_iterations = 50
               convergence_tolerance = 1e-4
               output debug

Refer to the Dakota Reference Manual :ref:`Keyword Reference <keyword-reference-area>` for more
detail on the input commands for the Gauss-Newton algorithm.

The Gauss-Newton algorithm is gradient-based and is best suited for
efficient navigation to a local least-squares solution in the vicinity
of the initial point. Global optima in multimodal design spaces may be
missed. Gauss-Newton supports bound, linear, and nonlinear constraints.
For the nonlinearly-constrained case, constraint Hessians (required for
full-Newton nonlinear interior point optimization algorithms) are
approximated using quasi-Newton secant updates. Thus, both the objective
and constraint Hessians are approximated using first-order information.

.. _`nls:solution:nlssol`:

NLSSOL
~~~~~~

The NLSSOL algorithm is bundled with NPSOL. It uses an SQP-based
approach to solve generally-constrained nonlinear least-squares
problems. It periodically employs the Gauss-Newton Hessian approximation
to accelerate the search. Like the Gauss-Newton algorithm of
Section  :ref:`Gauss-Newton <nls:solution:gauss>`), its derivative order is
balanced in that it requires only first-order information for the
least-squares terms and nonlinear constraints. License: commercial; see
NPSOL   :ref:`Methods for Constrained Problems <opt:methods:gradient:constrained>`.

This approach can be selected using the ``nlssol_sqp`` method
specification. An example specification follows:

::

       method,
             nlssol_sqp
               convergence_tolerance = 1e-8

Refer to the Dakota Reference Manual  :ref:`Keyword Reference <keyword-reference-area>` for more
detail on the input commands for NLSSOL.

.. _`nls:solution:nl2sol`:

NL2SOL
~~~~~~

The NL2SOL algorithm :cite:p:`Den81` is a secant-based
least-squares algorithm that is :math:`q`-superlinearly convergent. It
adaptively chooses between the Gauss-Newton Hessian approximation and
this approximation augmented by a correction term from a secant update.
NL2SOL tends to be more robust (than conventional Gauss-Newton
approaches) for nonlinear functions and “large residual” problems, i.e.,
least-squares problems for which the residuals do not tend towards zero
at the solution. License: publicly available.

.. _`nls:solution:future`:

Additional Features
~~~~~~~~~~~~~~~~~~~

Dakota's tailored derivative-based least squares solvers (but not
general optimization solvers) output confidence intervals on estimated
parameters. The reported confidence intervals are univariate
(per-parameter), based on local linearization, and will contain the
true value of the parameters with 95% confidence. Their calculation
essentially follows the exposition in :cite:p:`Seb03` and is
summarized here.

Denote the variance estimate at the optimal calibrated parameters
:math:`\hat{x}` by

.. math::

   \hat{\sigma}^2 = \frac{1}{N_{dof}}\sum_{i=1}^{n} T_i(\hat{x})^2,

where :math:`T_i` are the least squares terms (typically residuals)
discussed above and :math:`N_{dof} = n - p` denotes the number of degrees of
freedom (total residuals :math:`n` less the number of calibrated parameters
:math:`p`). Let

.. math::

   J = \left[ \frac{\partial T(\hat{x})}{\partial x} \right]

denote the :math:`n \times p` matrix of partial derivatives of the residuals
with respect to the calibrated paramters. Then the standard error :math:`SE_i`
for calibrated parameter :math:`x_i` is given by

.. math::

   SE_i = \hat{\sigma} \sqrt{\left( J^T J \right)^{-1}_{ii} }.

Using a Student's t-distribution with :math:`N_{dof}` degrees of freedom,
the 95% confidence interval for each parameter is given by

.. math::

   \hat{x}_i \pm t(0.975, N_{dof}) \cdot SE_i.

In the case where estimated gradients are extremely inaccurate or the
model is very nonlinear, the confidence intervals reported are likely
inaccurate as well.  Further, confidence intervals cannot be
calculated when the number of least-squares terms is less than the
number of parameters to be estimated, when using vendor numerical
gradients, or where there are replicate experiments.  See
:cite:p:`Vug07` for more details about confidence intervals, and note
that there are alternative approaches such as Bonferroni confidence
intervals and joint confidence intervals based on linear
approximations or F-tests.

Least squares calibration terms (responses) can be weighted. When
observation error variance is provided alongside calibration data, its
inverse is applied to yield the typical variance-weighted least squares
formulation. Alternately, the ``calibration_terms weights``
specification can be used to weight the squared residuals. (Neither set
of weights are adjusted during calibration as they would be in
iteratively re-weighted least squares.) When response scaling is active,
it is applied after error variance weighting and before ``weights``
application. The ``calibration_terms`` keyword documentation in the
Dakota Reference Manual  :ref:`Keyword Reference <keyword-reference-area>` has more detail about
weighting and scaling of the residual terms.

.. _`nls:examples`:

Examples
--------

Both the Rosenbrock and textbook example problems can be formulated as
nonlinear least-squares problems. Refer to :ref:`Additional Examples <additional>` for more information on these
formulations.

..
   TODO:
   %Figure~\ref{nls:figure01}
   %shows an excerpt from output for the Rosenbrock example solved by
   %the Gauss-Newton method.
   %
   %\begin{figure}
   %\begin{bigbox}
   %\begin{small}
   %\begin{verbatim}
   %     Active response data for function evaluation 1:
   %     Active set vector = { 3 3 } Deriv vars vector = { 1 2 }
   %                          -4.4000000000e+00 least_sq_term_1
   %                           2.2000000000e+00 least_sq_term_2
   %      [  2.4000000000e+01  1.0000000000e+01 ] least_sq_term_1 gradient
   %      [ -1.0000000000e+00  0.0000000000e+00 ] least_sq_term_2 gradient
   %\end{verbatim}
   %\end{small}
   %\end{bigbox}
   %\caption{Example of intermediate output from a Gauss-Newton method.}
   %\label{nls:figure01}
   %\end{figure}


Figure  :numref:`nls:nl2sol:example1:figure01` shows an excerpt from the
output obtained when running NL2SOL on a five-dimensional problem. Note
that the optimal parameter estimates are printed, followed by the
residual norm and values of the individual residual terms, followed by
the confidence intervals on the parameters.


.. code-block::
   :caption: Dakota results for the multiobjective optimization example.
   :name:  nls:nl2sol:example1:figure01

      ------------------------------
      
      <<<<< Iterator nl2sol completed.
      <<<<< Function evaluation summary: 27 total (26 new, 1 duplicate)
      <<<<< Best parameters          =
                         3.7541004764e-01 x1
                         1.9358463401e+00 x2
                        -1.4646865611e+00 x3
                         1.2867533504e-02 x4
                         2.2122702030e-02 x5
      <<<<< Best residual norm =  7.3924926090e-03; 0.5 * norm^2 =  2.7324473487e-05
      <<<<< Best residual terms      =
                        -2.5698266189e-03
                         4.4759880011e-03
                         9.9223430643e-04
                        -1.0634409194e-03

      ...

      Confidence Interval for x1 is [  3.7116510206e-01,  3.7965499323e-01 ]
      Confidence Interval for x2 is [  1.4845485507e+00,  2.3871441295e+00 ]
      Confidence Interval for x3 is [ -1.9189348458e+00, -1.0104382765e+00 ]
      Confidence Interval for x4 is [  1.1948590669e-02,  1.3786476338e-02 ]
      Confidence Interval for x5 is [  2.0289951664e-02,  2.3955452397e-02 ]



The analysis driver script (the script being driven by Dakota) has to
perform several tasks in the case of parameter estimation using
nonlinear least-squares methods. The analysis driver script must: (1)
read in the values of the parameters supplied by Dakota; (2) run the
computer simulation with these parameter values; (3) retrieve the
results from the computer simulation; (4) compute the difference between
each computed simulation value and the corresponding experimental or
measured value; and (5) write these residuals (differences) to an
external file that gets passed back to Dakota. Note there will be one
line per residual term, specified with ``calibration_terms`` in the
Dakota input file. It is the last two steps which are different from
most other Dakota applications.

To simplify specifying a least squares problem, one may provide Dakota a
data file containing experimental results or other calibration data. In
the case of scalar calibration terms, this file may be specified with .
In this case, Dakota will calculate the residuals (that is, the
simulation model results minus the experimental results), and the
user-provided script can omit this step: the script can just return the
simulation outputs of interest. An example of this can be found in the
file named ``dakota/share/dakota/examples/users/textbook_nls_datafile.in``.
In this example, there are 3 residual terms. The data file
of experimental results associated with this example is 
``textbook_nls_datafile.lsq.dat``. These three
values are subtracted from the least-squares terms to produce residuals
for the nonlinear least-squares problem. Note that the file may be
annotated (specified by ``annotated``) or freeform (specified by
``freeform``). The number of experiments in the calibration data file
may be specified with , with one row of data per experiment. When
multiple experiments are present, the total number of least squares
terms will be the number of calibration terms times the number of
experiments.

Finally, the calibration data file may contain additional information
than just the observed experimental responses. If the observed data has
measurement error associated with it, this can be specified in columns
of such error data after the response data. The type of measurement
error is specified by ``variance_type``. For scalar calibration terms,
the ``variance_type`` can be either ``none`` (the user does not specify
a measurement variance associated with each calibration term) or
``scalar`` (the user specifies one measurement variance per calibration
term). For field calibration terms, the ``variance_type`` can also be
``diagonal`` or ``matrix``. These are explained in more detail in the
Reference manual. See the :ref:`Keyword Reference <keyword-reference-area>`
for more information. Additionally, there is sometimes the need to specify
configuration variables. These are often used in Bayesian calibration
analysis. These are specified as ``num_config_variables``. If the user
specifies a positive number of configuration variables, it is expected
that they will occur in the text file before the responses.

.. _`nls:usage`:

Usage Guidelines
----------------

..
   TODO: borrow from and refer to opt.

Calibration problems can be transformed to general optimization problems
where the objective is some type of aggregated error metric. For
example, the objective could be the sum of squared error terms. However,
it also could be the mean of the absolute value of the error terms, the
maximum difference between the simulation results and observational
results, etc. In all of these cases, one can pose the calibration
problem as an optimization problem that can be solved by any of Dakota’s
optimizers. In this situation, when applying an general optimization
solver to a calibration problem, the guidelines in
Guide :numref:`opt:usage:guideopt` still apply.

In some cases, it will be better to use a nonlinear least-squares method
instead of a general optimizer to determine optimal parameter values
which result in simulation responses that “best fit” the observational
data. Nonlinear least squares methods exploit the special structure of a
sum of the squares objective function. They can be much more efficient
than general optimizers. However, these methods require the gradients of
the function with respect to the parameters being calibrated. If the
model is not able to produce gradients, one can use finite differencing
to obtain gradients. However, the gradients must be reasonably accurate
for the method to proceed. Note that the nonlinear least-squares methods
only operate on a sum of squared errors as the objective. Also, the user
must return each residual term separately to Dakota, whereas the user
can return an aggregated error measure in the case of general
optimizers.

The three nonlinear least-squares methods are the Gauss-Newton method in
OPT++, NLSSOL, and NL2SOL. Any of these may be tried; they give similar
performance on many problems. NL2SOL tends to be more robust than
Gauss-Newton, especially for nonlinear functions and large-residual
problems where one is not able to drive the residuals to zero at the
solution. NLSSOL does require that the user has the NPSOL library. Note
that all of these methods are local in the sense that they are
gradient-based and depend on an initial starting point. Often they are
used in conjunction with a multi-start method, to perform several
repetitions of the optimization at different starting points in the
parameter space. Another approach is to use a general global optimizer
such as a genetic algorithm or DIRECT as mentioned above. This can be
much more expensive, however, in terms of the number of function
evaluations required.
