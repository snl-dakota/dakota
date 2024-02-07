.. _`topic-interval_estimation`:

Interval Estimation
===================

In interval analysis, one assumes that nothing is known about
an epistemic uncertain variable except that its value lies
somewhere within an interval. In this situation, it is NOT
assumed that the value has a uniform probability of occuring
within the interval. Instead, the interpretation is that
any value within the interval is a possible value or a potential
realization of that variable. In interval analysis, the
uncertainty quantification problem is one of determining the
resulting bounds on the output (defining the output interval)
given interval bounds on the inputs. Again, any output response
that falls within the output interval is a possible output
with no frequency information assigned to it.

We have the capability to perform interval analysis using either
``global_interval_est`` or ``local_interval_est``.  In
the global approach, one uses either a global optimization method or a
sampling method to assess the bounds.  ``global_interval_est``
allows the user to specify either ``lhs``, which performs Latin
Hypercube Sampling and takes the minimum and maximum of the samples as
the bounds (no optimization is performed) or ``ego``. In the case
of ``ego``, the efficient global optimization method is used to
calculate bounds. The ego method is described in
Section for additional
details on these nested approaches.
These interval methods can also be used as the outer loop within an
interval-valued probability analysis for propagating mixed aleatory
and epistemic uncertainty -- refer to
Section for additional details.



Interval analysis is often used to model epistemic uncertainty.
In interval analysis, the
uncertainty quantification problem is one of determining the
resulting bounds on the output (defining the output interval)
given interval bounds on the inputs.

We can do interval analysis using either
% ``global_interval_est`` or ``local_interval_est``.
In the global approach, one uses either a global optimization
method or a sampling method to assess the bounds, whereas the
local method uses gradient information in a derivative-based
optimization approach.

An example of interval estimation
is shown in :numref:`uq:examples:interval_out`

.. Code block below seems to be duplicate of `uq:examples:interval_out`, consider removing it or using just a reference
.. code-block::

    
    ------------------------------------------------------------------
    Min and Max estimated values for each response function:
    weight:  Min = 1.0000169352e+00  Max = 9.9999491948e+01
    stress:  Min = -9.7749994284e-01  Max = 2.1499428450e+01
    displ:  Min = -9.9315672724e-01  Max = 6.7429714485e+01
    -----------------------------------------------------------------
