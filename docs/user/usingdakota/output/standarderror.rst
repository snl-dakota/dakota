.. _`output:error`:

=====================
Error Messages Output
=====================

A variety of error messages are printed by Dakota in the event that an
error is detected in the input specification. Some of the more common
input errors, and the associated error messages, are described below.
See also the Common Specification Mistakes section in the Dakota
Reference Manual :cite:p:`RefMan`.

Incorrectly spelled specifications, such as ``“numericl_gradients”``,
will result in error messages of the form:

::

   Input line 31: unrecognized identifier 'numericl_gradients'.
   Input line 31: unrecognized identifier 'method_source'.
   Input line 31: unrecognized identifier 'dakota'.
   Input line 31: unrecognized identifier 'interval_type'.
   Input line 31: unrecognized identifier 'central'.
   Input line 31: unrecognized identifier 'fd_gradient_step_size'.
   Input line 31: One of the following 4 entities
   must be specified for responses...
       analytic_gradients
       mixed_gradients
       no_gradients
       numerical_gradients

In this example the line numbers given are approximate, as all input
following an errant keywords is considered a single line through the end
of the block.

The input parser catches syntax errors, but not logic errors. The fact
that certain input combinations are erroneous must be detected after
parsing, at object construction time. For example, if a ``no_gradients``
specification for a response data set is combined with selection of a
gradient-based optimization method, then this error must be detected
during set-up of the optimizer (see last line of listing):

::

   Error: gradient-based minimizers require a gradient specification.

Many such errors can be detected earlier by running ``dakota -check``.

Another common mistake involves a mismatch between the amount of data
expected on a function evaluation and the data returned by the user’s
simulation code or driver. The available response data is specified in
the responses keyword block, and the subset of this data needed for a
particular evaluation is managed by the active set vector. For example,
if Dakota expects function values and gradients to be returned (as
indicated by an active set vector containing 3’s), but the user’s
simulation code only returns function values, then the following error
message is generated:

::

       At EOF: insufficient data for functionGradient 1

Unfortunately, descriptive error messages are not available for all
possible failure modes of Dakota. If you encounter core dumps,
segmentation faults, or other failures, please request help using the
support mechanisms described on the `Dakota
website <http://dakota.sandia.gov/>`__.