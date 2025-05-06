.. _aboutdakota-main:

""""""""""""
About Dakota
""""""""""""

=======
Summary
=======

The Dakota project delivers both state-of-the-art research and robust, usable software for optimization and UQ. Broadly,
the Dakota software's advanced parametric analyses enable design exploration, model calibration, risk analysis, and quantification
of margins and uncertainty with computational models. The Dakota toolkit provides a flexible, extensible interface between such
simulation codes and its iterative systems analysis methods, which include:

- optimization with gradient and nongradient-based methods;
- uncertainty quantification with sampling, reliability, stochastic expansion, and epistemic methods;
- parameter estimation using nonlinear least squares (deterministic) or Bayesian inference (stochastic); and
- sensitivity/variance analysis with design of experiments and parameter study methods.

These capabilities may be used on their own or as components within advanced strategies such as hybrid optimization,
surrogate-based optimization, mixed integer nonlinear programming, or optimization under uncertainty.

.. _aboutdakota-use:

=======================
Intended Use and Impact
=======================

Computational methods developed in structural mechanics, heat transfer, fluid mechanics, shock physics, and many other fields
of engineering can be an enormous aid to understanding the complex physical systems they simulate. Often, it is desired to use
these simulations as virtual prototypes to obtain an optimized design for a particular system, or to develop confidence in performing
predictions for systems that cannot be observed or tested directly. This effort seeks to enhance the utility of these computational
simulations by enabling their use for a variety of iterative analyses, so that simulations may be used not just for single-point
solutions, but also achieve broader impact in the areas of credible prediction and optimal design.

This allows analysts to address the fundamental engineering questions of foremost importance to our programs, such as

- "What is the best design?"
- "How safe is it?"
- "How much confidence do I have in my answer?"

with respect to system performance objectives such as:

- minimizing weight, cost, or defects;
- limiting a critical temperature, stress, or vibration response;
- maximizing performance, reliability, throughput, reconfigurability, agility, or design robustness

A systematic approach for addressing these questions will lead to higher confidence in our computational simulations, in turn
leading to better designs and improved system performance. Moreover, a reduced dependence on physical testing can shorten
the design cycle and reduce development costs.

.. _aboutdakota-motivation:

=================================
Motivation for Dakota Development
=================================

Computational models are commonly used in engineering design and scientific discovery activities for simulating complex
physical systems in disciplines such as fluid mechanics, structural dynamics, heat transfer, nonlinear structural mechanics,
shock physics, and many others. These simulators can be an enormous aid to engineers who want to develop an understanding
and/or predictive capability for complex behaviors typically observed in the corresponding physical systems. Simulators often
serve as virtual prototypes, where a set of predefined system parameters, such as size or location dimensions and material
properties, are adjusted to improve the performance of a system, as defined by one or more system performance objectives.
Such optimization or tuning of the virtual prototype requires executing the simulator, evaluating performance objective(s),
and adjusting the system parameters in an iterative, automated, and directed way. System performance objectives can be formulated,
for example, to minimize weight, cost, or defects; to limit a critical temperature, stress, or vibration response; or to
maximize performance, reliability, throughput, agility, or design robustness. In addition, one would often like to design
computer experiments, run parameter studies, or perform uncertainty quantification (UQ). These approaches reveal how system
performance changes as a design or uncertain input variable changes. Sampling methods are often used in uncertainty quantification
to calculate a distribution on system performance measures, and to understand which uncertain inputs contribute most to the
variance of the outputs.

A primary goal for Dakota development is to provide engineers and other disciplinary scientists with a systematic and rapid
means to obtain improved or optimal designs or understand sensitivity or uncertainty using simulation-based models. These
capabilities generally lead to improved designs and system performance in earlier design stages, alleviating dependence on
physical prototypes and testing, shortening design cycles, and reducing product development costs. In addition to providing
this practical environment for answering system performance questions, the Dakota toolkit provides an extensible platform for
the research and rapid prototyping of customized methods and meta-algorithms.

=======
History
=======

The Dakota project started in 1994 as an internal research and development activity at Sandia National Laboratories in Albuquerque,
New Mexico. The original goal was to provide a common set of optimization tools for a group of engineers solving structural analysis
and design problems. Prior to the Dakota project, there was no focused effort to archive optimization methods for reuse on other
projects. Thus, engineers found themselves repeatedly building new custom interfaces between the engineering analysis software
and optimization software. This was especially burdensome when using parallel computing, as each project developed a unique
program to coordinate concurrent simulations on a network of workstations or a parallel computer. The initial Dakota toolkit
provided the engineering and analysis community at Sandia access to a variety of optimization algorithms, hiding the complexity 
of the optimization software interfaces from the users. Engineers could readily switch between optimization software packages by
simply changing a few lines in a Dakota input file. In addition to structural analysis, Dakota has been applied to computational
fluid dynamics, nonlinear dynamics, shock physics, heat transfer, electrical circuits, and many other science and engineering models.

Dakota has grown significantly beyond an optimization toolkit. In addition to its state-of-the-art optimization methods, Dakota
includes methods for global sensitivity and variance analysis, parameter estimation, uncertainty quantification, and verification,
as well as meta-level strategies for surrogate-based optimization, hybrid optimization, and optimization under uncertainty.
Available to all these algorithms is parallel computation support; ranging from desktop multiprocessor computers to massively
parallel computers typically found at national laboratories and supercomputer centers.

As of Version 5.0, Dakota is publicly released as open source under a GNU Lesser General Public License and is available for free
download world-wide under the `LGPL software use agreement <http://www.gnu.org/licenses/lgpl.html>`__. Dakota
Versions 3.0 through 4.2+ were licensed under the GNU General Public License. Dakota public release facilitates research and software
collaborations among Dakota developers at Sandia National Laboratories and other institutions, including academic, government, and
corporate entities. See the :ref:`Dakota FAQ <faq-main>` for more information on the public release rationale and ways to contribute.
You may also peruse the :ref:`listing of current and former contributors and third-party library developers <contributors>`.

====================
C++ Software Toolkit
====================

Written in C++, the Dakota toolkit provides a flexible, extensible interface between simulation codes and a variety of iterative
systems analysis methods, including optimization, uncertainty quantification, deterministic/stochastic calibration, and
parametric/sensitivity/variance analysis. These capabilities may be used on their own or as foundational components within advanced
solution strategies that coordinate multiple computational models and iteration methods.  Initiated in 1994, it strives to span the
research to production spectrum, providing both a mature tool for production use as well as a foundation for new algorithm research.

Dakota is open source under GNU LGPL, with applications spanning defense programs for DOE and DOD, climate modeling,
computational materials, nuclear power, renewable energy, and many others.
