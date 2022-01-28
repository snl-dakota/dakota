Variables
=========

.. _`variables:overview`:

Overview
--------

The ``variables`` specification in a Dakota input file specifies the
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

This chapter will present a brief overview of the main types of
variables and their uses, as well as cover some user issues relating to
file formats and the active set vector. For a detailed description of
variables section syntax and example specifications, refer to the
variables keywords in the Dakota Reference
Manual :cite:p:`RefMan`.

.. _`variables:design`:

Design Variables
----------------

Design variables are those variables which are modified in the course of
determining an optimal design. These variables may be continuous
(real-valued between bounds), discrete range (integer-valued between
bounds), discrete set of integers (integer-valued from finite set),
discrete set of strings (string-valued from finite set), and discrete
set of reals (real-valued from finite set).

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
of string values (:math:`x` can be ``’direct’``, ``’gmres’``, or
``’jacobi’``), or a set of real values (e.g., :math:`x` can be
identically ``4.2``, ``6.4``, or ``8.5``).

Discrete variables may be classified as either “categorical” or
“noncategorical.” In the latter noncategorical case, the discrete
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
of some feature (e.g., number of support brackets). Dakota supports a
``categorical`` specification to indicate which discrete real and
discrete integer variables are restricted vs. relaxable. String
variables cannot be relaxed.

Gradient-based optimization methods cannot be directly applied to
problems with discrete variables since derivatives only exist for a
variable continuum. For problems with noncategorical variables, the
experimental branch and bound capability (``branch_and_bound``) can be
used to relax the discrete requirements and apply gradient-based methods
to a series of generated subproblems. For problems with categorical
variables, nongradient-based methods (e.g., ``coliny_ea``) are commonly
used; however, most of those methods do not take advantage of any
structure that may be associated with the categorical variables. The
exception is ``mesh_adaptive_search``. If it is possible to define a
subjective relationship between the different values a given categorical
variable can take on, the user can communicate that relationship in the
form of an adjacency matrix. The ``mesh_adaptive_search`` will take that
relationship into consideration. Further documentation can be found
in :cite:p:`RefMan` under the keywords ``adjacency_matrix``
and ``neighbor_order``. Branch and bound techniques are discussed in
Section `[adv_meth:minlp] <#adv_meth:minlp>`__ and nongradient-based
methods are further described in Chapter `[opt] <#opt>`__.

In addition to engineering applications, many non-engineering
applications in the fields of scheduling, logistics, and resource
allocation contain discrete design parameters. Within the Department of
Energy, solution techniques for these problems impact programs in
stockpile evaluation and management, production planning,
nonproliferation, transportation (routing, packing, logistics),
infrastructure analysis and design, energy production, environmental
remediation, and tools for massively parallel computing such as domain
decomposition and meshing.

.. _`variables:design:ddiv`:

Discrete Design Integer Variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are two types of discrete design integer variables supported by
Dakota.

-  The ``discrete_design_range`` specification supports a range of
   consecutive integers between specified ``lower_bounds`` and
   ``upper_bounds``.

-  The specification supports a set of enumerated integer values through
   the ``elements`` specification. The set of values specified is stored
   internally as an STL set container, which enforces an ordered, unique
   representation of the integer data. Underlying this set of ordered,
   unique integers is a set of indices that run from 0 to one less than
   the number of set values. These indices are used by some iterative
   algorithms (e.g., parameter studies, SCOLIB iterators) for simplicity
   in discrete value enumeration when the actual corresponding set
   values are immaterial. In the case of parameter studies, this index
   representation is exposed through certain step and partition control
   specifications (see Chapter `[ps] <#ps>`__).

.. _`variables:design:ddsv`:

Discrete Design String Variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There is one type of discrete design string variable supported by
Dakota.

-  The ``discrete_design_set string`` specification supports a set of
   enumerated string values through the ``elements`` specification. As
   for the discrete integer set variables described in
   Section `1.2.2.1 <#variables:design:ddiv>`__, internal storage of the
   set values is ordered and unique and an underlying index
   representation is exposed for the specification of some iterative
   algorithms.

Each string element value must be quoted in the Dakota input file and
may contain alphanumeric, dash, underscore, and colon. White space,
quote characters, and backslash/meta-characters are not permitted.

.. _`variables:design:ddrv`:

Discrete Design Real Variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There is one type of discrete design real variable supported by Dakota.

-  The ``discrete_design_set real`` specification specification supports
   a set of enumerated real values through the ``set_values``
   specification. As for the discrete integer set variables described in
   Section `1.2.2.1 <#variables:design:ddiv>`__, internal storage of the
   set values is ordered and unique and an underlying index
   representation is exposed for the specification of some iterative
   algorithms.

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

.. _`variables:uncertain:auv`:

Aleatory Uncertain Variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Aleatory uncertainties are irreducible variabilities inherent in nature.
They are commonly modeled using probability distributions, and
probabilistic methods are commonly used for propagating input aleatory
uncertainties described by probability distribution specifications. The
two following sections describe the continuous and discrete aleatory
uncertain variables supported by Dakota.

For aleatory random variables, Dakota supports a user-supplied
correlation matrix to provide correlations among the input variables. By
default, the correlation matrix is set to the identity matrix, i.e., no
correlation among the uncertain variables.

For additional information on random variable probability distributions,
refer to :cite:p:`Hal00` and :cite:p:`Swi04`. Refer
to the Dakota Reference Manual :cite:p:`RefMan` for more
detail on the uncertain variable specifications and to
Chapter `[uq] <#uq>`__ for a description of methods available to
quantify the uncertainty in the response.

.. _`variables:uncertain:cauv`:

Continuous Aleatory Uncertain Variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  Normal: a probability distribution characterized by a mean and
   standard deviation. Also referred to as Gaussian. Bounded normal is
   also supported by some methods with an additional specification of
   lower and upper bounds.

-  Lognormal: a probability distribution characterized by a mean and
   either a standard deviation or an error factor. The natural logarithm
   of a lognormal variable has a normal distribution. Bounded lognormal
   is also supported by some methods with an additional specification of
   lower and upper bounds.

-  Uniform: a probability distribution characterized by a lower bound
   and an upper bound. Probability is constant between the bounds.

-  Loguniform: a probability distribution characterized by a lower bound
   and an upper bound. The natural logarithm of a loguniform variable
   has a uniform distribution.

-  Triangular: a probability distribution characterized by a mode, a
   lower bound, and an upper bound.

-  Exponential: a probability distribution characterized by a beta
   parameter.

-  Beta: a flexible probability distribution characterized by a lower
   bound and an upper bound and alpha and beta parameters. The uniform
   distribution is a special case.

-  Gamma: a flexible probability distribution characterized by alpha and
   beta parameters. The exponential distribution is a special case.

-  Gumbel: the Type I Largest Extreme Value probability distribution.
   Characterized by alpha and beta parameters.

-  Frechet: the Type II Largest Extreme Value probability distribution.
   Characterized by alpha and beta parameters.

-  Weibull: the Type III Smallest Extreme Value probability
   distribution. Characterized by alpha and beta parameters.

-  Histogram Bin: an empirically-based probability distribution
   characterized by a set of :math:`(x,y)` pairs that map out histogram
   bins (a continuous interval with associated bin count).

.. _`variables:uncertain:dauv`:

Discrete Aleatory Uncertain Variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following types of discrete aleatory uncertain variables are
available:

-  Poisson: integer-valued distribution used to predict the number of
   discrete events that happen in a given time interval.

-  Binomial: integer-valued distribution used to predict the number of
   failures in a number of independent tests or trials.

-  Negative Binomial: integer-valued distribution used to predict the
   number of times to perform a test to have a target number of
   successes.

-  Geometric: integer-valued distribution used to model the number of
   successful trials that might occur before a failure is observed.

-  Hypergeometric: integer-valued distribution used to model the number
   of failures observed in a set of tests that has a known proportion of
   failures.

-  Histogram Point (integer, string, real): an empirically-based
   probability distribution characterized by a set of integer-valued
   :math:`(i,c)`, string-valued :math:`(s,c)`, and/or real-valued
   :math:`{r,c}` pairs that map out histogram points (each a discrete
   point value :math:`i`, :math:`s`, or :math:`r`, with associated count
   :math:`c`).

.. _`variables:uncertain:euv`:

Epistemic Uncertain Variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Epistemic uncertainties are reducible uncertainties resulting from a
lack of knowledge. For epistemic uncertainties, use of probability
distributions is based on subjective prior knowledge rather than
objective data, and we may alternatively explore non-probabilistic
specifications based on intervals or Dempster-Shafer structures. Dakota
currently supports the following epistemic uncertain variable types.

.. _`variables:uncertain:ceuv`:

Continuous Epistemic Uncertain Variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  Continuous Interval: a real-valued interval-based specification
   characterized by sets of lower and upper bounds and Basic Probability
   Assignments (BPAs) associated with each interval. The intervals may
   be overlapping, contiguous, or disjoint, and a single interval (with
   probability = 1) per variable is an important special case. The
   interval distribution is not a probability distribution, as the exact
   structure of the probabilities within each interval is not known. It
   is commonly used with epistemic uncertainty methods.

.. _`variables:uncertain:deuv`:

Discrete Epistemic Uncertain Variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  Discrete Interval: an integer-valued variant of the Continuous
   Interval described above ( `1.3.2.1 <#variables:uncertain:ceuv>`__).

-  Discrete Set (integer, string, and real): Similar to discrete design
   set variables `1.2.2 <#variables:design:ddv>`__, these epistemic
   variables admit a finite number of values (elements) for type
   integer, string, or real, each with an associated probability.

.. _`variables:state`:

State Variables
---------------

State variables consist of “other” variables which are to be mapped
through the simulation interface, in that they are not to be used for
design and they are not modeled as being uncertain. State variables
provide a convenient mechanism for parameterizing additional model
inputs which, in the case of a numerical simulator, might include solver
convergence tolerances, time step controls, or mesh fidelity parameters.
For additional model parameterizations involving strings (e.g.,
“mesh1.exo”), refer to the analysis components specification described
in Section `1.6.1 <#variables:parameters:standard>`__ and in the
Interface Commands chapter of the Dakota Reference
Manual :cite:p:`RefMan`. Similar to the design variables
discussed in Section `1.2 <#variables:design>`__, state variables can be
specified with a continuous range (real-valued between bounds), a
discrete range (integer-valued between bounds), a discrete
integer-valued set, a discrete string-valued set, or a discrete
real-valued set.

State variables, as with other types of variables, are viewed
differently depending on the method in use. Since these variables are
neither design nor uncertain variables, algorithms for optimization,
least squares, and uncertainty quantification do not iterate on these
variables; i.e., they are not active and are hidden from the algorithm.
However, Dakota still maps these variables through the user’s interface
where they affect the computational model in use. This allows
optimization, least squares, and uncertainty quantification studies to
be executed under different simulation conditions (which will result, in
general, in different results). Parameter studies and design of
experiments methods, on the other hand, are general-purpose iterative
techniques which do not draw a distinction between variable types. They
include state variables in the set of variables to be iterated, which
allows these studies to explore the effect of state variable values on
the response data of interest.

In the future, state variables might be used in direct coordination with
an optimization, least squares, or uncertainty quantification algorithm.
For example, state variables could be used to enact model adaptivity
through the use of a coarse mesh or loose solver tolerances in the
initial stages of an optimization with continuous model refinement as
the algorithm nears the optimal solution.

.. _`variables:mixed`:

Management of Mixed Variables by Iterator
-----------------------------------------

.. _`variables:mixedview`:

View
~~~~

As alluded to in the previous section, the iterative method selected for
use in Dakota partially determines what subset, or view, of the
variables data is active in the iteration.
(Section `1.5.3 <#variables:precedence>`__ contains a discussion of how
user overrides, response function type, and method are used to determine
active variable view.) The general case of having a mixture of various
different types of variables is supported within all of the Dakota
methods even though certain methods will only modify certain types of
variables (e.g., optimizers and least squares methods only modify design
variables, and uncertainty quantification methods typically only utilize
uncertain variables). This implies that variables which are not under
the direct control of a particular iterator will be mapped through the
interface in an unmodified state. This allows for a variety of
parameterizations within the model in addition to those which are being
used by a particular iterator, which can provide the convenience of
consolidating the control over various modeling parameters in a single
file (the Dakota input file). An important related point is that the
variable set that is active with a particular iterator is the same
variable set for which derivatives are typically computed (see
Section `[responses:active] <#responses:active>`__).

There are certain situations where the user may want to explicitly
control the subset of variables that is considered active for a certain
Dakota method. This is done by specifying the keyword ``active`` in the
variables specification block, followed by one of the following:
``all``, ``design``, ``uncertain``, ``aleatory``, ``epistemic``, or
``state``. Specifying one of these subsets of variables will allow the
Dakota method to operate on the specified variable types and override
the defaults. For example, the default behavior for a nondeterministic
sampling method is to sample the uncertain variables. However, if the
user specified ``active`` ``all`` in the variables specification block,
the sampling would be performed over all variables (e.g. design and
state variables as well as uncertain variables). This may be desired in
situations such as surrogate based optimization under uncertainty, where
a surrogate may be built over both design and uncertain variables.
Another situation where one may want the fine-grained control available
by specifying one of these variable types is when one has state
variables but only wants to sample over the design variables when
constructing a surrogate model. Finally, more sophisticated uncertainty
studies may involve various combinations of epistemic vs. aleatory
variables being active in nested models.

.. _`variables:domain`:

Domain
~~~~~~

Another control that the user can specify in the variables specification
block controls the domain type. We have two domains currently: mixed and
relaxed. Both domain types can have design, uncertain, and state
variables. The domain specifies how the discrete variables are treated.
If the user specifies ``mixed`` in the variable specification block, the
continuous and discrete variables are treated separately. If the user
specifies ``relaxed`` in the variable specification block, the discrete
variables are relaxed and treated as continuous variables. This may be
useful in optimization problems involving both continuous and discrete
variables when a user would like to use an optimization method that is
designed for continuous variable optimization. All Dakota methods have a
default value of mixed for the domain type except for the
branch-and-bound method which has a default domain type of relaxed. Note
that the branch-and-bound method is experimental and still under
development at this time.

.. _`variables:precedence`:

Precedence
~~~~~~~~~~

If the user does not specify any explicit override of the active view of
the variables, Dakota then considers the response function
specification. If the user specifies objective functions or calibration
terms in the response specification block, the active variables will be
the design variables. If the user specifies the more generic response
type, ``response_functions``, general response functions do not have a
specific interpretation the way ``objective_functions`` or
``calibration_terms`` do. In the case of generic response functions,
Dakota then tries to infer the active view from the method. If the
method is a parameter study, or any of the methods available under dace,
psuade, or fsu methods, the active view is set to all variables. For
uncertainty quantification methods, if the method is sampling, then the
view is set to aleatory if only aleatory variables are present,
epistemic if only epistemic variables are present, or uncertain
(covering both aleatory and epistemic) if both are present. If the
uncertainty method involves interval estimation or evidence
calculations, the view is set to epistemic. For other uncertainty
quantification methods not mentioned in the previous sentences (e.g.,
reliability methods or stochastic expansion methods), the view is set to
aleatory. Finally, for verification studies using the Richardson
extrapolation method, the active view is set to state. Note that in
surrogate-based optimization, where the surrogate is built on points
defined by the method defined by the ``dace_method_pointer``, the
sampling used to generate the points is performed only over the design
variables as a default unless otherwise specified (e.g. state variables
will not be sampled for surrogate construction).

With respect to domain type, if the user does not specify an explicit
override of ``mixed`` or ``relaxed``, Dakota infers the domain type from
the method. As mentioned above, all methods currently use a mixed domain
as a default, except the branch-and-bound method which is under
development.

.. _`variables:parameters`:

Dakota Parameters File Data Format
----------------------------------

Simulation interfaces which employ system calls and forks to create
separate simulation processes must communicate with the simulation code
through the file system. This is accomplished through the reading and
writing of parameters and results files. Dakota uses a particular format
for this data input/output. Depending on the user’s interface
specification, Dakota will write the parameters file in either standard
or APREPRO format (future XML formats are planned). The former option
uses a simple “``value tag``” format, whereas the latter option uses a
“``{ tag = value }``” format for compatibility with the APREPRO
utility :cite:p:`Sja92` (as well as DPrePro, BPREPRO, and
JPrePost variants).

.. _`variables:parameters:standard`:

Parameters file format (standard)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prior to invoking a simulation, Dakota creates a parameters file which
contains the current parameter values and a set of function requests.
The standard format for this parameters file is shown in
Figure `[variables:figure01] <#variables:figure01>`__.

.. container:: bigbox

   | ``<int>``\ :raw-latex:`\ `\ ``variables``
   | ``<double>``\ :raw-latex:`\ `\ ``<label_cdv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{cdv}`\ ``)``
   | ``<int>``\ :raw-latex:`\ `\ ``<label_ddiv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{ddiv}`\ ``)``
   | ``<string>``\ :raw-latex:`\ `\ ``<label_ddsv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{ddsv}`\ ``)``
   | ``<double>``\ :raw-latex:`\ `\ ``<label_ddrv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{ddrv}`\ ``)``
   | ``<double>``\ :raw-latex:`\ `\ ``<label_cauv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{cauv}`\ ``)``
   | ``<int>``\ :raw-latex:`\ `\ ``<label_dauiv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{dauiv}`\ ``)``
   | ``<string>``\ :raw-latex:`\ `\ ``<label_dausv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{dausv}`\ ``)``
   | ``<double>``\ :raw-latex:`\ `\ ``<label_daurv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{daurv}`\ ``)``
   | ``<double>``\ :raw-latex:`\ `\ ``<label_ceuv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{ceuv}`\ ``)``
   | ``<int>``\ :raw-latex:`\ `\ ``<label_deuiv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{deuiv}`\ ``)``
   | ``<string>``\ :raw-latex:`\ `\ ``<label_deusv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{deusv}`\ ``)``
   | ``<double>``\ :raw-latex:`\ `\ ``<label_deurv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{deurv}`\ ``)``
   | ``<double>``\ :raw-latex:`\ `\ ``<label_csv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{csv}`\ ``)``
   | ``<int>``\ :raw-latex:`\ `\ ``<label_dsiv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{dsiv}`\ ``)``
   | ``<string>``\ :raw-latex:`\ `\ ``<label_dssv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{dssv}`\ ``)``
   | ``<double>``\ :raw-latex:`\ `\ ``<label_dsrv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{dsrv}`\ ``)``
   | ``<int>``\ :raw-latex:`\ `\ ``functions``
   | ``<int>``\ :raw-latex:`\ `\ ``ASV_i:label_response``\ :math:`\sb{i}`\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``m)``
   | ``<int>``\ :raw-latex:`\ `\ ``derivative_variables``
   | ``<int>``\ :raw-latex:`\ `\ ``DVV_i:label_cdv``\ :math:`\sb{i}`\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``p)``
   | ``<int>``\ :raw-latex:`\ `\ ``analysis_components``
   | ``<string>``\ :raw-latex:`\ `\ ``AC_i:analysis_driver_name``\ :math:`\sb{i}`\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``q)``
   | ``<string>``\ :raw-latex:`\ `\ ``eval_id``

Integer values are denoted by “``<int>``”, “``<double>``” denotes a
double precision value, and “``<string>``” denotes a string value. Each
of the colored blocks (black for variables, blue for active set vector,
red for derivative variables vector, and green for analysis components)
denotes an array which begins with an array length and a descriptive
tag. These array lengths are useful for dynamic memory allocation within
a simulator or filter program.

The first array for variables begins with the total number of variables
(``n``) with its identifier string “``variables``.” The next ``n`` lines
specify the current values and descriptors of all of the variables
within the parameter set *in the following order*: continuous design,
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
discrete real state (real set) variables. This ordering is consistent
with the lists in Sections `1.2.2.1 <#variables:design:ddiv>`__,
`1.3.1.1 <#variables:uncertain:cauv>`__
and `1.3.1.2 <#variables:uncertain:dauv>`__ and the specification order
in dakota.input.summary. The lengths of these vectors add to a total of
:math:`n` (that is,
:math:`n = n_{cdv} + n_{ddiv} + n_{ddsv} + n_{ddrv} + n_{cauv}
+ n_{dauiv} + n_{dausv} + n_{daurv} + n_{ceuv} + n_{deuiv} + n_{deusv}
+ n_{deurv} + n_{csv} + n_{dsiv} + n_{dssv} + n_{dsrv}`). If any of the
variable types are not present in the problem, then its block is omitted
entirely from the parameters file. The tags are the variable descriptors
specified in the user’s Dakota input file, or if no descriptors have
been specified, default descriptors are used.

The second array for the active set vector (ASV) begins with the total
number of functions (``m``) and its identifier string “``functions``.”
The next ``m`` lines specify the request vector for each of the ``m``
functions in the response data set followed by the tags
“``ASV_i:label_response``”, where the label is either a user-provided
response descriptor or a default-generated one. These integer codes
indicate what data is required on the current function evaluation and
are described further in Section `1.7 <#variables:asv>`__.

The third array for the derivative variables vector (DVV) begins with
the number of derivative variables (``p``) and its identifier string
“``derivative_variables``.” The next ``p`` lines specify integer
variable identifiers followed by the tags “``DVV_i:label_cdv``”. These
integer identifiers are used to identify the subset of variables that
are active for the calculation of derivatives (gradient vectors and
Hessian matrices), and correspond to the list of variables in the first
array (e.g., an identifier of 2 indicates that the second variable in
the list is active for derivatives). The labels are again taken from
user-provided or default variable descriptors.

The final array for the analysis components (AC) begins with the number
of analysis components (``q``) and its identifier string
“``analysis_components``.” The next ``q`` lines provide additional
strings for use in specializing a simulation interface followed by the
tags “``AC_i:analysis_driver_name``”, where ``analysis_driver_name``
indicates the driver associated with this component. These strings are
specified in a user’s input file for a set of ``analysis_drivers`` using
the ``analysis_components`` specification. The subset of the analysis
components used for a particular analysis driver is the set passed in a
particular parameters file.

The final entry ``eval_id`` in the parameters file is the evaluation ID,
by default an integer indicating interface evaluation ID number. When
hierarchical tagging is enabled as described
in `[interfaces:file:tagging1] <#interfaces:file:tagging1>`__, the
identifier will be a colon-separated string, e.g., 4:9:2. Several
standard-format parameters file examples are shown in
Section `[interfaces:mappings] <#interfaces:mappings>`__.

.. _`variables:parameters:aprepro`:

Parameters file format (APREPRO)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For the APREPRO format option, the same data is present and the same
ordering is used as in the standard format. The only difference is that
values are associated with their tags within “``{ tag = value }``”
constructs as shown in
Figure `[variables:figure02] <#variables:figure02>`__. An APREPRO-format
parameters file example is shown in
Section `[interfaces:mappings] <#interfaces:mappings>`__.

The use of the APREPRO format option allows direct usage of these
parameters files by the APREPRO utility, which is a file pre-processor
that can significantly simplify model parameterization. Similar
pre-processors include DPrePro, BPREPRO, and JPrePost. *[Note: APREPRO
is a Sandia-developed pre-processor that is not currently distributed
with Dakota. DPrePro is a Perl script distributed with Dakota that
performs many of the same functions as APREPRO, and is optimized for use
with Dakota parameters files in either format. BPREPRO and JPrePost are
additional Perl and JAVA tools, respectively, in use at other sites.]*
When a parameters file in APREPRO format is included within a template
file (using an include directive), the APREPRO utility recognizes these
constructs as variable definitions which can then be used to populate
targets throughout the template file :cite:p:`Sja92`. DPrePro,
conversely, does not require the use of includes since it processes the
Dakota parameters file and template simulation file separately to create
a simulation input file populated with the variables data.

.. container:: bigbox

   | ``{``\ :raw-latex:`\ `\ ``DAKOTA_VARS``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<int>``\ :raw-latex:`\ `\ ``}``
   | ``{``\ :raw-latex:`\ `\ ``<label_cdv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<double>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{cdv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_ddiv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<int>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{ddiv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_ddsv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<string>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{ddsv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_ddrv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<double>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{ddrv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_cauv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<double>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{cauv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_dauiv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<int>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{dauiv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_dausv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<string>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{dausv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_daurv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<double>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{daurv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_ceuv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<double>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{ceuv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_deuiv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<int>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{deuiv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_deusv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<string>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{deusv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_deurv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<double>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{deurv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_csv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<double>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{csv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_dsiv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<int>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{dsiv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_dssv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<string>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{dssv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``<label_dsrv``\ :math:`\sb{i}`\ ``>``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<double>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``n``\ :math:`\sb{dsrv}`\ ``)``
   | ``{``\ :raw-latex:`\ `\ ``DAKOTA_FNS``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<int>``\ :raw-latex:`\ `\ ``}``
   | ``{``\ :raw-latex:`\ `\ ``ASV_i:label_response``\ :math:`\sb{i}`\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<int>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``m)``
   | ``{``\ :raw-latex:`\ `\ ``DAKOTA_DER_VARS``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<int>``\ :raw-latex:`\ `\ ``}``
   | ``{``\ :raw-latex:`\ `\ ``DVV_i:label_cdv``\ :math:`\sb{i}`\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<int>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``p)``
   | ``{``\ :raw-latex:`\ `\ ``DAKOTA_AN_COMPS``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<int>``\ :raw-latex:`\ `\ ``}``
   | ``{``\ :raw-latex:`\ `\ ``AC_i:analysis_driver_name``\ :math:`\sb{i}`\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<string>``\ :raw-latex:`\ `\ ``}``\ :raw-latex:`\ `\ ``(i``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``1``\ :raw-latex:`\ `\ ``to``\ :raw-latex:`\ `\ ``q)``
   | ``{``\ :raw-latex:`\ `\ ``DAKOTA_EVAL_ID``\ :raw-latex:`\ `\ ``=``\ :raw-latex:`\ `\ ``<string>``\ :raw-latex:`\ `\ ``}``

.. _`variables:asv`:

The Active Set Vector
---------------------

The active set vector contains a set of integer codes, one per response
function, which describe the data needed on a particular execution of an
interface. Integer values of 0 through 7 denote a 3-bit binary
representation of all possible combinations of value, gradient, and
Hessian requests for a particular function, with the most significant
bit denoting the Hessian, the middle bit denoting the gradient, and the
least significant bit denoting the value. The specific translations are
shown in Table `1.1 <#variables:table01>`__.

.. container::
   :name: variables:table01

   .. table:: Active set vector integer codes.

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

The active set vector in Dakota gets its name from managing the active
set, i.e., the set of functions that are active on a particular function
evaluation. However, it also manages the type of data that is needed for
functions that are active, and in that sense, has an extended meaning
beyond that typically used in the optimization literature.

.. _`variables:asv:control`:

Active set vector control
~~~~~~~~~~~~~~~~~~~~~~~~~

Active set vector control may be turned off to allow the user to
simplify the supplied interface by removing the need to check the
content of the active set vector on each evaluation. The Interface
Commands chapter in the Dakota Reference
Manual :cite:p:`RefMan` provides additional information on
this option ( ). Of course, this option trades some efficiency for
simplicity and is most appropriate for those cases in which only a
relatively small penalty occurs when computing and returning more data
than may be needed on a particular function evaluation.
