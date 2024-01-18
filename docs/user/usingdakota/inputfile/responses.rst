.. _`responses:main`:

Responses
=========

.. _`responses:overview`:

Overview
--------

A :dakkw:`responses` block in a Dakota input file specifies the types
of data that can be returned from an interface during Dakota’s
execution. The specification includes the number and type of response
functions (objective functions, nonlinear constraints, calibration
terms, etc.) as well as availability of first and second derivatives
(gradient vectors and Hessian matrices) of them. A brief overview of
the response types and their uses follows, as well as discussion of
response file formats and derivative vector and matrix sizing. See
:dakkw:`responses` for additional details and examples.

.. _`responses:overview:types`:

Response function types
~~~~~~~~~~~~~~~~~~~~~~~

The types of responses specified in an input file are commonly paired
with the iterative technique called for in a :dakkw:`method`
specification block:

- **Optimization** data set comprised of :dakkw:`responses-objective_functions`,
  :dakkw:`responses-objective_functions-nonlinear_inequality_constraints`, and
  :dakkw:`responses-objective_functions-nonlinear_equality_constraints`. This
  data set is primarily for use with optimization methods, e.g., the
  methods in :ref:`opt`.  When using :ref:`results files
  <responses:results>`, the responses must be ordered: objectives,
  inequalities, then equalities.

- **Calibration** data set comprised of :dakkw:`responses-calibration_terms`,
  :dakkw:`responses-calibration_terms-nonlinear_inequality_constraints`,
  and
  :dakkw:`responses-calibration_terms-nonlinear_equality_constraints`. This
  data set is primarily for use calibration
  algorithms, e.g., the methods in :ref:`nls` and :ref:`uq:bayesian`. When using
  :ref:`results files <responses:results>` the responses must
  be ordered: calibration terms, inequalities, then equalities.

- **Generic** data set comprised of
  :dakkw:`responses-response_functions`. This data set is appropriate
  for use with uncertainty quantification methods, e.g., the methods
  in :ref:`uq`.

Certain general-purpose iterative techniques, such as parameter studies
and design of experiments methods, can be used with any of these data
sets.

.. _`responses:overview:gradient`:

Gradient availability
~~~~~~~~~~~~~~~~~~~~~

Gradient availability for these response functions may be described by:

- :dakkw:`responses-no_gradients`: gradients will not be used.

- :dakkw:`responses-numerical_gradients`: gradients are needed and will be
  approximated by finite differences.

- :dakkw:`responses-analytic_gradients`: gradients are needed and will
  be supplied by the simulation code (without any finite differencing
  by Dakota).

- :dakkw:`responses-mixed_gradients`: the simulation will supply some
  gradient components and Dakota will approximate the others by finite
  differences.

The gradient specification relates to the method in use.  Gradients
are typically needed for studies such as gradient-based optimization,
reliability analysis for uncertainty quantification, or local
sensitivity analysis.

.. _`responses:overview:hessian`:

Hessian availability
~~~~~~~~~~~~~~~~~~~~

Hessian availability for the response functions is similar to the
gradient availability specifications, with the addition of support for
“quasi-Hessians":

- :dakkw:`responses-no_hessians`: Hessians will not be used.

- :dakkw:`responses-numerical_hessians`: Hessians are needed and will
  be approximated by finite differences. These finite differences may
  involve first-order differences of gradients (if analytic gradients
  are available for the response function of interest) or second-order
  differences of function values (in all other cases).

- :dakkw:`responses-quasi_hessians`: Hessians are needed and will be
  approximated by secant updates (BFGS or SR1) from a series of
  gradient evaluations.

- :dakkw:`responses-analytic_hessians`: Hessians are needed and are
  available directly from the simulation code.

- :dakkw:`responses-mixed_hessians`: Hessians are needed and will be
  obtained from a mix of numerical, analytic, and “quasi" sources.

The Hessian specification also relates to the iterative method in use;
Hessians commonly would be used in gradient-based optimization by full
Newton methods or in reliability analysis with second-order limit
state approximations or second-order probability integrations.

Field Data
~~~~~~~~~~

..
   TODO: This section needs a re-write

Prior to Dakota 6.1, Dakota responses were always treated as scalar
responses. That is, if the user specified ``response_functions = 5``,
Dakota treated the five responses as five separate scalar quantities.
There are some cases where responses are a "field" quantity, meaning
that the responses are a function of one or more independent variables
such as time and/or spatial location. In these cases, the responses
should be treated as a field. For example, it can become extremely
cumbersome to represent 5000 values from a time-temperature trace or a
current-voltage curve in Dakota. With scalar response quantities, we
ignore the independent variable(s). For example, if we have a response
:math:`R` as a function of time :math:`t`, the user currently gives
Dakota a set of discrete responses at particular times and Dakota
isn't aware of the time values.

With the field data capability, the user can specify that they have
one field response of size :math:`5000 \times 1` (for example). Dakota
will have a large set of data :math:`R=f(t)`, with both the response
:math:`R` and independent coordinates :math:`t` specified. The
independent variable(s) can be useful in interpolation between
simulation responses and experimental observations. It also can be
useful in surrogate construction. We plan to handle correlation or
structure between field responses, which is currently not handled when
we treat the responses as individual, separate scalar quantities.

For all three major response types (objective functions, calibration
terms, and generic response functions), one can specify field
responses (e.g. with
:dakkw:`responses-objective_functions-field_objectives`,
:dakkw:`responses-calibration_terms-field_calibration_terms`, and
:dakkw:`responses-response_functions-field_responses`). For each type
of field response, one can specify the length of the field (e.g., with
``lengths=5000``) and the number of independent coordinates
(``num_coordinates_per_field``). The user can specify the independent
coordinates by specifying and providing the coordinates in files named
:file:`<response_descriptor>.coords`. In the case of field data from
physical experiments used to calibrate field data from simulation
experiments, the specification is more involved: the user should refer
to the :dakkw:`responses` keywords for specific syntax. 
All methods can handle field data, but currently the calibration
methods are the only methods specialized for field data, specifically
they interpolate the simulation field data to the experiment field
data points to calculate the residual terms. This is applicable to
:dakkw:`method-nl2sol`, :dakkw:`method-nlssol`,
:dakkw:`method-optpp_g_newton`, the MCMC Bayesian methods, as well as
general optimization methods that recast the residuals into a
sum-of-squared errors. Other methods simply handle the field
responses as a vector of scalar responses, as they did historically. Future
versions might include additional methods to explicitly
handle field data, including dimension reduction.

.. _`responses:results`:

Dakota Results File Data Format
-------------------------------

Simulation interfaces using system calls and forks to create separate
simulation processes must communicate with the simulation through the
file system. Dakota accomplishes this by writing parameters files with
variable values and reading results files with response values. For
the results file, only one text file format is supported (versus the
two parameter file formats described in
:ref:`variables:parameters`). Ordering of response functions is as
listed in :ref:`responses:overview:types`, i.e., objective functions
or calibration terms are first, followed by nonlinear inequality
constraints, followed by nonlinear equality constraints).

After a simulation, Dakota expects to read a file containing responses
reflecting the current parameters and corresponding to the function
requests in the active set vector. The response data must be in the
format shown in :numref:`fig:responses:results_format`.

.. code-block::
   :name: fig:responses:results_format
   :caption: Results file data format.

   <double> <fn_label_1>
   <double> <fn_label_2>
   ...
   <double> <fn_label_m>
   [ <double> <double> .. <double> ]
   [ <double> <double> .. <double> ]
   ...
   [ <double> <double> .. <double> ]
   [[ <double> <double> .. <double> ]]
   [[ <double> <double> .. <double> ]]
   ...
   [[ <double> <double> .. <double> ]]
   <double> <md_label_1>
   <double> <md_label_2>
   ...
   <double> <md_label_r)>

The first block of data conveys the requested function values
:math:`1, \ldots, m` and is followed by a block of requested gradients
delimited by single brackets, followed by a block of requested
Hessians delimited by double brackets. If the amount of data in the
file does not match the function active set request vector, Dakota
will abort execution with an error message.

Function values have no bracket delimiters, but each may be followed
by its own non-numeric label. Labels must be separated from numeric
function values by white space (one or more blanks, tabs, or newline
characters) and they must not contain any white space themselves
(e.g., use ``response1`` or ``response_1``, but not ``response 1``).
Labels also must not resemble numerical values.

By default, function value labels are optional and are ignored by
Dakota; they are permitted only as a convenience to the user. However,
if strict checking is activated by including the ``labeled`` keyword in
the interface section of the Dakota input file, then labels are required
for every function value. Further, labels must exactly match the
response descriptors of their corresponding function values. These
stricter labeling requirements enable Dakota to detect and report when
function values are returned out-of-order, or when specific function
values are repeated or missing.

Gradient vectors are surrounded by single brackets :math:`[\ldots
n_{dvv}-\textrm{vector of doubles} \ldots]`. Labels are not used and
must not be present. White space separating the brackets from the data
is optional.

Hessian matrices are surrounded by double brackets :math:`[[\ldots
n_{dvv} \times n_{dvv}-\textrm{matrix of doubles} \ldots]]`. Hessian
components (numeric values for second partial derivatives) are listed
by rows and separated by white space; in particular, they can be
spread across multiple lines for readability. Labels are not used and
must not be present. White space after the initial double bracket and
before the final one is optional, but none can appear within the
double brackets.

Any requested metadata values must appear at the end of the file
(after any requested values, gradients, or Hessians). Their format
requirements are the same as function values discussed above, and are
similarly validated by the ``labeled`` keyword when specified.

The format of the numeric fields may be floating point or scientific
notation. In the latter case, acceptable exponent characters are ``E``
or ``e``. A common problem when dealing with Fortran programs is that
a C++ read of a numeric field using ``D`` or ``d`` as the exponent
(i.e., a double precision value from Fortran) may fail or be truncated.
In this case, the ``D`` exponent characters must be replaced either
through modifications to the Fortran source or compiler flags or through
a separate post-processing step (e.g., using the UNIX ``sed`` utility).

.. _`responses:active`:

Active Variables for Derivatives
--------------------------------

An important question for proper management of both gradient and
Hessian data is: if several different types of variables are used,
*for which variables are response function derivatives needed?* That
is, how is :math:`n_{dvv}` determined? The short answer is that the
derivative variables vector (DVV), communicated in the parameters
file, specifies the set of variables to be used for computing
derivatives, and :math:`n_{dvv}` is the length of this vector.

In most cases, the DVV is defined directly from the set of active
continuous variables for the iterative method in use. Since methods
operate over a subset, or view, of the variables that is active in the
iteration, it is this same set of variables for which derivatives are
most commonly computed. Derivatives are never needed with respect to any
discrete variables (since these derivatives do not in general exist) and
the active continuous variables depend on view override specifications,
inference by response type, and inference by method type, in that order,
as described in :ref:`variables:mixed`.

..
   TODO:

   %For optimization and calibration problems,
   %the active continuous variables are the \emph{continuous design
   %  variables} ($n_{dvv}=n_{cdv}$), since they are the variables the
   %minimizer manipulates.  Similarly, for uncertainty quantification
   %methods that use gradient and/or Hessian information, the active
   %continuous variables are the \emph{continuous uncertain variables}
   %($n_{dvv}=n_{cauv}$ for aleatory methods, $n_{dvv}=n_{ceuv}$ for
   %epistemic methods, $n_{dvv}=n_{cauv}+n_{ceuv}$ for methods that handle
   %both), with the exception of \texttt{all\_variables} mode.  And
   %lastly, parameter study methods that are cataloging gradient and/or
   %Hessian information do not draw a distinction among continuous
   %variables; therefore, the active continuous variables are defined from
   %\emph{all continuous variables} that are specified
   %($n_{dvv}=n_{cdv}+n_{cauv}+n_{ceuv}+n_{csv}$).  Additional detail on
   %these variables views is provided in Table~\ref{responses:active_tab}.
   
   %\begin{table}
   %\centering
   %\caption{Variable views for different iterators.}
   %\label{responses:active_tab}\vspace{2mm}
   %\begin{tabular}{|c|c|c|}
   %\hline
   %\textbf{Method} & \textbf{Default Active view} & \textbf{Derivative variables} \\
   %\hline
   %branch and bound         & Merged Design   & $n_{cdv}+n_{ddiv}+n_{ddrv}$ \\
   %\hline
   %optimization,            & Mixed Design    & $n_{cdv}$ \\
   %nonlinear least squares  &                 &           \\
   %\hline
   %sampling (standard mode) & Mixed Uncertain & $n_{cauv}+n_{ceuv}$ \\
   %\hline
   %local reliability,       & Mixed Aleatory Uncertain & $n_{cauv}$ \\
   %global reliability (standard mode),  &              &            \\
   %stochastic expansion (standard mode) &              &            \\
   %\hline
   %interval estimation,     & Mixed Epistemic Uncertain & $n_{ceuv}$ \\
   %evidence                 &                           &            \\
   %\hline
   %parameter studies,       & Mixed All & $n_{cdv}+n_{cauv}+n_{ceuv}+n_{csv}$\\
   %design of experiments,   &           & \\
   %uncertainty quantification (all\_variables mode) & & \\
   %\hline
   %\end{tabular}
   %\end{table}

In a few cases, derivatives are needed with respect to the *inactive*
continuous variables. This occurs for nested models where a top-level
method sets derivative requirements (with respect to its active
continuous variables) on the final solution of the lower-level/inner method
(for which the top-level active variables are inactive). For example, in
an uncertainty analysis within a nested design under uncertainty
algorithm, derivatives of the lower-level response functions may be
needed with respect to the design variables, which are active continuous
variables at the top level but are inactive within the uncertainty
quantification. These instances are the reason for the creation and
inclusion of the DVV vector -- to clearly indicate the variables whose
partial derivatives are needed.

In all cases, if the DVV is honored, then the correct derivative
components are returned. In simple cases, such as optimization and
calibration studies that only specify design variables and for
nondeterministic analyses that only specify uncertain variables,
derivative component subsets are not an issue and the exact content of
the DVV may be safely ignored.
