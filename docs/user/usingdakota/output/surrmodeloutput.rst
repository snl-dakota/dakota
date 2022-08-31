=======================
Surrogate Model Exports
=======================

Most Dakota surrogate models, including all those implemented in
Surfpack, all those in the experimental surrogate module, and some
stochastic expansion approaches support the keyword . When specified,
any approximate evaluations performed on the surrogate model will be
output to the specified data file. The data file can be exported in any
of the tabular formats described in
Section `[input:tabularformat] <#input:tabularformat>`__ (default
annotated). This facilitates plotting or external diagnostics of the
surrogate model. Additionally, the Gaussian Process surrogate models can
export variance predictions through .

In addition, the Surfpack and experimental families of global surrogate
models can be exported to text and binary archives for later reuse in
Dakota or other contexts, such as evaluation from C or Python, or with
the ``surfpack`` executable or library API. Select Surfpack models can
export to a human-readable and self-documenting algebraic form, suitable
for reuse in user-developed tools. The keyword group is used to specify
model export filenames and formats. It is described in the Dakota
Reference Manual :cite:p:`RefMan`.