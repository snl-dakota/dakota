.. _`topic-uncertainty_quantification`:

Uncertainty Quantification
==========================

Dakota provides a variety of methods for propagating both
aleatory and epistemic uncertainty.

At a high level, uncertainty quantification (UQ) or nondeterministic
analysis is the process of characterizing input uncertainties, forward
propagating these uncertainties through a computational model, and
performing statistical or interval assessments on the resulting
responses. This process determines the effect of uncertainties and
assumptions on model outputs or results. In Dakota, uncertainty
quantification methods specifically focus on the forward propagation
part of the process, where probabilistic or interval information on
parametric inputs are mapped through the computational model to assess
statistics or intervals on outputs. For an overview of these
approaches for engineering applications, consult :cite:p:`Hal00`.

UQ is related to sensitivity analysis in that the common goal is to
gain an understanding of how variations in the parameters affect the
response functions of the engineering design problem. However, for UQ,
some or all of the components of the parameter vector, are considered
to be uncertain as specified by particular probability distributions
(e.g., normal, exponential, extreme value), or other uncertainty
structures. By assigning specific distributional structure to the
inputs, distributional structure for the outputs (i.e, response
statistics) can be inferred.  This migrates from an analysis that is
more {\em qualitative} in nature, in the case of sensitivity analysis,
to an analysis that is more rigorously {\em quantitative}.

UQ methods are often distinguished by their ability to propagate
aleatory or epistemic input uncertainty characterizations, where
aleatory uncertainties are irreducible variabilities inherent in
nature and epistemic uncertainties are reducible uncertainties
resulting from a lack of knowledge. Since sufficient data is generally
available for aleatory uncertainties, probabilistic methods are
commonly used for computing response distribution statistics based on
input probability distribution specifications. Conversely, for
epistemic uncertainties, any use of probability distributions is based
on subjective knowledge rather than objective data, and we may
alternatively explore nonprobabilistic methods based on interval
specifications.

Dakota contains capabilities for performing nondeterministic analysis
with both types of input uncertainty. These UQ methods have been
developed by Sandia Labs, in conjunction with collaborators in
academia :cite:p:`Gha99,Gha91,Eld05,Tang10a`.

The aleatory UQ methods in Dakota include various sampling-based
approaches (e.g., Monte Carlo and Latin Hypercube sampling), local and
global reliability methods, and stochastic expansion (polynomial chaos
expansions and stochastic collocation) approaches. The epistemic UQ
methods include local and global interval analysis and Dempster-Shafer
evidence theory. These are summarized below and then described in more
depth in subsequent sections of this chapter. Dakota additionally
supports mixed aleatory/epistemic UQ via interval-valued probability,
second-order probability, and Dempster-Shafer theory of
evidence. These involve advanced model recursions and are described in
Section<!-- TODO ~{adv_models:mixed_uq}-->.

The choice of uncertainty quantification method depends on how the
input uncertainty is characterized, the computational budget, and the
desired output accuracy.  The recommendations for UQ methods are
summarized in a :ref:`UQ Guidelines Table<usage:guideuq>` are
discussed in the remainder of the section.

TODO: Put table in Doxygen if still needed
