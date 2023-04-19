.. _help-main:

""""""""""""""
Help Resources
""""""""""""""

=====================
Public User Community
=====================

Dakota is an open source and publicly available project with a
significant worldwide user base. Our vision is that Dakota users will
increasingly help each other with questions and issues. We especially
encourage general usage questions about how to use Dakota, including
how to install, choose algorithms, or interface to an application
code, but all Dakota topics are welcome.

Dakota Github Discussions
-------------------------

Vist Dakota's `GitHub Discussions page <https://github.com/orgs/snl-dakota/discussions>`_ 
for questions about installing, building, and using Dakota, sharing
Dakota success stories, reporting bugs, and making feature requests.

If you don't already have a GitHub account, you'll need to create one
to participate.

.. _help-screencasts:

===========
Screencasts
===========

In addition to this manual, the Dakota team has developed `a series of video screencasts <https://www.youtube.com/playlist?list=PLouetuxaIMDo-NMFXT-hlHYhOkePLrayY>`_ that are aimed at helping
users learn how to use Dakota at varying stages of complexity.  These screencasts complement the material presented in the Dakota GUI manual, since many of the videos use the GUI to demonstrate Dakota usage.

- **Series 1: Introduction to Dakota**

    * `1.1: Running a Simple Dakota Example <https://www.youtube.com/watch?v=ofi13UTq_Is>`_
    * `1.2: Input File Format <https://www.youtube.com/watch?v=f1l8DIXd9Gs>`_
    * `1.3: More Method Examples with Rosenbrock <https://www.youtube.com/watch?v=jPd5zarUs1o>`_

- **Series 2: Sensitivity Analysis**

    * `2.1: Introduction to Sensitivity Analysis <https://www.youtube.com/watch?v=YshRCgm_f1Y>`_
    * `2.2: Sampling <https://www.youtube.com/watch?v=dnqoUCw6wSo>`_
    * 2.3: Variance-Based Decomposition (coming soon)
  
.. _help-training:
 
==================
Training Resources
==================

Slides and streaming videos for several introductory Dakota training topics are now available. The videos are recordings of live training conducted internally at Sandia and feature:

* Slide presentations and lectures by Dakota team members
* Live demos of Dakota
* Interaction with trainees
* Dakota exercises that provide hands-on experience with using the Reference Manual, creating input files, interfacing simulations with Dakota, interpreting Dakota output, and more

Viewers may follow along with the exercises by downloading the materials for each module. The exercises were created for use with Dakota 6.3 on OS X or Linux, but users of slightly different versions of Dakota (6.0 or greater) and Windows users of Dakota may encounter a few difficulties.

Updated (2016) materials and presentations can be downloaded `here <https://dakota.sandia.gov/sites/default/files/training/materials-20160428.zip>`__, but they may not match those used in the videos.

.. note::

   Exercises in the Model Characterization and Sensitivity Analysis modules make use of plotting tools created specially for the
   training. Python 2.7 and the matplotlib and pandas libraries are required by the tools. Installing either Anaconda or Canopy is
   a convenient way to satisfy these requirements.

.. warning::

   *Cantilever beam errata:* In the content below (slides and simulation drivers), the stress equation for the cantilever beam
   incorrectly has a fixed length L = 100. `Corrected cantilever beam slide <https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_CorrectedCantilever.pdf>`__.
   The analysis driver is corrected in versions of Dakota newer than 2019-09-05. Thanks to Anjali Sandip for reporting.

+-------------------------------------------+----------------------------------------+------------------------+-------------------------+
| Module                                    | Learning Goals                         | Approx. Time (minutes) | Video/Slides/Exercises  |
+===========================================+========================================+========================+=========================+
| Overview                                  | - What is Dakota?                      | 45                     | `Slides`__              |
|                                           | - Why use Dakota?                      |                        |                         |
|                                           | - Prerequisites                        |                        |                         |
+-------------------------------------------+----------------------------------------+------------------------+-------------------------+
| Model Characterization                    | - See how Dakota can automate what you | 100                    | `Video`__ /             |
|                                           |   are already doing                    |                        | `Slides`__ /            |
|                                           | - Know what model characteristics will |                        | `Exercises`__           |
|                                           |   affect how you use Dakota            |                        |                         |
|                                           | - Be able to run a basic study to      |                        |                         |
|                                           |   characterize a model                 |                        |                         |
+-------------------------------------------+----------------------------------------+------------------------+-------------------------+
| Input Syntax / Building Blocks            | - Develop an accurate "mental model"   | 60                     | `Video`__ /             |
|                                           |   of Dakota components                 |                        | `Slides`__ /            |
|                                           | - Understand how to configure Dakota   |                        | `Exercises`__           |
|                                           |   components using a Dakota input file |                        |                         |
|                                           | - Become familiar with the Dakota      |                        |                         |
|                                           |   Reference Manual                     |                        |                         |
+-------------------------------------------+----------------------------------------+------------------------+-------------------------+
| Interfacing a User's Simulation to Dakota | - Mechanics of how Dakota communicates | 130                    | `Video 1`__ /           |
|                                           |   with and runs a simulation           |                        | `Video 2`__ /           |
|                                           | - Requirements this places on the user |                        | `Slides`__ /            |
|                                           |   and interface                        |                        | `Exercises`__           |
|                                           | - Basic strategies for developing a    |                        |                         |
|                                           |   simulation interface                 |                        |                         |
|                                           | - Convenience features Dakota provides |                        |                         |
|                                           |   for managing simulation runs         |                        |                         |
|                                           | - Note: This module covers "black box" |                        |                         |
|                                           |   interfacing, not "library mode"      |                        |                         |
|                                           |   Dakota                               |                        |                         |
+-------------------------------------------+----------------------------------------+------------------------+-------------------------+
| Sensitivity Analysis                      | - Sensitivity analysis goals and       | 90                     | `Video`__ /             |
|                                           |   examples                             |                        | `Slides`__ /            |
|                                           | - Global sensitivity analysis          |                        | `Exercises`__           |
|                                           |   approaches and metrics available     |                        |                         |
|                                           |   in Dakota                            |                        |                         |
|                                           | - Dakota examples for parameter        |                        |                         |
|                                           |   studies and global sensitivity       |                        |                         |
|                                           |   analysis                             |                        |                         |
+-------------------------------------------+----------------------------------------+------------------------+-------------------------+
| Surrogate Models                          | - Define a surrogate model             | 50                     | `Video`__ /             |
|                                           | - Identify situations where it may be  |                        | `Slides`__ /            |
|                                           |   appropriate to use a surrogate model |                        | `Exercises`__           |
|                                           | - Learn how to specify a surrogate     |                        |                         |
|                                           |   model in Dakota                      |                        |                         |
|                                           | - Run a surrogate model in Dakota and  |                        |                         |
|                                           |   examine outputs based on the         |                        |                         |
|                                           |   surrogate model                      |                        |                         |
|                                           | - Identify some common diagnostics     |                        |                         |
|                                           |   for surrogates                       |                        |                         |
|                                           | - Understand different ways surrogates |                        |                         |
|                                           |   are used in Dakota                   |                        |                         |
+-------------------------------------------+----------------------------------------+------------------------+-------------------------+
| Optimization                              | - Understand potential goals of        | 100                    | `Video`__ /             |
|                                           |   optimization and optimization        |                        | `Slides`__ /            |
|                                           |   terminology                          |                        | `Exercises`__           |
|                                           | - Learn how to communicate the         |                        |                         |
|                                           |   relevant problem information to      |                        |                         |
|                                           |   Dakota                               |                        |                         |
|                                           | - Become familiar with several types   |                        |                         |
|                                           |   of optimization solvers and how to   |                        |                         |
|                                           |   choose from among them based on      |                        |                         |
|                                           |   problem type and goals               |                        |                         |
+-------------------------------------------+----------------------------------------+------------------------+-------------------------+
| Calibration                               | - Why you might want to tune models to | 70                     | `Video`__ /             |
|                                           |   match data via calibration           |                        | `Slides`__ /            |
|                                           |   (parameter estimation)               |                        | `Exercises`__           |
|                                           | - How to formulate calibration         |                        |                         |
|                                           |   problems and present them to Dakota  |                        |                         |
|                                           | - What Dakota methods can help you     |                        |                         |
|                                           |   achieve calibration goals            |                        |                         |
+-------------------------------------------+----------------------------------------+------------------------+-------------------------+
| Uncertainty Quantification                | - Uncertainty quantification goals and | 125                    | `Video`__ /             |
|                                           |   examples                             |                        | `Slides`__ /            |
|                                           | - Examples for uncertainty             |                        | `Exercises`__           |
|                                           |   quantification                       |                        |                         |
|                                           | - Focus on forward propogation         |                        |                         |
+-------------------------------------------+----------------------------------------+------------------------+-------------------------+
| Parallel Options                          | - Discuss what to consider when        | 60                     | `Video`__ /             |
|                                           |   designing a parallelized study       |                        | `Slides`__              |
|                                           | - Understand what Dakota provides and  |                        |                         |
|                                           |   its limitations                      |                        |                         |
|                                           | - Be able to choose the best           |                        |                         |
|                                           |   parallelism approach                 |                        |                         |
|                                           | - Know how to configure Dakota and     |                        |                         |
|                                           |   your interface for your parallelism  |                        |                         |
|                                           |   approach                             |                        |                         |
+-------------------------------------------+----------------------------------------+------------------------+-------------------------+

.. __: https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_Overview.pdf
__ http://digitalops.sandia.gov/Mediasite/Play/536240e97b444ee19a24d55c72fd52941d
__ https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_ModelCharacterization.pdf
__ https://dakota.sandia.gov/sites/default/files/training/characterization-151215.zip
__ http://digitalops.sandia.gov/Mediasite/Play/16134f3f4b6842d2b145a9600cbbcbbd1d
__ https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_InputComponents.pdf
__ https://dakota.sandia.gov/sites/default/files/training/input-151215.zip
__ http://digitalops.sandia.gov/Mediasite/Play/82fa69553ac64d9b997a59316100d23a1d
__ http://digitalops.sandia.gov/Mediasite/Play/5c5f47304b934159a40347f3ba74ad851d
__ https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_Interfacing.pdf
__ https://dakota.sandia.gov/sites/default/files/training/interfacing-151215.zip
__ http://digitalops.sandia.gov/Mediasite/Play/e273e948e94a4f4a9fbdd385c1ef4c8a1d
__ https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_SensitivityAnalysis.pdf
__ https://dakota.sandia.gov/sites/default/files/training/sens_analysis-220216.zip
__ http://digitalops.sandia.gov/Mediasite/Play/b249f5f9347d4d9580be23dca66d9c1d1d
__ https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_SurrogateModels.pdf
__ https://dakota.sandia.gov/sites/default/files/training/surrogate-220216.zip
__ http://digitalops.sandia.gov/Mediasite/Play/a13c912f3e994c4ea010aacd903b12111d
__ https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_Optimization.pdf
__ https://dakota.sandia.gov/sites/default/files/training/optimization-220216.zip
__ http://digitalops.sandia.gov/Mediasite/Play/d265c6a47b1a4fe6a2f4052f97325af91d
__ https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_Calibration.pdf
__ https://dakota.sandia.gov/sites/default/files/training/calibration-220216.zip
__ http://digitalops.sandia.gov/Mediasite/Play/8105e6e9c2cb45089cf24cd4585fc8cb1d
__ https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_UncertaintyQuantification.pdf
__ https://dakota.sandia.gov/sites/default/files/training/uncertainty_analysis-220216.zip
__ http://digitalops.sandia.gov/Mediasite/Play/48810b3090ec4b58becd000ffa6e71741d
__ https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_Parallelism.pdf
