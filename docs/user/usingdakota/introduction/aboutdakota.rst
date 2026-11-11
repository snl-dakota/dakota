.. _aboutdakota-main:

"""""""""""""""
What is Dakota?
"""""""""""""""

Dakota is open-source software from Sandia National Laboratories, released
under the GNU Lesser General Public License (LGPL). It enables outer-loop
analyses (e.g., optimization, uncertainty quantification (UQ), sensitivity
analysis, and calibration) for simulations. Dakota treats your simulation as a
:term:`black box`: it supplies parameter values and gets back results, and it
needs no further information about how your code works.

====================================
Dakota enables outer-loop analyses
====================================

Most questions about a simulation can't be answered by a single run. Dakota
runs :term:`outer-loop analyses <outer-loop analysis>`, generating many runs
of a simulation code with different input values. For each run, Dakota chooses
the parameter values, manages the run, and collects the simulation outputs, or
:term:`responses`. It then analyzes this :term:`ensemble` of input--output
pairs to extract useful information, such as optimal values or response
statistics.

.. figure:: /usingdakota/img/intro/ensemble_loop.*
   :alt: Dakota sends variable values x1 and x2 to your simulation, which is
         treated as a black box, and receives the response f; repeating this
         builds a table of input--output pairs with one row per run.
   :align: center

   Dakota sends variable values to your simulation and receives responses.
   Each run adds a row to the ensemble containing a given input--output pair.

Any code that takes inputs and produces outputs can be interfaced with Dakota:
black-box executables with input files, Python scripts, Matlab functions, etc.

=====================================
Build one interface, use many methods
=====================================

You connect your simulation to Dakota once through an :term:`analysis driver`,
the program Dakota runs for each evaluation to map from parameter values to
responses. Once you have a functioning analysis driver, every Dakota method can
use it. Switching from a sensitivity study to an optimization requires only a
change to the Dakota study, not to your code.

.. figure:: /usingdakota/img/intro/one_interface_many_methods.*
   :alt: Five Dakota method families (parameter studies, sensitivity analysis,
         uncertainty quantification, optimization and calibration) all connect
         through one analysis driver to your simulation.
   :align: center

   One analysis driver connects your simulation to every Dakota method.

.. _aboutdakota-use:

=============================
Questions Dakota helps answer
=============================

Dakota supports four main kinds of studies, which answer different questions:

.. list-table::
   :widths: 50 50

   * - **Sensitivity analysis**

       Which inputs matter most?

       :ref:`Learn more <sa>`
     - **Uncertainty quantification**

       How do uncertain inputs affect my results, and how confident can I be
       in them?

       :ref:`Learn more <uq>`
   * - **Optimization**

       What is the best design?

       :ref:`Learn more <opt>`
     - **Calibration**

       Which parameter values best match my data?

       :ref:`Learn more <nls>`

In addition to these four main kinds of studies, :ref:`parameter studies <ps>`
and :ref:`design of experiments <dace>` explore how your responses change over
a range or grid of inputs.

==============
Why use Dakota
==============

- **Practical for costly simulations.** Many methods are designed to reach an
  answer in as few simulation runs as possible.
- **Deterministic and probabilistic analysis in one tool.** For example, you
  can optimize a design while accounting for uncertainty in its inputs.
- **Robust to difficult responses.** Derivative-free and global methods handle
  responses that are noisy, non-smooth or have several local optima.
- **Surrogate and multifidelity methods.** Dakota can replace an expensive
  simulation with a cheap approximation, or combine models of different cost
  and accuracy.
- **Parallel from desktop to supercomputer.** Dakota can run many simulations
  at once, on a laptop or on a large distributed cluster.
- **Production software.** Dakota is tested and maintained, with a new release
  every May and November.

======================
Beyond the black box
======================

Most users start with an analysis driver that exchanges files with their code,
because this form of interfacing works with nearly any code. If your model is
written in Python, Dakota can also call it directly and exchange values in
memory. This approach avoids file input and output for more rapid model
evaluations. This :ref:`Python direct interface <advint:existingdirect:python>`
may not be included in pre-built downloads and must be enabled when compiling
Dakota from source. You can also embed Dakota in your own C++ or Python
application through its :ref:`library API <library-api>`.

========
Pedigree
========

Dakota was created at Sandia National Laboratories in 1994 to give engineers a
common set of optimization tools, instead of a new custom interface for every
project. It has since grown to include sensitivity analysis, uncertainty
quantification, calibration and advanced combined strategies. Dakota is used
in fields including defense, climate modeling, materials, nuclear power and
renewable energy. Read more in :ref:`Dakota History and Motivation
<history-main>`.

**Next:** :ref:`studyanatomy-main`