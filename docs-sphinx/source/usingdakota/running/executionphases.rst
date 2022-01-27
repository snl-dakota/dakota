""""""""""""""""
Execution Phases
""""""""""""""""

Dakota has three execution phases: pre-run, run, and post-run.

- pre-run can be used to generate variable sets
- run (core run) invokes the simulation to evaluate variables, producing responses
- post-run accepts variable/response sets and analyzes the results (for example, calculate correlations from a set of samples). Currently only two modes are supported and only for sampling, parameter study, and DACE methods:

(1) pre-run only with optional tabular output of variables:

..code-block::

    dakota -i dakota.in -pre_run [::myvariables.dat]

(2) post-run only with required tabular input of variables/responses:

..code-block::

    dakota -i dakota.in -post_run myvarsresponses.dat::
