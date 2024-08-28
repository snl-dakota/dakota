.. _`variables:main`:

Variables
=========

.. _`variables:overview`:

.. _topic-variables:

Overview
--------

The :dakkw:`variables` specification in a Dakota input file specifies the
parameter set to be iterated by a particular method. In the case of an
optimization study, these variables are adjusted in order to locate an
optimal design; in the case of parameter studies/sensitivity
analysis/design of experiments, these parameters are perturbed to
explore the parameter space; and in the case of uncertainty analysis,
the variables are associated with distribution/interval
characterizations which are used to compute corresponding
distribution/interval characterizations for response functions. To
accommodate these and other types of studies, Dakota supports design,
uncertain, and state variable types for continuous and discrete variable
domains. Uncertain types can be further categorized as either aleatory
or epistemic, and discrete domains can include discrete range, discrete
integer set, discrete string set, and discrete real set.

This chapter surveys key variables concepts, categories, and specific
types, and addresses variable-related file formats and the active set
vector. See the :dakkw:`variables` keyword for additional
specification details.

.. note::

   In several contexts, Dakota inputs must express variable
   specifications in what is referred to as "input specification
   order." This means the ordering of variables types given in the
   primary :dakkw:`variables` table.

Key Dakota variable concepts include:

- Category (design, uncertain (aleatory/epistemic), state) which
  groups variables by their primary use.

- Active View: the subset of variables (categories) being explored in
  a particular study.

- Type: a specific named variable

- Domain: continuous vs. discrete (integer-, string-, or real-valued).
  Discrete variables span categories and are specified via ranges,
  admissible sets, and integer-valued discrete probability
  distributions.

.. note::

   Characterizing the properties of a specific type of variable, e.g.,
   :dakkw:`variables-discrete_design_set` or
   :dakkw:`variables-lognormal_uncertain` often requires providing
   arrays of data. For example a list of ``means`` or set
   ``elements_per_variable``. The ordering of these arrays must match
   the ordering of the ``descriptors`` for that variable type.

.. _`variables:design`:

Design Variables
----------------

Design variables are adjusted in the course of determining an optimal
design or an optimal set of deterministic calibration
parameters. These variables may be continuous (real-valued between
bounds), discrete range (integer-valued between bounds), discrete set
of integers (integer-valued from finite set), discrete set of strings
(string-valued from finite set), and discrete set of reals
(real-valued from finite set). Continuous design variables are the
most common design variable type in engineering applications. All but
a handful of the optimization algorithms in Dakota support continuous
design variables exclusively.

.. _`variables:design:cdv`:

Continuous Design Variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The most common type of design variables encountered in engineering
applications are of the continuous type. These variables may assume any
real value (e.g., ``12.34``, ``-1.735e+07``) within their bounds. All
but a handful of the optimization algorithms in Dakota support
continuous design variables exclusively.

.. _`variables:design:ddv`:

Discrete Design Variables
~~~~~~~~~~~~~~~~~~~~~~~~~

Engineering design problems may contain discrete variables such as
material types, feature counts, stock gauge selections, etc. These
variables may assume only a fixed number of values, as compared to a
continuous variable which has an uncountable number of possible values
within its range. Discrete variables may involve a range of consecutive
integers (:math:`x` can be any integer between ``1`` and ``10``), a set
of integer values (:math:`x` can be ``101``, ``212``, or ``355``), a set
of string values (:math:`x` can be ``'direct'``, ``'gmres'``, or
``'jacobi'``), or a set of real values (e.g., :math:`x` can be
identically ``4.2``, ``6.4``, or ``8.5``).

Discrete variables may be classified as either "categorical" or
"noncategorical." In the latter noncategorical case, the discrete
requirement can be relaxed during the solution process since the model
can still compute meaningful response functions for values outside the
allowed discrete range or set. For example, a discrete variable
representing the thickness of a structure is generally a noncategorical
variable since it can assume a continuous range of values during the
algorithm iterations, even if it is desired to have a stock gauge
thickness in the end. In the former categorical case, the discrete
requirement cannot be relaxed since the model cannot obtain a solution
for values outside the range or set. For example, feature counts are
generally categorical discrete variables, since most computational
models will not support a non-integer value for the number of instances
of some feature (e.g., number of support brackets). An optional
``categorical`` specification indicates which discrete real and
discrete integer variables are restricted vs. relaxable. String
variables cannot be relaxed.

Gradient-based optimization methods cannot be directly applied to
problems with discrete variables since derivatives only exist for a
variable continuum. For problems with noncategorical variables, the
experimental branch and bound capability
(:dakkw:`method-branch_and_bound`) can be
used to relax the discrete requirements and apply gradient-based methods
to a series of generated subproblems. For problems with categorical
variables, nongradient-based methods (e.g., :dakkw:`method-coliny_ea`)
are commonly
used; however, most of those methods do not take advantage of any
structure that may be associated with the categorical variables. The
exception is :dakkw:`method-mesh_adaptive_search`.
If it is possible to define a
subjective relationship between the different values a given categorical
variable can take on, that relationship can be expressed via a
:dakkw:`variables`
:dakkw:`method-mesh_adaptive_search-adjacency_matrix` option. The
method will take that relationship into consideration, together with
any expressed
:dakkw:`method-mesh_adaptive_search-neighbor_order`. Branch and bound
techniques are expanded on in :ref:`adv_meth:minlp` and
nongradient-based methods are further described in :ref:`opt`.

In addition to engineering applications, many non-engineering
applications in the fields of scheduling, logistics, and resource
allocation contain discrete design parameters. Within the Department of
Energy, solution techniques for these problems impact programs in
stockpile evaluation and management, production planning,
nonproliferation, transportation (routing, packing, logistics),
infrastructure analysis and design, energy production, environmental
remediation, and tools for massively parallel computing such as domain
decomposition and meshing.

**Discrete Design Variable Types:**

- The :dakkw:`variables-discrete_design_range` type supports a range
  of consecutive integers between specified
  :dakkw:`variables-discrete_design_range-lower_bounds` and
  :dakkw:`variables-discrete_design_range-upper_bounds`.

- The :dakkw:`discrete_design_set` type admits a set of enumerated
  integer, string, or real values through an ``elements``
  specification. The set of values must be specified as an ordered,
  unique set and is stored internally the same way, with a
  corresponding set of indices that run from 0 to one less than the
  number of set values. These indices are used by some iterative
  algorithms (e.g., parameter studies, SCOLIB methods) for
  simplicity in discrete value enumeration when the actual
  corresponding set values are immaterial. In the case of
  :ref:`parameter studies <ps>`, this index representation is required
  in certain step and partition controls.

  Each string element value must be quoted in the Dakota input file
  and may contain alphanumeric, dash, underscore, and colon. White
  space, quote characters, and backslash/meta-characters are not
  permitted.

.. _`variables:uncertain`:

Uncertain Variables
-------------------

Deterministic variables (i.e., those with a single known value) do not
capture the behavior of the input variables in all situations. In many
cases, the exact value of a model parameter is not precisely known. An
example of such an input variable is the thickness of a heat treatment
coating on a structural steel I-beam used in building construction. Due
to variability and tolerances in the coating process, the thickness of
the layer is known to follow a normal distribution with a certain mean
and standard deviation as determined from experimental data. The
inclusion of the uncertainty in the coating thickness is essential to
accurately represent the resulting uncertainty in the response of the
building.

Uncertain variables directly support the use of probabilistic
uncertainty quantification methods such as sampling, reliability, and
stochastic expansion methods.  They also admit lower and upper
distribution bounds (whether explicitly defined, implicitly defined,
or inferred), which permits allows their use in methods that rely on a
bounded region to define a set of function evaluations (i.e., design
of experiments and some parameter study methods).

.. _`variables:uncertain:auv`:

Aleatory Uncertain Variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Aleatory uncertainty is also known as inherent variability,
irreducible uncertainty, or randomness. It is typically modeled using
probability distributions, and probabilistic methods are commonly used
for propagating input aleatory uncertainties described by probability
distribution specifications. The two following sections describe the
continuous and discrete aleatory uncertain variables supported by
Dakota.

..
   TODO: %characterized by having a sufficiently rich set of data as to allow modeling

.. _`variables:uncertain:cauv`:

Continuous Aleatory Uncertain Variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- :ref:`Normal <variables-normal_uncertain>`: a probability
  distribution characterized by a mean and
  standard deviation. Also referred to as Gaussian. Bounded normal is
  also supported by some methods with an additional specification of
  lower and upper bounds.

- :ref:`Lognormal <variables-lognormal_uncertain>`: a probability
  distribution characterized by a mean and
  either a standard deviation or an error factor. The natural logarithm
  of a lognormal variable has a normal distribution. Bounded lognormal
  is also supported by some methods with an additional specification of
  lower and upper bounds.

- :ref:`Uniform <variables-uniform_uncertain>`: a probability
  distribution characterized by a lower bound
  and an upper bound. Probability is constant between the bounds.

- :ref:`Loguniform <variables-loguniform_uncertain>`: a probability
  distribution characterized by a lower bound
  and an upper bound. The natural logarithm of a loguniform variable
  has a uniform distribution.

- :ref:`Triangular <variables-triangular_uncertain>`: a probability distribution
  characterized by a mode, a lower bound, and an upper bound.

- :ref:`Exponential <variables-exponential_uncertain>`: a probability
  distribution characterized by a beta parameter.

- :ref:`Beta <variables-beta_uncertain>`: a flexible probability
  distribution characterized by a lower
  bound and an upper bound and alpha and beta parameters. The uniform
  distribution is a special case.

- :ref:`Gamma <variables-gamma_uncertain>`: a flexible probability
  distribution characterized by alpha and
  beta parameters. The exponential distribution is a special case.

- :ref:`Gumbel <variables-gumbel_uncertain>`: the Type I Largest
  Extreme Value probability distribution.
  Characterized by alpha and beta parameters.

- :ref:`Frechet <variables-frechet_uncertain>`: the Type II Largest
  Extreme Value probability distribution.
  Characterized by alpha and beta parameters.

- :ref:`Weibull <variables-weibull_uncertain>`: the Type III Smallest
  Extreme Value probability
  distribution. Characterized by alpha and beta parameters.

- :ref:`Histogram Bin <variables-histogram_bin_uncertain>`: an
  empirically-based probability distribution
  characterized by a set of :math:`(x,y)` pairs that map out histogram
  bins (a continuous interval with associated bin count).

.. _`variables:uncertain:dauv`:

Discrete Aleatory Uncertain Variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following types of discrete aleatory uncertain variables are
available:

- :ref:`Poisson <variables-poisson_uncertain>`: integer-valued
  distribution used to predict the number of
  discrete events that happen in a given time interval.

- :ref:`Binomial <variables-binomial_uncertain>`: integer-valued
  distribution used to predict the number of
  failures in a number of independent tests or trials.

- :ref:`Negative Binomial <variables-negative_binomial_uncertain>`:
  integer-valued distribution used to predict the
  number of times to perform a test to have a target number of
  successes.

- :ref:`Geometric <variables-geometric_uncertain>`: integer-valued
  distribution used to model the number of
  successful trials that might occur before a failure is observed.

- :ref:`Hypergeometric <variables-hypergeometric_uncertain>`:
  integer-valued distribution used to model the number
  of failures observed in a set of tests that has a known proportion of
  failures.

- :ref:`Histogram Point (integer, string, real)
  <variables-histogram_point_uncertain>`: an empirically-based
  probability distribution characterized by a set of integer-valued
  :math:`(i,c)`, string-valued :math:`(s,c)`, and/or real-valued
  :math:`{r,c}` pairs that map out histogram points (each a discrete
  point value :math:`i`, :math:`s`, or :math:`r`, with associated count
  :math:`c`).

For aleatory random variables, Dakota admits an
:dakkw:`variables-uncertain_correlation_matrix` that specifies
correlations among the input variables. The correlation matrix
defaults to the identity matrix, i.e., no correlation among the
uncertain variables.

For additional information on random variable probability
distributions, refer to :cite:p:`Hal00` and :cite:p:`Swi04`. Refer to
:dakkw:`variables` for more detail on the uncertain variable
specifications and to :ref:`uq` for available methods to quantify the
uncertainty in the response.

.. _`variables:uncertain:euv`:

Epistemic Uncertain Variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Epistemic uncertainty is reducible uncertainty due to lack of
knowledge. Characterization of epistemic uncertainties is often based
on subjective prior knowledge rather than objective data.

..
   TODO: 
   %data is generally sparse, making the use of probability theory
   %questionable and leading to nonprobabilistic methods based on
   %interval or fuzzy specifications.

In Dakota, epistemic uncertainty can be characterized by interval- or
set-valued variables (see relevant keywords below) that are propagated
to calculate bounding intervals on simulation output using interval
analysis methods. These epistemic variable types can optionally
include belief structures or basic probability assignments for use in
Dempster-Shafer theory of evidence methods. Epistemic uncertainty can
alternately be modeled with probability density functions, although
results from UQ studies are then typically interpreted as
possibilities or bounds, as opposed to a probability distribution of
responses.

Dakota supports the following epistemic uncertain variable types:

- :ref:`Continuous Interval <variables-continuous_interval_uncertain>`:
  a real-valued interval-based specification
  characterized by sets of lower and upper bounds and Basic Probability
  Assignments (BPAs) associated with each interval. The intervals may
  be overlapping, contiguous, or disjoint, and a single interval (with
  probability = 1) per variable is an important special case. The
  interval distribution is not a probability distribution, as the exact
  structure of the probabilities within each interval is not known. It
  is commonly used with epistemic uncertainty methods.

- :ref:`Discrete Interval <variables-discrete_interval_uncertain>`: an
  integer-valued variant of the Continuous Interval variable.
  
- :ref:`Discrete Set (integer, string, and real)
  <variables-discrete_uncertain_set>`: Similar to discrete design set
  variables, these epistemic variables admit a finite number of values
  (``elements``) for type integer, string, or real, each with an
  associated probability.

In the discrete case, interval variables may be used to specify
categorical choices which are epistemic.  For example, if there are
three possible forms for a physics model (model 1, 2, or 3) and there
is epistemic uncertainty about which one is correct, a discrete
uncertain interval or a discrete set could represent this type of
uncertainty.

Through :dakkw:`model-nested`, Dakota can perform combined aleatory /
epistemic analyses such as second-order probability or probability of
frequency. For example, a variable can be assumed to have a lognormal
distribution with specified variance, with its mean expressed as an
epistemic uncertainty lying in an expert-specified interval. See
examples in :ref:`adv_models`.

.. _`variables:state`:

State Variables
---------------

State variables consist of auxiliary variables to be mapped through
the simulation interface, but are not to be designed nor modeled as
uncertain. State variables provide a means to parameterize additional
model inputs which, in the case of a numerical simulator, might
include solver convergence tolerances, time step controls, or mesh
fidelity parameters.

.. note::

   The term "state variable" is overloaded in math, science, and
   engineering. For Dakota it typically means a fixed parameter and
   does not refer to, e.g., the solution variables of a differential
   equation.

State variable configuration mirrors that of :ref:`design variables
<variables:design>`. They can be specified via
:dakkw:`variables-continuous_state` (real-valued between bounds),
:dakkw:`variables-discrete_state_range` (integer-valued between
bounds), or :dakkw:`variables-discrete_state_set` (a discrete
integer-, string-, or real-valued set). Model parameterizations with
strings (e.g., "mesh1.exo"), are also possible using an interface
:dakkw:`interface-analysis_drivers-analysis_components` specification
(see also :ref:`variables:parameters:standard`)

State variables, as with other types of variables, are viewed
differently depending on the method in use. By default, only parameter
studies, design of experiments, and verification methods will vary
state variables. This can be overridden as discussed in
:ref:`variables:mixedview`.

Since these variables are neither design nor uncertain variables,
algorithms for optimization, least squares, and uncertainty
quantification do not iterate on these variables by default. They are
inactive and hidden from the algorithm.  However, Dakota still maps
these variables through the user’s interface where they affect the
computational model in use. This allows optimization, least squares,
and uncertainty quantification studies to be executed under different
simulation conditions (which will result, in general, in different
results). Parameter studies and design of experiments methods, on the
other hand, are general-purpose iterative techniques which do not by
default draw a distinction between variable types. They include state
variables in the set of variables to be studied, which permit them to
explore the effect of state variable values on the responses of
interest.

When a state variable is held fixed, the specified ``initial_state``
is used as its sole value. If the state variable is defined only by
its bounds, then the initial_state will be inferred from the variable
bounds or valid set values. If a method iterates on a state variable,
the variable is treated as a design variable with the given bounds, or
as a uniform uncertain variable with the given bounds.

In some cases, state variables are used direct coordination with an
optimization, least squares, or uncertainty quantification algorithm.
For example, state variables could be used to enact model adaptivity
through the use of a coarse mesh or loose solver tolerances in the
initial stages of an optimization with continuous model refinement as
the algorithm nears the optimal solution. They also are used to
control model fidelity in some UQ approaches.

.. _`variables:mixed`:

Management of Mixed Variables by Method
-----------------------------------------

.. _`variables:mixedview`:

Active Variables View
~~~~~~~~~~~~~~~~~~~~~

As alluded to in the previous section, the iterative method selected
for use in Dakota partially determines what subset, or view, of the
variables data is active in the study. In general, a mixture of
various different types of variables is supported within all methods,
though by default certain methods will only modify certain types of
variables. For example, by default, optimizers and least squares
methods only modify design variables, and uncertainty quantification
methods typically only utilize uncertain variables. This implies that
variables which are not directly controlled by a particular method
will be mapped through the interface unmodified. This allows for
parameterizations within the model beyond those used by a the method,
which can provide the convenience of consolidating the control over
various modeling parameters in a single file (the Dakota input
file). An important related point is that the active variable set
dictates over which continuous variables derivatives are typically
computed (see :ref:`responses:active`).

**Default Variables View:** The default active variables view is
determined from a combination of the response function type and
method. If :dakkw:`responses-objective_functions` or
:dakkw:`responses-calibration_terms` is given in the response
specification block, the design variables will be active.

General :dakkw:`responses-response_functions` do not have a specific
interpretation the way objective functions or calibration terms
do. For these, the active view is inferred from the method.

- For parameter studies, or any of the dace, psuade, or fsu methods,
  the active view is set to *all* variables.

- For sampling uncertainty quantification methods, the view is set to
  *aleatory* if only aleatory variables are present, *epistemic* if only
  epistemic variables are present, or *uncertain* (covering both
  aleatory and epistemic) if both are present.

- For interval estimation or evidence calculations, the view is set to
  *epistemic*.

- For other uncertainty quantification, e.g., reliability methods or
  stochastic expansion methods, the view is set to *aleatory*.

- Finally, for verification studies using
  :dakkw:`method-richardson_extrap` studies, the active view is set to
  *state*.

.. note::

   For surrogate-based optimization, where the surrogate is built over
   points generated by a ``dace_method_pointer``, the point generation
   is only over the design variables unless otherwise specified, i.e.,
   state variables will not be sampled for surrogate construction.

**Explicit View Control:** The subset of active variables for a Dakota
method can be explicitly controlled by specifying the variables
keyword :dakkw:`variables-active`, together with one of
:dakkw:`variables-all`, :dakkw:`variables-design`,
:dakkw:`variables-uncertain`, :dakkw:`variables-aleatory`,
:dakkw:`variables-epistemic`, or :dakkw:`variables-state`.  This
causes the Dakota method to operate on the specified variable types,
and overriding the defaults. For example, the default behavior for a
nondeterministic sampling method is to sample the uncertain
variables. However, if the user specified ``active all`` in the
variables block, the sampling would be performed over all variables
(e.g. design and state variables in addition to uncertain
variables). This may be desired in situations such as surrogate based
optimization under uncertainty, where a surrogate may be built over
both design and uncertain variables. Another situation where one may
want the fine-grained control available by specifying one of these
variable types is when one has state variables but only wants to
sample over the design variables when constructing a surrogate
model. Finally, more sophisticated uncertainty studies may involve
various combinations of epistemic vs. aleatory variables being active
in nested models.

.. _`variables:domain`:

Variable Domain
~~~~~~~~~~~~~~~

The variable domain setting controls how discrete variables (whether
design, uncertain, or state) are treated. If :dakkw:`variables-mixed`
is specified, the continuous and non-categorical discrete variables
are treated separately. When :dakkw:`variables-relaxed`, the discrete
variables are relaxed and treated as continuous variables.

Domain control can be useful in optimization problems involving both
continuous and discrete variables in order to apply a continuous
optimizer to a mixed variable problem. All methods default to a mixed
domain except for the experimental branch-and-bound method, which
defaults to relaxed.

.. _`variables:usage`:

Usage Notes
-----------

..
   TODO: Consider putting info from parameter studies on Initial
   Values and Bounds here. Also merge with info the keywords/variables
   section. Also this from the historical reference manual:

   For continuous and discrete range variables, the \c lower_bounds
   and \c upper_bounds restrict the size of the feasible design space
   and are frequently used to prevent nonphysical designs.  Default
   values are positive and negative machine limits for upper and lower
   bounds (+/- \c DBL_MAX, \c INT_MAX, \c INT_MIN from the \c float.h
   and \c limits.h system header files).

**Specifying set variables:** Sets of integers, reals, and strings
have similar specifications, though different value types. The
variables are specified using three keywords:

* Variable declaration keyword, e.g.,
  :dakkw:`variables-discrete_design_set`: specifies the number of
  variables being defined.

* ``elements_per_variable``: a list of positive integers specifying
  how many set members each variable admits

  - Length: # of variables
  - Default: equal apportionment of elements among variables

* elements: a list of the permissible integer values in ALL sets,
  concatenated together.

  - Length: sum of ``elements_per_variable``, or an integer multiple
    of number of variables
  - The order is very important here.
  - The list is partitioned according to the values of
    ``elements_per_variable``, and each partition is assigned to a
    variable.

* The ordering of elements_per_variable, and the partitions of
  elements must match the strings from descriptors


.. _`variables:parameters`:

Dakota Parameters File Data Formats
-----------------------------------

Simulation interfaces employ :dakkw:`forks <interface-analysis_drivers-fork>`
or :dakkw:`interface-analysis_drivers-system` calls to run simulation
workflows via user-developed drivers. Dakota communicates variable or parameter
values to the driver and receives back response values using the file system,
through the writing of parameters files and reading of results files.

Prior to invoking an analysis driver (or optional input or output filter),
Dakota creates a parameters file that contains the current parameter
values and a set of function requests. See the 
:ref:`interfacing <interfaces:overview>` portion of the manual for full
details.

Dakota supports three parameters file formats: standard, APREPRO, and
JSON. They are explained in full detail in the following sections.
Briefly, the standard format is unique to Dakota and uses a simple
``value tag`` syntax to communicate information about the evaluation.
In the APREPRO format, which is intended for use with the APREPRO
template processing utility :cite:p:`Sja92`, information is contained
in a series of of statements of the form ``{ tag = value }``. JSON is
JSON (JavaScript Object Notation), a common format for data interchange.


.. _`variables:parameters:standard`:

Parameters file format (standard)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The standard parameters file format for a single evaluation is 
shown in :numref:`variables:std_params_format`.

.. code-block::
   :caption: Parameters file data format - standard option
   :name: variables:std_params_format

   <int>    variables
   <double> <label_cdv_i>         (i = 1 to n_cdv)
   <int>    <label_ddiv_i>        (i = 1 to n_ddiv)
   <string> <label_ddsv_i>        (i = 1 to n_ddsv)
   <double> <label_ddrv_i>        (i = 1 to n_ddrv)
   <double> <label_cauv_i>        (i = 1 to n_cauv)
   <int>    <label_dauiv_i>       (i = 1 to n_dauiv)
   <string> <label_dausv_i>       (i = 1 to n_dausv)
   <double> <label_daurv_i>       (i = 1 to n_daurv)
   <double> <label_ceuv_i>        (i = 1 to n_ceuv)
   <int>    <label_deuiv_i>       (i = 1 to n_deuiv)
   <string> <label_deusv_i>       (i = 1 to n_deusv)
   <double> <label_deurv_i>       (i = 1 to n_deurv)
   <double> <label_csv_i>         (i = 1 to n_csv)
   <int>    <label_dsiv_i>        (i = 1 to n_dsiv)
   <string> <label_dssv_i>        (i = 1 to n_dssv)
   <double> <label_dsrv_i>        (i = 1 to n_dsrv)
   <int>    functions
   <int>    ASV_i:label_response_i       (i = 1 to m)
   <int>    derivative_variables
   <int>    DVV_i:label_cdv_i            (i = 1 to p)
   <int>    analysis_components
   <string> AC_i:analysis_driver_name_i  (i = 1 to q)
   <string> eval_id
   <int>    metadata
   <string> MD_i                         (i = 1 to r)

Integer values are denoted by ``<int>``, ``<double>`` denotes a double
precision value, and ``<string>`` denotes a string value. Each of the
major blocks denotes an array which begins with an array length and a
descriptive tag. These array lengths can be useful for dynamic memory
allocation within a simulator or filter program.

When using Dakota's :dakkw:`interface-batch` interface with the standard
format, information for multiple evaluations is written in a concatenated
fashion to a single batch parameters file. The format for each evaluation
is as shown in :numref:`variables:std_params_format`.

The first array for variables begins with the total number of variables
(``n``) with its identifier string ``variables``. The next ``n`` lines
specify the current values and descriptors of all of the variables
within the parameter set *in input specification order*: continuous design,
discrete integer design (integer range, integer set), discrete string
design (string set), discrete real design (real set), continuous
aleatory uncertain (normal, lognormal, uniform, loguniform, triangular,
exponential, beta, gamma, gumbel, frechet, weibull, histogram bin),
discrete integer aleatory uncertain (poisson, binomial, negative
binomial, geometric, hypergeometric, histogram point integer), discrete
string aleatory uncertain (histogram point string), discrete real
aleatory uncertain (histogram point real), continuous epistemic
uncertain (real interval), discrete integer epistemic uncertain
(interval, then set), discrete string epistemic uncertain (set),
discrete real epistemic uncertain (set), continuous state, discrete
integer state (integer range, integer set), discrete string state, and
discrete real state (real set) variables.

.. note::

   The authoritative variable ordering (as noted above in
   :ref:`variables:overview`) is given by the primary table in
   :dakkw:`variables`.

The lengths of these vectors add to a total of :math:`n`, i.e.,

.. math::

   n = n_{cdv} + n_{ddiv} + n_{ddsv} + n_{ddrv} + n_{cauv}
   + n_{dauiv} + n_{dausv} + n_{daurv} + n_{ceuv} + n_{deuiv} + n_{deusv}
   + n_{deurv} + n_{csv} + n_{dsiv} + n_{dssv} + n_{dsrv}.

If any of the variable types are not present in the problem, then its
block is omitted entirely from the parameters file. The labels come
from the variable descriptors specified in the Dakota input file, or
default descriptors based on variable type if not specified.

The second array for the active set vector (ASV) begins with the total
number of functions (``m``) and its identifier string ``functions``.
The next ``m`` lines specify the request vector for each of the ``m``
functions in the response data set followed by the tags
``ASV_i:label_response``, where the label is either a user-provided
response descriptor or a default-generated one. These integer codes
indicate what data is required on the current function evaluation and
are described further in :ref:`variables:asv`.

The third array for the derivative variables vector (DVV) begins with
the number of derivative variables (``p``) and its identifier string
``derivative_variables``. The next ``p`` lines specify integer
variable identifiers followed by the tags ``DVV_i:label_cdv``. These
integer identifiers are used to identify the subset of variables that
are active for the calculation of derivatives (gradient vectors and
Hessian matrices), and correspond to the list of variables in the first
array (e.g., an identifier of 2 indicates that the second variable in
the list is active for derivatives). The labels are again taken from
user-provided or default variable descriptors.

The fourth array for the analysis components (AC) begins with the number
of analysis components (``q``) and its identifier string
``analysis_components``. The next ``q`` lines provide additional
strings for use in specializing a simulation interface followed by the
tags ``AC_i:analysis_driver_name``, where ``analysis_driver_name``
indicates the driver associated with this component. These strings are
specified in the input file for a set of ``analysis_drivers`` using
the ``analysis_components`` specification. The subset of the analysis
components used for a particular analysis driver is the set passed in a
particular parameters file.

The next entry ``eval_id`` in the parameters file is the evaluation
ID, by default an integer indicating interface evaluation ID
number. When hierarchical tagging is enabled as described in
:ref:`interfaces:file:tagging1`, the identifier will be a
colon-separated string, e.g., ``4:9:2``.

The final array for the metadata (MD) begins with the number of
metadata fields requested (``r``) and its identifier string
``metadata``. The next ``r`` lines provide the names of
each metadata field followed by the tags ``MD_i``.

.. note::

   Several standard-format parameters file examples are shown in
   :ref:`interfaces:mappings`.


.. _`variables:parameters:aprepro`:

Parameters file format (APREPRO)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For the APREPRO format option, the same data is present in the same
order as the standard format. The only difference is that values are
associated with their tags using ``{ tag = value }`` markup as shown
in :numref:`variables:aprepro_params_format`. An APREPRO-format
parameters file example is shown in :ref:`interfaces:mappings`. The
APREPRO format allows direct usage of Dakota parameters files by the
APREPRO utility and Dakota's DPrePro, which are file pre-processors
that can significantly simplify model parameterization.

.. note::

   APREPRO :cite:p:`Sja92` is a Sandia-developed pre-processor that is
   not distributed with Dakota.

   DPrePro is a Python script distributed with Dakota that performs
   many of the same functions as APREPRO, as well as general template
   processing, and is optimized for use with Dakota parameters files
   in either format.

   BPREPRO and JPrePost are Perl and Java tools, respectively, in use
   at other sites.

When a parameters file in APREPRO format is included within a template
file (using an ``include`` directive), APREPRO recognizes these
constructs as variable definitions which can then be used to populate
targets throughout the template file. DPrePro, conversely, does not
require the use of includes since it processes the Dakota parameters
file and template simulation file separately to create a simulation
input file populated with the variables data.


.. code-block::
   :caption: Parameters file data format - APREPRO option
   :name: variables:aprepro_params_format

   { DAKOTA_VARS = <int> }
   { <label_cdv_i> = <double> }         (i = 1 to n_cdv)
   { <label_ddiv_i> = <int> }           (i = 1 to n_ddiv)
   { <label_ddsv_i> = <string> }        (i = 1 to n_ddsv)
   { <label_ddrv_i> = <double> }        (i = 1 to n_ddrv)
   { <label_cauv_i> = <double> }        (i = 1 to n_cauv)
   { <label_dauiv_i> = <int> }          (i = 1 to n_dauiv)
   { <label_dausv_i> = <string> }       (i = 1 to n_dausv)
   { <label_daurv_i> = <double> }       (i = 1 to n_daurv)
   { <label_ceuv_i> = <double> }        (i = 1 to n_ceuv)
   { <label_deuiv_i> = <int> }          (i = 1 to n_deuiv)
   { <label_deusv_i> = <string> }       (i = 1 to n_deusv)
   { <label_deurv_i> = <double> }       (i = 1 to n_deurv)
   { <label_csv_i> = <double> }         (i = 1 to n_csv)
   { <label_dsiv_i> = <int> }           (i = 1 to n_dsiv)
   { <label_dssv_i> = <string> }        (i = 1 to n_dssv)
   { <label_dsrv_i> = <double> }        (i = 1 to n_dsrv)
   { DAKOTA_FNS = <int> }
   { ASV_i:label_response_i = <int> }              (i = 1 to m)
   { DAKOTA_DER_VARS = <int> }
   { DVV_i:label_cdv_i = <int> }                   (i = 1 to p)
   { DAKOTA_AN_COMPS = <int> }
   { AC_i:analysis_driver_name_i = <string> }      (i = 1 to q)
   { DAKOTA_EVAL_ID = <string> }
   { DAKOTA_METADATA = <int> }
   { MD_i = <string> }                            (i = 1 to r)

As with the standard format, :dakkw:`interface-batch` parameters files
are simply a concatenation of the information for evaluations in the batch
in ARREPRO format.


.. _`variables:parameters:json`:

Parameters file format (JSON)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The JSON format encodes information using two structures, `objects`
and `arrays`. An object is a collection of name/value pairs. In many
programming languages it may be known as a dictionary, associative array,
map, or hash table. An array is an ordered list of values, and is commonly
known as an array, vector, or list. Objects and arrays may contain other
objects or arrays, or scalar values that have "primitive" types such as 
strings, numbers, or booleans.

In Dakota's JSON format, information about each evaluation is stored in a 
top-level object. The object contains the names (also known as keys):

.. code-block:: JSON
   :caption: Top-level organization of an evaluation in JSON
   :name: variables:json_params_format:top

   {
     "variables": [],
     "responses": [],
     "derivative_variables": []
     "analysis_components": [],
     "eval_id": "",
     "metadata": []
   }

Unlike in the standard and APREPRO formats, the numbers of variables,
responses, derviatve variables, etc, are not explicitly included in the JSON
parameters file. They are unnecessary for parsing the file and are simply
the lengths of the array in question. Another difference between the JSON
format and the standard and APREPRO formats arises when using Dakota's
:dakkw:`interface-batch` interface. The top-level data structure of a
JSON format batch parameters file is an array, which contains evaluation
objects.

Variables
^^^^^^^^^

Variable labels and values are stored within objects that are elements
of the `variables` array. Each object resembles the following, where
the variable value is an integer, double, or string, as appropriate.


.. code-block:: JSON
   :caption: Array element of `variables` object
   :name: variables:json_params_format:variables

   {
     "label": "<label_var_i>",
     "value": <variable value>
   }

The order of the variables in the array is the same as for the 
standard and APREPRO format files, described in the previous two
sections.

Responses
^^^^^^^^^

The `responses` name is associated with an array of objects that
store the label and active set for each expected response.

.. code-block:: JSON
   :caption: Array element of `responses` object
   :name: variables:json_params_format:responses

   {
     "label": "<label_response_i>",
     "active_set": <int>
   }


Derivative Variables
^^^^^^^^^^^^^^^^^^^^

Gradients and Hessians, if requested, are expected to be computed
with respect to the `derivative_variables`. The array associated
with this key contains 1-based indices into the `variables` array.

Analysis Components
^^^^^^^^^^^^^^^^^^^

The `analysis_components` name is associated with an array of
analysis components objects of the form:

.. code-block:: JSON
   :caption: Array element of `analysis_components` object
   :name: variables:json_params_format:an_comps

   {
     "driver": "<driver_string>",
     "component": "<an_comp_i>"
   }

Evaluation ID and Metadata
^^^^^^^^^^^^^^^^^^^^^^^^^^

Finally, the evaluation ID is a string associated with the `eval_id`
key, and the `metadata` name refers to an array of strings, a list
of the expected metadata responses.
   


.. _`variables:asv`:

The Active Set Vector
---------------------

The active set vector (ASV) specifies the function value or derivative
response data needed for a particular interface evaluation. Dakota's
ASV gets its name from managing the active set, i.e., the set of
functions that are required by a method on a particular function
evaluation. However, it also indicates the derivative data needed for
active functions, so has an extended meaning beyond that typically
used in the optimization literature.

.. note::

   By default a simulation interface is expected to parse the ASV and
   only return the requested functions, gradients, and Hessians. To
   alleviate this requirement, see deactivating below.

The active set vector is comprised of vector of integer codes 0--7,
one per response function. The integer values 0 through 7 denote a
3-bit binary representation of all possible combinations of value (1),
gradient (2), and Hessian (4) requests for a particular function, with
the most significant bit denoting the Hessian, the middle bit denoting
the gradient, and the least significant bit denoting the value. The
specific translations are shown in :numref:`variables:asv_codes`.

.. table:: Active set vector integer codes.
   :name: variables:asv_codes

   +--------------+-----------------------+----------------------------------------+
   | Integer Code | Binary representation | Meaning                                |
   +==============+=======================+========================================+
   | 7            | 111                   | Get Hessian, gradient, and value       |
   +--------------+-----------------------+----------------------------------------+
   | 6            | 110                   | Get Hessian and gradient               |
   +--------------+-----------------------+----------------------------------------+
   | 5            | 101                   | Get Hessian and value                  |
   +--------------+-----------------------+----------------------------------------+
   | 4            | 100                   | Get Hessian                            |
   +--------------+-----------------------+----------------------------------------+
   | 3            | 011                   | Get gradient and value                 |
   +--------------+-----------------------+----------------------------------------+
   | 2            | 010                   | Get gradient                           |
   +--------------+-----------------------+----------------------------------------+
   | 1            | 001                   | Get value                              |
   +--------------+-----------------------+----------------------------------------+
   | 0            | 000                   | No data required, function is inactive |
   +--------------+-----------------------+----------------------------------------+


**Disabling the ASV:** Active set vector control may be turned off to
obviate the need for the interface script to check and respond to its
contents. When :dakkw:`interface-deactivate`
:dakkw:`interface-deactivate-active_set_vector` is specified, the
interface is expected to return all function, gradient, and Hessian
information enabled in the :dakkw:`responses` block on every function
evaluation.

This option affords a simpler interface implemention, but of course in
trade for efficiency. Disabling is most appropriate for cases in which
only a relatively small penalty occurs when computing and returning
more data than needed on a particular function evaluation.
