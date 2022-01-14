About Dakota
============

```{eval-rst}
.. _readmoreaboutdakota:
```

## Summary

The Dakota project delivers both state-of-the-art research and robust, usable software for optimization and UQ. Broadly, the Dakota software's advanced parametric analyses enable design exploration, model calibration, risk analysis, and quantification of margins and uncertainty with computational models. The Dakota toolkit provides a flexible, extensible interface between such simulation codes and its iterative systems analysis methods, which include:

* optimization with gradient and nongradient-based methods;
* uncertainty quantification with sampling, reliability, stochastic expansion, and epistemic methods;
* parameter estimation using nonlinear least squares (deterministic) or Bayesian inference (stochastic); and
* sensitivity/variance analysis with design of experiments and parameter study methods.

These capabilities may be used on their own or as components within advanced strategies such as hybrid optimization, surrogate-based optimization, mixed integer nonlinear programming, or optimization under uncertainty.

## Intended Use and Impact

Computational methods developed in structural mechanics, heat transfer, fluid mechanics, shock physics, and many other fields of engineering can be an enormous aid to understanding the complex physical systems they simulate. Often, it is desired to use these simulations as virtual prototypes to obtain an optimized design for a particular system, or to develop confidence in performing predictions for systems that cannot be observed or tested directly. This effort seeks to enhance the utility of these computational simulations by enabling their use for a variety of iterative analyses, so that simulations may be used not just for single-point solutions, but also achieve broader impact in the areas of credible prediction and optimal design.

This allows analysts to address the fundamental engineering questions of foremost importance to our programs, such as

* "What is the best design?"
* "How safe is it?"
* "How much confidence do I have in my answer?"

with respect to system performance objectives such as:

* minimizing weight, cost, or defects;
* limiting a critical temperature, stress, or vibration response;
* maximizing performance, reliability, throughput, reconfigurability, agility, or design robustness

A systematic approach for addressing these questions will lead to higher confidence in our computational simulations, in turn leading to better designs and improved system performance. Moreover, a reduced dependence on physical testing can shorten the design cycle and reduce development costs.

## C++ Software Toolkit

Written in C++, the Dakota toolkit provides a flexible, extensible interface between simulation codes and a variety of iterative systems analysis methods, including optimization, uncertainty quantification, deterministic/stochastic calibration, and parametric/sensitivity/variance analysis. These capabilities may be used on their own or as foundational components within advanced solution strategies that coordinate multiple computational models and iteration methods.  Initiated in 1994, it strives to span the research to production spectrum, providing both a mature tool for production use as well as a foundation for new algorithm research.

Dakota is open source under GNU LGPL, with applications spanning defense programs for DOE and DOD, climate modeling, computational materials, nuclear power, renewable energy, and many others.
