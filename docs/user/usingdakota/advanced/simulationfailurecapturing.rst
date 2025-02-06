.. _failure:

Simulation Failure Capturing
============================

Dakota provides the capability to manage failures in simulation codes
within its system call, fork, and direct simulation interfaces (see
:ref:`Simulation Interfaces<interfaces:sim>` for simulation interface
descriptions). Failure capturing consists of three operations: failure
detection, failure communication, and failure mitigation.

.. _`failure:detection`:

Failure detection
-----------------

Since the symptoms of a simulation failure are highly code and
application dependent, it is the user’s responsibility to detect
failures within their ``analysis_driver``, ``input_filter``, or
``output_filter``. One popular example of simulation monitoring is to
rely on a simulation’s internal detection of errors. In this case, the
UNIX ``grep`` utility can be used within a user’s driver/filter script
to detect strings in output files which indicate analysis failure. For
example, the following simple C shell script excerpt

::

       grep ERROR analysis.out > /dev/null
       if ( $status == 0 )
         echo "FAIL" > results.out
       endif

will pass the ``if`` test and communicate simulation failure to Dakota
if the ``grep`` command finds the string ``ERROR`` anywhere in the
``analysis.out`` file.  The ``/dev/null`` device file is called the
“bit bucket” and the ``grep`` command
output is discarded by redirecting it to this destination. The
``$status`` shell variable contains the exit status of the last command
executed :cite:p:`And86`, which is the exit status of ``grep``
in this case (0 if successful in finding the error string, nonzero
otherwise). For Bourne shells :cite:p:`Bli96`, the ``$?``
shell variable serves the same purpose as ``$status`` for C shells. In a
related approach, if the return code from a simulation can be used
directly for failure detection purposes, then ``$status`` or ``$?``
could be queried immediately following the simulation execution using an
``if`` test like that shown above.

If the simulation code is not returning error codes or providing direct
error diagnostic information, then failure detection may require
monitoring of simulation results for sanity (e.g., is the mesh
distorting excessively?) or potentially monitoring for continued process
existence to detect a simulation segmentation fault or core dump. While
this can get complicated, the flexibility of Dakota’s interfaces allows
for a wide variety of user-defined monitoring approaches.

.. _`failure:communication`:

Failure communication
---------------------

Once a failure is detected, it must be communicated so that Dakota can
take the appropriate corrective action. The form of this communication
depends on the type of simulation interface in use.

In the system call and fork simulation interfaces, a detected simulation
failure is communicated to Dakota through the results file. When using
the :ref:`standard <responses:results:standard>` results file format,
the string “``fail``” should appear at the beginning of the results file.
Any data appearing after the fail string will be ignored. Also, Dakota’s
detection of this string is case insensitive, so “``FAIL``”, “``Fail``”,
etc., are equally valid. For :ref:`JSON <responses:results:json>`, failure
is communicated to Dakota by including the name:value pair ``"fail": "true"``
in the evaluation object. Both the name and value must be lowercase.

In the direct simulation interface case, a detected simulation failure
is communicated to Dakota through the return code provided by the user’s
``analysis_driver``, ``input_filter``, or ``output_filter``. As shown in
:ref:`Extension<advint:direct:extension>`, the
prototype for simulations linked within the direct interface includes an
integer return code. This code has the following meanings: zero (false)
indicates that all is normal and nonzero (true) indicates an exception
(i.e., a simulation failure).

.. _`failure:mitigation`:

Failure mitigation
------------------

Once the analysis failure has been communicated, Dakota will attempt to
recover from the failure using one of the following four mechanisms, as
governed by the :dakkw:`interface` specification in the user’s input file.

.. _`failure:mitigation:abort`:

Abort (default)
~~~~~~~~~~~~~~~

If the :dakkw:`interface-failure_capture-abort` option is active (the default), then Dakota will
terminate upon detecting a failure. Note that if the problem causing the
failure can be corrected, Dakota’s restart capability (see
:ref:`The Dakota Restart Utility<dakota_restart_utility>`) can be used to continue the study.

.. _`failure:mitigation:retry`:

Retry
~~~~~

If the :dakkw:`interface-failure_capture-retry` option is specified, then Dakota will re-invoke the
failed simulation up to the specified number of retries. If the
simulation continues to fail on each of these retries, Dakota will
terminate. The retry option is appropriate for those cases in which
simulation failures may be resulting from transient computing
environment issues, such as shared disk space, software license access,
or networking problems.

.. _`failure:mitigation:recover`:

Recover
~~~~~~~

If the :dakkw:`interface-failure_capture-recover` option is specified, then Dakota will not attempt the
failed simulation again. Rather, it will return a “dummy” set of
function values as the results of the function evaluation. The dummy
function values to be returned are specified by the user. Any gradient
or Hessian data requested in the active set vector will be zero. This
option is appropriate for those cases in which a failed simulation may
indicate a region of the design space to be avoided and the dummy values
can be used to return a large objective function or constraint violation
which will discourage an optimizer from further investigating the
region.

.. _`failure:mitigation:continuation`:

Continuation
~~~~~~~~~~~~

If the :dakkw:`interface-failure_capture-continuation` option is specified, then Dakota will attempt to
step towards the failing “target” simulation from a nearby “source”
simulation through the use of a continuation algorithm. This option is
appropriate for those cases in which a failed simulation may be caused
by an inadequate initial guess. If the “distance” between the source and
target can be divided into smaller steps in which information from one
step provides an adequate initial guess for the next step, then the
continuation method can step towards the target in increments
sufficiently small to allow for convergence of the simulations.

When the failure occurs, the interval between the last successful
evaluation (the source point) and the current target point is halved and
the evaluation is retried. This halving is repeated until a successful
evaluation occurs. The algorithm then marches towards the target point
using the last interval as a step size. If a failure occurs while
marching forward, the interval will be halved again. Each invocation of
the continuation algorithm is allowed a total of ten failures (ten
halvings result in up to 1024 evaluations from source to target) prior
to aborting the Dakota process.

While Dakota manages the interval halving and function evaluation
invocations, the user is responsible for managing the initial guess for
the simulation program. For example, in a GOMA input
file :cite:p:`Sch95`, the user specifies the files to be used
for reading initial guess data and writing solution data. When using the
last successful evaluation in the continuation algorithm, the
translation of initial guess data can be accomplished by simply copying
the solution data file leftover from the last evaluation to the initial
guess file for the current evaluation (and in fact this is useful for
all evaluations, not just continuation). However, a more general
approach would use the *closest* successful evaluation (rather than the
*last* successful evaluation) as the source point in the continuation
algorithm. This will be especially important for nonlocal methods (e.g.,
genetic algorithms) in which the last successful evaluation may not
necessarily be in the vicinity of the current evaluation. This approach
will require the user to save and manipulate previous solutions (likely
tagged with evaluation number) so that the results from a particular
simulation (specified by Dakota after internal identification of the
closest point) can be used as the current simulation’s initial guess.
This more general approach is not yet supported in Dakota.

.. _`failure:special`:

Special values
--------------

In IEEE arithmetic, “NaN” indicates “not a number” and
:math:`\pm`\ “Inf” or :math:`\pm`\ “Infinity" indicates positive or
negative infinity. These special values may be returned directly in
function evaluation results from a simulation interface or they may be
specified in a user’s input file within the :dakkw:`interface-failure_capture-recover` specification
described in :ref:`Recover<failure:mitigation:recover>`. There is a
key difference between these two cases. In the former case of direct
simulation return, failure mitigation can be managed on a per response
function basis. When using :dakkw:`interface-failure_capture-recover`, however, the failure applies to
the complete set of simulation results.

In both of these cases, the handling of NaN or Inf is managed using
iterator-specific approaches. Currently, nondeterministic sampling
methods (see :ref:`Sampling Methods<uq:sampling>`), polynomial chaos
expansions using either regression approaches or spectral projection
with random sampling (see :ref:`Stochastic Expansion Methods<uq:expansion>`),
and the NL2SOL method for nonlinear least squares (see
:ref:`NL2SOL<nls:solution:nl2sol>`) are the only
methods with special numerical exception handling: the sampling methods
simply omit any samples that are not finite from the statistics
generation, the polynomial chaos methods omit any samples that are not
finite from the coefficient estimation, and NL2SOL treats NaN or
Infinity in a residual vector (i.e., values in a results file for a
function evaluation) computed for a trial step as an indication that the
trial step was too long and violates an unstated constraint; NL2SOL
responds by trying a shorter step.

