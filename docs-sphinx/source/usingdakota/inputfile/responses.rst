Responses
=========

.. _`responses:overview`:

Overview
--------

The ``responses`` specification in a Dakota input file controls the
types of data that can be returned from an interface during Dakota’s
execution. The specification includes the number and type of response
functions (objective functions, nonlinear constraints, calibration
terms, etc.) as well as availability of first and second derivatives
(gradient vectors and Hessian matrices) for these response functions.

This chapter will present a brief overview of the response data sets and
their uses, as well as cover some user issues relating to file formats
and derivative vector and matrix sizing. For a detailed description of
responses section syntax and example specifications, refer to the
Responses Commands chapter in the Dakota Reference
Manual TODO.

.. _`responses:overview:types`:

Response function types
~~~~~~~~~~~~~~~~~~~~~~~

The types of response functions listed in the responses specification
should be consistent with the iterative technique called for in the
method specification:

-  | an optimization data set comprised of ``num_objective_functions``,
   | ``num_nonlinear_inequality_constraints``, and
     ``num_nonlinear_equality_constraints``. This data set is
     appropriate for use with optimization methods (e.g., the methods in
     Chapter `[opt] <#opt>`__). When using results files
     (`1.2 <#responses:results>`__), the responses must be ordered:
     objectives, inequalities, then equalities.

-  | a calibration data set comprised of ``calibration_terms``,
   | ``num_nonlinear_inequality_constraints``, and
     ``num_nonlinear_equality_constraints``. This data set is
     appropriate for use with nonlinear least squares algorithms (e.g.,
     the methods in Chapter `[nls] <#nls>`__). When using results files
     (`1.2 <#responses:results>`__), the responses must be ordered:
     calibration terms, inequalities, then equalities.

-  a generic data set comprised of ``num_response_functions``. This data
   set is appropriate for use with uncertainty quantification methods
   (e.g., the methods in Section `[uq] <#uq>`__).

Certain general-purpose iterative techniques, such as parameter studies
and design of experiments methods, can be used with any of these data
sets.

.. _`responses:overview:gradient`:

Gradient availability
~~~~~~~~~~~~~~~~~~~~~

Gradient availability for these response functions may be described by:

-  ``no_gradients``: gradients will not be used.

-  ``numerical_gradients``: gradients are needed and will be
   approximated by finite differences.

-  ``analytic_gradients``: gradients are needed and will be supplied by
   the simulation code (without any finite differencing by Dakota).

-  ``mixed_gradients``: the simulation will supply some gradient
   components and Dakota will approximate the others by finite
   differences.

The gradient specification also links back to the iterative method used.
Gradients commonly are needed when the iterative study involves
gradient-based optimization, reliability analysis for uncertainty
quantification, or local sensitivity analysis.

.. _`responses:overview:hessian`:

Hessian availability
~~~~~~~~~~~~~~~~~~~~

Hessian availability for the response functions is similar to the
gradient availability specifications, with the addition of support for
“quasi-Hessians":

-  ``no_hessians``: Hessians will not be used.

-  ``numerical_gradients``: Hessians are needed and will be approximated
   by finite differences. These finite differences may involve
   first-order differences of gradients (if analytic gradients are
   available for the response function of interest) or second-order
   differences of function values (in all other cases).

-  ``quasi_hessians``: Hessians are needed and will be approximated by
   secant updates (BFGS or SR1) from a series of gradient evaluations.

-  ``analytic_hessians``: Hessians are needed and are available directly
   from the simulation code.

-  ``mixed_hessians``: Hessians are needed and will be obtained from a
   mix of numerical, analytic, and “quasi" sources.

The Hessian specification also links back to the iterative method in
use; Hessians commonly would be used in gradient-based optimization by
full Newton methods or in reliability analysis with second-order limit
state approximations or second-order probability integrations.

Field Data
~~~~~~~~~~

Prior to Dakota 6.1, Dakota responses have been treated as scalar
responses. That is, if the user specifies ``response_functions=5``,
Dakota treats the five responses as five separate scalar quantities.
There are some cases where responses are a “field” quantity, meaning
that the responses are a function of one or more independent variables
such as time and/or spatial location. In these cases, the responses
should be treated as field data. For example, it can become extremely
cumbersome to represent 5000 values from a time-temperature trace or a
current-voltage curve in Dakota. With scalar response quantities, we
ignore the independent variable(s). For example, if we have a response
:math:`R` as a function of time :math:`t`, the user currently gives
Dakota a set of discrete responses at particular times and Dakota
doesn’t know the times.

With the field data capability, the user can specify that they have one
field response of size 5000 x 1 (for example). Dakota will have a large
set of data :math:`R=f(t)`, with both the response :math:`R` and
independent coordinates :math:`t` specified to Dakota. The independent
variable(s) can be useful in interpolation between simulation responses
and experimental observations. It also can be useful in surrogate
construction. We plan to handle correlation or structure between field
responses, which is currently not handled when we treat the responses as
individual, separate scalar quantities.

| For all three major response types (, , and
| ), one can specify field responses (e.g. with ,
| , and ). For each type of field response, one can specify the length
  of the field (e.g. ) and the number of independent coordinates (). The
  user can specify the independent coordinates by specifying and
  providing the coordinates in files named . In the case of field data
  from physical experiments used to calibrate field data from simulation
  experiments, the specification is more involved: the user should refer
  to the Dakota Reference manual to get the syntax. Note that at this
  time, field responses may be specified by the user as outlined above.
  All methods can handle field data, but currently the calibration
  methods are the only methods specialized for field data, specifically
  they interpolate the simulation field data to the experiment field
  data points to calculate the residual terms. This is applicable to , ,
  , the MCMC Bayesian methods, as well as general optimization methods
  that recast the residuals into a sum-of-squares error. The other
  methods simply handle the field responses as a number of scalar
  responses currently. In future versions, we are planning some
  additional features with methods that can handle field data, including
  reduction of the field data.

.. _`responses:results`:

Dakota Results File Data Format
-------------------------------

Simulation interfaces using system calls and forks to create separate
simulation processes must communicate with the simulation through the
file system. This is done by reading and writing files of parameters and
results. Dakota uses its own format for this data input/output. For the
results file, only one format is supported (versus the two
parameter-file formats described in
Section `[variables:parameters] <#variables:parameters>`__). Ordering of
response functions is as listed in
Section `1.1.1 <#responses:overview:types>`__ (e.g., objective functions
or calibration terms are first, followed by nonlinear inequality
constraints, followed by nonlinear equality constraints).

After a simulation, Dakota expects to read a file containing responses
reflecting the current parameters and corresponding to the function
requests in the active set vector. The response data must be in the
format shown in Figure `[responses:figure01] <#responses:figure01>`__.

TODO

The first block of data (shown in black) conveys the requested function
values and is followed by a block of requested gradients (shown in
blue), followed by a block of requested Hessians (shown in red). If the
amount of data in the file does not match the function request vector,
Dakota will abort execution with an error message.

Function values have no bracket delimiters, but each may be followed by
its own non-numeric label. Labels must be separated from numeric
function values by white space (one or more blanks, tabs, or newline
characters) and they must not contain any white space themselves (e.g.,
use “``response1``” or “``response_1``,” but not “``response 1``”).
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

Gradient vectors are surrounded by single brackets
[…\ :math:`n_{dvv}`-vector of doubles…]. Labels are not used and must
not be present. White space separating the brackets from the data is
optional.

Hessian matrices are surrounded by double brackets
[[…\ :math:`n_{dvv} \times n_{dvv}` matrix of doubles…]]. Hessian
components (numeric values for second partial derivatives) are listed by
rows and separated by white space; in particular, they can be spread
across multiple lines for readability. Labels are not used and must not
be present. White space after the initial double bracket and before the
final one is optional, but none can appear within the double brackets.

The format of the numeric fields may be floating point or scientific
notation. In the latter case, acceptable exponent characters are “``E``”
or “``e.``” A common problem when dealing with Fortran programs is that
a C++ read of a numeric field using “``D``” or “``d``” as the exponent
(i.e., a double precision value from Fortran) may fail or be truncated.
In this case, the “``D``” exponent characters must be replaced either
through modifications to the Fortran source or compiler flags or through
a separate post-processing step (e.g., using the UNIX ``sed`` utility).

.. _`responses:active`:

Active Variables for Derivatives
--------------------------------

An important question for proper management of both gradient and Hessian
data is: if several different types of variables are used, *for which
variables are response function derivatives needed?* That is, how is
:math:`n_{dvv}` determined? The short answer is that the derivative
variables vector (DVV) specifies the set of variables to be used for
computing derivatives, and :math:`n_{dvv}` is the length of this vector.

In most cases, the DVV is defined directly from the set of active
continuous variables for the iterative method in use. Since methods
operate over a subset, or view, of the variables that is active in the
iteration, it is this same set of variables for which derivatives are
most commonly computed. Derivatives are never needed with respect to any
discrete variables (since these derivatives do not in general exist) and
the active continuous variables depend on view override specifications,
inference by response type, and inference by method type, in that order,
as described in Section `[variables:mixed] <#variables:mixed>`__.

In a few cases, derivatives are needed with respect to the *inactive*
continuous variables. This occurs for nested iteration where a top-level
iterator sets derivative requirements (with respect to its active
continuous variables) on the final solution of the lower-level iterator
(for which the top-level active variables are inactive). For example, in
an uncertainty analysis within a nested design under uncertainty
algorithm, derivatives of the lower-level response functions may be
needed with respect to the design variables, which are active continuous
variables at the top level but are inactive within the uncertainty
quantification. These instances are the reason for the creation and
inclusion of the DVV vector — to clearly indicate the variables whose
partial derivatives are needed.

In all cases, if the DVV is honored, then the correct derivative
components are returned. In simple cases, such as optimization and
calibration studies that only specify design variables and for
nondeterministic analyses that only specify uncertain variables,
derivative component subsets are not an issue and the exact content of
the DVV may be safely ignored.
