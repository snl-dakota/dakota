.. _`sec:output:pce`:

============================
Stochastic expansion exports
============================

Polynomial chaos expansion (PCE) methods compute coefficients for
response expansions which employ a basis of multivariate orthogonal
polynomials. The ``polynomial_chaos`` method calculates these
coefficients based on a number of approaches described in
Section `[uq:expansion] <#uq:expansion>`__). One may output the PCE
coefficients to a file using the keyword
``export_expansion_file = STRING``. Each row of the exported file will
contain a coefficient, followed by the multi-index indicating which
basis terms correspond to it. Only free-form format
(Section `[input:tabularformat] <#input:tabularformat>`__) is supported
for this file.

When using numerical integration schemes with structured rules, Dakota
can also output the integration points and corresponding weights to a
tabular file. This output is generated when ``method output`` is
``verbose`` or higher. Weights and points are printed to a file
``dakota_quadrature_tabular.dat`` (tensor product quadrature),
``dakota_sparse_tabular.dat`` (sparse grids), or
``dakota_cubature_tabular.dat`` (cubature methods), with one line per
integration point.