.. dakota-docs documentation main file, created by
   sphinx-quickstart on Wed Nov 10 08:43:34 2021.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Dakota |version|
================

The Dakota project delivers both state-of-the-art research and robust, usable
software for optimization and uncertainty quantification (UQ). Dakota enables
:term:`outer-loop analyses <outer-loop analysis>`, such as optimization, UQ,
sensitivity analysis and calibration. Its flexible interfacing structure lets
you connect a wide range of simulation codes to Dakota, from black-box
executables to Python functions. Build one interface to your code, called an
:term:`analysis driver`, and every Dakota method can use it.

.. figure:: /usingdakota/img/intro/one_interface_many_methods.*
   :alt: Five Dakota method families (parameter studies, sensitivity analysis,
         uncertainty quantification, optimization and calibration) all connect
         through one analysis driver to your simulation.
   :align: center

   One analysis driver connects your simulation to every Dakota method.

==========
Start here
==========

- **New to Dakota?** Read the Getting Started pages in order:
  :ref:`aboutdakota-main`, :ref:`studyanatomy-main`,
  :ref:`helloworld-main`, and then
  :ref:`couplingtosimulations-main`.
- **Installing Dakota?** See :ref:`setupdakota-main`.
- **Already using Dakota?** Go to the
  :ref:`Keyword Reference <keyword-reference-area>`,
  :ref:`Study Types <studytypes-main>`, :doc:`Examples <usingdakota/examples>`
  or :ref:`Release Notes <releasenotes-main>`.

========
Get help
========

Ask questions, report bugs and request features on Dakota's
:ref:`GitHub Discussions <help-discussions>` page. :ref:`help-main` also lists
video screencasts and training materials.

===========
Cite Dakota
===========

If you use Dakota in your work, please cite it as described in
:ref:`citing-dakota`.

.. toctree::
   :caption: Install
   :maxdepth: 2

   setupdakota

.. toctree::
   :caption: Getting Started with Dakota
   :maxdepth: 1

   usingdakota/introduction/aboutdakota
   usingdakota/introduction/studyanatomy
   usingdakota/introduction/helloworld
   usingdakota/introduction/couplingtosimulations

.. toctree::
   :caption: Using Dakota
   :maxdepth: 2

   usingdakota/usingdakota
   usingdakotagui/usingdakotagui
   externaltools/externaltools

.. toctree::
   :caption: Building and Developing Dakota
   :maxdepth: 2

   building/building
   developingdakota/developingdakota

.. toctree::
   :caption: More
   :maxdepth: 2

   misc/misc