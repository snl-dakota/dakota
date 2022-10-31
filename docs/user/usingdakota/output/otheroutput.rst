.. _`other:output`:

====================
Other Dakota Outputs
====================

.. _`other:output:pce`:

----------------------------
Stochastic expansion exports
----------------------------

Polynomial chaos expansion (PCE) methods compute coefficients for
response expansions which employ a basis of multivariate orthogonal
polynomials. The :dakkw:`polynomial_chaos` method calculates these
coefficients based on :ref:`a number of stochastic expansion approaches <uq:expansion>`.
One may output the PCE coefficients to a file using the keyword
``export_expansion_file = STRING``. Each row of the exported file will
contain a coefficient, followed by the multi-index indicating which
basis terms correspond to it. Only :ref:`free-form format <input:tabularformat>` is supported
for this file.

When using numerical integration schemes with structured rules, Dakota
can also output the integration points and corresponding weights to a
tabular file. This output is generated when :dakkw:`method` :dakkw:`method-output` is
:dakkw:`method-output-verbose` or higher. Weights and points are printed to a file
``dakota_quadrature_tabular.dat`` (tensor product quadrature),
``dakota_sparse_tabular.dat`` (sparse grids), or
``dakota_cubature_tabular.dat`` (cubature methods), with one line per
integration point.

.. _`other:output:surr`:

-----------------------
Surrogate Model Exports
-----------------------

Most Dakota surrogate models, including all those implemented in
Surfpack, all those in the experimental surrogate module, and some
stochastic expansion approaches support the keyword . When specified,
any approximate evaluations performed on the surrogate model will be
output to the specified data file. The data file can be exported in any
of the :ref:`tabular formats <input:tabularformat>` (default
annotated). This facilitates plotting or external diagnostics of the
surrogate model. Additionally, the Gaussian Process surrogate models can
export variance predictions through .

In addition, the Surfpack and experimental families of global surrogate
models can be exported to text and binary archives for later reuse in
Dakota or other contexts, such as evaluation from C or Python, or with
the ``surfpack`` executable or library API. Select Surfpack models can
export to a human-readable and self-documenting algebraic form, suitable
for reuse in user-developed tools. The
:ref:`keyword group for surfpack <model-surrogate-global-gaussian_process-surfpack>`
is used to specify model export filenames and formats.

.. _`other:output:prerun`:

-----------------------------
Variables Output from Pre-run
-----------------------------

The pre-run mode (supported only for select methods) permits
specification of an output file to which Dakota will write parameter
(variables) data in :ref:`any supported tabular data format <input:tabularformat>`
(default annotated) with data columns corresponding to each variable.
This file can be generated with sampling, parameter study, and DACE
methods by invoking

.. code-block::

   dakota -i dakota.in -pre_run ::variables.dat

for example, to output the variables (samples) in an LHS study. If a
user adds the corresponding response values to this file, it may then be
imported using Dakota’s post-run mode. Command line pre-run will always
export in annotated format. To export pre-run data in other formats,
specify :dakkw:`environment-pre_run` in the input file instead of at the command-line,
and provide a format option.