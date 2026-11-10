.. _history-main:

""""""""""""""""""""""""""""""
Dakota History and Motivation
""""""""""""""""""""""""""""""

This page provides background on why Dakota exists and how it developed. For a
short introduction, see :ref:`aboutdakota-main`.

.. _aboutdakota-motivation:

=================================
Motivation for Dakota Development
=================================

Computational models simulate complex physical systems in fields such as fluid
mechanics, structural dynamics, heat transfer and shock physics. Engineers
often use them as virtual prototypes: they adjust parameters such as
dimensions or material properties to improve a system's performance, or they
make predictions for systems that can't be observed or tested directly.

Doing this well means running the simulation many times in an iterative,
automated and directed way, whether to optimize a design, run parameter
studies, design computer experiments or quantify uncertainty. These analyses
help answer fundamental engineering questions such as:

- "What is the best design?"
- "How safe is it?"
- "How much confidence do I have in my answer?"

These questions are asked about performance objectives such as minimizing
weight, cost or defects; limiting a critical temperature, stress or vibration
response; or maximizing performance, reliability, throughput or design
robustness.

A primary goal of Dakota is to give engineers and scientists a systematic and
rapid way to answer these questions with simulation-based models. Answering
them earlier in the design process leads to better designs, less dependence on
physical prototypes and testing, shorter design cycles and lower development
costs. Dakota also provides an extensible platform for researching and
prototyping new methods and meta-algorithms.

=======
History
=======

The Dakota project started in 1994 as an internal research and development
activity at Sandia National Laboratories in Albuquerque, New Mexico. Its
original goal was to give a group of engineers solving structural analysis and
design problems a common set of optimization tools. Before Dakota, there was
no focused effort to reuse optimization methods across projects, so engineers
repeatedly built custom interfaces between their analysis software and
optimization software. This was especially burdensome for parallel computing,
where each project wrote its own program to coordinate concurrent simulations.
With Dakota, engineers could switch between optimization packages by changing
a few lines in an input file.

Dakota has since grown well beyond an optimization toolkit. It now includes
methods for global sensitivity analysis, parameter estimation, uncertainty
quantification and verification, as well as meta-level strategies such as
surrogate-based optimization, hybrid optimization and optimization under
uncertainty, all with parallel computing support from desktop multiprocessor
computers to massively parallel computers. Beyond structural analysis, Dakota
has been applied to computational fluid dynamics, nonlinear dynamics, shock
physics, heat transfer, electrical circuits and many other science and
engineering models.

Since Version 5.0, Dakota has been released as open source under the
`GNU Lesser General Public License <http://www.gnu.org/licenses/lgpl.html>`__
(LGPL). Versions 3.0 through 4.2+ used the GNU General Public License. Public
release supports research and software collaborations between Sandia and
academic, government and industry partners. See the :ref:`Dakota FAQ
<faq-main>` for the reasons behind the public release and ways to contribute,
and the :ref:`list of current and former contributors and third-party library
developers <contributors>`.

====================
C++ Software Toolkit
====================

Dakota is written in C++. Its methods can be used on their own or as building
blocks in advanced strategies that coordinate multiple computational models
and methods. Dakota aims to span the research-to-production spectrum,
providing both a mature tool for production use and a foundation for new
algorithm research. Its applications include defense programs for DOE and DOD,
climate modeling, computational materials, nuclear power and renewable energy.