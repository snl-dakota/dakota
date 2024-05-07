:orphan:

.. _releasenotes-next:

""""""""""""""""""""""""""""""""""""""
Version 6.20 (2024/05/15; pre-release)
""""""""""""""""""""""""""""""""""""""

**Highlight: New options for using MCMC algorithms from the MUQ library**

When selecting `bayes_calibration muq' under the `method' section of a Dakota input file,
the user can now select the 'mala' MCMC algorithm (in addition to the already supported four
algorithms). The user can now also select values for parameters in each of these algorithms.

*Enabling / Accessing:* 

Dakota currently interfaces with five MCMC algorithms from MUQ:
metropolis_hastings, adaptive_metropolis, delayed_rejection, dram, and mala.
Dakota also allows the user to select values for seven parameters related
to these five methods (the prefix in each parameter indicates which MCMC
algorithm the parameter relates itself to): dr_num_stages, dr_scale_type,
dr_scale, am_period_num_steps, am_starting_step, am_scale, and mala_step_size.

*Documentation:* 

See the eight keywords :dakkw:`method-bayes_calibration-muq-mala`,
:dakkw:`method-bayes_calibration-muq-am_period_num_steps`,
:dakkw:`method-bayes_calibration-muq-am_scale`,
:dakkw:`method-bayes_calibration-muq-am_starting_step`,
:dakkw:`method-bayes_calibration-muq-dr_num_stages`,
:dakkw:`method-bayes_calibration-muq-dr_scale`,
:dakkw:`method-bayes_calibration-muq-dr_scale_type`, and
:dakkw:`method-bayes_calibration-muq-mala_step_size`.

**Improvements by Category**

*Interfaces, Input/Output*

*Models*

*Optimization Methods*

*UQ Methods*

*MLMF Sampling*


 
**Miscellaneous Enhancements and Bugfixes**

- Enh:
- Bug fix:

**Deprecated and Changed**

**Compatibility**

**Other Notes and Known Issues**
