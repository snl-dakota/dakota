=============================
Variables Output from Pre-run
=============================

The pre-run mode (supported only for select methods) permits
specification of an output file to which Dakota will write parameter
(variables) data in any supported tabular format (default annotated; see
Section `[input:tabularformat] <#input:tabularformat>`__) with data
columns corresponding to each variable. This file can be generated with
sampling, parameter study, and DACE methods by invoking

::

       dakota -i dakota.in -pre_run ::variables.dat

for example, to output the variables (samples) in an LHS study. If a
user adds the corresponding response values to this file, it may then be
imported using Dakota’s post-run mode. Command line pre-run will always
export in annotated format. To export pre-run data in other formats,
specify ``pre_run`` in the input file instead of at the command-line,
and provide a format option.