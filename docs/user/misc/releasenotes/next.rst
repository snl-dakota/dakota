:orphan:

.. _releasenotes-next:

""""""""""""""""""""""""""""""""""""""
Version 6.19 (2023/11/15; pre-release)
""""""""""""""""""""""""""""""""""""""

**Highlight: New sampling-based method for main effects**

Based on :cite:p:`Li16`, Dakota can now obtain estimates
of first order Sobol indices (main effects) from `sampling`
studies. Previous versions of Dakota could obtain esimates of
main and total effects using a "pick and freeze" sampling strategy,
which typically required a very large number of samples (hundreds or
thousands per variable) and for those samples to be structured in a
particular way. While the new method produces only main
effects, the requirement on sample design has been lifted, and
typically far fewer samples are needed for convergence.

*Enabling / Accessing:* 

The :dakkw:`method-sampling-variance_based_decomp` keyword now has 
suboptions. The :dakkw:`vbd_sampling_method pick_and_freeze <method-sampling-variance_based_decomp-vbd_sampling_method-pick_and_freeze>`
option is the default, and causes Dakota to use the method that has long been available
to compute main and total effects. The 
:dakkw:`vbd_sampling_method binned <method-sampling-variance_based_decomp-vbd_sampling_method-binned>`
option causes the new method to be used.

*Documentation:* 

Keyword reference for the :dakkw:`method-sampling-variance_based_decomp-vbd_sampling_method-binned`
VBD method.


**Highlight: Low-discrepancy (quasi-Monte Carlo) sampling**

Two new strategies for choosing low-discrepancy points in sampling
studies are available in this release. These include lattice rules
and digital nets. The well-known Sobol sequence is an example of a
digital net. Just as in Latin hypercube sampling, these strategies
choose points that cover the parameter space more uniformly than ordinary
Monte Carlo, leading to faster convergence of UQ results.

*Enabling / Accessing:* 

In a ``sampling`` study, choose 
:dakkw:`sample_type low_discrepancy <method-sampling-sample_type-low_discrepancy>`.

*Documentation:* 

* :dakkw:`method-sampling-sample_type-low_discrepancy` keyword.
* Discussion of :ref:`low-disrepancy methods <uq:sampling>`.

**Improvements by Category**

*Interfaces, Input/Output*

* Copying of ``dakota.interfacing`` objects was improved by adding ``__deepcopy__`` implementations
* New examples:

  * Demonstrating use of Dakota's direct :dakkw:`interface-analysis_drivers-python` interface with a 
    `pre-built tensorflow model <https://github.com/snl-dakota/dakota-examples/tree/master/official/drivers/Python/linked_di/tensorflow>`_.
  * Demonstrating use of ``dakota.interfacing.dprepro`` in a black-box interface.
    (For `Windows <https://github.com/snl-dakota/dakota-examples/tree/master/official/drivers/black-box_simulation_windows>`_
    and `Linux/macOS <https://github.com/snl-dakota/dakota-examples/tree/master/official/drivers/black-box_simulation>`_)
   
*Models*

*Optimization Methods*

*UQ Methods*

* Improved support for the MIT Uncertainty Quantification Library (MUQ); although MUQ is not enabled
  in our pre-built downloads, it is now buildable within Dakota by a wider variety of toolchains.
* Low discrepancy sampling strategies (see highlight)

*MLMF Sampling*

*Sensitivity Analysis*

* Binned method for sampling-based variance-based decomposition (see highlight)
* New examples explaining use of
  `correlation coefficients <https://github.com/snl-dakota/dakota-examples/tree/master/official/global_sensitivity/correlations>`_
  for global sensitivity analysis.
 
**Miscellaneous Enhancements and Bugfixes**

- Enh: Documentation of Dakota's regresion test system expanded.

- Bug fix: The ``@python_interface`` decorator in the ``dakota.interfacing`` module now propertly
  interprets the dvv list provided by Dakota's direct :dakkw:`interface-analysis_drivers-python` interface.

- Bug fix: RPATH handling on Linux-based platforms improved.


**Deprecated and Changed**

**Compatibility**

- Support for building Dakota with C++17 has been greatly expanded and is expected to work for
  GCC, Intel, and Clang compilers. Support for Microsoft Visual Studio in progress.

**Other Notes and Known Issues**
