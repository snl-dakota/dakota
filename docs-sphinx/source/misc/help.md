Help Resources
==============

## Public User Community

Dakota is an open source and publicly available project with a significant worldwide user base. Our vision is that Dakota users will increasingly help each other with questions and issues. We encourage general usage questions about how to use Dakota, including how to install, choose algorithms, or interface to an application code, be posted to our public users' email list. Simply email your question to [dakota-users@software.sandia.gov](mailto:dakota-users@software.sandia.gov). Your first message will be held for moderation, and after it is approved, you will be subscribed to the list. Please refer to the guidance in our FAQ when preparing your request. Also, note that while we make every attempt to respond to all support inquiries, funding and staffing limitations demand that we prioritize requests as follows:

* Sandia National Laboratories users and integrators
* CRADA and other DOE users and partners, e.g., NNSA/ASC labs, NEAMS and CASL Nuclear Energy initiatives
* Close research or software collaborators
* Other users

## Mailing Lists

Dakota has two public mailing lists. The dakota-announce list provides a mechanism for the development team to distribute announcements to the user community. We send email by this mechanism only a few times per year. The dakota-users list provides a public discussion forum for users. To subscribe to either, simply email your question or request. Your first message will be held for moderation, and after it is approved, you will be subscribed to the list. Alternately, to join without posting, see the subscription management information below.

**Note:** If you submit a general usage question to dakota-users that our development team wants to track to benefit others or the project, the Dakota team may elect to repost the information at a later time to the list, include it in our examples or training, etc. In that case, any personal idenfitying information including name and email address will be removed. 

* **Announce list:** dakota-announce@software.sandia.gov
* **Users list:** dakota-users@software.sandia.gov

When asking a question on dakota-users, please provide the information indicated in our FAQ. Also, we strongly encourage interaction between users. Please feel welcome to respond to questions or concerns raised by others.

The archives of dakota-users and dakota-announce currently are unavailable due to Sandia policy.

### Managing Your Subscriptions

| | Dakota Users List | Dakota Announce List |
|---|---|---|
| Join List | dakota-users-join@software.sandia.gov | dakota-announce-join@software.sandia.gov |
| Email List | dakota-users@software.sandia.gov | n/a |
| Leave List | dakota-users-leave@software.sandia.gov | dakota-announce-leave@software.sandia.gov |
| Contact List Owners | dakota-users-owner@software.sandia.gov | dakota-announce-owner@software.sandia.gov |

## Screencasts

In addition to this manual, the Dakota team has developed [a series of video screencasts](https://www.youtube.com/playlist?list=PLouetuxaIMDo-NMFXT-hlHYhOkePLrayY) that are aimed at helping users learn how to use Dakota at varying stages of complexity.  These screencasts complement the material presented in the Dakota GUI manual, since many of the videos use the GUI to demonstrate Dakota usage.

* **Series 1: Introduction to Dakota**
  * [1.1: Running a Simple Dakota Example](https://www.youtube.com/watch?v=ofi13UTq_Is)
  * [1.2: Input File Format](https://www.youtube.com/watch?v=f1l8DIXd9Gs)
  * [1.3: More Method Examples with Rosenbrock](https://www.youtube.com/watch?v=jPd5zarUs1o)
* **Series 2: Sensitivity Analysis**
  * [2.1: Introduction to Sensitivity Analysis](https://www.youtube.com/watch?v=YshRCgm_f1Y)
  * [2.2: Sampling](https://www.youtube.com/watch?v=dnqoUCw6wSo)
  * 2.3: Variance-Based Decomposition (coming soon)
  
## Training Resources

Slides and streaming videos for several introductory Dakota training topics are now available. The videos are recordings of live training conducted internally at Sandia and feature:

* Slide presentations and lectures by Dakota team members
* Live demos of Dakota
* Interaction with trainees
* Dakota exercises that provide hands-on experience with using the Reference Manual, creating input files, interfacing simulations with Dakota, interpreting Dakota output, and more

Viewers may follow along with the exercises by downloading the materials for each module. The exercises were created for use with Dakota 6.3 on OS X or Linux, but users of slightly different versions of Dakota (6.0 or greater) and Windows users of Dakota will encounter few difficulties.

Exercises in the Model Characterization and Sensitivity Analysis modules make use of plotting tools created specially for the training. Python 2.7 and the matplotlib and pandas libraries are required by the tools. Installing either Anaconda or Canopy is a convenient way to satisfy these requirements.

*Updates to Videos:*

* Updated (2016) materials and presentations can be downloaded here, but they may not match those used in the videos.
* Cantilever beam errata: In the content below (slides and simulation drivers), the stress equation for the cantilever beam incorrectly has a fixed length L = 100. Corrected cantilever beam slide. The analysis driver is corrected in versions of Dakota newer than 2019-09-05. Thanks to Anjali Sandip for reporting.

| Module | Learning Goals | Approx. Time (minutes) | Video/Slides/Exercises |
| --- | --- | --- | --- |
| Overview | - What is Dakota? <br /> - Why use Dakota? <br /> - Prerequisites <br /> | 45 | [Slides](https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_Overview.pdf) |
| Model Characterization | - See how Dakota can automate what you are already doing <br /> - Know what model characteristics will affect how you use Dakota <br /> - Be able to run a basic study to characterize a model <br /> | 100 | [Video](http://digitalops.sandia.gov/Mediasite/Play/536240e97b444ee19a24d55c72fd52941d) / [Slides](https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_ModelCharacterization.pdf) / [Exercises](https://dakota.sandia.gov/sites/default/files/training/characterization-151215.zip) |
| Input Syntax / Building Blocks | - Develop an accurate "mental model" of Dakota components <br /> - Understand how to configure Dakota components using a Dakota input file <br /> - Become familiar with the Dakota Reference Manual <br /> | 60 | [Video](http://digitalops.sandia.gov/Mediasite/Play/16134f3f4b6842d2b145a9600cbbcbbd1d) / [Slides](https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_InputComponents.pdf) / [Exercises](https://dakota.sandia.gov/sites/default/files/training/input-151215.zip) |
| Interfacing a User's Simulation to Dakota | - Mechanics of how Dakota communicates with and runs a simulation <br /> - Requirements this places on the user and interface <br /> - Basic strategies for developing a simulation interface <br /> - Convenience features Dakota provides for managing simulation runs <br /> - Note: This module covers "black box" interfacing, not "library mode" Dakota <br /> | 130 | [Video 1](http://digitalops.sandia.gov/Mediasite/Play/82fa69553ac64d9b997a59316100d23a1d) / [Video 2](http://digitalops.sandia.gov/Mediasite/Play/5c5f47304b934159a40347f3ba74ad851d) / [Slides](https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_Interfacing.pdf) / [Exercises](https://dakota.sandia.gov/sites/default/files/training/interfacing-151215.zip) |
| Sensitivity Analysis | - Sensitivity analysis goals and examples <br /> - Global sensitivity analysis approaches and metrics available in Dakota <br /> - Dakota examples for parameter studies and global sensitivity analysis <br /> | 90 | [Video](http://digitalops.sandia.gov/Mediasite/Play/e273e948e94a4f4a9fbdd385c1ef4c8a1d) / [Slides](https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_SensitivityAnalysis.pdf) / [Exercises](https://dakota.sandia.gov/sites/default/files/training/sens_analysis-220216.zip) |
| Surrogate Models | - Define a surrogate model <br /> - Identify situations where it may be appropriate to use a surrogate model <br /> - Learn how to specify a surrogate model in Dakota <br /> - Run a surrogate model in Dakota and examine outputs based on the surrogate model <br /> - Identify some common diagnostics for surrogates <br /> - Understand different ways surrogates are used in Dakota <br /> | 50 | [Video](http://digitalops.sandia.gov/Mediasite/Play/b249f5f9347d4d9580be23dca66d9c1d1d) / [Slides](https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_SurrogateModels.pdf) / [Exercises](https://dakota.sandia.gov/sites/default/files/training/surrogate-220216.zip) |
| Optimization | - Understand potential goals of optimization and optimization terminology <br /> - Learn how to communicate the relevant problem information to Dakota <br /> - Become familiar with several types of optimization solvers and how to choose from among them based on problem type and goals | 100 | [Video](http://digitalops.sandia.gov/Mediasite/Play/a13c912f3e994c4ea010aacd903b12111d) / [Slides](https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_Optimization.pdf) / [Exercises](https://dakota.sandia.gov/sites/default/files/training/optimization-220216.zip) |
| Calibration | - Why you might want to tune models to match data via calibration (parameter estimation) <br /> - How to formulate calibration problems and present them to Dakota <br /> - What Dakota methods can help you achieve calibration goals | 70 | [Video](http://digitalops.sandia.gov/Mediasite/Play/d265c6a47b1a4fe6a2f4052f97325af91d) / [Slides](https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_Calibration.pdf) / [Exercises](https://dakota.sandia.gov/sites/default/files/training/calibration-220216.zip) |
| Uncertainty Quantification | - Uncertainty quantification goals and examples <br /> - Examples for uncertainty quantification <br /> - Focus on forward propogation <br /> | 125 | [Video](http://digitalops.sandia.gov/Mediasite/Play/8105e6e9c2cb45089cf24cd4585fc8cb1d) / [Slides](https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_UncertaintyQuantification.pdf) / [Exercises](https://dakota.sandia.gov/sites/default/files/training/uncertainty_analysis-220216.zip) |
| Parallel Options | - Discuss what to consider when designing a parallelized study <br /> - Understand what Dakota provides and its limitations <br /> - Be able to choose the best parallelism approach <br /> - Know how to configure Dakota and your interface for your parallelism approach | 60 | [Video](http://digitalops.sandia.gov/Mediasite/Play/48810b3090ec4b58becd000ffa6e71741d) / [Slides](https://dakota.sandia.gov/sites/default/files/training/DakotaTraining_Parallelism.pdf) |